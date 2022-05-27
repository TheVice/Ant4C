/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "tests_base_xml.h"
#include "tests_argument_parser.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "interpreter.h"
#include "load_tasks.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "string_unit.h"
#include "target.h"
#include "text_encoding.h"
#include "version.h"
};

#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <algorithm>

class TestModule
{
protected:
	static std::string current_directory;

	bool is_project_created;
	std::list<std::string> paths;
	std::string path_to_module;
	buffer the_project;

protected:
	static bool init_current_directory(buffer* tmp)
	{
		if (current_directory.empty())
		{
			if (!buffer_resize(tmp, 0) ||
				!path_get_directory_for_current_process(tmp))
			{
				return false;
			}

			current_directory = buffer_to_string(tmp);
		}

		return !current_directory.empty();
	}

	std::string get_path_to_module(const std::string& base_directory, size_t& size)
	{
		auto path = base_directory;
		add_slash(path);
		size = path.size();

		for (const auto& sub_path : paths)
		{
#if defined(_MSC_VER)
			auto sub_path_in_range(string_to_range(sub_path));
			auto start = sub_path_in_range.start;

			if (!path_get_file_name(
					&sub_path_in_range.start,
					sub_path_in_range.finish))
			{
				return "";
			}

			path += range_to_string(start, sub_path_in_range.start);
#ifndef NDEBUG
			path += "Debug\\";
#else
			path += "Release\\";
#endif
			path += range_to_string(sub_path_in_range);
#else
			path += sub_path;
#endif

			if (file_exists(reinterpret_cast<const uint8_t*>(path.c_str())))
			{
				break;
			}

			path.resize(size);
		}

		return path;
	}

	TestModule() :
		is_project_created(false),
		paths(),
		path_to_module(),
		the_project()
	{
		SET_NULL_TO_BUFFER(the_project);
	}

	virtual void SetUp()
	{
		if (!is_project_created)
		{
			ASSERT_TRUE(init_current_directory(&the_project)) << buffer_free(&the_project);
			ASSERT_TRUE(buffer_resize(&the_project, 0)) << buffer_free(&the_project);
			ASSERT_TRUE(project_new(&the_project)) << project_free(&the_project);
			is_project_created = true;
		}
		else
		{
			project_clear(&the_project);
		}

		size_t current_directory_size;
		path_to_module = get_path_to_module(current_directory, current_directory_size);
		//
		ASSERT_GT(path_to_module.size(), current_directory_size)
				<< path_to_module << std::endl
				<< current_directory << std::endl
				<< project_free(&the_project);
		//
		pugi::xml_document load_tasks;
		auto node = load_tasks.append_child("loadtasks");
		ASSERT_FALSE(node.empty()) << project_free(&the_project);
		auto attribute = node.append_attribute("module");
		ASSERT_FALSE(attribute.empty()) << project_free(&the_project);
		ASSERT_TRUE(attribute.set_value(path_to_module.c_str())) << project_free(&the_project);
		//
		std::ostringstream string_stream;
		load_tasks.print(string_stream);
		const auto code = string_stream.str();
		const auto code_in_range(string_to_range(code));
		//
		ASSERT_TRUE(
			project_load_from_content(
				code_in_range.start, code_in_range.finish, &the_project, 0, 0))
				<< project_free(&the_project);
	}

	virtual ~TestModule()
	{
		if (is_project_created)
		{
			project_unload(&the_project);
		}
		else
		{
			buffer_release(&the_project);
		}
	}
};

class TestNetModule : public TestsBaseXml, public TestModule
{
protected:
	TestNetModule() : TestsBaseXml(), TestModule()
	{
#if defined(_WIN32)
		paths.push_back("modules\\net\\ant4c.net.module.dll");
		paths.push_back("modules\\net\\libant4c.net.module.dll");
#else
		paths.push_back("modules/net/libant4c.net.module.so");
		paths.push_back("modules/net/libant4c.net.module.dylib");
#endif
	}
};

std::string TestModule::current_directory;

TEST_F(TestNetModule, functions)
{
	for (const auto& node : nodes)
	{
		for (const auto& code : node.node().select_nodes("code"))
		{
			std::string code_str = "<property name=\"code\" value=\"";
			const auto code_command(std::string(code.node().child_value()));
			code_str += code_command;
			code_str += "\" />";
			const auto code_in_range(string_to_range(code_str));
			//
			ASSERT_TRUE(
				project_load_from_content(
					code_in_range.start, code_in_range.finish, &the_project, 0, 0))
					<< code_command;
		}

		--node_count;
	}
}

class GlobalPropertiesHolder
{
protected:
	buffer global_properties;

	GlobalPropertiesHolder() : global_properties()
	{
		SET_NULL_TO_BUFFER(global_properties);
	}

	virtual void SetUp()
	{
		if (!buffer_size(&global_properties))
		{
			ASSERT_TRUE(TestArgumentParser::get_properties(&global_properties, 0)) << properties_free(&global_properties);
		}
	}

	~GlobalPropertiesHolder()
	{
		property_release(&global_properties);
	}
};

class TestNetModuleViaBuildFile : public TestNetModule, public GlobalPropertiesHolder
{
protected:
	static std::string path_to_build_file;

	TestNetModuleViaBuildFile() : TestNetModule(), GlobalPropertiesHolder()
	{
		predefine_arguments.insert(std::make_pair("--build_file=", &path_to_build_file));
	}

	virtual void SetUp() override
	{
		TestsBaseXml::SetUp();
		GlobalPropertiesHolder::SetUp();

		if (path_to_build_file.empty())
		{
			auto result = parse_input_arguments();
			assert(result);
			ASSERT_TRUE(result);
			//
			result = path_to_build_file.empty();
			assert(!result);
			ASSERT_FALSE(result);
		}

		if (!is_project_created)
		{
			common_set_module_priority(1);
			ASSERT_TRUE(init_current_directory(&the_project)) << buffer_free(&the_project);
			ASSERT_TRUE(buffer_resize(&the_project, 0)) << buffer_free(&the_project);
			ASSERT_TRUE(project_new(&the_project)) << project_free(&the_project);
			is_project_created = true;
		}
		else
		{
			project_clear(&the_project);
		}

		size_t current_directory_size;
		path_to_module = get_path_to_module(current_directory, current_directory_size);
		//
		ASSERT_GT(path_to_module.size(), current_directory_size)
				<< path_to_module << std::endl
				<< current_directory << std::endl
				<< project_free(&the_project);
	}
};

std::string TestNetModuleViaBuildFile::path_to_build_file;

TEST_F(TestNetModuleViaBuildFile, project_load_from_build_file)
{
	const auto current_directory_in_range(string_to_range(current_directory));
	//
	static const auto property_name = reinterpret_cast<const uint8_t*>("path_to_module");
	static const uint8_t property_name_length = 14;

	for (const auto& node : nodes)
	{
		std::cout << "[ RUN      ]" << std::endl;
		/*const std::string path(node.node().attribute("path").as_string());
		const auto path_to_build_file(string_to_range(path));*/
		const auto path_to_build_file_in_range(string_to_range(path_to_build_file));
		const std::string target_name(node.node().select_node("target").node().child_value());
		const auto target_name_in_range(string_to_range(target_name));
		//
		project_clear(&the_project);
		//
		ASSERT_TRUE(project_property_set_value(
						&the_project,
						property_name,
						property_name_length,
						reinterpret_cast<const uint8_t*>(path_to_module.c_str()),
						static_cast<ptrdiff_t>(path_to_module.size()),
						0, 0, 1, 0))
				<< path_to_module << std::endl
				<< path_to_build_file << std::endl
				<< current_directory << std::endl
				<< target_name << std::endl;
		//
		ASSERT_TRUE(property_add_at_project(&the_project, &global_properties, 0))
				<< path_to_module << std::endl
				<< path_to_build_file << std::endl
				<< current_directory << std::endl
				<< target_name << std::endl;
		//
		ASSERT_TRUE(project_load_from_build_file(
						&path_to_build_file_in_range, &current_directory_in_range,
						Default, &the_project, 0, 0))
				<< path_to_module << std::endl
				<< path_to_build_file << std::endl
				<< current_directory << std::endl;

		if (range_is_null_or_empty(&target_name_in_range))
		{
			ASSERT_TRUE(project_evaluate_default_target(&the_project, 0))
					<< path_to_module << std::endl
					<< path_to_build_file << std::endl
					<< current_directory << std::endl;
		}
		else
		{
			ASSERT_TRUE(target_evaluate_by_name(&the_project,
												target_name_in_range.start,
												target_name_in_range.finish, 0))
					<< path_to_module << std::endl
					<< path_to_build_file << std::endl
					<< current_directory << std::endl
					<< target_name << std::endl;
		}

		std::cout << "[       OK ]" << std::endl;
		--node_count;
	}
}

static bool ends_with_(const std::string& input, const std::string& value)
{
	return value.size() <= input.size() && value == input.substr(input.size() - value.size(), value.size());
}

class TestNetModuleWithParameters : public TestsBase, public TestModule, public testing::Test
{
protected:
	static std::list<std::string> hostfxr_files;
	static std::string dotnet_root;
	static std::string dotnet_executable;

protected:
	TestNetModuleWithParameters() : TestsBase(), TestModule()
	{
#if defined(_WIN32)
		paths.push_back("modules\\net\\ant4c.net.module.dll");
		paths.push_back("modules\\net\\libant4c.net.module.dll");
#else
		paths.push_back("modules/net/libant4c.net.module.so");
		paths.push_back("modules/net/libant4c.net.module.dylib");
#endif
		predefine_arguments.insert(std::make_pair("--dotnet_root=", &dotnet_root));
	}

	virtual void SetUp() override
	{
		TestsBase::SetUp();
		TestModule::SetUp();
		//
		ASSERT_FALSE(dotnet_root.empty());

		if (hostfxr_files.empty())
		{
			buffer path;
			SET_NULL_TO_BUFFER(path);
			//
			ASSERT_TRUE(string_to_buffer(dotnet_root, &path)) << buffer_free(&path);
#if defined(_WIN32)
			static const uint8_t* sub_path = reinterpret_cast<const uint8_t*>("\\host\\fxr\0");
#else
			static const uint8_t* sub_path = reinterpret_cast<const uint8_t*>("/host/fxr\0");
#endif
			ASSERT_TRUE(
				path_combine_in_place(
					&path, buffer_size(&path), sub_path, sub_path + 10)) << buffer_free(&path);
			//
			buffer output;
			SET_NULL_TO_BUFFER(output);
			//
			ASSERT_TRUE(directory_enumerate_file_system_entries(&path, 1, 1, &output, 0))
					<< buffer_free(&path) << buffer_free(&output);
			//
			buffer_release(&path);
			auto output_in_a_string = buffer_to_string(&output);
			buffer_release(&output);
			//
			auto position = output_in_a_string.cbegin();
			auto prev_position = position;

			while (position < output_in_a_string.cend())
			{
				position = std::find(position, output_in_a_string.cend(), '\0');
				const auto path_to_hostfxr(std::string(prev_position, position));
#if defined(_WIN32)
				static const std::string file_name("hostfxr.dll");
#elif (defined(__APPLE__) && defined(__MACH__))
				static const std::string file_name("libhostfxr.dylib");
#else
				static const std::string file_name("libhostfxr.so");
#endif

				if (ends_with_(path_to_hostfxr, file_name))
				{
					hostfxr_files.push_back(path_to_hostfxr);
				}

				position = std::find_if(position, output_in_a_string.cend(), [](const char& c)
				{
					return '\0' != c;
				});
				//
				prev_position = position;
			}

			hostfxr_files.sort([](const std::string & path1, const std::string & path2)
			{
				auto p1(string_to_range(path1));
				auto p2(string_to_range(path2));

				if (!path_get_directory_name(p1.start, &p1.finish))
				{
					return false;
				}

				if (!path_get_directory_name(p2.start, &p2.finish))
				{
					return false;
				}

				ptrdiff_t i1, i2;

				if (-1 == (i1 = string_last_index_of(p1.start, p1.finish, &PATH_DELIMITER, &PATH_DELIMITER + 1)))
				{
					return false;
				}

				if (-1 == (i2 = string_last_index_of(p2.start, p2.finish, &PATH_DELIMITER, &PATH_DELIMITER + 1)))
				{
					return false;
				}

				if (!string_substring(p1.start, p1.finish, i1, -1, &p1))
				{
					return false;
				}

				if (!string_substring(p2.start, p2.finish, i2, -1, &p2))
				{
					return false;
				}

				uint8_t v1[VERSION_SIZE];
				uint8_t v2[VERSION_SIZE];

				if (!version_parse(p1.start, p1.finish, v1))
				{
					return false;
				}

				if (!version_parse(p2.start, p2.finish, v2))
				{
					return false;
				}

				return 0 < version_greater(v1, v2);
			});
		}

		ASSERT_FALSE(hostfxr_files.empty());

		if (dotnet_executable.empty())
		{
#if defined(_WIN32)
			dotnet_executable = join_path(dotnet_root, "dotnet.exe");
#else
			dotnet_executable = join_path(dotnet_root, "dotnet");
#endif
		}
	}
};

std::list<std::string> TestNetModuleWithParameters::hostfxr_files;
std::string TestNetModuleWithParameters::dotnet_root;
std::string TestNetModuleWithParameters::dotnet_executable;

TEST_F(TestNetModuleWithParameters, hostfxr)
{
	static const std::string True("True");
	//
	buffer arguments;
	SET_NULL_TO_BUFFER(arguments);
	//
	ASSERT_TRUE(common_get_attributes_and_arguments_for_task(
					nullptr, nullptr, 3,
					nullptr, nullptr, nullptr, &arguments))
			<< buffer_free_with_inner_buffers(&arguments);
	//
	auto the_module = buffer_buffer_data(&arguments, 2);
	ASSERT_TRUE(string_to_buffer(path_to_module, the_module))
			<< buffer_free_with_inner_buffers(&arguments);
	ASSERT_TRUE(buffer_push_back(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments);
	//
	buffer project;
	SET_NULL_TO_BUFFER(project);
	ASSERT_TRUE(project_new(&project))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
	uint8_t verbose = 0;
	ASSERT_TRUE(load_tasks_evaluate_task(&project, nullptr, &arguments, verbose))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//TODO: for (const auto& hostfxr : hostfxr_files)
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	range function;
	std::string command;
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	//
	ASSERT_TRUE(buffer_resize(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	ASSERT_TRUE(buffer_push_back(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
	command = "hostfxr::functions()";
	function = string_to_range(command);
	//
	ASSERT_TRUE(buffer_resize(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	ASSERT_TRUE(buffer_push_back(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
	ASSERT_LT(0, buffer_size(the_module))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
	command = "hostfxr::functions(',')";
	function = string_to_range(command);
	//
	ASSERT_TRUE(buffer_resize(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
	ASSERT_LT(0, buffer_size(the_module))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
	std::list<std::string>::const_iterator result;
	std::list<std::string> functions;
	functions.push_back("");
	command = buffer_to_string(the_module);
	//
	std::for_each(command.cbegin(), command.cend(), [&functions](const char& c)
	{
		if (',' == c)
		{
			functions.push_back("");
		}
		else
		{
			functions.rbegin()->push_back(c);
		}
	});

	for (const auto& fun : functions)
	{
		command = "hostfxr::is-function-exists('";
		command += fun;
		command += "')";
		function = string_to_range(command);
		//
		ASSERT_TRUE(buffer_resize(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(buffer_push_back(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		ASSERT_STREQ(True.c_str(), buffer_char_data(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	}

	result = std::find(functions.begin(), functions.end(), "main");
	ASSERT_NE(functions.end(), result)
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	command = "hostfxr::main(";
	command += "'" + dotnet_executable + "', '--version'";
	command += ")";
	function = string_to_range(command);
	//
	ASSERT_TRUE(buffer_resize(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	ASSERT_TRUE(buffer_push_back(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
	ASSERT_STREQ("0", buffer_char_data(the_module, 0))
			<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
#if defined(__GNUC__) && __GNUC__ <= 5
	{
		auto result_ = functions.begin();
		std::advance(result_, std::distance(functions.cbegin(), result));
		functions.erase(result_);
	}
#else
	functions.erase(result);
#endif
	result = std::find(functions.begin(), functions.end(), "resolve-sdk");

	if (functions.end() != result)
	{
		command = "hostfxr::resolve-sdk('', '')";
		function = string_to_range(command);
		//
		ASSERT_TRUE(buffer_resize(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(buffer_push_back(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		ASSERT_LT(2, buffer_size(the_module))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
#ifdef WIN32
		ASSERT_TRUE(directory_exists(buffer_data(the_module, 0)))
				<< std::endl << buffer_free_with_inner_buffers(&arguments) << project_free(&project);
#endif
#if defined(__GNUC__) && __GNUC__ <= 5
		auto result_ = functions.begin();
		std::advance(result_, std::distance(functions.cbegin(), result));
		functions.erase(result_);
#else
		functions.erase(result);
#endif
	}

	result = std::find(functions.begin(), functions.end(), "resolve-sdk2");

	if (functions.end() != result)
	{
		command = "hostfxr::resolve-sdk2('', '', '1')";
		function = string_to_range(command);
		//
		ASSERT_TRUE(buffer_resize(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		ASSERT_LT(2, buffer_size(the_module))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
#ifdef WIN32
		command = buffer_to_string(the_module);
		auto prev_position = command.cbegin();
		auto position = prev_position;

		while (prev_position < (position = std::find(position, command.cend(), '\0')))
		{
			auto path(std::string(prev_position, position));
			auto pos = path.find(' ');
			ASSERT_NE(std::string::npos, pos)
					<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
			pos++;
			path = path.substr(pos);
			//
			ASSERT_TRUE(directory_exists(reinterpret_cast<const uint8_t*>(path.c_str())))
					<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
			//
			position = std::find_if(position, command.cend(), [](const char& c)
			{
				return '\0' != c;
			});
			//
			prev_position = position;
		}

#endif
#if defined(__GNUC__) && __GNUC__ <= 5
		auto result_ = functions.begin();
		std::advance(result_, std::distance(functions.cbegin(), result));
		functions.erase(result_);
#else
		functions.erase(result);
#endif
	}

	result = std::find(functions.begin(), functions.end(), "get-available-sdks");

	if (functions.end() != result)
	{
		command = "hostfxr::get-available-sdks()";
		function = string_to_range(command);
		//
		ASSERT_TRUE(buffer_resize(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		ASSERT_LT(2, buffer_size(the_module))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
#ifdef WIN32
		command = buffer_to_string(the_module);
		auto prev_position = command.cbegin();
		auto position = prev_position;

		while (prev_position < (position = std::find(position, command.cend(), '\0')))
		{
			auto path(std::string(prev_position, position));
			//
			ASSERT_TRUE(directory_exists(reinterpret_cast<const uint8_t*>(path.c_str())))
					<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
			//
			position = std::find_if(position, command.cend(), [](const char& c)
			{
				return '\0' != c;
			});
			//
			prev_position = position;
		}

#endif
#if defined(__GNUC__) && __GNUC__ <= 5
		auto result_ = functions.begin();
		std::advance(result_, std::distance(functions.cbegin(), result));
		functions.erase(result_);
#else
		functions.erase(result);
#endif
	}

	result = std::find(functions.begin(), functions.end(), "get-native-search-directories");

	if (functions.end() != result)
	{
		ASSERT_TRUE(buffer_resize(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(path_get_temp_file_name(the_module))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(buffer_push_back(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);

		if (file_exists(buffer_data(the_module, 0)))
		{
			ASSERT_TRUE(file_delete(buffer_data(the_module, 0)))
					<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		}

		ASSERT_TRUE(directory_create(buffer_data(the_module, 0)))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		command = "exec";
		command += " program=\"";
		command += dotnet_executable;
		command += "\" commandline=\"new console\"";
		command += " workingdir=\"";
		command += buffer_char_data(the_module, 0);
		command += "\"";
		command += "/>";
		//
		function = string_to_range(command);
		function.finish = function.start + 4;
		//
		ASSERT_TRUE(interpreter_evaluate_task(
						&project, nullptr,
						&function, function.start + command.size(),
						nullptr, 0, verbose))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		command = "exec";
		command += " program=\"";
		command += dotnet_executable;
		command += "\" commandline=\"build\"";
		command += " workingdir=\"";
		command += buffer_char_data(the_module, 0);
		command += "\"";
		//
		ASSERT_TRUE(buffer_resize(the_module, buffer_size(the_module) - 1))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
#if defined(_WIN32)
		static const uint8_t* sub_path = reinterpret_cast<const uint8_t*>("\\1.txt");
#else
		static const uint8_t* sub_path = reinterpret_cast<const uint8_t*>("/1.txt");
#endif
		ASSERT_TRUE(
			path_combine_in_place(
				the_module, buffer_size(the_module),
				sub_path, sub_path + 6))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		ASSERT_TRUE(buffer_push_back(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		command += " output=\"";
		command += buffer_char_data(the_module, 0);
		command += "\"";
		command += "/>";
		//
		function = string_to_range(command);
		function.finish = function.start + 4;
		//
		ASSERT_TRUE(interpreter_evaluate_task(
						&project, nullptr,
						&function, function.start + command.size(),
						nullptr, 0, verbose))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		std::ifstream file(buffer_char_data(the_module, 0));
		//
		ASSERT_TRUE(file.is_open())
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		command.clear();
		std::copy(std::istreambuf_iterator<char>(file),
				  std::istreambuf_iterator<char>(), std::back_inserter(command));
		//
		file.close();
		//
		auto index = command.find(" -> ");
		ASSERT_NE(std::string::npos, index)
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		command = command.substr(index + 4);
		//
		index = command.find('\n');
		ASSERT_NE(std::string::npos, index)
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		command = command.substr(0, index);
		//
		ASSERT_TRUE(buffer_resize(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(string_to_buffer(command, the_module))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(buffer_push_back(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		command = "hostfxr::get-native-search-directories('exec', '";
		command += buffer_char_data(the_module, 0);
		command += "')";
		function = string_to_range(command);
		//
		ASSERT_TRUE(buffer_resize(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
#ifdef WIN32
		command = buffer_to_string(the_module);
		auto prev_position = command.cbegin();
		auto position = prev_position;

		while (prev_position < (position = std::find(position, command.cend(), ';')))
		{
			std::string path(prev_position, position);
			//
			auto position_ = std::find_if(command.rbegin(), command.rend(), [](const char& c)
			{
				return '\\' != c && ';' != c;
			});
			//
			path.resize(command.rend() - position_);
			//
			ASSERT_TRUE(directory_exists(reinterpret_cast<const uint8_t*>(path.c_str())))
					<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
			//
			position = std::find_if(position, command.cend(), [](const char& c)
			{
				return ';' != c;
			});
			//
			prev_position = position;
		}

#endif
#if defined(__GNUC__) && __GNUC__ <= 5
		auto result_ = functions.begin();
		std::advance(result_, std::distance(functions.cbegin(), result));
		functions.erase(result_);
#else
		functions.erase(result);
#endif
	}

	result = std::find(functions.begin(), functions.end(), "get-dotnet-environment-info");

	if (functions.end() != result)
	{
		command = "hostfxr::get-dotnet-environment-info('";
		command += dotnet_root;
		command += "')";
		function = string_to_range(command);
		//
		ASSERT_TRUE(buffer_resize(the_module, 0))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
		//
		ASSERT_LT(0, buffer_size(the_module))
				<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
#if defined(__GNUC__) && __GNUC__ <= 5
		auto result_ = functions.begin();
		std::advance(result_, std::distance(functions.cbegin(), result));
		functions.erase(result_);
#else
		functions.erase(result);
#endif
	}

	project_unload(&project);
	buffer_release_with_inner_buffers(&arguments);
}
