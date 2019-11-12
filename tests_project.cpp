/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "conversion.h"
#include "echo.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "text_encoding.h"
};

class TestProject : public TestsBaseXml
{
};

TEST(TestProject_, project_new)
{
	static const uint8_t expected_return = 0;
	static const uint8_t verbose = 0;
	//
	void* project = NULL;
	ASSERT_TRUE(project_new(&project));
	ASSERT_NE(nullptr, project);
	//
	const void* the_property = NULL;
	//
	the_property = NULL;
	uint8_t returned = project_get_base_directory(project, &the_property);
	ASSERT_EQ(expected_return, returned);
	//
	the_property = NULL;
	returned = project_get_buildfile_path(project, &the_property);
	ASSERT_EQ(expected_return, returned);
	//
	/*returned = project_get_buildfile_uri(project, NULL, &output);
	ASSERT_EQ(expected_return, returned) << buffer_free(&output);
	ASSERT_STREQ("file:///", buffer_to_string(&output).c_str()) << buffer_free(&output);*/
	//
	the_property = NULL;
	returned = project_get_default_target(project, &the_property);
	ASSERT_EQ(expected_return, returned);
	//
	the_property = NULL;
	returned = project_get_name(project, &the_property);
	ASSERT_EQ(expected_return, returned);
	//
	void* the_non_const_property = NULL;
	ASSERT_FALSE(project_property_get_pointer(project, (const uint8_t*)"program.version", 15,
				 &the_non_const_property));
	ASSERT_EQ(nullptr, the_non_const_property);
	//
	ASSERT_TRUE(project_property_set_value(project,
										   (const uint8_t*)"program.version", 15,
										   (const uint8_t*)"YYYY.MM.DD.?", 12,
										   0, 0, 1, verbose));
	//
	ASSERT_TRUE(project_property_get_pointer(project, (const uint8_t*)"program.version", 15,
				&the_non_const_property));
	ASSERT_NE(nullptr, the_non_const_property);
	the_property = NULL;
	the_non_const_property = NULL;
	//
	project_unload(project);
}
#if 0
TEST(TestProject_, project_property_set_value)
{
}
#endif
TEST_F(TestProject, project_load_from_content)
{
	struct buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const void* the_property = NULL;
		//
		const std::string content(node.node().select_node("content").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(
											node.node().select_node("return").node().child_value());
		const std::string expected_name(node.node().select_node("name").node().child_value());
		const std::string expected_default_target(node.node().select_node("default").node().child_value());
		const std::string expected_base_directory(node.node().select_node("base_directory").node().child_value());
		const uint8_t verbose = 0;
		//TODO: const auto targets = node.node().select_nodes("target");
		//
		const range content_in_range = string_to_range(content);
		ASSERT_EQ(content.empty(), range_is_null_or_empty(&content_in_range))
				<< buffer_free(&output);
		//
		void* project = NULL;
		ASSERT_TRUE(project_new(&project))
				<< buffer_free(&output) << project_free(project);
		//
		const uint8_t returned = project_load_from_content(content_in_range.start, content_in_range.finish,
								 project, verbose);
		ASSERT_EQ(expected_return, returned)
				<< content << std::endl << buffer_free(&output) << project_free(project);

		if (returned)
		{
			the_property = NULL;
			ASSERT_NE(expected_base_directory.empty(), project_get_base_directory(project, &the_property))
					<< buffer_free(&output) << project_free(project);
			const std::string returned_base_directory(property_to_string(the_property, &output));
			ASSERT_EQ(expected_base_directory, returned_base_directory)
					<< buffer_free(&output) << project_free(project);
		}

		the_property = NULL;
		ASSERT_FALSE(project_get_buildfile_path(project, &the_property))
				<< buffer_free(&output) << project_free(project);
		//
		/*ASSERT_FALSE(project_get_buildfile_uri(project, NULL, &output))
				<< buffer_free(&output) << project_free(project);
		ASSERT_STREQ("file:///", buffer_to_string(&output).c_str())
				<< buffer_free(&output) << project_free(project);*/
		//
		the_property = NULL;
		ASSERT_EQ(!expected_name.empty(), project_get_name(project, &the_property))
				<< buffer_free(&output) << project_free(project);
		const std::string returned_name(property_to_string(the_property, &output));
		ASSERT_EQ(expected_name, returned_name)
				<< buffer_free(&output) << project_free(project);
		//
		the_property = NULL;
		ASSERT_EQ(!expected_default_target.empty(), project_get_default_target(project, &the_property))
				<< buffer_free(&output) << project_free(project);
		const std::string returned_default_target(property_to_string(the_property, &output));
		ASSERT_EQ(expected_default_target, returned_default_target)
				<< buffer_free(&output) << project_free(project);
		/*TODO: for (const auto& target : targets)
		{
			const std::string target_name(target.node().child_value());
			ASSERT_TRUE(project_target_exists(project, (const uint8_t*)target_name.c_str(), (uint8_t)target_name.size()))
					<< buffer_free(&output) << project_free(project) << std::endl
					<< "Target name - '" << target_name << "'." << std::endl;
		}*/
		project_unload(project);
		project = NULL;
		//
		--node_count;
	}

	buffer_release(&output);
}
#if !defined(_WIN32)
TEST_F(TestProject, project_load_from_build_file)
{
	struct buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		const std::string content(node.node().select_node("content").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(
											node.node().select_node("return").node().child_value());
		uint8_t verbose = 0;
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
		ASSERT_TRUE(path_get_temp_file_name(&tmp)) << buffer_free(&tmp);
		const std::string tmp_path(buffer_to_string(&tmp));
		//
		ASSERT_TRUE(echo(0, Default, buffer_data(&tmp, 0), Info,
						 (const uint8_t*)content.c_str(), content.size(), 0, verbose))
				<< tmp_path << std::endl << buffer_free(&tmp);
		//
		void* project = NULL;
		ASSERT_TRUE(project_new(&project)) << buffer_free(&tmp);
		//
		const uint8_t returned = project_load_from_build_file(
									 buffer_data(&tmp, 0), project, verbose);
		ASSERT_EQ(expected_return, returned)
				<< tmp_path << std::endl << buffer_free(&tmp);
		//
		project_unload(project);
		//
		--node_count;
	}

	buffer_release(&tmp);
}
#endif
#if 0
project_exec_function
#endif
TEST(TestProgram, program_exec_function)
{
	struct buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_FALSE(program_exec_function(5,/*version_ TODO:${program::version()}*/
									   (const buffer*)&program_exec_function, 0, &output)) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	ASSERT_TRUE(program_exec_function(6,/*current_directory TODO:${program::current-directory()}*/
									  (const buffer*)&program_exec_function, 0, &output)) << buffer_free(&output);
	ASSERT_LT(0, buffer_size(&output)) << buffer_free(&output);
	//
	buffer_release(&output);
}

TEST(TestProject_, project_get_attributes_and_arguments_for_task)
{
	buffer task_arguments;
	SET_NULL_TO_BUFFER(task_arguments);
	/**/
	const uint8_t** task_attributes = NULL;
	const uint8_t* task_attributes_lengths = NULL;
	uint8_t task_attributes_count = 0;
	/**/
	const uint8_t returned = project_get_attributes_and_arguments_for_task(
								 &task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments);
	/**/
	ASSERT_TRUE(returned) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_NE(nullptr, task_attributes) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_NE(nullptr, task_attributes_lengths) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_EQ(3, task_attributes_count) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_LT(0, buffer_size(&task_arguments)) << buffer_free_with_inner_buffers(&task_arguments);
	/**/
	task_attributes_count = 0;
	buffer* argument = NULL;

	while (NULL != (argument = buffer_buffer_data(&task_arguments, task_attributes_count++)))
	{
		ASSERT_FALSE(buffer_size(argument)) << buffer_free_with_inner_buffers(&task_arguments);
	}

	ASSERT_EQ(4, task_attributes_count) << buffer_free_with_inner_buffers(&task_arguments);
	buffer_release_with_inner_buffers(&task_arguments);
}
