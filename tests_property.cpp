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
#include "interpreter.h"
#include "project.h"
#include "property.h"
#include "string_unit.h"
};

#include <string>
#include <cstddef>
#include <cstdint>
#include <ostream>

class TestProperty : public TestsBaseXml
{
};

TEST(TestProperty_, property_at_all)
{
	buffer properties;
	SET_NULL_TO_BUFFER(properties);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	const uint8_t* property_name = (const uint8_t*)"My_property";
	const uint8_t property_name_length = (uint8_t)common_count_bytes_until(property_name, 0);
	//
	void* the_property = NULL;
	ASSERT_FALSE(property_exists(&properties, property_name, property_name_length, &the_property))
			<< buffer_free(&output) << properties_free(&properties);
	//
	ASSERT_FALSE(property_get_by_pointer(the_property, &output))
			<< buffer_free(&output) << properties_free(&properties);
	//
	uint8_t dynamic = 0;
	ASSERT_FALSE(property_is_dynamic(the_property, &dynamic))
			<< buffer_free(&output) << properties_free(&properties);
	//
	uint8_t read_only = 0;
	ASSERT_FALSE(property_is_readonly(the_property, &read_only))
			<< buffer_free(&output) << properties_free(&properties);
	//
	const uint8_t* property_value = (const uint8_t*)"Property value";
	const uint8_t property_value_length = (uint8_t)common_count_bytes_until(property_value, 0);
	ASSERT_TRUE(
		property_set_by_name(&properties, property_name, property_name_length,
							 property_value, property_value_length,
							 property_value_is_byte_array,
							 0, 0, 1, 0))
			<< buffer_free(&output) << properties_free(&properties);
	//
	ASSERT_TRUE(property_exists(&properties, property_name, property_name_length, &the_property))
			<< buffer_free(&output) << properties_free(&properties);
	//
	ASSERT_TRUE(property_get_by_pointer(the_property, &output))
			<< buffer_free(&output) << properties_free(&properties);
	//
	ASSERT_EQ(property_value_length, buffer_size(&output))
			<< buffer_free(&output) << properties_free(&properties);
	//
	ASSERT_TRUE(string_equal((const uint8_t*)property_value,
							 (const uint8_t*)property_value + property_value_length,
							 buffer_data(&output, 0),
							 buffer_data(&output, 0) + property_value_length))
			<< buffer_free(&output) << properties_free(&properties);
	//
	ASSERT_TRUE(property_is_dynamic(the_property, &dynamic))
			<< buffer_free(&output) << properties_free(&properties);
	ASSERT_EQ(0, dynamic)
			<< buffer_free(&output) << properties_free(&properties);
	//
	ASSERT_TRUE(property_is_readonly(the_property, &read_only))
			<< buffer_free(&output) << properties_free(&properties);
	ASSERT_EQ(1, read_only)
			<< buffer_free(&output) << properties_free(&properties);
	//
	ASSERT_EQ(ATTEMPT_TO_WRITE_READ_ONLY_PROPERTY,
			  property_set_by_pointer(the_property, the_property, 0, property_value_is_byte_array, 0, 1, 0))
			<< buffer_free(&output) << properties_free(&properties);
	//
	buffer_release(&output);
	property_release(&properties);
}

TEST(TestProperty_, property_get)
{
	std::string code("<property name=\"my\" value=\"${my}\" failonerror=\"false\" />");
	auto code_in_range(string_to_range(code));
	//
	struct buffer the_project;
	SET_NULL_TO_BUFFER(the_project);
	//
	ASSERT_TRUE(project_new(&the_project));
	//
	ASSERT_EQ(FAIL_WITH_OUT_ERROR, project_load_from_content(code_in_range.start, code_in_range.finish,
			  &the_project,
			  0, 0))
			<< project_free(&the_project);
	//
	project_clear(&the_project);
	//
	code = "<project><property name=\"my\" value=\"${my}\" failonerror=\"false\" /></project>";
	code_in_range = string_to_range(code);
	//
	ASSERT_EQ(FAIL_WITH_OUT_ERROR, project_load_from_content(code_in_range.start, code_in_range.finish,
			  &the_project,
			  0, 0))
			<< project_free(&the_project);
	//
	project_unload(&the_project);
}

TEST_F(TestProperty, property_task)
{
	static const std::string property_str("property");
	static const auto task_id = interpreter_get_task((const uint8_t*)property_str.c_str(),
								(const uint8_t*)property_str.c_str() + 8);
	//
	buffer properties;
	SET_NULL_TO_BUFFER(properties);

	for (const auto& node : nodes)
	{
		property_release(&properties);
		ASSERT_TRUE(properties_load_from_node(node, "properties/property", &properties))
				<< properties_free(&properties);
		//
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		const auto output_properties = node.node().select_nodes("output_properties/property");
		//
		struct buffer the_project;
		SET_NULL_TO_BUFFER(the_project);
		//
		ASSERT_TRUE(project_new(&the_project)) << properties_free(&properties) << project_free(&the_project);
		//
		ASSERT_TRUE(property_add_at_project(&the_project, &properties, verbose))
				<< properties_free(&properties) << project_free(&the_project);
		//
		property_release(&properties);

		for (const auto& record_node : node.node().select_nodes("record"))
		{
			const std::string record = property_str + " " + record_node.node().child_value();
			const auto record_in_range(string_to_range(record));
			//
			struct range task_in_range;
			task_in_range.start = (const uint8_t*)record.c_str();
			task_in_range.finish = (const uint8_t*)task_in_range.start + property_str.size();
			//
			const auto returned = interpreter_evaluate_task(
									  &the_project, NULL, task_id,
									  &task_in_range, record_in_range.finish,
									  0, verbose);
			//
			ASSERT_EQ(expected_return, returned)
					<< record << std::endl
					<< properties_free(&properties) << project_free(&the_project);
		}

		for (const auto& property : output_properties)
		{
			std::string name;
			std::string expected_value;
			uint8_t expected_dynamic;
			uint8_t over_write;
			uint8_t expected_read_only;
			uint8_t fail_on_error;
			uint8_t local_verbose;
			//
			property_load_from_node(property.node(), name, expected_value, expected_dynamic,
									over_write, expected_read_only, fail_on_error, local_verbose);
			//
			ASSERT_TRUE(buffer_resize(&properties, 0)) << buffer_free(&properties) << project_free(&the_project);
			ASSERT_TRUE(project_property_get_by_name(
							&the_project, (const uint8_t*)name.c_str(), (uint8_t)name.size(),
							&properties, verbose))
					<< name << std::endl << buffer_free(&properties) << project_free(&the_project);
			ASSERT_EQ(expected_value, buffer_to_string(&properties)) << buffer_free(&properties);
			ASSERT_TRUE(buffer_resize(&properties, 0)) << buffer_free(&properties) << project_free(&the_project);
			//
			void* the_property = NULL;
			ASSERT_TRUE(project_property_exists(
							&the_project, (const uint8_t*)name.c_str(), (uint8_t)name.size(), &the_property, verbose))
					<< name << buffer_free(&properties) << project_free(&the_project);
			//
			uint8_t returned_dynamic = 0;
			ASSERT_TRUE(property_is_dynamic(the_property,
											&returned_dynamic)) << name << buffer_free(&properties) << project_free(&the_project);
			ASSERT_EQ(expected_dynamic, returned_dynamic) << name << buffer_free(&properties) << project_free(
						&the_project);
			//
			uint8_t returned_read_only = 0;
			ASSERT_TRUE(property_is_readonly(the_property,
											 &returned_read_only)) << name << buffer_free(&properties) << project_free(&the_project);
			ASSERT_EQ(expected_read_only, returned_read_only) << name << buffer_free(&properties) << project_free(
						&the_project);
		}

		project_unload(&the_project);
		--node_count;
	}

	property_release(&properties);
}
