/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022, 2024 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"
#include "string_unit.h"
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
		auto input_in_a_range = string_to_range(input);

		if (!range_is_null_or_empty(&input_in_a_range))
		{
			static const uint8_t less = '<';
			input_in_a_range.start = string_find_any_symbol_like_or_not_like_that(
										 input_in_a_range.start, input_in_a_range.finish, &less, &less + 1, 1, 1);
			input_in_a_range.start = string_enumerate(input_in_a_range.start, input_in_a_range.finish, nullptr);
		}

		const auto finish = xml_get_tag_finish_pos(input_in_a_range.start, input_in_a_range.finish);
		ASSERT_LE(finish, input_in_a_range.finish) << input;

		if (nullptr == finish)
		{
			ASSERT_TRUE(expected_output.empty()) << input;
		}
		else
		{
			const auto output(range_to_string(finish, input_in_a_range.finish));
			ASSERT_EQ(expected_output, output) << input;
		}

		--node_count;
	}
}

TEST_F(TestXml, xml_get_sub_nodes_elements)
{
	std::string elements_buffer(buffer_size_of(), 0);
	auto elements = reinterpret_cast<void*>(&elements_buffer[0]);
	ASSERT_TRUE(buffer_init(elements, buffer_size_of()));
	//
	std::string expected_elements_buffer(buffer_size_of(), 0);
	auto expected_elements = reinterpret_cast<void*>(&expected_elements_buffer[0]);
	ASSERT_TRUE(buffer_init(expected_elements, buffer_size_of())) << buffer_free(elements);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string return_str(node.node().select_node("return").node().child_value());
		std::string sub_nodes;

		for (const auto& sub_node : node.node().select_nodes("sub_node"))
		{
			sub_nodes.append(sub_node.node().child_value());
			sub_nodes.push_back(0);
		}

		const auto sub_nodes_names(string_to_range(sub_nodes));
		ASSERT_TRUE(buffer_resize_and_free_inner_buffers(expected_elements)) <<
				buffer_free(elements) <<
				buffer_free_with_inner_buffers(expected_elements);

		for (const auto& output : node.node().select_nodes("output"))
		{
			const auto size = buffer_size(expected_elements);
			//
			ASSERT_TRUE(buffer_append_buffer(expected_elements, nullptr, 1)) <<
					buffer_free(elements) <<
					buffer_free_with_inner_buffers(expected_elements);
			//
			auto buffer_for_expected_output = buffer_data(expected_elements, size);
			//
			ASSERT_TRUE(buffer_resize(expected_elements, size)) <<
					buffer_free(elements) <<
					buffer_free_with_inner_buffers(expected_elements);
			//
			ASSERT_NE(nullptr, buffer_for_expected_output) <<
					buffer_free(elements) <<
					buffer_free_with_inner_buffers(expected_elements);
			ASSERT_TRUE(buffer_init(buffer_for_expected_output, buffer_size_of()));
			//
			ASSERT_TRUE(buffer_resize(expected_elements, size + buffer_size_of())) <<
					buffer_free(elements) <<
					buffer_free_with_inner_buffers(expected_elements);
			//
			const std::string expected_output(output.node().child_value());
			ASSERT_TRUE(string_to_buffer(expected_output, buffer_for_expected_output)) <<
					buffer_free(elements) <<
					buffer_free_with_inner_buffers(expected_elements);
		}

		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint16_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		//
		ASSERT_TRUE(buffer_resize(elements, 0)) <<
												buffer_free(elements) <<
												buffer_free_with_inner_buffers(expected_elements);
		//
		const auto returned = xml_get_sub_nodes_elements(
								  input_in_a_range.start, input_in_a_range.finish,
								  &sub_nodes_names, elements);
		//
		ASSERT_EQ(expected_return, returned) <<
											 input << std::endl <<
											 buffer_free(elements) <<
											 buffer_free_with_inner_buffers(expected_elements);

		if (!expected_return)
		{
			--node_count;
			continue;
		}

		ptrdiff_t i = 0;
		range* element = nullptr;
		void* expected_element = nullptr;

		while (nullptr != (expected_element = buffer_buffer_data(expected_elements, i)))
		{
			element = buffer_range_data(elements, i++);
			ASSERT_NE(nullptr, element) <<
										buffer_free(elements) <<
										buffer_free_with_inner_buffers(expected_elements);
			ASSERT_EQ(buffer_to_string(expected_element), range_to_string(element->start, element->finish)) <<
					buffer_free(elements) <<
					buffer_free_with_inner_buffers(expected_elements);
		}

		element = buffer_range_data(elements, i);
		ASSERT_EQ(nullptr, element) <<
									buffer_free(elements) <<
									buffer_free_with_inner_buffers(expected_elements);
		//
		--node_count;
	}

	buffer_release(elements);
	buffer_release_with_inner_buffers(expected_elements);
}

TEST_F(TestXml, xml_get_tag_name)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);

		if (!input.empty())
		{
			static const uint8_t less = '<';
			input_in_a_range.start = string_find_any_symbol_like_or_not_like_that(
										 input_in_a_range.start, input_in_a_range.finish, &less, &less + 1, 0, 1);
		}

		const auto tag_name_finish = xml_get_tag_name(input_in_a_range.start, input_in_a_range.finish);
		input_in_a_range.finish = tag_name_finish;
		ASSERT_EQ(expected_return, !range_in_parts_is_null_or_empty(input_in_a_range.start, input_in_a_range.finish))
				<< "'" << input << "'" << std::endl;
		const auto output(range_to_string(input_in_a_range));
		ASSERT_EQ(expected_output, output) << "'" << input << "'" << std::endl;
		//
		--node_count;
	}
}

TEST_F(TestXml, xml_get_attribute_value)
{
	std::string value_buffer(buffer_size_of(), 0);
	auto value = reinterpret_cast<void*>(&value_buffer[0]);
	ASSERT_TRUE(buffer_init(value, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string attribute(node.node().select_node("attribute").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		ASSERT_TRUE(buffer_resize(value, 0)) << buffer_free(value);
		//
		input_in_a_range = string_to_range(input);
		const auto returned = xml_get_attribute_value(input_in_a_range.start, input_in_a_range.finish,
							  attribute.empty() ?
							  nullptr : reinterpret_cast<const uint8_t*>(attribute.c_str()), attribute.size(), value);
		//
		ASSERT_EQ(expected_return, returned)
				<< "input - '" << input << "'" << std::endl
				<< "attribute - '" << attribute << "'" << std::endl
				<< buffer_free(value);
		ASSERT_EQ(returned, xml_get_attribute_value(input_in_a_range.start, input_in_a_range.finish,
				  attribute.empty() ?
				  nullptr : reinterpret_cast<const uint8_t*>(attribute.c_str()), attribute.size(), nullptr))
				<< "input - '" << input << "'" << std::endl
				<< "attribute - '" << attribute << "'" << std::endl
				<< buffer_free(value);
		const auto output(buffer_to_string(value));
		ASSERT_EQ(expected_output, output)
				<< "input - '" << input << "'" << std::endl
				<< "attribute - '" << attribute << "'" << std::endl
				<< buffer_free(value);
		//
		--node_count;
	}

	buffer_release(value);
}

TEST_F(TestXml, xml_get_element_value)
{
	std::string value_buffer(buffer_size_of(), 0);
	auto value = reinterpret_cast<void*>(&value_buffer[0]);
	ASSERT_TRUE(buffer_init(value, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto element = string_to_range(return_str);
		//
		const auto expected_return =
			static_cast<uint8_t>(int_parse(element.start, element.finish));
		element = string_to_range(input);
		//
		ASSERT_TRUE(buffer_resize(value, 0)) << buffer_free(value);
		const auto returned = xml_get_element_value(element.start, element.finish, value);
		//
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(value);
		const auto output(buffer_to_string(value));
		ASSERT_EQ(expected_output, output) << input << std::endl << buffer_free(value);
		//
		--node_count;
	}

	buffer_release(value);
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
	std::string value_buffer(buffer_size_of(), 0);
	auto value = reinterpret_cast<void*>(&value_buffer[0]);
	ASSERT_TRUE(buffer_init(value, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(value, 0)) << buffer_free(value);
	const auto returned = xml_get_element_value(element.start, element.finish, value);
	//
	ASSERT_TRUE(returned) << input << std::endl << buffer_free(value);
	const auto output(buffer_to_string(value));
	ASSERT_EQ(expected_output, output) << input << std::endl << buffer_free(value);
	//
	buffer_release(value);
}
