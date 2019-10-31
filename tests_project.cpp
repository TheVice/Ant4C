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
	struct buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	uint8_t returned = project_get_base_directory(project, NULL, &output);
	ASSERT_EQ(expected_return, returned) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	returned = project_get_buildfile_path(project, NULL, &output);
	ASSERT_EQ(expected_return, returned) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	returned = project_get_buildfile_uri(project, NULL, &output);
	ASSERT_EQ(expected_return, returned) << buffer_free(&output);
	ASSERT_STREQ("file:///", buffer_to_string(&output).c_str()) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
	//
	returned = project_get_default_target(project, NULL, &output);
	ASSERT_EQ(expected_return, returned) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	returned = project_get_name(project, NULL, &output);
	ASSERT_EQ(expected_return, returned) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	buffer_release(&output);
	//
	void* the_property = NULL;
	ASSERT_FALSE(project_property_get_pointer(project, (const uint8_t*)"program.version", 15, &the_property));
	ASSERT_EQ(nullptr, the_property);
	//
	ASSERT_TRUE(project_property_set_value(project, NULL,
										   (const uint8_t*)"program.version", 15,
										   (const uint8_t*)"YYYY.MM.DD.?", 12,
										   0, 0, 1, verbose));
	//
	ASSERT_TRUE(project_property_get_pointer(project, (const uint8_t*)"program.version", 15, &the_property));
	ASSERT_NE(nullptr, the_property);
	the_property = NULL;
	//
	project_unload(project);
}

uint8_t project_free(void* project)
{
	project_unload(project);
	return 0;
}

TEST(TestProject_, project_property_set_value)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
	/*TODO: for (const auto& node : nodes)
	{*/
	void* project = NULL;
	ASSERT_TRUE(project_new(&project)) << buffer_free(&output);
	ASSERT_NE(nullptr, project) << buffer_free(&output);
	//
	const char* expected_return_1 = "7";
	const char* expected_return_2 = "7 -1";
	uint8_t verbose = 0;
	//
	ASSERT_TRUE(project_property_set_value(project, NULL,
										   (const uint8_t*)"my_property", 11,
										   (const uint8_t*)"${math::truncate(math::addition('3', '4'))}", 43,
										   0, 0, 0, verbose)) <<
												   buffer_free(&output) << project_free(project);
	//
	void* the_property = NULL;
	ASSERT_TRUE(project_property_get_pointer(project, (const uint8_t*)"my_property", 11, &the_property)) <<
			buffer_free(&output) << project_free(project);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) <<
										   buffer_free(&output) << project_free(project);
	ASSERT_TRUE(property_get_by_pointer(project, NULL, the_property, &output)) <<
			buffer_free(&output) << project_free(project);
	//
	ASSERT_STREQ(expected_return_1, buffer_to_string(&output).c_str()) <<
			buffer_free(&output) << project_free(project);
	//
	ASSERT_TRUE(property_set_by_pointer(project, NULL, the_property,
										(const uint8_t*)"${property::get-value('my_property')} ${math::truncate(math::addition('3', '-4'))}", 82,
										property_value_is_byte_array, 0, 0, verbose)) <<
												buffer_free(&output) << project_free(project);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) <<
										   buffer_free(&output) << project_free(project);
	ASSERT_TRUE(property_get_by_pointer(project, NULL, the_property, &output)) <<
			buffer_free(&output) << project_free(project);
	//
	ASSERT_STREQ(expected_return_2, buffer_to_string(&output).c_str()) <<
			buffer_free(&output) << project_free(project);
	//
	project_unload(project);
	//}
	buffer_release(&output);
}

TEST_F(TestProject, project_load_from_content)
{
#if defined(_WIN32)
	static const std::string base_dir_str("A:\\");
#else
	static const std::string base_dir_str("/");
#endif
	static const range base_dir = string_to_range(base_dir_str);
	//
	const std::string base_directory = base_dir_str;
	//
	struct buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const std::string content(node.node().select_node("content").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(
											node.node().select_node("return").node().child_value());
		const std::string expected_name(node.node().select_node("name").node().child_value());
		const std::string expected_default_target(node.node().select_node("default").node().child_value());
		const uint8_t verbose = 0;
		const auto targets = node.node().select_nodes("target");
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
								 base_dir.start, range_size(&base_dir), project, verbose);
		ASSERT_EQ(expected_return, returned)
				<< buffer_free(&output) << project_free(project);

		if (returned)
		{
			ASSERT_TRUE(buffer_resize(&output, 0))
					<< buffer_free(&output) << project_free(project);
			ASSERT_EQ(!base_directory.empty(), project_get_base_directory(project, NULL, &output))
					<< buffer_free(&output) << project_free(project);
			const std::string returned_base_directory(buffer_to_string(&output));
			ASSERT_EQ(base_directory, returned_base_directory)
					<< buffer_free(&output) << project_free(project);
		}

		ASSERT_TRUE(buffer_resize(&output, 0))
				<< buffer_free(&output) << project_free(project);
		ASSERT_FALSE(project_get_buildfile_path(project, NULL, &output))
				<< buffer_free(&output) << project_free(project);
		ASSERT_FALSE(buffer_size(&output))
				<< buffer_free(&output) << project_free(project);
		//
		ASSERT_TRUE(buffer_resize(&output, 0))
				<< buffer_free(&output) << project_free(project);
		ASSERT_FALSE(project_get_buildfile_uri(project, NULL, &output))
				<< buffer_free(&output) << project_free(project);
		ASSERT_STREQ("file:///", buffer_to_string(&output).c_str())
				<< buffer_free(&output) << project_free(project);
		//
		ASSERT_TRUE(buffer_resize(&output, 0))
				<< buffer_free(&output) << project_free(project);
		ASSERT_EQ(!expected_name.empty(), project_get_name(project, NULL, &output))
				<< buffer_free(&output) << project_free(project);
		const std::string returned_name(buffer_to_string(&output));
		ASSERT_EQ(expected_name, returned_name)
				<< buffer_free(&output) << project_free(project);
		//
		ASSERT_TRUE(buffer_resize(&output, 0))
				<< buffer_free(&output) << project_free(project);
		ASSERT_EQ(!expected_default_target.empty(), project_get_default_target(project, NULL, &output))
				<< buffer_free(&output) << project_free(project);
		const std::string returned_default_target(buffer_to_string(&output));
		ASSERT_EQ(expected_default_target, returned_default_target)
				<< buffer_free(&output) << project_free(project);

		for (const auto& target : targets)
		{
			const std::string target_name(target.node().child_value());
			ASSERT_TRUE(project_target_exists(project, (const uint8_t*)target_name.c_str(), (uint8_t)target_name.size()))
					<< buffer_free(&output) << project_free(project) << std::endl
					<< "Target name - '" << target_name << "'." << std::endl;
		}

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
project_add_properties
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
