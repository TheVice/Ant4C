/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "tests_base_xml.h"
#include "gtest_argument_parser.h"

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
	uint8_t the_project_buffer[BUFFER_SIZE_OF];
	void* the_project;

protected:
	static bool init_current_directory(void* tmp)
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
		size = base_directory.size();

		for (const auto& sub_path : paths)
		{
#if defined(_MSC_VER)
			auto sub_path_in_a_range(string_to_range(sub_path));
			auto start = sub_path_in_a_range.start;

			if (!path_get_file_name(
					&sub_path_in_a_range.start, sub_path_in_a_range.finish))
			{
				return "";
			}

			auto path = join_path(base_directory, range_to_string(start, sub_path_in_a_range.start));
#ifndef NDEBUG
			path = join_path(path, "Debug");
#else
			path = join_path(path, "Release");
#endif
			path = join_path(path, range_to_string(sub_path_in_a_range));
#else
			auto path = join_path(base_directory, sub_path);
#endif

			if (file_exists(reinterpret_cast<const uint8_t*>(path.c_str())))
			{
				auto position = std::find(path.cbegin() + size, path.cend(), PATH_DELIMITER);
				//
				position = std::find_if(position, path.cend(), [](const char& ch)
				{
					return PATH_DELIMITER != ch;
				});
				//
				size = std::distance(path.cbegin(), position);
				return path;
			}
		}

		return base_directory;
	}

	TestModule() :
		is_project_created(false),
		paths(),
		path_to_module(),
		the_project_buffer(),
		the_project((void*)the_project_buffer)
	{
	}

	virtual void SetUp()
	{
		if (!is_project_created)
		{
			ASSERT_TRUE(buffer_init(the_project, BUFFER_SIZE_OF)) << buffer_free(the_project);
			ASSERT_TRUE(init_current_directory(the_project)) << buffer_free(the_project);
			ASSERT_TRUE(buffer_resize(the_project, 0)) << buffer_free(the_project);
			ASSERT_TRUE(project_new(the_project)) << project_free(the_project);
			is_project_created = true;
		}
		else
		{
			project_clear(the_project);
		}

		size_t current_directory_size;
		path_to_module = get_path_to_module(current_directory, current_directory_size);
		//
		ASSERT_GT(path_to_module.size(), current_directory_size)
				<< path_to_module << std::endl
				<< current_directory << std::endl
				<< project_free(the_project);
		//
		pugi::xml_document load_tasks;
		auto node = load_tasks.append_child("loadtasks");
		//
		ASSERT_FALSE(node.empty()) << project_free(the_project);
		//
		auto attribute = node.append_attribute("module");
		ASSERT_FALSE(attribute.empty()) << project_free(the_project);
		ASSERT_TRUE(attribute.set_value(path_to_module.c_str())) << project_free(the_project);
		//
		std::ostringstream string_stream;
		load_tasks.print(string_stream);
		const auto code = string_stream.str();
		const auto code_in_a_range(string_to_range(code));
		//
		ASSERT_TRUE(
			project_load_from_content(
				code_in_a_range.start, code_in_a_range.finish, the_project, 0, 0))
				<< project_free(the_project);
	}

	virtual ~TestModule()
	{
		if (is_project_created)
		{
			project_unload(the_project);
		}
		else
		{
			buffer_release(the_project);
		}
	}
};

class TestNetModule : public TestsBaseXml, public TestModule
{
protected:
	TestNetModule() : TestsBaseXml(), TestModule()
	{
#if defined(_WIN32)
		paths.push_back("ant4c.net.module.dll");
		paths.push_back("libant4c.net.module.dll");
#else
		paths.push_back("libant4c.net.module.so");
		paths.push_back("libant4c.net.module.dylib");
#endif
	}

	virtual void SetUp() override
	{
		TestsBaseXml::SetUp();
		TestModule::SetUp();
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
			const std::string command(code.node().child_value());
			code_str += command;
			code_str += "\" />";
			const auto code_in_a_range(string_to_range(code_str));
			//
			ASSERT_TRUE(
				project_load_from_content(
					code_in_a_range.start, code_in_a_range.finish, the_project, 0, 0))
					<< command;
		}

		--node_count;
	}
}

class TestNetModuleViaBuildFile : public TestNetModule
{
protected:
	static std::string path_to_build_file;

	TestNetModuleViaBuildFile() : TestNetModule()
	{
		predefine_arguments.emplace(std::make_pair("--build_file=", &path_to_build_file));
	}

	virtual void SetUp() override
	{
		TestsBaseXml::SetUp();

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
			ASSERT_TRUE(init_current_directory(the_project)) << buffer_free(the_project);
			ASSERT_TRUE(buffer_resize(the_project, 0)) << buffer_free(the_project);
			ASSERT_TRUE(project_new(the_project)) << project_free(the_project);
			is_project_created = true;
		}
		else
		{
			project_clear(the_project);
		}

		size_t current_directory_size;
		path_to_module = get_path_to_module(current_directory, current_directory_size);
		//
		ASSERT_GT(path_to_module.size(), current_directory_size)
				<< path_to_module << std::endl
				<< current_directory << std::endl
				<< project_free(the_project);
		//
		ASSERT_TRUE(GTestArgumentParser::set_to_project_properties(the_project, 0))
				<< path_to_module << std::endl
				<< current_directory << std::endl
				<< project_free(the_project);
		//
		load_nodes(the_project);
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
		project_clear(the_project);
		//
		ASSERT_TRUE(project_property_set_value(
						the_project,
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
		ASSERT_TRUE(GTestArgumentParser::set_to_project_properties(the_project, 0))
				<< path_to_module << std::endl
				<< path_to_build_file << std::endl
				<< current_directory << std::endl
				<< target_name << std::endl;
		//
		ASSERT_TRUE(project_load_from_build_file(
						&path_to_build_file_in_range, &current_directory_in_range,
						Default, the_project, 0, 0))
				<< path_to_module << std::endl
				<< path_to_build_file << std::endl
				<< current_directory << std::endl;

		if (range_is_null_or_empty(&target_name_in_range))
		{
			ASSERT_TRUE(project_evaluate_default_target(the_project, 0))
					<< path_to_module << std::endl
					<< path_to_build_file << std::endl
					<< current_directory << std::endl;
		}
		else
		{
			ASSERT_TRUE(target_evaluate_by_name(the_project,
												target_name_in_range.start,
												target_name_in_range.finish, 0))
					<< path_to_module << std::endl
					<< path_to_build_file << std::endl
					<< current_directory << std::endl
					<< target_name << std::endl;
		}

		std::cout << "[       OK ]" << std::endl;
		//
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
	static std::string dotnet_root;
	static std::string dotnet_executable;
	//static uint8_t host_version[VERSION_SIZE];
	//static uint8_t host_versions[10][VERSION_SIZE];
	static std::list<std::string> hostfxr_files;
	static const std::string True;
	static const std::string Exec;

	range function;
	std::string command;
	uint8_t the_output_buffer[BUFFER_SIZE_OF];
	void* the_output;
	uint8_t verbose;

	/*static void GetHostVersion(struct buffer* tmp)
	{
		range function;
		std::string command;
		//
		ASSERT_TRUE(path_get_temp_file_name(tmp)) << buffer_free(tmp);
		//
		command = Exec;
		command += " program=\"";
		command += dotnet_executable;
		command += "\" commandline=\"--version\"";
		command += " output=\"";
		command += buffer_char_data(tmp, 0);
		command += "\"";
		command += " />";
		//
		function = string_to_range(command);
		function.finish = function.start + Exec.size();
		//
		ASSERT_TRUE(interpreter_evaluate_task(
						nullptr, nullptr,
						&function, function.start + command.size(),
						nullptr, 0, 0))
				<< buffer_free(tmp);
		//
		std::ifstream file(buffer_char_data(tmp, 0));
		ASSERT_TRUE(file.is_open()) << buffer_free(tmp);
		//
		command.clear();
		std::copy(std::istreambuf_iterator<char>(file),
				  std::istreambuf_iterator<char>(), std::back_inserter(command));
		//
		file.close();
		//
		function = string_to_range(command);
		ASSERT_TRUE(version_parse(function.start, function.finish, host_version))
				<< buffer_free(tmp);
		//
		static const char* versions[] = { "7", "6", "5", "3.1", "3", "2.2", "2.1", "2", "1.1", "1" };

		for (uint8_t i = 0, count = COUNT_OF(versions); i < count; ++i)
		{
			command = versions[i];
			function = string_to_range(command);
			//
			ASSERT_TRUE(version_parse(function.start, function.finish, host_versions[i]))
					<< buffer_free(tmp);
		}

		ASSERT_TRUE(buffer_resize(tmp, 0)) << buffer_free(tmp);
	}*/

	static void FillHostFxrFiles()
	{
		std::string path_buffer(buffer_size_of(), 0);
		auto path = reinterpret_cast<void*>(&path_buffer[0]);
		ASSERT_TRUE(buffer_init(path, buffer_size_of()));
		//
		//GetHostVersion(path);
		//
		ASSERT_TRUE(string_to_buffer(dotnet_root, path)) << buffer_free(path);
#if defined(_WIN32)
		static const uint8_t* sub_path = reinterpret_cast<const uint8_t*>("\\host\\fxr\0");
#else
		static const uint8_t* sub_path = reinterpret_cast<const uint8_t*>("/host/fxr\0");
#endif
		ASSERT_TRUE(
			path_combine_in_place(
				path, buffer_size(path), sub_path, sub_path + 10)) << buffer_free(path);
		//
		std::string output_buffer(buffer_size_of(), 0);
		auto output = reinterpret_cast<void*>(&output_buffer[0]);
		ASSERT_TRUE(buffer_init(output, buffer_size_of())) << buffer_free(path);
		//
		ASSERT_TRUE(directory_enumerate_file_system_entries(path, 1, 1, output, 0))
				<< buffer_free(path) << buffer_free(output);
		//
		buffer_release(path);
		auto output_in_a_string = buffer_to_string(output);
		buffer_release(output);
		//
		auto position = output_in_a_string.cbegin();
		auto prev_position = position;

		while (position < output_in_a_string.cend())
		{
			position = std::find(position, output_in_a_string.cend(), '\0');
			const std::string path_to_hostfxr(prev_position, position);
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
			auto path_1(string_to_range(path1));
			auto path_2(string_to_range(path2));

			if (!path_get_directory_name(path_1.start, &path_1.finish))
			{
				return false;
			}

			if (!path_get_directory_name(path_2.start, &path_2.finish))
			{
				return false;
			}

			ptrdiff_t index_1, index_2;

			if (-1 == (index_1 = string_last_index_of(path_1.start, path_1.finish, &PATH_DELIMITER, &PATH_DELIMITER + 1)))
			{
				return false;
			}

			if (-1 == (index_2 = string_last_index_of(path_2.start, path_2.finish, &PATH_DELIMITER, &PATH_DELIMITER + 1)))
			{
				return false;
			}

			if (!string_substring(path_1.start, path_1.finish, index_1, -1, &path_1))
			{
				return false;
			}

			if (!string_substring(path_2.start, path_2.finish, index_2, -1, &path_2))
			{
				return false;
			}

			uint8_t version_1[VERSION_SIZE];
			uint8_t version_2[VERSION_SIZE];

			if (!version_parse(path_1.start, path_1.finish, version_1))
			{
				return false;
			}

			if (!version_parse(path_2.start, path_2.finish, version_2))
			{
				return false;
			}

			return 0 < version_greater(version_1, version_2);
		});
	}

protected:
	TestNetModuleWithParameters() :
		TestsBase(),
		TestModule(),
		function(),
		command(),
		the_output_buffer(),
		the_output((void*)the_output_buffer),
		verbose(0)
	{
		buffer_init(the_output, BUFFER_SIZE_OF);
#if defined(_WIN32)
		paths.push_back("ant4c.net.module.dll");
		paths.push_back("libant4c.net.module.dll");
#else
		paths.push_back("libant4c.net.module.so");
		paths.push_back("libant4c.net.module.dylib");
#endif
		predefine_arguments.emplace(std::make_pair("--dotnet_root=", &dotnet_root));
	}

	virtual void SetUp() override
	{
		TestsBase::SetUp();
		TestModule::SetUp();
		//
		ASSERT_FALSE(dotnet_root.empty());

		if (dotnet_executable.empty())
		{
#if defined(_WIN32)
			dotnet_executable = join_path(dotnet_root, "dotnet.exe");
#else
			dotnet_executable = join_path(dotnet_root, "dotnet");
#endif
		}

		if (hostfxr_files.empty())
		{
			FillHostFxrFiles();
		}

		ASSERT_FALSE(hostfxr_files.empty());
	}

	~TestNetModuleWithParameters()
	{
		buffer_release(the_output);
	}
};

std::string TestNetModuleWithParameters::dotnet_root;
std::string TestNetModuleWithParameters::dotnet_executable;
//uint8_t TestNetModuleWithParameters::host_version[VERSION_SIZE];
//uint8_t TestNetModuleWithParameters::host_versions[10][VERSION_SIZE];
std::list<std::string> TestNetModuleWithParameters::hostfxr_files;
const std::string TestNetModuleWithParameters::True("True");
const std::string TestNetModuleWithParameters::Exec("exec");

/*TEST_F(TestNetModuleWithParameters, hostfxr_initialize)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
}*/

TEST_F(TestNetModuleWithParameters, hostfxr_functions)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::functions()";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	//
	ASSERT_LT(0, buffer_size(the_output));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::functions(',')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	//
	ASSERT_LT(0, buffer_size(the_output));
	//
	std::cout << "[       OK ]" << std::endl;
}

TEST_F(TestNetModuleWithParameters, hostfxr_main)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::is-function-exists('";
	command += "main";
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	//
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::main(";
	command += "'" + dotnet_executable + "', '--version'";
	command += ")";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	//
	command = "net::result-to-string('";
	command += buffer_char_data(the_output, 0);
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	ASSERT_TRUE(starts_with_(buffer_char_data(the_output, 0), "[net]::Success"));
	std::cout << "[       OK ]" << std::endl;
}

TEST_F(TestNetModuleWithParameters, DISABLED_hostfxr_resolve_sdk)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose))
			<< command << std::endl;
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::is-function-exists('";
	command += "resolve-sdk";
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose))
			<< command << std::endl;
	//
	std::cout << "[       OK ]" << std::endl;

	if (True == buffer_to_string(the_output))
	{
		command = "hostfxr::resolve-sdk('', '')";
		function = string_to_range(command);
		std::cout << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose))
				<< command << std::endl;
		ASSERT_TRUE(buffer_push_back(the_output, 0));
		//
#ifdef WIN32
		ASSERT_TRUE(directory_exists(buffer_uint8_t_data(the_output, 0)))
				<< buffer_to_string(the_output) << std::endl;
#else
		command = "net::result-to-string('";
		command += buffer_char_data(the_output, 0);
		command += "')";
		function = string_to_range(command);
		std::cout << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose))
				<< command << std::endl;
		ASSERT_TRUE(buffer_push_back(the_output, 0));
		//
		std::cout << "[       OK ]" << std::endl;
		//
		ASSERT_TRUE(starts_with_(buffer_char_data(the_output, 0), "[net]::Success"))
				<< buffer_to_string(the_output) << std::endl;
		std::cout << "[       OK ]" << std::endl;
		//
		std::cerr << "[Warning]: function do not return valid data." << std::endl;
#endif
		std::cout << "[       OK ]" << std::endl;
	}
	else
	{
		std::cerr <<
				  "[Warning]: current version of host do not support 'hostfxr::resolve-sdk' function." << std::endl;
	}
}

TEST_F(TestNetModuleWithParameters, DISABLED_hostfxr_resolve_sdk2)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::is-function-exists('";
	command += "resolve-sdk2";
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	//
	std::cout << "[       OK ]" << std::endl;

	if (True == buffer_to_string(the_output))
	{
		command = "hostfxr::resolve-sdk2('', '', '1')";
		function = string_to_range(command);
		std::cout << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
		//
		ASSERT_LT(2, buffer_size(the_output));
#ifdef WIN32
		command = buffer_to_string(the_output);
		auto prev_position = command.cbegin();
		auto position = prev_position;

		while (prev_position < (position = std::find(position, command.cend(), '\0')))
		{
			std::string path(prev_position, position);
			auto position_ = path.find(' ');
			ASSERT_NE(std::string::npos, position_);
			position_++;
			path = path.substr(position_);
			//
			ASSERT_TRUE(directory_exists(reinterpret_cast<const uint8_t*>(path.c_str())));
			//
			position = std::find_if(position, command.cend(), [](const char& ch)
			{
				return '\0' != ch;
			});
			//
			prev_position = position;
		}

#else
		command = "net::result-to-string('";
		ASSERT_TRUE(buffer_push_back(the_output, 0));
		command += buffer_char_data(the_output, 0);
		command += "')";
		function = string_to_range(command);
		std::cerr << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
		ASSERT_TRUE(buffer_push_back(the_output, 0));
		//
		std::cerr << buffer_char_data(the_output, 0) << std::endl;
		std::cerr << "[       OK ]" << std::endl;
		//
		std::cerr << "[Warning]: function do not return valid data." << std::endl;
#endif
		std::cout << "[       OK ]" << std::endl;
	}
	else
	{
		std::cerr <<
				  "[Warning]: current version of host do not support 'hostfxr::resolve-sdk2' function." << std::endl;
	}
}

TEST_F(TestNetModuleWithParameters, DISABLED_hostfxr_get_available_sdks)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::is-function-exists('";
	command += "get-available-sdks";
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	//
	std::cout << "[       OK ]" << std::endl;

	if (True == buffer_to_string(the_output))
	{
		command = "hostfxr::get-available-sdks()";
		function = string_to_range(command);
		std::cout << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
		//
#ifdef WIN32
		command = buffer_to_string(the_output);
		auto prev_position = command.cbegin();
		auto position = prev_position;

		while (prev_position < (position = std::find(position, command.cend(), '\0')))
		{
			std::string path(prev_position, position);
			//
			ASSERT_TRUE(directory_exists(reinterpret_cast<const uint8_t*>(path.c_str())));
			//
			position = std::find_if(position, command.cend(), [](const char& ch)
			{
				return '\0' != ch;
			});
			//
			prev_position = position;
		}

#else
		ASSERT_EQ(0, buffer_size(the_output));
		std::cerr << "[Warning]: function do not return valid data." << std::endl;
#endif
		std::cout << "[       OK ]" << std::endl;
	}
	else
	{
		std::cerr <<
				  "[Warning]: current version of host do not support 'hostfxr::get-available-sdks' function." << std::endl;
	}
}

TEST_F(TestNetModuleWithParameters, DISABLED_hostfxr_get_native_search_directories)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::is-function-exists('";
	command += "get-native-search-directories";
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	//
	std::cout << "[       OK ]" << std::endl;

	if (True == buffer_to_string(the_output))
	{
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(path_get_temp_file_name(the_output));
		ASSERT_TRUE(buffer_push_back(the_output, 0));

		if (file_exists(buffer_uint8_t_data(the_output, 0)))
		{
			ASSERT_TRUE(file_delete(buffer_uint8_t_data(the_output, 0)));
		}

		ASSERT_TRUE(directory_create(buffer_uint8_t_data(the_output, 0)));
		//
		command = Exec;
		command += " program=\"";
		command += dotnet_executable;
		command += "\" commandline=\"new console\"";
		command += " workingdir=\"";
		command += buffer_char_data(the_output, 0);
		command += "\"";
		command += " />";
		//
		function = string_to_range(command);
		function.finish = function.start + Exec.size();
		std::cout << "[ RUN      ]" << " <" << command << std::endl;
		//
		ASSERT_TRUE(interpreter_evaluate_task(
						the_project, nullptr,
						&function, function.start + command.size(),
						nullptr, 0, verbose));
		//
		std::cout << "[       OK ]" << std::endl;
		command = Exec;
		command += " program=\"";
		command += dotnet_executable;
		command += "\" commandline=\"build\"";
		command += " workingdir=\"";
		command += buffer_char_data(the_output, 0);
		command += "\"";
		//
		ASSERT_TRUE(buffer_resize(the_output, buffer_size(the_output) - 1));
		//
#if defined(_WIN32)
		static const uint8_t* sub_path = reinterpret_cast<const uint8_t*>("\\1.txt");
#else
		static const uint8_t* sub_path = reinterpret_cast<const uint8_t*>("/1.txt");
#endif
		ASSERT_TRUE(
			path_combine_in_place(
				the_output, buffer_size(the_output),
				sub_path, sub_path + 6));
		//
		ASSERT_TRUE(buffer_push_back(the_output, 0));
		//
		command += " output=\"";
		command += buffer_char_data(the_output, 0);
		command += "\"";
		command += " />";
		//
		function = string_to_range(command);
		function.finish = function.start + Exec.size();
		std::cout << "[ RUN      ]" << " <" << command << std::endl;
		//
		ASSERT_TRUE(interpreter_evaluate_task(
						the_project, nullptr,
						&function, function.start + command.size(),
						nullptr, 0, verbose));
		//
		std::cout << "[       OK ]" << std::endl;
		std::ifstream file(buffer_char_data(the_output, 0));
		//
		ASSERT_TRUE(file.is_open());
		//
		command.clear();
		std::copy(std::istreambuf_iterator<char>(file),
				  std::istreambuf_iterator<char>(), std::back_inserter(command));
		//
		file.close();
		//
		static const std::string arrow(" -> ");
		auto index = command.find(arrow);
		ASSERT_NE(std::string::npos, index);
		command = command.substr(index + arrow.size());
		//
		index = command.find('\n');
		ASSERT_NE(std::string::npos, index);
		command = command.substr(0, index);
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(string_to_buffer(command, the_output));
		ASSERT_TRUE(buffer_push_back(the_output, 0));
		//
		command = "hostfxr::get-native-search-directories('exec', '";
		command += buffer_char_data(the_output, 0);
		command += "')";
		function = string_to_range(command);
		std::cout << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
#ifdef WIN32
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
		//
		command = buffer_to_string(the_output);
		auto prev_position = command.cbegin();
		auto position = prev_position;

		while (prev_position < (position = std::find(position, command.cend(), ';')))
		{
			std::string path(prev_position, position);
			//
			auto position_ = std::find_if(command.rbegin(), command.rend(), [](const char& ch)
			{
				return PATH_DELIMITER != ch && ';' != ch;
			});
			//
			path.resize(command.rend() - position_);
			//
			ASSERT_TRUE(directory_exists(reinterpret_cast<const uint8_t*>(path.c_str())));
			//
			position = std::find_if(position, command.cend(), [](const char& ch)
			{
				return ';' != ch;
			});
			//
			prev_position = position;
		}

#else
		ASSERT_FALSE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
		std::cerr << "[Warning]: function do not return valid data." << std::endl;
#endif
		std::cout << "[       OK ]" << std::endl;
	}
	else
	{
		std::cerr <<
				  "[Warning]: current version of host do not support 'hostfxr::get-native-search-directories' function." <<
				  std::endl;
	}
}

TEST_F(TestNetModuleWithParameters, hostfxr_main_startupinfo)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::is-function-exists('";
	command += "main-startupinfo";
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	//
	std::cout << "[       OK ]" << std::endl;

	if (True == buffer_to_string(the_output))
	{
		command = "hostfxr::main-startupinfo('";
		command += dotnet_executable;
		command += "', '', '";
#ifdef WIN32
		command += "dotnet.dll";
		//#elif (defined(__APPLE__) && defined(__MACH__))
#else
		command += "libdotnet.so";
#endif
		command += "', '";
		command += dotnet_executable;
		command += "', '--info')";
		function = string_to_range(command);
		std::cout << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
		ASSERT_TRUE(buffer_push_back(the_output, 0));
		//
		command = "net::result-to-string('";
		command += buffer_char_data(the_output, 0);
		command += "')";
		function = string_to_range(command);
		std::cout << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
		ASSERT_TRUE(buffer_push_back(the_output, 0));
		//
		std::cout << "[       OK ]" << std::endl;
		//
		ASSERT_TRUE(starts_with_(buffer_char_data(the_output, 0), "[net]::Success"));
		std::cout << "[       OK ]" << std::endl;
	}
	else
	{
		std::cerr <<
				  "[Warning]: current version of host do not support 'hostfxr::main-startupinfo' function." <<
				  std::endl;
	}
}
/*close
get-runtime-delegate
get-runtime-properties
get-runtime-property-value
initialize-for-dotnet-command-line
initialize-for-runtime-config
run-app
set-error-writer
set-runtime-property-value
main-bundle-startupinfo*/
TEST_F(TestNetModuleWithParameters, hostfxr_get_dotnet_environment_info)
{
	const auto hostfxr = *(hostfxr_files.cbegin());
	//
	command = "hostfxr::initialize('";
	command += hostfxr;
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	ASSERT_TRUE(buffer_push_back(the_output, 0));
	ASSERT_STREQ(True.c_str(), buffer_char_data(the_output, 0));
	//
	std::cout << "[       OK ]" << std::endl;
	//
	command = "hostfxr::is-function-exists('";
	command += "get-dotnet-environment-info";
	command += "')";
	function = string_to_range(command);
	std::cout << "[ RUN      ]" << " " << command << std::endl;
	//
	ASSERT_TRUE(buffer_resize(the_output, 0));
	ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
	//
	std::cout << "[       OK ]" << std::endl;

	if (True == buffer_to_string(the_output))
	{
		command = "hostfxr::get-dotnet-environment-info('";
		command += dotnet_root;
		command += "')";
		function = string_to_range(command);
		std::cout << "[ RUN      ]" << " " << command << std::endl;
		//
		ASSERT_TRUE(buffer_resize(the_output, 0));
		ASSERT_TRUE(interpreter_evaluate_function(the_project, nullptr, &function, the_output, verbose));
		//
		ASSERT_LT(0, buffer_size(the_output));
		//
		std::cout << "[       OK ]" << std::endl;
	}
	else
	{
		std::cerr <<
				  "[Warning]: current version of host do not support 'hostfxr::get-dotnet-environment-info' function." <<
				  std::endl;
	}
}

#if 0
{
	//if (True == )
	//command = "hostpolicy::initialize('";
	//command += std::to_string(369365249);
	//command += "')";
	//function = string_to_range(command);
	////
	//ASSERT_TRUE(buffer_resize(the_module, 0))
	//	<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//ASSERT_TRUE(interpreter_evaluate_function(&project, nullptr, &function, the_module, verbose))
	//	<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//ASSERT_TRUE(buffer_push_back(the_module, 0))
	//	<< buffer_free_with_inner_buffers(&arguments) << project_free(&project);
	//
}
#endif
