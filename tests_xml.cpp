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
#include "range.h"
#include "xml.h"
};

class TestXml : public TestsBaseXml
{
};

TEST_F(TestXml, xml_get_tag_finish_pos)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		const range input_in_range = string_to_range(input);
		const char* finish = xml_get_tag_finish_pos(input_in_range.start, input_in_range.finish);
		//
		ASSERT_LE(finish, input_in_range.finish) << input;

		if (NULL == finish)
		{
			ASSERT_TRUE(expected_output.empty()) << input;
		}
		else
		{
			const std::string output(finish, input_in_range.finish - finish);
			ASSERT_EQ(expected_output, output) << input;
		}

		--node_count;
	}
}

TEST_F(TestXml, xml_get_sub_nodes_elements)
{
	buffer elements;
	SET_NULL_TO_BUFFER(elements);
	buffer expected_elements;
	SET_NULL_TO_BUFFER(expected_elements);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		ASSERT_TRUE(buffer_resize_and_free_inner_buffers(&expected_elements)) <<
				buffer_free(&elements) << buffer_free_with_inner_buffers(&expected_elements);

		for (const auto& output : node.node().select_nodes("output"))
		{
			const std::string expected_output(output.node().child_value());
			buffer buffer_for_expected_output;
			SET_NULL_TO_BUFFER(buffer_for_expected_output);
			string_to_buffer(expected_output, &buffer_for_expected_output);
			ASSERT_TRUE(buffer_append_buffer(&expected_elements, &buffer_for_expected_output, 1)) <<
					buffer_free(&elements) << buffer_free_with_inner_buffers(&expected_elements);
		}

		const uint16_t expected_return = (uint16_t)int_parse(
											 node.node().select_node("return").node().child_value());
		//
		const range input_in_range = string_to_range(input);
		//
		ASSERT_TRUE(buffer_resize(&elements, 0)) <<
				buffer_free(&elements) << buffer_free_with_inner_buffers(&expected_elements);
		const uint16_t returned = xml_get_sub_nodes_elements(input_in_range.start, input_in_range.finish, &elements);
		//
		ASSERT_EQ(expected_return, returned) <<
											 buffer_free(&elements) << buffer_free_with_inner_buffers(&expected_elements);
		//
		ptrdiff_t i = 0;
		range* element = NULL;
		buffer* expected_element = NULL;

		while (NULL != (expected_element = buffer_buffer_data(&expected_elements, i)))
		{
			element = buffer_range_data(&elements, i);
			ASSERT_NE(nullptr, element) <<
										buffer_free(&elements) << buffer_free_with_inner_buffers(&expected_elements);
			++i;
			ASSERT_EQ(buffer_to_string(expected_element), range_to_string(element->start, element->finish)) <<
					buffer_free(&elements) << buffer_free_with_inner_buffers(&expected_elements);
		}

		element = buffer_range_data(&elements, i);
		ASSERT_EQ(nullptr, element) <<
									buffer_free(&elements) << buffer_free_with_inner_buffers(&expected_elements);
		//
		--node_count;
	}

	buffer_release(&elements);
	buffer_release_with_inner_buffers(&expected_elements);
}

TEST_F(TestXml, xml_get_tag_name)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const char* start = input.empty() ? NULL : input.data() + 1; //"NOTE: '+ 1' provided for skip '<' symbol."
		const char* finish = input.empty() ? NULL : input.data() + input.size();
		//
		range name;
		name.start = name.finish = NULL;
		const uint8_t returned = xml_get_tag_name(start, finish, &name);
		//
		ASSERT_EQ(expected_return, returned) << "'" << input << "'" << std::endl;
		ASSERT_EQ(returned, !range_is_null_or_empty(&name)) << "'" << input << "'" << std::endl;
		const std::string output(range_to_string(name));
		ASSERT_EQ(expected_output, output) << "'" << input << "'" << std::endl;
		//
		--node_count;
	}
}

TEST_F(TestXml, xml_get_attribute_value)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string attribute(node.node().select_node("attribute").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const range input_in_range = string_to_range(input);
		//
		range value;
		value.start = value.finish = NULL;
		const uint8_t returned = xml_get_attribute_value(input_in_range.start, input_in_range.finish,
								 attribute.empty() ? NULL : attribute.data(), attribute.size(), &value);
		//
		ASSERT_EQ(expected_return, returned) << "input - '" << input << "'" << std::endl <<
											 "attribute - '" << attribute << "'" << std::endl;
		ASSERT_EQ(returned, !range_is_null_or_empty(&value)) << "input - '" << input << "'" << std::endl <<
				"attribute - '" << attribute << "'" << std::endl;
		ASSERT_EQ(returned, xml_is_attribute_exists(input_in_range.start, input_in_range.finish,
				  attribute.empty() ? NULL : attribute.data(), attribute.size()));
		const std::string output(range_to_string(value));
		ASSERT_EQ(expected_output, output) << "input - '" << input << "'" << std::endl <<
										   "attribute - '" << attribute << "'" << std::endl;
		//
		--node_count;
	}
}

TEST_F(TestXml, xml_get_element_value)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		const range element = string_to_range(input);
		//
		range value;
		value.start = value.finish = NULL;
		const uint8_t returned = xml_get_element_value(&element, &value);
		//
		ASSERT_EQ(expected_return, returned) << input;
		const std::string output(range_to_string(value));
		ASSERT_EQ(expected_output, output) << input;
		//
		--node_count;
	}
}
