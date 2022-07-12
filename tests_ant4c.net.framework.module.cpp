/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2022 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "echo.h"
#include "environment.h"
#include "exec.h"
#include "file_system.h"
#include "path.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include "meta_host.h"
#include "runtime_info.h"
};

#include <string>
#include <ostream>

#include <unknwn.h>

uint8_t unknown_free(void* interface_)
{
	if (interface_)
	{
		reinterpret_cast<IUnknown*>(interface_)->Release();
	}

	return 0;
}

uint8_t meta_host_free(void* interface_)
{
	unknown_free(interface_);
	meta_host_release();
	return 0;
}

uint8_t meta_host_free()
{
	return meta_host_free(nullptr);
}

TEST(TestMetaHost, meta_host_init)
{
	ASSERT_TRUE(meta_host_init());
	meta_host_release();
}

TEST(TestMetaHost, meta_host_get_runtime)
{
	static const std::string inputs[] = { "v4.0.30319" };
	void* the_runtime = nullptr;

	for (const auto& input : inputs)
	{
		const auto input_in_range = string_to_range(input);
		ASSERT_TRUE(
			meta_host_get_runtime(input_in_range.start, input_in_range.finish, &the_runtime))
				<< input << std::endl << meta_host_free();
		unknown_free(the_runtime);
		the_runtime = nullptr;
	}

	meta_host_release();
}

TEST(TestMetaHost, meta_host_get_version_from_file)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);
	ASSERT_TRUE(path_get_temp_file_name(&tmp)) << buffer_free(&tmp);
	//
	const auto input_path(buffer_to_string(&tmp));
	const auto input_range(string_to_range(input_path));
	//
	ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
	//
	ASSERT_FALSE(meta_host_get_version_from_file(input_range.start, input_range.finish, &tmp))
			<< meta_host_free() << buffer_free(&tmp);
	ASSERT_FALSE(buffer_size(&tmp)) << meta_host_free() << buffer_free(&tmp);
	//
	static const auto code = reinterpret_cast<const uint8_t*>("namespace Ant4C { class Class { } }");
	//
	ASSERT_TRUE(echo(0, Default, input_range.start, Info, code, 35, 1, 0))
			<< meta_host_free() << buffer_free(&tmp);
	//
	ASSERT_TRUE(meta_host_get_version_from_file(input_range.start, input_range.finish, &tmp))
			<< meta_host_free() << buffer_free(&tmp);
	ASSERT_FALSE(buffer_size(&tmp)) << meta_host_free() << buffer_free(&tmp);
	//
	ASSERT_TRUE(path_get_temp_file_name(&tmp)) << meta_host_free() << buffer_free(&tmp);
	//
	const auto output_path(buffer_to_string(&tmp));
	const auto output_range(string_to_range(output_path));
	//
	ASSERT_TRUE(buffer_resize(&tmp, 0)) << meta_host_free() << buffer_free(&tmp);
	//
	void* the_runtime = nullptr;
	ASSERT_TRUE(meta_host_get_runtime_v4(&the_runtime)) << meta_host_free() << buffer_free(&tmp);
	ASSERT_TRUE(runtime_info_get_version_string(the_runtime, &tmp))
			<< meta_host_free(the_runtime) << buffer_free(&tmp);
	const auto version(buffer_to_string(&tmp));
	ASSERT_TRUE(buffer_resize(&tmp, 0)) << meta_host_free(the_runtime) << buffer_free(&tmp);
	ASSERT_TRUE(runtime_info_get_runtime_directory(the_runtime, &tmp))
			<< meta_host_free(the_runtime) << buffer_free(&tmp);
	//
	unknown_free(the_runtime);
	the_runtime = nullptr;
	//
	const auto* start = buffer_data(&tmp, 0);
	const auto* finish = start + buffer_size(&tmp);
	static const uint8_t zero = 0;
	finish = string_find_any_symbol_like_or_not_like_that(
				 finish, start, &zero, &zero + 1, 1, -1);
	finish = string_find_any_symbol_like_or_not_like_that(
				 finish, start, &zero, &zero + 1, 0, -1);

	if (finish != start)
	{
		++finish;
	}

	ASSERT_TRUE(buffer_resize(&tmp, finish - start)) << meta_host_free() << buffer_free(&tmp);
	//
	static const auto CSC_path = reinterpret_cast<const uint8_t*>("csc.exe");
	ASSERT_TRUE(path_combine_in_place(&tmp, 0, CSC_path, CSC_path + 7))
			<< meta_host_free(the_runtime) << buffer_free(&tmp);
	ASSERT_TRUE(buffer_push_back(&tmp, 0)) << meta_host_free(the_runtime) << buffer_free(&tmp);
	//
	std::string command_line = "";
	command_line += "/target:library /out:\"";
	command_line += output_path;
	command_line += "\" \"";
	command_line += input_path;
	command_line += "\"";
	//
	const auto command_line_in_range = string_to_range(command_line);
	ASSERT_TRUE(
		exec(nullptr, nullptr, 0, &tmp, nullptr, &command_line_in_range,
			 nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0, 0))
			<< meta_host_free(the_runtime) << buffer_free(&tmp);
	//
	ASSERT_TRUE(buffer_resize(&tmp, 0)) << meta_host_free() << buffer_free(&tmp);
	//
	ASSERT_TRUE(
		meta_host_get_version_from_file(output_range.start, output_range.finish, &tmp))
			<< meta_host_free() << buffer_free(&tmp);
	ASSERT_TRUE(buffer_size(&tmp)) << meta_host_free() << buffer_free(&tmp);
	//
	meta_host_release();
	//
	ASSERT_EQ(version, buffer_to_string(&tmp)) << buffer_free(&tmp);
	buffer_release(&tmp);
}

TEST(TestMetaHost, meta_host_enumerate_installed_runtimes)
{
	buffer installed_runtimes;
	SET_NULL_TO_BUFFER(installed_runtimes);
	ASSERT_TRUE(meta_host_enumerate_installed_runtimes(&installed_runtimes))
			<< buffer_free(&installed_runtimes) << meta_host_free();
	meta_host_release();
	buffer_release(&installed_runtimes);
}
//meta_host_enumerate_loaded_runtimes
//meta_host_exit_process
#if 0
TEST(TestRunTimeInfo, run_time_at_all)
{
	void* the_runtime = nullptr;
	ASSERT_TRUE(meta_host_get_runtime_v4(&the_runtime)) << meta_host_free();
	buffer output;
	SET_NULL_TO_BUFFER(output);
	ASSERT_TRUE(
		runtime_info_get_version_string(the_runtime, &output))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	ASSERT_TRUE(
		runtime_info_get_runtime_directory(the_runtime, &output))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	uint8_t is_loaded = 0;
	ASSERT_FALSE(runtime_info_is_loaded(the_runtime, nullptr, &is_loaded))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << meta_host_free(the_runtime) << buffer_free(&output);
	runtime_info_load_error_string(the_runtime, S_OK, -1, &output);
	//
	void* interface_ = nullptr;
	ASSERT_TRUE(runtime_info_get_interface_of_cor_runtime_host(the_runtime, &interface_))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	ASSERT_NE(nullptr, interface_) << meta_host_free(the_runtime) << buffer_free(&output);
	unknown_free(interface_);
	//
	interface_ = nullptr;
	ASSERT_TRUE(runtime_info_get_interface_of_clr_runtime_host(the_runtime, &interface_))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	ASSERT_NE(nullptr, interface_) << meta_host_free(the_runtime) << buffer_free(&output);
	unknown_free(interface_);
	//
	interface_ = nullptr;
	ASSERT_TRUE(
		runtime_info_get_interface_of_type_name_factory(the_runtime, &interface_))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	ASSERT_NE(nullptr, interface_) << meta_host_free(the_runtime) << buffer_free(&output);
	unknown_free(interface_);
	//
	interface_ = nullptr;
	ASSERT_TRUE(runtime_info_get_interface_of_strong_name(the_runtime, &interface_))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	ASSERT_NE(nullptr, interface_) << meta_host_free(the_runtime) << buffer_free(&output);
	unknown_free(interface_);
	//
	is_loaded = 0;
	ASSERT_TRUE(runtime_info_is_loadable(the_runtime, &is_loaded))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << meta_host_free(the_runtime) << buffer_free(&output);
	ASSERT_TRUE(runtime_info_get_default_startup_flags(the_runtime, &output))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	//
	is_loaded = 0;
	unsigned long startup_flags = 0;
	ASSERT_TRUE(runtime_info_is_started(the_runtime, &is_loaded, &startup_flags))
			<< meta_host_free(the_runtime) << buffer_free(&output);
	//
	unknown_free(the_runtime);
	meta_host_release();
	buffer_release(&output);
	//runtime_info_set_default_startup_flags
}
#endif
TEST(TestLoadTasks_, ant4c_net_framework_module)
{
	static const std::string xml_file("ant4c.net.framework.xml");
	static const std::string clr_module_file("ant4c.net.framework.module.clr.dll");
	static const std::string csharp_project_file("ant4c.net.framework.module.clr.csproj");
	//
	uint8_t result;
	//
	const auto source_directory(get_path_to_directory_with_source(&result));
	ASSERT_TRUE(result);
	ASSERT_FALSE(source_directory.empty());
	//
	const auto module_source_directory = source_directory + "modules\\ant4c.net.framework\\";
	//
	const auto path_to_xml_file = module_source_directory + xml_file;
	ASSERT_TRUE(file_exists(reinterpret_cast<const uint8_t*>(path_to_xml_file.c_str()))) << path_to_xml_file;
	//
	const auto path_to_csharp_project_file = module_source_directory + csharp_project_file;
	ASSERT_TRUE(
		file_exists(
			reinterpret_cast<const uint8_t*>(path_to_csharp_project_file.c_str()))) << path_to_csharp_project_file;
#ifndef NDEBUG
	const auto path_to_clr_module =
		module_source_directory + "bin\\Debug\\net40\\" + clr_module_file;
#else
	const auto path_to_clr_module =
		module_source_directory + "bin\\Release\\net40\\" + clr_module_file;
#endif
	buffer path;
	SET_NULL_TO_BUFFER(path);
	range working_dir;

	if (!file_up_to_date(reinterpret_cast<const uint8_t*>(path_to_csharp_project_file.c_str()),
						 reinterpret_cast<const uint8_t*>(path_to_clr_module.c_str())))
	{
#ifndef _WIN64
		working_dir.start = reinterpret_cast<const uint8_t*>("ProgramW6432");
		working_dir.finish = working_dir.start + common_count_bytes_until(working_dir.start, 0);

		if (environment_variable_exists(working_dir.start, working_dir.finish))
		{
			ASSERT_TRUE(environment_get_variable(working_dir.start, working_dir.finish, &path))
					<< buffer_free(&path);
		}
		else
		{
#endif
			ASSERT_TRUE(environment_get_folder_path(ProgramFiles, &path)) << buffer_free(&path);
#ifndef _WIN64
		}

#endif
		static const uint8_t* dot_net = reinterpret_cast<const uint8_t*>("dotnet\\dotnet.exe");
		static const uint8_t dot_net_length = 17;
		ASSERT_TRUE(
			path_combine_in_place(
				&path, 0, dot_net, dot_net + dot_net_length)) << buffer_free(&path);
		//
		const auto path_to_dot_net(buffer_to_string(&path));
		ASSERT_TRUE(
			file_exists(
				reinterpret_cast<const uint8_t*>(path_to_dot_net.c_str())))
				<< path_to_dot_net << std::endl << buffer_free(&path);
		//
		working_dir = string_to_range(module_source_directory);
		//
		std::string arguments = "build /p:TargetFrameworks=net40 /p:Configuration=";
#ifndef NDEBUG
		arguments += "Debug";
#else
		arguments += "Release";
#endif
		arguments += " \"" + path_to_csharp_project_file + "\"";
		const auto arguments_in_range(string_to_range(arguments));
		//
		ASSERT_TRUE(
			exec(nullptr, nullptr, 0, &path, nullptr, &arguments_in_range,
				 nullptr, nullptr, nullptr, &working_dir, nullptr, 0, 0, 0)) << buffer_free(&path);
	}

	ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
	ASSERT_TRUE(path_get_directory_for_current_process(&path)) << buffer_free(&path);
#ifndef NDEBUG
	static const uint8_t* sub_path_to_module = reinterpret_cast<const uint8_t*>(
				"Debug\\ant4c.net.framework.module.dll");
	static const uint8_t sub_path_to_module_length = 64;
#else
	static const uint8_t* sub_path_to_module = reinterpret_cast<const uint8_t*>(
				"Release\\ant4c.net.framework.module.dll");
	static const uint8_t sub_path_to_module_length = 66;
#endif
	ASSERT_TRUE(path_combine_in_place(
					&path, 0, sub_path_to_module, sub_path_to_module + sub_path_to_module_length)) << buffer_free(&path);
	const auto path_to_module(buffer_to_string(&path));
	ASSERT_TRUE(file_exists(reinterpret_cast<const uint8_t*>(path_to_module.c_str())))
			<< path_to_module << std::endl << buffer_free(&path);
	//
	ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
	ASSERT_TRUE(path_get_directory_for_current_image(&path)) << buffer_free(&path);
	//
	working_dir = string_to_range(clr_module_file);
	ASSERT_TRUE(path_combine_in_place(&path, 0, working_dir.start, working_dir.finish)) << buffer_free(&path);
	//
	const auto path_to_clr_module_at_image_folder(buffer_to_string(&path));

	if (!file_up_to_date(reinterpret_cast<const uint8_t*>(path_to_clr_module.c_str()),
						 reinterpret_cast<const uint8_t*>(path_to_clr_module_at_image_folder.c_str())))
	{
		ASSERT_TRUE(file_copy(reinterpret_cast<const uint8_t*>(path_to_clr_module.c_str()),
							  reinterpret_cast<const uint8_t*>(path_to_clr_module_at_image_folder.c_str())))
				<< path_to_clr_module << std::endl
				<< path_to_clr_module_at_image_folder << std::endl
				<< buffer_free(&path);
	}

	const auto path_to_build_file(string_to_range(path_to_xml_file));
	//
	ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
	ASSERT_TRUE(project_new(&path)) << project_free(&path);
	//
	working_dir.start = reinterpret_cast<const uint8_t*>("path_to_module");
	working_dir.finish = reinterpret_cast<const uint8_t*>(path_to_module.c_str());
	ASSERT_TRUE(project_property_set_value(
					&path,
					working_dir.start, 14,
					working_dir.finish, static_cast<ptrdiff_t>(path_to_module.size()),
					0, 0, 1, 0)) << project_free(&path);
	//
	working_dir.start = reinterpret_cast<const uint8_t*>("path_to_file_with_clr");
	working_dir.finish = reinterpret_cast<const uint8_t*>(path_to_clr_module_at_image_folder.c_str());
	ASSERT_TRUE(project_property_set_value(
					&path,
					working_dir.start, 21,
					working_dir.finish, static_cast<ptrdiff_t>(path_to_clr_module_at_image_folder.size()),
					0, 0, 1, 0)) << project_free(&path);
	//
	working_dir = string_to_range(path_to_module);
	ASSERT_TRUE(
		path_get_directory_name(
			working_dir.start, &working_dir.finish)) << project_free(&path);
	//
	result = common_get_module_priority();
	common_set_module_priority(1);
	//
	ASSERT_TRUE(
		project_load_from_build_file(
			&path_to_build_file, &working_dir,
			Default, &path, 0, 0)) << project_free(&path);
	//
	common_set_module_priority(result);
	project_unload(&path);
}
