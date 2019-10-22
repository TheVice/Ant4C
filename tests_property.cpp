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
#include "property.h"
#include "string_unit.h"
};

#include <cstdint>
#include <cstring>
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
	const char* property_name = "My_property";
	const uint8_t property_name_length = (uint8_t)strlen(property_name);
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
	const char* property_value = "Property value";
	const uint8_t property_value_length = (uint8_t)strlen(property_value);
	ASSERT_TRUE(
		property_set_by_name(NULL, NULL, &properties, property_name, property_name_length,
							 property_value, property_value_length,
							 property_value_is_char_array,
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
	ASSERT_TRUE(string_equal(property_value, property_value + property_value_length,
							 buffer_char_data(&output, 0), buffer_char_data(&output,
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
	ASSERT_TRUE(property_set_by_name(NULL, NULL, &properties, "my_property", 11, "${math::PI()}", 13,
									 property_value_is_char_array,
									 0, 0, 0, verbose)) <<
											 buffer_free(&output) << properties_free(&properties);
	void* the_property = NULL;
	ASSERT_TRUE(property_get_pointer(&properties, "my_property", 11, &the_property)) <<
			buffer_free(&output) << properties_free(&properties);
	ASSERT_TRUE(property_get_by_pointer(NULL, NULL, the_property, &output)) <<
			buffer_free(&output) << properties_free(&properties);
	ASSERT_STREQ(expected_return, buffer_to_string(&output).c_str()) <<
			buffer_free(&output) << properties_free(&properties);
	ASSERT_FALSE(property_set_by_pointer(NULL, NULL, the_property,
										 "${property::get-value('my_property')} ${math::E()}", 50,
										 property_value_is_char_array, 0, 0, verbose)) <<
												 buffer_free(&output) << properties_free(&properties);
	//
	buffer_release(&output);
	property_clear(&properties);
}

TEST_F(TestProperty, property_set_from_xml_tag_record)
{
	buffer properties;
	SET_NULL_TO_BUFFER(properties);
	//
	buffer output_properties;
	SET_NULL_TO_BUFFER(output_properties);

	for (const auto& node : nodes)
	{
		property_clear(&properties);
		property_clear(&output_properties);
		//
		/*const uint8_t properties_loaded = */properties_load_from_node(node, "properties/property",
				&properties);
		//
		const std::string record(node.node().select_node("record").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		const uint8_t verbose = 0;
		//
		/*const uint8_t output_properties_loaded = */properties_load_from_node(node, "output_properties/property",
				&output_properties);
		//
		const uint8_t returned = property_set_from_xml_tag_record(NULL, NULL,
								 &properties, record.c_str(), record.empty() ? NULL : record.data() + record.size(),
								 verbose);
		//
		ASSERT_EQ(expected_return, returned) << properties_free(&properties) << properties_free(&output_properties);
		/*TODO: */
		//
		--node_count;
	}

	property_clear(&properties);
	property_clear(&output_properties);
}
