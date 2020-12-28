/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "range.h"
#include "path.h"
#include "exec.h"
#include "echo.h"
#include "text_encoding.h"
#include "project.h"

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
	const uint8_t* start = buffer_data(&tmp, 0);
	const uint8_t* finish = start + buffer_size(&tmp);
	static const uint8_t zero = 0;
	finish = find_any_symbol_like_or_not_like_that(finish, start, &zero, 1, 1, -1);
	finish = find_any_symbol_like_or_not_like_that(finish, start, &zero, 1, 0, -1);

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
TEST(ant4c_net_framework_module, at_all)
{
	static const std::string code_sample =
		"<project>"
		"  <loadtasks module=\"ant4c.net.framework.module.dll\" />"
#ifndef NDEBUG
		"  <fail unless=\"${directory::exists(path::combine(program::current-directory(), 'Debug'))}\" />"
		"  <property name=\"is-assembly\" value=\"${file::is-assembly(path::combine(path::combine(program::current-directory(), 'Debug'), ant4c.net.framework.module.dll))}\" />"
		"  <property name=\"is-assembly\" value=\"${file::is-assembly(path::combine(path::combine(program::current-directory(), 'Debug'), ant4c.net.framework.module.clr.dll))}\" />"
		"  <property name=\"clr-version-from-file\" value=\"${metahost::get-clr-version-from-file(path::combine(path::combine(program::current-directory(), 'Debug'), ant4c.net.framework.module.clr.dll))}\" />"
#else
		"  <fail unless=\"${directory::exists(path::combine(program::current-directory(), 'Release'))}\" />"
		"  <property name=\"is-assembly\" value=\"${file::is-assembly(path::combine(path::combine(program::current-directory(), 'Release'), ant4c.net.framework.module.dll))}\" />"
		"  <property name=\"is-assembly\" value=\"${file::is-assembly(path::combine(path::combine(program::current-directory(), 'Release'), ant4c.net.framework.module.clr.dll))}\" />"
		"  <property name=\"clr-version-from-file\" value=\"${metahost::get-clr-version-from-file(path::combine(path::combine(program::current-directory(), 'Release'), ant4c.net.framework.module.clr.dll))}\" />"
#endif
		"  <property name=\"is-assembly\" value=\"${file::is-assembly('ant4c.net.framework.module.clr.dll_')}\" failonerror=\"false\" />"
		"  <property name=\"framework-directory\" value=\"${framework::get-framework-directory()}\" />"
		"  <property name=\"framework-directory\" value=\"${framework::get-framework-directory('net-4.0')}\" />"
		"  <property name=\"frameworks\" value=\"${framework::get-frameworks()}\" />"
		"  <property name=\"frameworks\" value=\"${framework::get-frameworks('All')}\" />"
		"  <property name=\"clr-version\" value=\"${framework::get-clr-version()}\" />"
		"  <property name=\"runtime-framework\" value=\"${framework::get-runtime-framework()}\" />"
		"  <property name=\"framework-exists\" value=\"${framework::exists('net-4.0')}\" />"
		"  <property name=\"runtime_v2\" value=\"${metahost::runtime('v2.0.50727')}\" />"
		"  <property name=\"runtime_v4\" value=\"${metahost::runtime('v4.0.30319')}\" />"
		"</project>";
	//
	const auto content = string_to_range(code_sample);
	//
	buffer the_project;
	SET_NULL_TO_BUFFER(the_project);
	//
	auto returned = project_new(&the_project);
	ASSERT_TRUE(returned) << project_free(&the_project);
	//
	const auto current_priority = common_get_module_priority();
	common_set_module_priority(1);
	returned = project_load_from_content(content.start, content.finish, &the_project, 0, 0);
	common_set_module_priority(current_priority);
	ASSERT_TRUE(returned) << project_free(&the_project);
	//
	project_unload(&the_project);
}
