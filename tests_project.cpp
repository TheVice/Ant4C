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
#include "interpreter.h"
#include "load_file.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "target.h"
#include "text_encoding.h"
#include "xml.h"
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
		void* the_project = NULL;
		ASSERT_TRUE(project_new(&the_project)) << project_free(the_project);
		ASSERT_NE(nullptr, the_project) << project_free(the_project);
		//
		ASSERT_NE(code.empty(), project_load_from_content(
					  code_in_range.start, code_in_range.finish, the_project, 0, verbose)) << project_free(the_project);

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
			auto returned = project_property_set_value(the_project,
							(const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(),
							(const uint8_t*)property_value.c_str(), property_value.size(),
							dynamic, over_write, read_only, property_verbose);
			//
			ASSERT_EQ(expected_return, returned) << project_free(the_project);
			//
			void* the_property = NULL;
			returned = project_property_exists(
						   the_project, (const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(), &the_property,
						   property_verbose);
			//
			ASSERT_EQ(expected_return, returned) << project_free(the_project);

			if (expected_return)
			{
				uint8_t returned_dynamic = 0;
				uint8_t returned_read_only = 0;
				//
				//ASSERT_TRUE(property_get_by_pointer(the_property, returned_value));
				ASSERT_TRUE(property_is_dynamic(the_property, &returned_dynamic)) << project_free(the_project);
				ASSERT_TRUE(property_is_readonly(the_property, &returned_read_only)) << project_free(the_project);
				//
				ASSERT_EQ(dynamic, returned_dynamic) << project_free(the_project);
				ASSERT_EQ(read_only, returned_read_only) << project_free(the_project);
			}
		}

		project_unload(the_project);
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
		const std::string content(node.node().select_node("content").node().child_value());
		struct range content_in_range;
		const auto project_help = (uint8_t)INT_PARSE(
									  node.node().select_node("project_help").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		const std::string expected_name(node.node().select_node("name").node().child_value());
		const std::string expected_default_target(node.node().select_node("default").node().child_value());
		const std::string target_to_run(node.node().select_node("target_to_run").node().child_value());
		//
		std::string expected_base_directory(node.node().select_node("base_directory").node().child_value());

		if (!expected_base_directory.empty())
		{
			ASSERT_TRUE(buffer_resize(&output, 0))
					<< buffer_free(&output);
			content_in_range = string_to_range(expected_base_directory);
			ASSERT_TRUE(interpreter_evaluate_code(NULL, NULL, &content_in_range, &output, verbose))
					<< buffer_free(&output);
			expected_base_directory = buffer_to_string(&output);
		}

		const auto properties = node.node().select_nodes("property");
		const auto targets = node.node().select_nodes("target");
		//
		content_in_range = string_to_range(content);
		ASSERT_EQ(content.empty(), range_is_null_or_empty(&content_in_range))
				<< buffer_free(&output);
		//
		void* the_project = NULL;
		ASSERT_TRUE(project_new(&the_project))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned = project_load_from_content(content_in_range.start, content_in_range.finish,
							  the_project, project_help, verbose);
		ASSERT_EQ(expected_return, returned)
				<< content << std::endl << buffer_free(&output) << project_free(the_project);

		if (target_to_run.empty())
		{
			ASSERT_TRUE(project_evaluate_default_target(the_project, verbose))
					<< content << std::endl << buffer_free(&output) << project_free(the_project);
		}
		else
		{
			const auto target_to_run_in_range(string_to_range(target_to_run));
			ASSERT_TRUE(target_evaluate_by_name(the_project, &target_to_run_in_range, verbose))
					<< content << std::endl << buffer_free(&output) << project_free(the_project);
		}

		const void* the_property = NULL;
		ASSERT_NE(expected_base_directory.empty(), project_get_base_directory(the_project, &the_property))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned_base_directory(property_to_string(the_property, &output));
		ASSERT_EQ(expected_base_directory, returned_base_directory)
				<< buffer_free(&output) << project_free(the_project);
		//
		the_property = NULL;
		ASSERT_FALSE(project_get_buildfile_path(the_project, &the_property))
				<< buffer_free(&output) << project_free(the_project);
		//
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output) << project_free(the_project);
		ASSERT_FALSE(project_get_buildfile_uri(the_property, &output))
				<< buffer_free(&output) << project_free(the_project);
		ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output) << project_free(the_project);
		//
		the_property = NULL;
		ASSERT_EQ(!expected_name.empty(), project_get_name(the_project, &the_property))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned_name(property_to_string(the_property, &output));
		ASSERT_EQ(expected_name, returned_name)
				<< buffer_free(&output) << project_free(the_project);
		//
		the_property = NULL;
		ASSERT_EQ(!expected_default_target.empty(), project_get_default_target(the_project, &the_property))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned_default_target(property_to_string(the_property, &output));
		ASSERT_EQ(expected_default_target, returned_default_target)
				<< buffer_free(&output) << project_free(the_project);
		//
		static const uint8_t asterisk = '*';

		if (!project_target_exists(the_project, &asterisk, 1))
		{
			ASSERT_NE(expected_default_target.empty(), project_target_exists(the_project,
					  (const uint8_t*)expected_default_target.c_str(), (uint8_t)expected_default_target.size()))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< expected_default_target;
		}

		for (const auto& property_node : properties)
		{
			void* the_project_property = NULL;
			const std::string property_name(property_node.node().attribute("name").as_string());
			//
			ASSERT_TRUE(project_property_exists(the_project,
												(const uint8_t*)property_name.c_str(),
												(uint8_t)property_name.size(),
												&the_project_property, verbose))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			const uint8_t expected_is_dynamic = property_node.node().attribute("dynamic").as_bool();
			const uint8_t expected_is_read_only = property_node.node().attribute("readonly").as_bool();
			const std::string expected_property_value(property_node.node().attribute("value").as_string());
			//
			uint8_t is_dynamic = 0;
			uint8_t is_read_only = 0;
			//
			ASSERT_TRUE(property_is_dynamic(the_project_property, &is_dynamic))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_is_dynamic, is_dynamic)
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_TRUE(property_is_readonly(the_project_property, &is_read_only))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_is_read_only, is_read_only)
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_TRUE(buffer_resize(&output, 0))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_TRUE(property_get_by_pointer(the_project_property, &output))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_property_value, buffer_to_string(&output))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
		}

		for (const auto& target_name : targets)
		{
			std::string target_name_str(target_name.node().child_value());
			ASSERT_TRUE(project_target_exists(the_project, (const uint8_t*)target_name_str.c_str(),
											  (uint8_t)target_name_str.size()))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< "Target name - '" << target_name_str << "'." << std::endl;
			//
			void* target = NULL;
			ASSERT_TRUE(project_target_get(the_project, (const uint8_t*)target_name_str.c_str(),
										   (uint8_t)target_name_str.size(), &target, verbose))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< "Target name - '" << target_name_str << "'." << std::endl;
			//
			uint16_t index = 0;
			const range* depend_target_name = NULL;

			while (NULL != (depend_target_name = target_get_depend(target, index++)))
			{
				target_name_str = range_to_string(depend_target_name);
				ASSERT_TRUE(project_target_exists(the_project, (const uint8_t*)target_name_str.c_str(),
												  (uint8_t)target_name_str.size()))
						<< buffer_free(&output) << project_free(the_project) << std::endl
						<< "Target name - '" << target_name_str << "'." << std::endl;
			}
		}

		project_unload(the_project);
		the_project = NULL;
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
		void* the_project = NULL;
		ASSERT_TRUE(project_new(&the_project)) << buffer_free(&tmp) << project_free(the_project);
		//
		const auto path_in_range(string_to_range(path));
		const auto returned = project_load_from_build_file(
								  &path_in_range, &current_path_in_range, encoding, the_project, 0, verbose);
		//
		ASSERT_EQ(expected_return, returned)
				<< path << std::endl << buffer_free(&tmp) << project_free(the_project);
		//
		project_unload(the_project);
		//
		--node_count;
	}

	buffer_release(&tmp);
}
//project_exec_function

class TestProgram : public TestsBaseXml
{
};

TEST(TestProgram_, program_exec_function)
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

extern "C" {
	extern uint8_t program_get_properties(
		const void* the_project,
		const void* the_target,
		const struct buffer* properties_elements,
		struct buffer* properties,
		uint8_t is_root, uint8_t verbose);
};

TEST_F(TestProgram, program_get_properties)
{
	struct buffer properties_elements;
	SET_NULL_TO_BUFFER(properties_elements);
	//
	struct buffer properties;
	SET_NULL_TO_BUFFER(properties);
	//
	void* the_project = NULL;
	//
	ASSERT_TRUE(project_new(&the_project))
			<< buffer_free(&properties_elements)
			<< properties_free(&properties)
			<< project_free(the_project);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const auto input_in_range(string_to_range(input));
		//
		ASSERT_TRUE(xml_get_sub_nodes_elements(
						input_in_range.start, input_in_range.finish, NULL, &properties_elements))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);
		//
		ASSERT_TRUE(program_get_properties(the_project, NULL, &properties_elements, &properties, 1, 0))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);
		//
		ASSERT_TRUE(buffer_resize(&properties_elements, 0))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);

		for (const auto& property_node : node.node().select_nodes("properties/property"))
		{
			void* the_property = NULL;
			const std::string property_name(property_node.node().attribute("name").as_string());
			//
			ASSERT_TRUE(property_exists(
							&properties,
							(const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(), &the_property))
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			const uint8_t expected_is_dynamic = property_node.node().attribute("dynamic").as_bool();
			const uint8_t expected_is_read_only = property_node.node().attribute("readonly").as_bool();
			//
			uint8_t is_dynamic = 0;
			uint8_t is_read_only = 0;
			//
			ASSERT_TRUE(property_is_dynamic(the_property, &is_dynamic))
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_is_dynamic, is_dynamic)
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			ASSERT_TRUE(property_is_readonly(the_property, &is_read_only))
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_is_read_only, is_read_only)
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
		}

		property_release_inner(&properties);
		project_clear(the_project);
		//
		--node_count;
	}

	buffer_release(&properties_elements);
	property_release(&properties);
	project_unload(the_project);
}
