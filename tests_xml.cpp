/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "conversion.h"
#include "range.h"
#include "xml.h"
};

#include <list>
#include <string>
#include <cstddef>
#include <cstdint>
#include <ostream>

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
		const auto input_in_range = string_to_range(input);
		const auto finish = xml_get_tag_finish_pos(input_in_range.start, input_in_range.finish);
		//
		ASSERT_LE(finish, input_in_range.finish) << input;

		if (NULL == finish)
		{
			ASSERT_TRUE(expected_output.empty()) << input;
		}
		else
		{
			const std::string output(range_to_string(finish, input_in_range.finish));
			ASSERT_EQ(expected_output, output) << input;
		}

		--node_count;
	}
}

TEST_F(TestXml, xml_get_sub_nodes_elements)
{
	buffer elements;
	SET_NULL_TO_BUFFER(elements);
	//
	buffer sub_nodes_names;
	SET_NULL_TO_BUFFER(sub_nodes_names);
	//
	buffer expected_elements;
	SET_NULL_TO_BUFFER(expected_elements);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		//
		std::string sub_nodes;
		std::list<std::size_t> positions;

		for (const auto& sub_node : node.node().select_nodes("sub_node"))
		{
			sub_nodes.append(sub_node.node().child_value());
			positions.push_back(sub_nodes.size());
		}

		ASSERT_TRUE(buffer_resize(&sub_nodes_names, 0)) <<
				buffer_free(&elements) <<
				buffer_free(&sub_nodes_names) <<
				buffer_free_with_inner_buffers(&expected_elements);

		if (!sub_nodes.empty())
		{
			auto sub_node = string_to_range(sub_nodes);

			for (const auto& position : positions)
			{
				sub_node.finish = (const uint8_t*)sub_nodes.data();
				sub_node.finish += position;
				//
				ASSERT_TRUE(buffer_append_range(&sub_nodes_names, &sub_node, 1)) <<
						buffer_free(&elements) <<
						buffer_free(&sub_nodes_names) <<
						buffer_free_with_inner_buffers(&expected_elements);
				//
				sub_node.start = sub_node.finish;
			}
		}

		ASSERT_TRUE(buffer_resize_and_free_inner_buffers(&expected_elements)) <<
				buffer_free(&elements) <<
				buffer_free(&sub_nodes_names) <<
				buffer_free_with_inner_buffers(&expected_elements);
		ptrdiff_t i = 0;

		for (const auto& output : node.node().select_nodes("output"))
		{
			i = buffer_size(&expected_elements);
			//
			ASSERT_TRUE(buffer_append_buffer(&expected_elements, NULL, 1)) <<
					buffer_free(&elements) <<
					buffer_free(&sub_nodes_names) <<
					buffer_free_with_inner_buffers(&expected_elements);
			//
			auto buffer_for_expected_output = reinterpret_cast<buffer*>(buffer_data(&expected_elements, i));
			//
			ASSERT_TRUE(buffer_resize(&expected_elements, i)) <<
					buffer_free(&elements) <<
					buffer_free(&sub_nodes_names) <<
					buffer_free_with_inner_buffers(&expected_elements);
			//
			ASSERT_NE(nullptr, buffer_for_expected_output) <<
					buffer_free(&elements) <<
					buffer_free(&sub_nodes_names) <<
					buffer_free_with_inner_buffers(&expected_elements);
			SET_NULL_TO_BUFFER(*buffer_for_expected_output);
			//
			ASSERT_TRUE(buffer_resize(&expected_elements, i + sizeof(buffer))) <<
					buffer_free(&elements) <<
					buffer_free(&sub_nodes_names) <<
					buffer_free_with_inner_buffers(&expected_elements);
			//
			const std::string expected_output(output.node().child_value());
			ASSERT_TRUE(string_to_buffer(expected_output, buffer_for_expected_output)) <<
					buffer_free(&elements) <<
					buffer_free(&sub_nodes_names) <<
					buffer_free_with_inner_buffers(&expected_elements);
		}

		const uint16_t expected_return = (uint16_t)INT_PARSE(
											 node.node().select_node("return").node().child_value());
		//
		const range input_in_range = string_to_range(input);
		//
		ASSERT_TRUE(buffer_resize(&elements, 0)) <<
				buffer_free(&elements) <<
				buffer_free(&sub_nodes_names) <<
				buffer_free_with_inner_buffers(&expected_elements);
		//
		const uint16_t returned = xml_get_sub_nodes_elements(
									  input_in_range.start, input_in_range.finish, &sub_nodes_names, &elements);
		//
		ASSERT_EQ(expected_return, returned) <<
											 buffer_free(&elements) <<
											 buffer_free(&sub_nodes_names) <<
											 buffer_free_with_inner_buffers(&expected_elements);
		//
		i = 0;
		range* element = NULL;
		buffer* expected_element = NULL;

		while (NULL != (expected_element = buffer_buffer_data(&expected_elements, i)))
		{
			element = buffer_range_data(&elements, i);
			ASSERT_NE(nullptr, element) <<
										buffer_free(&elements) <<
										buffer_free(&sub_nodes_names) <<
										buffer_free_with_inner_buffers(&expected_elements);
			++i;
			ASSERT_EQ(buffer_to_string(expected_element), range_to_string(element->start, element->finish)) <<
					buffer_free(&elements) <<
					buffer_free(&sub_nodes_names) <<
					buffer_free_with_inner_buffers(&expected_elements);
		}

		element = buffer_range_data(&elements, i);
		ASSERT_EQ(nullptr, element) <<
									buffer_free(&elements) <<
									buffer_free(&sub_nodes_names) <<
									buffer_free_with_inner_buffers(&expected_elements);
		//
		--node_count;
	}

	buffer_release(&elements);
	buffer_release(&sub_nodes_names);
	buffer_release_with_inner_buffers(&expected_elements);
}

TEST_F(TestXml, xml_get_tag_name)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		//
		auto input_in_range = string_to_range(input);
		input_in_range.start = input.empty() ? NULL : input_in_range.start +
							   1; //"NOTE: '+ 1' provided for skip '<' symbol."
		//
		range name;
		name.start = name.finish = NULL;
		const auto returned = xml_get_tag_name(input_in_range.start, input_in_range.finish, &name);
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
	buffer value;
	SET_NULL_TO_BUFFER(value);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string attribute(node.node().select_node("attribute").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		//
		ASSERT_TRUE(buffer_resize(&value, 0)) << buffer_free(&value);
		//
		const auto input_in_range = string_to_range(input);
		const auto returned = xml_get_attribute_value(input_in_range.start, input_in_range.finish,
							  attribute.empty() ? NULL : (const uint8_t*)attribute.data(), attribute.size(), &value);
		//
		ASSERT_EQ(expected_return, returned)
				<< "input - '" << input << "'" << std::endl
				<< "attribute - '" << attribute << "'" << std::endl
				<< buffer_free(&value);
		ASSERT_EQ(returned, xml_get_attribute_value(input_in_range.start, input_in_range.finish,
				  attribute.empty() ? NULL : (const uint8_t*)attribute.data(), attribute.size(), NULL))
				<< "input - '" << input << "'" << std::endl
				<< "attribute - '" << attribute << "'" << std::endl
				<< buffer_free(&value);
		const std::string output(buffer_to_string(&value));
		ASSERT_EQ(expected_output, output)
				<< "input - '" << input << "'" << std::endl
				<< "attribute - '" << attribute << "'" << std::endl
				<< buffer_free(&value);
		//
		--node_count;
	}

	buffer_release(&value);
}

TEST_F(TestXml, xml_get_element_value)
{
	buffer value;
	SET_NULL_TO_BUFFER(value);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		const auto element = string_to_range(input);
		//
		ASSERT_TRUE(buffer_resize(&value, 0)) << buffer_free(&value);
		const auto returned = xml_get_element_value(element.start, element.finish, &value);
		//
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&value);
		const std::string output(buffer_to_string(&value));
		ASSERT_EQ(expected_output, output) << input << std::endl << buffer_free(&value);
		//
		--node_count;
	}

	buffer_release(&value);
}

TEST(TestXml_, xml_get_element_value)
{
	std::string input = "><![CDATA[\n";
	input += "<project>\n";
	input += "  <echo file=\"${file}\" append=\"${append}\" message=\"${math::truncate(math::addition(a, b))} \" />\n";
	input += "</project>\n";
	input += "  ]]>";
	//
	std::string expected_output = "\n";
	expected_output += "<project>\n";
	expected_output +=
		"  <echo file=\"${file}\" append=\"${append}\" message=\"${math::truncate(math::addition(a, b))} \" />\n";
	expected_output += "</project>\n";
	expected_output += "  ";
	//
	const auto element = string_to_range(input);
	//
	buffer value;
	SET_NULL_TO_BUFFER(value);
	//
	ASSERT_TRUE(buffer_resize(&value, 0)) << buffer_free(&value);
	const auto returned = xml_get_element_value(element.start, element.finish, &value);
	//
	ASSERT_TRUE(returned) << input << std::endl << buffer_free(&value);
	const std::string output(buffer_to_string(&value));
	ASSERT_EQ(expected_output, output) << input << std::endl << buffer_free(&value);
	//
	buffer_release(&value);
}
