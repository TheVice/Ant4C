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
};

#include <string>
#include <sstream>

class TestNetModule : public TestsBaseXml
{
	static const std::string paths[2];
	static std::string current_directory;

	bool is_project_new;

public:
	buffer the_project;

public:
	TestNetModule() :
		TestsBaseXml(),
		is_project_new(false),
		the_project()
	{
		SET_NULL_TO_BUFFER(the_project);
	}

	virtual void SetUp() override
	{
		TestsBaseXml::SetUp();

		if (!is_project_new)
		{
			if (current_directory.empty())
			{
				ASSERT_TRUE(path_get_directory_for_current_process(&the_project)) << buffer_free(&the_project);
				current_directory = buffer_to_string(&the_project);
			}

			ASSERT_TRUE(buffer_resize(&the_project, 0)) << buffer_free(&the_project);
			ASSERT_TRUE(project_new(&the_project)) << project_free(&the_project);
			is_project_new = true;
		}
		else
		{
			project_clear(&the_project);
		}

		auto path_to_module = current_directory;
		add_slash(path_to_module);
		const auto current_directory_size = path_to_module.size();

		for (const auto& sub_path : paths)
		{
#if defined(_MSC_VER)
			auto sub_path_in_range(string_to_range(sub_path));
			auto start = sub_path_in_range.start;
			//
			ASSERT_TRUE(
				path_get_file_name(
					sub_path_in_range.start,
					sub_path_in_range.finish,
					&sub_path_in_range)) << project_free(&the_project);
			//
			path_to_module += range_to_string(start, sub_path_in_range.start);
#ifndef NDEBUG
			path_to_module += "Debug\\";
#else
			path_to_module += "Release\\";
#endif
			path_to_module += range_to_string(sub_path_in_range);
#else
			path_to_module += sub_path;
#endif

			if (file_exists(reinterpret_cast<const uint8_t*>(path_to_module.c_str())))
			{
				break;
			}

			path_to_module.resize(current_directory_size);
		}

		ASSERT_NE(path_to_module.size(), current_directory_size)
				<< path_to_module << std::endl
				<< current_directory << std::endl
				<< project_free(&the_project);
#if 1
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
#else
		std::string code = "<loadtasks module=\"";
		code += path_to_module;
		code += "\" />";
#endif
		const auto code_in_range(string_to_range(code));
		//
		ASSERT_TRUE(
			project_load_from_content(
				code_in_range.start, code_in_range.finish, &the_project, 0, verbose))
				<< project_free(&the_project);
	}

	virtual ~TestNetModule()
	{
		project_unload(&the_project);
	}
};

const std::string TestNetModule::paths[2] =
{
#if defined(_WIN32)
	"modules\\net\\ant4c.net.module.dll",
	"modules\\net\\libant4c.net.module.dll"
#else
	"modules/net/libant4c.net.module.so",
	"modules/net/libant4c.net.module.dylib"
#endif
};

std::string TestNetModule::current_directory;

TEST_F(TestNetModule, host_interface_exec_function)
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
