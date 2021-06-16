/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "file_system.h"
#include "path.h"
#include "project.h"
#include "target.h"
#include "text_encoding.h"
};

#include <list>
#include <string>
#include <sstream>
#include <ostream>
#include <iostream>

class TestModule : public TestsBaseXml
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
					sub_path_in_range.start,
					sub_path_in_range.finish,
					&sub_path_in_range))
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

public:
	TestModule() :
		TestsBaseXml(),
		is_project_created(false),
		paths(),
		path_to_module(),
		the_project()
	{
		SET_NULL_TO_BUFFER(the_project);
	}

	virtual void SetUp() override
	{
		TestsBaseXml::SetUp();

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
				code_in_range.start, code_in_range.finish, &the_project, 0, verbose))
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

class TestNetModule : public TestModule
{
public:
	TestNetModule() : TestModule()
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
					code_in_range.start, code_in_range.finish, &the_project, 0, verbose))
					<< code_command;
		}

		--node_count;
	}
}

class TestNetModuleEx : public TestNetModule
{
	virtual void SetUp() override
	{
		TestsBaseXml::SetUp();

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
	}
};

TEST_F(TestNetModuleEx, project_load_from_build_file)
{
	const auto path_to_source(get_path_to_directory_with_source(&verbose));
	ASSERT_TRUE(verbose);
	verbose = 0;
#if defined(_WIN32)
	const auto path = path_to_source + "modules\\net\\net.xml";
#else
	const auto path = path_to_source + "modules/net/net.xml";
#endif
	const auto path_to_build_file(string_to_range(path));
	const auto current_directory_in_range(string_to_range(current_directory));
	//
	static const auto property_name = reinterpret_cast<const uint8_t*>("path_to_module");
	static const uint8_t property_name_length = 14;

	for (const auto& node : nodes)
	{
		std::cout << "[ RUN      ]" << std::endl;
		/*const std::string path(node.node().attribute("path").as_string());
		const auto path_to_build_file(string_to_range(path));*/
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
						0, 0, 1,
						verbose))
				<< path_to_module << std::endl
				<< path << std::endl
				<< current_directory << std::endl
				<< target_name << std::endl;
		//
		ASSERT_TRUE(project_load_from_build_file(
						&path_to_build_file, &current_directory_in_range,
						Default, &the_project, 0, verbose))
				<< path_to_module << std::endl
				<< path << std::endl
				<< current_directory << std::endl;

		if (range_is_null_or_empty(&target_name_in_range))
		{
			ASSERT_TRUE(project_evaluate_default_target(&the_project, verbose))
					<< path_to_module << std::endl
					<< path << std::endl
					<< current_directory << std::endl;
		}
		else
		{
			ASSERT_TRUE(target_evaluate_by_name(&the_project, &target_name_in_range, verbose))
					<< path_to_module << std::endl
					<< path << std::endl
					<< current_directory << std::endl
					<< target_name << std::endl;
		}

		std::cout << "[       OK ]" << std::endl;
		--node_count;
	}
}
