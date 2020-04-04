/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "load_file.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "target.h"
#include "text_encoding.h"
};

#include <string>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <iostream>

class TestProject : public TestsBaseXml
{
};

TEST_F(TestProject, project_property_set_value)
{
	for (const auto& node : nodes)
	{
		const auto code(get_data_from_nodes(node, "code"));
		//
		const auto code_in_range = string_to_range(code);
		//
		void* project = NULL;
		ASSERT_TRUE(project_new(&project)) << project_free(project);
		ASSERT_NE(nullptr, project) << project_free(project);
		//
		ASSERT_NE(code.empty(), project_load_from_content(
					  code_in_range.start, code_in_range.finish, project, 0, verbose)) << project_free(project);

		for (const auto& property : node.node().select_nodes("property"))
		{
			std::string property_name;
			std::string property_value;
			uint8_t dynamic = 0;
			uint8_t over_write = 0;
			uint8_t read_only = 0;
			uint8_t fail_on_error = 0;
			uint8_t property_verbose = 0;
			//
			property_load_from_node(property.node(), property_name, property_value, dynamic,
									over_write, read_only, fail_on_error, property_verbose);
			//
			property_verbose = MAX(verbose, property_verbose);
			//
			const auto expected_return = (uint8_t)INT_PARSE(property.node().select_node("return").node().child_value());
			//
			auto returned = project_property_set_value(project,
							(const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(),
							(const uint8_t*)property_value.c_str(), property_value.size(),
							dynamic, over_write, read_only, property_verbose);
			//
			ASSERT_EQ(expected_return, returned) << project_free(project);
			//
			void* the_property = NULL;
			returned = project_property_exists(
						   project, (const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(), &the_property,
						   property_verbose);
			//
			ASSERT_EQ(expected_return, returned) << project_free(project);

			if (expected_return)
			{
				uint8_t returned_dynamic = 0;
				uint8_t returned_read_only = 0;
				//
				//ASSERT_TRUE(property_get_by_pointer(the_property, returned_value));
				ASSERT_TRUE(property_is_dynamic(the_property, &returned_dynamic)) << project_free(project);
				ASSERT_TRUE(property_is_readonly(the_property, &returned_read_only)) << project_free(project);
				//
				ASSERT_EQ(dynamic, returned_dynamic) << project_free(project);
				ASSERT_EQ(read_only, returned_read_only) << project_free(project);
			}
		}

		project_unload(project);
		//
		--node_count;
	}
}

TEST_F(TestProject, project_load_from_content)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		std::cout << "[ RUN      ]" << std::endl;
		//
		const void* the_property = NULL;
		//
		const std::string content(node.node().select_node("content").node().child_value());
		const auto project_help = (uint8_t)INT_PARSE(
									  node.node().select_node("project_help").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		const std::string expected_name(node.node().select_node("name").node().child_value());
		const std::string expected_default_target(node.node().select_node("default").node().child_value());
		const std::string expected_base_directory(node.node().select_node("base_directory").node().child_value());
		const auto targets = node.node().select_nodes("target");
		//
		const auto content_in_range(string_to_range(content));
		ASSERT_EQ(content.empty(), range_is_null_or_empty(&content_in_range))
				<< buffer_free(&output);
		//
		void* project = NULL;
		ASSERT_TRUE(project_new(&project))
				<< buffer_free(&output) << project_free(project);
		//
		const uint8_t returned = project_load_from_content(content_in_range.start, content_in_range.finish,
								 project, project_help, verbose);
		ASSERT_EQ(expected_return, returned)
				<< content << std::endl << buffer_free(&output) << project_free(project);
		//
		the_property = NULL;
		ASSERT_NE(expected_base_directory.empty(), project_get_base_directory(project, &the_property))
				<< buffer_free(&output) << project_free(project);
		//
		const auto returned_base_directory(property_to_string(the_property, &output));
		ASSERT_EQ(expected_base_directory, returned_base_directory)
				<< buffer_free(&output) << project_free(project);
		//
		the_property = NULL;
		ASSERT_FALSE(project_get_buildfile_path(project, &the_property))
				<< buffer_free(&output) << project_free(project);
		//
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output) << project_free(project);
		ASSERT_FALSE(project_get_buildfile_uri(the_property, &output))
				<< buffer_free(&output) << project_free(project);
		ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output) << project_free(project);
		//
		the_property = NULL;
		ASSERT_EQ(!expected_name.empty(), project_get_name(project, &the_property))
				<< buffer_free(&output) << project_free(project);
		//
		const auto returned_name(property_to_string(the_property, &output));
		ASSERT_EQ(expected_name, returned_name)
				<< buffer_free(&output) << project_free(project);
		//
		the_property = NULL;
		ASSERT_EQ(!expected_default_target.empty(), project_get_default_target(project, &the_property))
				<< buffer_free(&output) << project_free(project);
		//
		const auto returned_default_target(property_to_string(the_property, &output));
		ASSERT_EQ(expected_default_target, returned_default_target)
				<< buffer_free(&output) << project_free(project);
		//
		ASSERT_NE(expected_default_target.empty(), project_target_exists(project,
				  (const uint8_t*)expected_default_target.c_str(), (uint8_t)expected_default_target.size()))
				<< buffer_free(&output) << project_free(project) << std::endl
				<< expected_default_target;

		for (const auto& target_name : targets)
		{
			std::string target_name_str(target_name.node().child_value());
			ASSERT_TRUE(project_target_exists(project, (const uint8_t*)target_name_str.c_str(),
											  (uint8_t)target_name_str.size()))
					<< buffer_free(&output) << project_free(project) << std::endl
					<< "Target name - '" << target_name_str << "'." << std::endl;
			//
			void* target = NULL;
			ASSERT_TRUE(project_target_get(project, (const uint8_t*)target_name_str.c_str(),
										   (uint8_t)target_name_str.size(), &target, verbose))
					<< buffer_free(&output) << project_free(project) << std::endl
					<< "Target name - '" << target_name_str << "'." << std::endl;
			//
			uint16_t index = 0;
			const range* depend_target_name = NULL;

			while (NULL != (depend_target_name = target_get_depend(target, index++)))
			{
				target_name_str = range_to_string(depend_target_name);
				ASSERT_TRUE(project_target_exists(project, (const uint8_t*)target_name_str.c_str(),
												  (uint8_t)target_name_str.size()))
						<< buffer_free(&output) << project_free(project) << std::endl
						<< "Target name - '" << target_name_str << "'." << std::endl;
			}
		}

		project_unload(project);
		project = NULL;
		//
		std::cout << "[       OK ]" << std::endl;
		//
		--node_count;
	}

	buffer_release(&output);
}

extern "C" {
	extern uint16_t argument_parser_get_encoding_from_name(const char* start, const char* finish);
};

TEST_F(TestProject, project_load_from_build_file)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);
	//
	ASSERT_TRUE(path_get_directory_for_current_process(&tmp)) << buffer_free(&tmp);
	const auto current_path(buffer_to_string(&tmp));
	const auto current_path_in_range(string_to_range(current_path));

	for (const auto& node : nodes)
	{
		const std::string content(node.node().select_node("content").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		const std::string str_encoding(node.node().select_node("encoding").node().child_value());
		const auto encoding_in_range(string_to_range(str_encoding));
		auto encoding = argument_parser_get_encoding_from_name(
							(const char*)encoding_in_range.start, (const char*)encoding_in_range.finish);

		if (FILE_ENCODING_UNKNOWN == encoding)
		{
			encoding = UTF8;
		}

		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
		ASSERT_TRUE(path_get_temp_file_name(&tmp)) << buffer_free(&tmp);
		const auto path(buffer_to_string(&tmp));
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
		ASSERT_TRUE(string_to_buffer(content, &tmp)) << buffer_free(&tmp);
		//
		ASSERT_TRUE(file_write_all((const uint8_t*)path.c_str(), &tmp)) << buffer_free(&tmp);
		//
		void* project = NULL;
		ASSERT_TRUE(project_new(&project)) << buffer_free(&tmp) << project_free(project);
		//
		const auto path_in_range(string_to_range(path));
		const auto returned = project_load_from_build_file(
								  &path_in_range, &current_path_in_range, encoding, project, 0, verbose);
		//
		ASSERT_EQ(expected_return, returned)
				<< path << std::endl << buffer_free(&tmp) << project_free(project);
		//
		project_unload(project);
		//
		--node_count;
	}

	buffer_release(&tmp);
}
//project_exec_function
TEST(TestProgram, program_exec_function)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	const uint8_t* version = (const uint8_t*)"version";
	ASSERT_FALSE(program_exec_function(project_get_function(version, version + 7),
									   (const buffer*)&program_exec_function, 0, &output)) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	const uint8_t* current_directory = (const uint8_t*)"current-directory";
	ASSERT_TRUE(program_exec_function(project_get_function(current_directory, current_directory + 17),
									  (const buffer*)&program_exec_function, 0, &output)) << buffer_free(&output);
	ASSERT_LT(0, buffer_size(&output)) << buffer_free(&output);
	//
	buffer_release(&output);
}
