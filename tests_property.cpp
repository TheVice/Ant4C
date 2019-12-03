/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
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

#include <cstdint>
#include <cstddef>

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
	uint8_t readonly = 0;
	ASSERT_FALSE(property_is_readonly(the_property, &readonly))
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
	ASSERT_TRUE(property_is_readonly(the_property, &readonly))
			<< buffer_free(&output) << properties_free(&properties);
	ASSERT_EQ(1, readonly)
			<< buffer_free(&output) << properties_free(&properties);
	//
	buffer_release(&output);
	property_clear(&properties);
}

TEST_F(TestProperty, property_task)
{
	static const uint8_t* property_str = (const uint8_t*)"property";
	static const uint8_t task_id = interpreter_get_task(property_str, property_str + 8);
	//
	buffer properties;
	SET_NULL_TO_BUFFER(properties);

	for (const auto& node : nodes)
	{
		property_clear(&properties);
		ASSERT_TRUE(properties_load_from_node(node, "properties/property", &properties))
				<< properties_free(&properties);
		//
		const std::string record(node.node().select_node("record").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		const auto output_properties = node.node().select_nodes("output_properties/property");
		const uint8_t verbose = 0;
		//
		void* project = NULL;
		ASSERT_TRUE(project_new(&project)) << properties_free(&properties) << project_free(&project);
		//
		ASSERT_TRUE(property_add_at_project(project, &properties, verbose)) << project_free(&project);
		//
		const uint8_t returned = interpreter_evaluate_task(project, NULL, task_id,
								 (const uint8_t*)record.c_str(), (const uint8_t*)record.c_str() + record.size(), verbose);
		ASSERT_EQ(expected_return, returned) << properties_free(&properties) << project_free(&project);
		property_clear(&properties);

		for (const auto& property : output_properties)
		{
			std::string name;
			std::string expected_value;
			uint8_t expected_dynamic;
			uint8_t overwrite;
			uint8_t expected_readonly;
			uint8_t fail_on_error;
			uint8_t local_verbose;
			//
			property_load_from_node(property.node(), name, expected_value, expected_dynamic,
									overwrite, expected_readonly, fail_on_error, local_verbose);
			ASSERT_TRUE(buffer_resize(&properties, 0)) << buffer_free(&properties);
			ASSERT_TRUE(property_get_by_name(project, (const uint8_t*)name.c_str(), (uint8_t)name.size(),
											 &properties))
					<< name << std::endl << buffer_free(&properties);
			ASSERT_EQ(expected_value, buffer_to_string(&properties)) << buffer_free(&properties);
			ASSERT_TRUE(buffer_resize(&properties, 0)) << buffer_free(&properties);
			//
			void* the_property = NULL;
			ASSERT_TRUE(project_property_exists(
							project, (const uint8_t*)name.c_str(), (uint8_t)name.size(), &the_property))
					<< name << buffer_free(&properties);
			//
			uint8_t returned_dynamic = 0;
			ASSERT_TRUE(property_is_dynamic(the_property, &returned_dynamic)) << name << buffer_free(&properties);
			ASSERT_EQ(expected_dynamic, returned_dynamic) << name << buffer_free(&properties);
			//
			uint8_t returned_readonly = 0;
			ASSERT_TRUE(property_is_readonly(the_property, &returned_readonly)) << name << buffer_free(&properties);
			ASSERT_EQ(expected_readonly, returned_readonly) << name << buffer_free(&properties);
		}

		project_unload(project);
		--node_count;
	}

	property_clear(&properties);
}
