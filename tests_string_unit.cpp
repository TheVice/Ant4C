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
#include "range.h"
#include "string_unit.h"
};

class TestStringUnit : public TestsBaseXml
{
};

void null_range_to_empty(range& input)
{
	if (NULL == input.start ||
		NULL == input.finish)
	{
		input.start = input.finish = (const uint8_t*)&input;
	}
}

/*string_contains
string_ends_with
string_get_length*/
TEST_F(TestStringUnit, string_index_of_any)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string value(node.node().select_node("value").node().child_value());
		const int8_t step = (int8_t)INT_PARSE(node.node().select_node("step").node().child_value());
		const ptrdiff_t expected_return = (ptrdiff_t)INT_PARSE(
											  node.node().select_node("return").node().child_value());
		//
		const range input_in_range = string_to_range(input);
		const range value_in_range = string_to_range(value);
		//
		const ptrdiff_t returned = 0 < step ?
								   string_index_of(input_in_range.start, input_in_range.finish, value_in_range.start, value_in_range.finish) :
								   string_last_index_of(input_in_range.start, input_in_range.finish, value_in_range.start,
										   value_in_range.finish);
		//
		ASSERT_EQ(expected_return, returned)
				<< "'" << input << "'" << std::endl
				<< "'" << value << "'" << std::endl
				<< "'" << step << "'" << std::endl;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_replace)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string to_be_replaced(node.node().select_node("to_be_replaced").node().child_value());
		const std::string by_replacement(node.node().select_node("by_replacement").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(
											node.node().select_node("return").node().child_value());
		//
		const range input_in_range = string_to_range(input);
		const range to_be_replaced_in_range = string_to_range(to_be_replaced);
		const range by_replacement_in_range = string_to_range(by_replacement);
		//
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		uint8_t returned = string_replace(
							   input_in_range.start, input_in_range.finish,
							   to_be_replaced_in_range.start, to_be_replaced_in_range.finish,
							   by_replacement_in_range.start, by_replacement_in_range.finish, &output);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&output);
		ASSERT_EQ(expected_output, buffer_to_string(&output)) << buffer_free(&output);
#if 0
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(string_to_buffer(input, &output)) << buffer_free(&output);
		//
		returned = string_replace_in_buffer(&output, to_be_replaced_in_range.start, to_be_replaced.size(),
											by_replacement_in_range.start, by_replacement.size());
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&output);
		ASSERT_EQ(expected_output, buffer_to_string(&output)) << buffer_free(&output);
#endif
		--node_count;
	}

	buffer_release(&output);
}

TEST_F(TestStringUnit, string_starts_with)
{
	for (const auto& node : nodes)
	{
		ASSERT_FALSE(node.node().attribute("input").empty());
		ASSERT_FALSE(node.node().attribute("value").empty());
		ASSERT_FALSE(node.node().attribute("return").empty());
		//
		const std::string input(node.node().attribute("input").value());
		const std::string value(node.node().attribute("value").value());
		const uint8_t expected_return = node.node().attribute("return").as_bool();
		//
		const range input_in_range = string_to_range(input);
		const range value_in_range = string_to_range(value);
		//
		const uint8_t returned_value = string_starts_with(
										   input_in_range.start, input_in_range.finish,
										   value_in_range.start, value_in_range.finish);
		ASSERT_EQ(expected_return, returned_value) << input << " " << value << std::endl;
		//
		--node_count;
	}
}
/*string_substring
string_to_lower
string_to_upper*/
TEST_F(TestStringUnit, string_trim)
{
	for (const auto& node : nodes)
	{
		const uint8_t mode = (uint8_t)INT_PARSE(node.node().select_node("mode").node().child_value());
		ASSERT_LE(12, mode);
		ASSERT_GE(14, mode);
		const char* input = node.node().select_node("input").node().child_value();
		ptrdiff_t input_length = INT_PARSE(node.node().select_node("input_length").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		range input_in_range;
		input_in_range.start = (const uint8_t*)input;
		input_in_range.finish = input_in_range.start + input_length;
		uint8_t returned = 0;

		switch (mode)
		{
			case 12:
				returned = string_trim(&input_in_range);
				break;

			case 13:
				returned = string_trim_end(&input_in_range);
				break;

			case 14:
				returned = string_trim_start(&input_in_range);
				break;

			default:
				break;
		}

		ASSERT_EQ(expected_return, returned) << "'" << input << "'" << std::endl << mode << std::endl;
		ASSERT_EQ(expected_output, range_to_string(input_in_range)) << "'" << input << "'" << std::endl << mode <<
				std::endl;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_quote)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const range input_in_range(string_to_range(input));
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		const uint8_t returned = string_quote(input_in_range.start, input_in_range.finish, &output);
		//
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&output);
		ASSERT_EQ(expected_output, buffer_to_string(&output)) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}

TEST_F(TestStringUnit, string_un_quote)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		range input_in_range(string_to_range(input));
		null_range_to_empty(input_in_range);
		const uint8_t returned = string_un_quote(&input_in_range);
		//
		ASSERT_EQ(expected_return, returned) << input;
		ASSERT_EQ(expected_output, range_to_string(input_in_range)) << input;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_equal)
{
	for (const auto& node : nodes)
	{
		const std::string input_1(node.node().select_node("input_1").node().child_value());
		const std::string input_2(node.node().select_node("input_2").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		range input_1_in_range(string_to_range(input_1));
		range input_2_in_range(string_to_range(input_2));
		//
		null_range_to_empty(input_1_in_range);
		null_range_to_empty(input_2_in_range);
		//
		const uint8_t returned = string_equal(input_1_in_range.start, input_1_in_range.finish,
											  input_2_in_range.start, input_2_in_range.finish);
		//
		ASSERT_EQ(expected_return, returned) << input_1 << std::endl << input_2;
		//
		--node_count;
	}
}
