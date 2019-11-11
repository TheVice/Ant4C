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
	ASSERT_FALSE(property_exists(&properties, property_name,
								 property_name_length)) << buffer_free(&output) << properties_free(
										 &properties);
	//
	void* the_property = NULL;
	ASSERT_FALSE(property_get_pointer(&properties, property_name, property_name_length,
									  &the_property)) << buffer_free(&output) << properties_free(
											  &properties);
	//
	ASSERT_FALSE(property_get_by_pointer(NULL, NULL, the_property,
										 &output)) << buffer_free(&output) << properties_free(
												 &properties);
	//
	uint8_t dynamic = 0;
	ASSERT_FALSE(property_is_dynamic(the_property, &dynamic)) << buffer_free(&output) << properties_free(
				&properties);
	//
	uint8_t readonly = 0;
	ASSERT_FALSE(property_is_readonly(the_property, &readonly)) << buffer_free(&output) << properties_free(
				&properties);
	//
	const uint8_t* property_value = (const uint8_t*)"Property value";
	const uint8_t property_value_length = (uint8_t)common_count_bytes_until(property_value, 0);
	ASSERT_TRUE(
		property_set_by_name(NULL, NULL, &properties, property_name, property_name_length,
							 property_value, property_value_length,
							 property_value_is_byte_array,
							 0, 0, 1, 0)) << buffer_free(&output) << properties_free(
									 &properties);
	//
	ASSERT_TRUE(property_exists(&properties, property_name,
								property_name_length)) << buffer_free(&output) << properties_free(
											&properties);
	//
	ASSERT_TRUE(property_get_pointer(&properties, property_name, property_name_length,
									 &the_property)) << buffer_free(&output) << properties_free(
											 &properties);
	//
	ASSERT_TRUE(property_get_by_pointer(NULL, NULL, the_property,
										&output)) << buffer_free(&output) << properties_free(
												&properties);
	//
	ASSERT_EQ(property_value_length, buffer_size(&output)) << buffer_free(&output) << properties_free(
				&properties);
	//
	ASSERT_TRUE(string_equal((const uint8_t*)property_value,
							 (const uint8_t*)property_value + property_value_length,
							 buffer_data(&output, 0), buffer_data(&output,
									 0) + property_value_length)) << buffer_free(&output) << properties_free(
											 &properties);
	//
	ASSERT_TRUE(property_is_dynamic(the_property, &dynamic)) << buffer_free(&output) << properties_free(
				&properties);
	ASSERT_EQ(0, dynamic) << buffer_free(&output) << properties_free(
							  &properties);
	//
	ASSERT_TRUE(property_is_readonly(the_property, &readonly)) << buffer_free(&output) << properties_free(
				&properties);
	ASSERT_EQ(1, readonly) << buffer_free(&output) << properties_free(
							   &properties);
	//
	buffer_release(&output);
	property_clear(&properties);
}

TEST(TestProperty_, property_set_value)
{
	uint8_t verbose = 0;
	//
	buffer properties;
	SET_NULL_TO_BUFFER(properties);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	const char* expected_return = "3.1415926535897931";
	//
	ASSERT_TRUE(property_set_by_name(NULL, NULL, &properties, (const uint8_t*)"my_property", 11,
									 (const uint8_t*)"${math::PI()}", 13,
									 property_value_is_byte_array,
									 0, 0, 0, verbose)) <<
											 buffer_free(&output) << properties_free(&properties);
	void* the_property = NULL;
	ASSERT_TRUE(property_get_pointer(&properties, (const uint8_t*)"my_property", 11, &the_property)) <<
			buffer_free(&output) << properties_free(&properties);
	ASSERT_TRUE(property_get_by_pointer(NULL, NULL, the_property, &output)) <<
			buffer_free(&output) << properties_free(&properties);
	ASSERT_STREQ(expected_return, buffer_to_string(&output).c_str()) <<
			buffer_free(&output) << properties_free(&properties);
	ASSERT_FALSE(property_set_by_pointer(NULL, NULL, the_property,
										 (const uint8_t*)"${property::get-value('my_property')} ${math::E()}", 50,
										 property_value_is_byte_array, 0, 0, verbose)) <<
												 buffer_free(&output) << properties_free(&properties);
	//
	buffer_release(&output);
	property_clear(&properties);
}
#if 0
TEST_F(TestProperty, property_set_from_xml_tag_record)
{
}
#endif
TEST(TestProperty_, property_get_attributes_and_arguments_for_task)
{
	buffer task_arguments;
	SET_NULL_TO_BUFFER(task_arguments);
	/**/
	const uint8_t** task_attributes = NULL;
	const uint8_t* task_attributes_lengths = NULL;
	uint8_t task_attributes_count = 0;
	/**/
	const uint8_t returned = property_get_attributes_and_arguments_for_task(
								 &task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments);
	/**/
	ASSERT_TRUE(returned) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_NE(nullptr, task_attributes) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_NE(nullptr, task_attributes_lengths) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_EQ(7, task_attributes_count) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_LT(0, buffer_size(&task_arguments)) << buffer_free_with_inner_buffers(&task_arguments);
	/**/
	task_attributes_count = 0;
	buffer* argument = NULL;

	while (NULL != (argument = buffer_buffer_data(&task_arguments, task_attributes_count++)))
	{
		if (2 == task_attributes_count || 7 == task_attributes_count)
		{
			ASSERT_FALSE(buffer_size(argument)) << buffer_free_with_inner_buffers(&task_arguments);
			continue;
		}

		uint8_t bool_value = 0;
		ASSERT_TRUE(buffer_size(argument)) << (int)task_attributes_count << std::endl <<
										   buffer_free_with_inner_buffers(&task_arguments);
		ASSERT_TRUE(bool_parse(buffer_data(argument, 0), buffer_size(argument), &bool_value))
				<< buffer_free_with_inner_buffers(&task_arguments);

		if (3 == task_attributes_count || 5 == task_attributes_count)
		{
			ASSERT_TRUE(bool_value) << buffer_free_with_inner_buffers(&task_arguments);
		}
		else
		{
			ASSERT_FALSE(bool_value) << buffer_free_with_inner_buffers(&task_arguments);
		}
	}

	ASSERT_EQ(8, task_attributes_count) << buffer_free_with_inner_buffers(&task_arguments);
	buffer_release_with_inner_buffers(&task_arguments);
}
