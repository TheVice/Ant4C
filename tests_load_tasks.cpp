/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2022 TheVice
 *
 */

#include "tests_exec.h"

extern "C" {
#include "load_tasks.h"
#include "buffer.h"
#include "file_system.h"
#include "path.h"
#include "project.h"
#include "target.h"
#include "text_encoding.h"
}

#include <string>
#include <ostream>
#include <iostream>

class TestLoadTasks : public TestExec
{
};
#if defined(__ANDROID__)
TEST_F(TestLoadTasks, DISABLED_project_load_from_build_file)
#else
TEST_F(TestLoadTasks, project_load_from_build_file)
#endif
{
	const auto path_to_source(get_path_to_directory_with_source(&verbose));
	ASSERT_TRUE(verbose);
	//
	std::string the_project_buffer(buffer_size_of(), 0);
	auto the_project = reinterpret_cast<void*>(&the_project_buffer[0]);
	ASSERT_TRUE(buffer_init(the_project, buffer_size_of()));
	//
	const auto directory_with_image(
		get_path_to_directory_with_image(the_project, &verbose));
	ASSERT_TRUE(verbose) << buffer_free(the_project);
	ASSERT_TRUE(buffer_resize(the_project, 0)) << buffer_free(the_project);
	//
	const auto current_directory(get_directory_for_current_process(the_project, &verbose));
	ASSERT_TRUE(verbose) << buffer_free(the_project);
	ASSERT_TRUE(buffer_resize(the_project, 0)) << buffer_free(the_project);
	//
	const auto current_directory_in_range(string_to_range(current_directory));
	verbose = 0;
	//
	ASSERT_TRUE(project_new(the_project)) << project_free(the_project);

	for (const auto& node : nodes)
	{
		static const auto property_name = "path_to_module";
		static const uint8_t property_name_length = 14;
		//
		const std::string name_of_module(node.node().select_node(property_name).node().child_value());
		ASSERT_FALSE(name_of_module.empty()) << project_free(the_project);
		//
		auto path_to_module = directory_with_image + name_of_module;
		auto exists = file_exists(reinterpret_cast<const uint8_t*>(path_to_module.c_str()));

		if (!exists && directory_with_image != current_directory)
		{
#if defined(_MSC_VER)
			auto name_of_module_in_range(string_to_range(name_of_module));
			const auto start = name_of_module_in_range.start;
			//
			ASSERT_TRUE(
				path_get_file_name(
					&name_of_module_in_range.start,
					name_of_module_in_range.finish))
					<< "Path to module: '" << path_to_module << "'" << std::endl
					<< project_free(&the_project);
			//
			path_to_module = current_directory;
			path_to_module += std::string(
								  reinterpret_cast<const char*>(start),
								  reinterpret_cast<const char*>(name_of_module_in_range.start));
#ifndef NDEBUG
			path_to_module += "Debug\\";
#else
			path_to_module += "Release\\";
#endif
			path_to_module += range_to_string(name_of_module_in_range);
			exists = file_exists(reinterpret_cast<const uint8_t*>(path_to_module.c_str()));

			if (!exists)
			{
#endif
				path_to_module = current_directory + name_of_module;
				exists = file_exists(reinterpret_cast<const uint8_t*>(path_to_module.c_str()));
#if defined(_MSC_VER)
			}

#endif
		}

		if (!exists)
		{
			std::cout << __FUNCTION__ << "[" << __LINE__ << "]:" << " file" << std::endl;
			std::cout << "'" << path_to_module << "'" << std::endl;
			std::cout << "doesn't exist." << std::endl;
			std::cout << "Directory with image: '" << directory_with_image << "'" << std::endl;
			std::cout << "Current directory: '" << current_directory << "'" << std::endl;
			std::cout << __FUNCTION__ << ": test case " << node_count << " will be skip." << std::endl;
			std::cout << std::endl;
			//
			--node_count;
			continue;
		}

		const std::string name_of_script(node.node().select_node("build_file").node().child_value());
		ASSERT_FALSE(name_of_script.empty()) << project_free(the_project);
		//
		const auto path_to_script = path_to_source + name_of_script;
		exists = file_exists(reinterpret_cast<const uint8_t*>(path_to_script.c_str()));
		//
		ASSERT_TRUE(exists) << "Path to script: '" << path_to_script << "'" << std::endl
							<< "Path to source: '" << path_to_source << "'" << std::endl
							<< project_free(the_project);
		//
		const std::string target_name(node.node().select_node("target").node().child_value());
		const auto target_name_in_range(string_to_range(target_name));
		//
		project_clear(the_project);
		const auto path_to_script_file_in_range(string_to_range(path_to_script));
		static const uint8_t project_help = 0;
		//
		ASSERT_TRUE(project_property_set_value(
						the_project,
						reinterpret_cast<const uint8_t*>(property_name),
						property_name_length,
						reinterpret_cast<const uint8_t*>(path_to_module.c_str()),
						static_cast<ptrdiff_t>(path_to_module.size()),
						0, 0, 1, verbose))
				<< "Path to module: '" << path_to_module << "'" << std::endl
				<< "Path to script: '" << path_to_script << "'" << std::endl
				<< "Name of module: '" << name_of_module << "'" << std::endl
				<< "Directory with image: '" << directory_with_image << "'" << std::endl
				<< "Current directory: '" << current_directory << "'" << std::endl
				<< project_free(the_project);
		//
		ASSERT_TRUE(project_load_from_build_file(
						&path_to_script_file_in_range,
						&current_directory_in_range,
						Default, the_project, project_help, verbose))
				<< "Path to module: '" << path_to_module << "'" << std::endl
				<< "Path to script: '" << path_to_script << "'" << std::endl
				<< "Name of module: '" << name_of_module << "'" << std::endl
				<< "Directory with image: '" << directory_with_image << "'" << std::endl
				<< "Current directory: '" << current_directory << "'" << std::endl
				<< project_free(the_project);

		if (range_is_null_or_empty(&target_name_in_range))
		{
			ASSERT_TRUE(project_evaluate_default_target(the_project, verbose))
					<< "Path to module: '" << path_to_module << "'" << std::endl
					<< "Path to script: '" << path_to_script << "'" << std::endl
					<< "Name of module: '" << name_of_module << "'" << std::endl
					<< "Directory with image: '" << directory_with_image << "'" << std::endl
					<< "Current directory: '" << current_directory << "'" << std::endl
					<< project_free(the_project);
		}
		else
		{
			ASSERT_TRUE(target_evaluate_by_name(the_project,
												target_name_in_range.start,
												target_name_in_range.finish, verbose))
					<< "Path to module: '" << path_to_module << "'" << std::endl
					<< "Path to script: '" << path_to_script << "'" << std::endl
					<< "Name of module: '" << name_of_module << "'" << std::endl
					<< "Directory with image: '" << directory_with_image << "'" << std::endl
					<< "Current directory: '" << current_directory << "'" << std::endl
					<< "Target name: '" << target_name << "'" << std::endl
					<< project_free(the_project);
		}

		--node_count;
	}

	project_unload(the_project);
}
