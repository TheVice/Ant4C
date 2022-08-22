/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"
};

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <iterator>

class TestStringUnit : public TestsBaseXml
{
};

TEST(TestCommon, common_get_string_at)
{
	range input;
	input.start =
		reinterpret_cast<const uint8_t*>("a\0bc\0def\0\0f\0ed\0cba\0\0");
	input.finish = input.start + 20;
	//
	std::vector<std::string> expected_outputs;
	expected_outputs.push_back("a");
	expected_outputs.push_back("bc");
	expected_outputs.push_back("def");
	expected_outputs.push_back("");
	expected_outputs.push_back("");
	//
	expected_outputs.push_back("f");
	expected_outputs.push_back("ed");
	expected_outputs.push_back("cba");
	expected_outputs.push_back("");
	expected_outputs.push_back("");
	//
	expected_outputs.push_back("");
	expected_outputs.push_back("");
	expected_outputs.push_back("");
	expected_outputs.push_back("");
	expected_outputs.push_back("");
	//
	auto expected_output = expected_outputs.begin();
	const auto finish = expected_outputs.end();

	for (int x = 0; x < 3; ++x)
	{
		for (int y = 0; y < 5; ++y)
		{
			const auto ptr =
				common_get_string_at(input.start, input.finish, x, y);
			std::string output;

			if (ptr)
			{
				output.append(reinterpret_cast<const char*>(ptr));
			}

			ASSERT_NE(finish, expected_output)
					<< x << " " << y << std::endl;
			ASSERT_EQ(*expected_output, output)
					<< x << " " << y << std::endl;
			++expected_output;
		}
	}

	ASSERT_EQ(finish, expected_output)
			<< std::distance(expected_output, finish);
}
/*string_contains*/
TEST_F(TestStringUnit, string_ends_with)
{
	for (const auto& node : nodes)
	{
		ASSERT_FALSE(node.node().attribute("input").empty());
		ASSERT_FALSE(node.node().attribute("value").empty());
		ASSERT_FALSE(node.node().attribute("return").empty());
		//
		const std::string input(node.node().attribute("input").value());
		const std::string value(node.node().attribute("value").value());
		const uint8_t expected_return =
			node.node().attribute("return").as_bool();
		//
		const auto input_in_a_range(string_to_range(input));
		const auto value_in_a_range(string_to_range(value));
		//
		const auto returned_value = string_ends_with(
										input_in_a_range.start, input_in_a_range.finish,
										value_in_a_range.start, value_in_a_range.finish);
		ASSERT_EQ(expected_return, returned_value)
				<< input << " " << value << std::endl;
		//
		--node_count;
	}
}
/*string_enumerate*/
TEST_F(TestStringUnit, string_equal)
{
	for (const auto& node : nodes)
	{
		const std::string input_1(node.node().select_node("input_1").node().child_value());
		const std::string input_2(node.node().select_node("input_2").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_1_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_1_in_a_range.start, input_1_in_a_range.finish));
		//
		input_1_in_a_range = string_to_range(input_1);
		auto input_2_in_a_range(string_to_range(input_2));
		//
		null_range_to_empty(input_1_in_a_range);
		null_range_to_empty(input_2_in_a_range);
		//
		const auto returned = string_equal(
								  input_1_in_a_range.start, input_1_in_a_range.finish,
								  input_2_in_a_range.start, input_2_in_a_range.finish);
		//
		ASSERT_EQ(expected_return, returned) << input_1 << std::endl << input_2;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_find_any_symbol_like_or_not_like_that)
{
	for (const auto& node : nodes)
	{
		const std::string that(
			node.node().select_node("that").node().child_value());
		const auto that_in_a_range(string_to_range(that));
		//
		const std::string like_str(
			node.node().select_node("like").node().child_value());
		auto input_in_a_range = string_to_range(like_str);
		const auto like =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string step_str(
			node.node().select_node("step").node().child_value());
		input_in_a_range = string_to_range(step_str);
		const auto step =
			static_cast<int8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string input(
			node.node().select_node("input").node().child_value());
		input_in_a_range = string_to_range(input);
		//
		const std::string expected_output(
			node.node().select_node("output").node().child_value());
		//
		const auto returned_ = string_find_any_symbol_like_or_not_like_that(
								   0 < step ? input_in_a_range.start : input_in_a_range.finish,
								   0 < step ? input_in_a_range.finish : input_in_a_range.start,
								   that_in_a_range.start, that_in_a_range.finish,
								   like, step);
		//
		ASSERT_LE(returned_, input_in_a_range.finish) <<
				input << std::endl << that << std::endl <<
				like << std::endl << step;
		//
		const std::string returned(
			reinterpret_cast<const char*>(returned_),
			reinterpret_cast<const char*>(input_in_a_range.finish));
		//
		ASSERT_EQ(expected_output, returned)
				<< input << std::endl
				<< that << std::endl
				<< static_cast<int>(like) << std::endl
				<< static_cast<int>(step);
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_get_length)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<ptrdiff_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		const auto returned = string_get_length(input_in_a_range.start, input_in_a_range.finish);
		//
		ASSERT_EQ(expected_return, returned) << input;
		//
		--node_count;
	}
}
/*string_index_of*/
TEST_F(TestStringUnit, string_index_of_any)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string value(node.node().select_node("value").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<ptrdiff_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		const auto value_in_a_range(string_to_range(value));
		//
		const auto returned = string_index_of_any(
								  input_in_a_range.start, input_in_a_range.finish,
								  value_in_a_range.start, value_in_a_range.finish);
		//
		ASSERT_EQ(expected_return, returned)
				<< "'" << input << "'" << std::endl
				<< "'" << value << "'" << std::endl;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_index_of_value)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string value(node.node().select_node("value").node().child_value());
		//
		const std::string step_str(node.node().select_node("step").node().child_value());
		auto input_in_a_range = string_to_range(step_str);
		const auto step =
			static_cast<int8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<ptrdiff_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		const auto value_in_a_range(string_to_range(value));
		//
		const auto returned = 0 < step ?
							  string_index_of(input_in_a_range.start, input_in_a_range.finish,
											  value_in_a_range.start, value_in_a_range.finish) :
							  string_last_index_of(
								  input_in_a_range.start, input_in_a_range.finish,
								  value_in_a_range.start, value_in_a_range.finish);
		//
		ASSERT_EQ(expected_return, returned)
				<< "'" << input << "'" << std::endl
				<< "'" << value << "'" << std::endl
				<< "'" << static_cast<int>(step) << "'" << std::endl;
		//
		--node_count;
	}
}
/*string_last_index_of*/
TEST_F(TestStringUnit, string_last_index_of_any)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string value(node.node().select_node("value").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<ptrdiff_t>(int_parse(
									   input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		const auto value_in_a_range(string_to_range(value));
		//
		const auto returned = string_last_index_of_any(
								  input_in_a_range.start, input_in_a_range.finish,
								  value_in_a_range.start, value_in_a_range.finish);
		//
		ASSERT_EQ(expected_return, returned)
				<< "'" << input << "'" << std::endl
				<< "'" << value << "'" << std::endl;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_pad)
{
	static const std::string right("right");
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string value(node.node().select_node("value").node().child_value());
		//
		const std::string result_length_str(
			node.node().select_node("length").node().child_value());
		auto input_in_a_range = string_to_range(result_length_str);
		const auto result_length =
			static_cast<ptrdiff_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const uint8_t side = (right == std::string(
								  node.node().select_node("side").node().child_value()));
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		input_in_a_range = string_to_range(input);
		const auto value_in_a_range(string_to_range(value));
		uint8_t returned = 0;
		//
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);

		if (side)
		{
			returned = string_pad_right(
						   input_in_a_range.start, input_in_a_range.finish,
						   value_in_a_range.start, value_in_a_range.finish,
						   result_length, output);
		}
		else
		{
			returned = string_pad_left(
						   input_in_a_range.start, input_in_a_range.finish,
						   value_in_a_range.start, value_in_a_range.finish,
						   result_length, output);
		}

		ASSERT_EQ(expected_return, returned)
				<< input << std::endl << value << std::endl
				<< result_length << std::endl
				<< "side -> " << static_cast<int>(side) << std::endl
				<< buffer_free(output);
		ASSERT_EQ(expected_output, buffer_to_string(output))
				<< input << std::endl << value << std::endl
				<< result_length << std::endl
				<< "side -> " << static_cast<int>(side) << std::endl
				<< buffer_free(output);
		//
		--node_count;
	}

	buffer_release(output);
}

TEST_F(TestStringUnit, string_quote)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		//
		const auto returned = string_quote(input_in_a_range.start, input_in_a_range.finish, output);
		//
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(output);
		ASSERT_EQ(expected_output, buffer_to_string(output)) << buffer_free(output);
		//
		--node_count;
	}

	buffer_release(output);
}

TEST_F(TestStringUnit, string_replace)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string to_be_replaced(node.node().select_node("to_be_replaced").node().child_value());
		const std::string by_replacement(node.node().select_node("by_replacement").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		const auto to_be_replaced_in_a_range(string_to_range(to_be_replaced));
		const auto by_replacement_in_a_range(string_to_range(by_replacement));
		//
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		//
		auto returned = string_replace(
							input_in_a_range.start, input_in_a_range.finish,
							to_be_replaced_in_a_range.start, to_be_replaced_in_a_range.finish,
							by_replacement_in_a_range.start, by_replacement_in_a_range.finish, output);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(output);
		ASSERT_EQ(expected_output, buffer_to_string(output)) << buffer_free(output);
		//
		--node_count;
	}

	buffer_release(output);
}

TEST_F(TestStringUnit, string_replace_double_char_with_single)
{
	for (const auto& node : nodes)
	{
		const std::string input_str(node.node().select_node("input").node().child_value());
		const std::string to_be_replaced_str(node.node().select_node("to_be_replaced").node().child_value());
		const std::string return_str(node.node().select_node("return").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		auto tmp = input_str;
		uint8_t* input =
			tmp.empty() ? nullptr : reinterpret_cast<uint8_t*>(&tmp[0]);
		ptrdiff_t size = tmp.size();
		input_in_a_range = string_to_range(to_be_replaced_str);
		//
		const auto returned =
			string_replace_double_char_with_single(
				input, &size, input_in_a_range.start, input_in_a_range.finish);
		//
		ASSERT_EQ(expected_return, returned)
				<< input_str << std::endl << to_be_replaced_str;
		//
		const std::string output(
			input ? reinterpret_cast<const char*>(input) : "", input ? size : 0);
		//
		ASSERT_EQ(expected_output, output)
				<< input_str << std::endl << to_be_replaced_str;
		//
		--node_count;
	}
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
		const uint8_t expected_return =
			node.node().attribute("return").as_bool();
		//
		const auto input_in_a_range(string_to_range(input));
		const auto value_in_a_range(string_to_range(value));
		//
		const auto returned_value = string_starts_with(
										input_in_a_range.start, input_in_a_range.finish,
										value_in_a_range.start, value_in_a_range.finish);
		ASSERT_EQ(expected_return, returned_value)
				<< input << " " << value << std::endl;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_substring)
{
	for (const auto& node : nodes)
	{
		const std::string input(
			node.node().select_node("input").node().child_value());
		const std::string index_str(
			node.node().select_node("index").node().child_value());
		const std::string length_str(
			node.node().select_node("length").node().child_value());
		//
		const std::string expected_output(
			node.node().select_node("output").node().child_value());
		const std::string return_str(
			node.node().select_node("return").node().child_value());
		//
		const auto input_in_a_range(string_to_range(input));
		const auto index_in_a_range(string_to_range(index_str));
		const auto length_in_a_range(string_to_range(length_str));
		const auto return_in_a_range(string_to_range(return_str));
		//
		const auto index = static_cast<ptrdiff_t>(
							   int64_parse(index_in_a_range.start, index_in_a_range.finish));
		const auto length = static_cast<ptrdiff_t>(
								int64_parse(length_in_a_range.start, length_in_a_range.finish));
		const auto expected_return = static_cast<uint8_t>(
										 int_parse(return_in_a_range.start, return_in_a_range.finish));
		//
		auto output = string_to_range("");
		const auto returned = string_substring(
								  input_in_a_range.start,
								  input_in_a_range.finish,
								  index, length, &output);
		//
		ASSERT_EQ(expected_return, returned)
				<< input << std::endl << index_str << std::endl
				<< length_str << std::endl << expected_output << std::endl;
		//
		ASSERT_EQ(expected_output, range_to_string(output))
				<< input << std::endl << index_str << std::endl
				<< length_str << std::endl;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_transform_to_case)
{
	static const std::string lower("lower");
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const uint8_t required_case = (lower == std::string(
										   node.node().select_node("letter_case").node().child_value()));
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		input_in_a_range = string_to_range(input);
		uint8_t returned = 0;
		//
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);

		if (required_case)
		{
			returned = string_to_lower(
						   input_in_a_range.start, input_in_a_range.finish, output);
		}
		else
		{
			returned = string_to_upper(
						   input_in_a_range.start, input_in_a_range.finish, output);
		}

		ASSERT_EQ(expected_return, returned)
				<< input << std::endl
				<< "letter case -> " << static_cast<int>(required_case) << std::endl
				<< buffer_free(output);
		ASSERT_EQ(expected_output, buffer_to_string(output))
				<< input << std::endl
				<< "letter case -> " << static_cast<int>(required_case) << std::endl
				<< buffer_free(output);
		//
		--node_count;
	}

	buffer_release(output);
}

TEST_F(TestStringUnit, char_to_case)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(output, 16)) << buffer_free(output);

	for (const auto& node : nodes)
	{
		const auto input_node = node.node();
		//
		const auto input = input_node.attribute("input").as_uint();
		const auto expected_upper = input_node.attribute("upper").as_uint();
		const auto expected_lower = input_node.attribute("lower").as_uint();
		//
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		ASSERT_TRUE(text_encoding_encode_UTF8(&input, &input + 1, output)) << buffer_free(output);
		//
		const auto input_in_a_range(buffer_to_range(output));
		const auto size(range_size(&input_in_a_range));

		for (uint8_t i = 0; i < 2; ++i)
		{
			ASSERT_TRUE(buffer_resize(output, size)) << buffer_free(output);

			if (i)
			{
				ASSERT_TRUE(string_to_upper(input_in_a_range.start, input_in_a_range.finish,
											output)) << buffer_free(output);
			}
			else
			{
				ASSERT_TRUE(string_to_lower(input_in_a_range.start, input_in_a_range.finish,
											output)) << buffer_free(output);
			}

			auto output_in_a_range(buffer_to_range(output));
			output_in_a_range.start += size;
			//
			ASSERT_TRUE(text_encoding_decode_UTF8(
							output_in_a_range.start, output_in_a_range.finish, output)) << buffer_free(output);
			const auto output_size = buffer_size(output) - size - range_size(&output_in_a_range);
			ASSERT_EQ(static_cast<ptrdiff_t>(sizeof(uint32_t)), output_size) << buffer_free(output);
			//
			const auto returned_output = *(reinterpret_cast<const uint32_t*>(buffer_data(
											   output, size + range_size(&output_in_a_range))));
			ASSERT_EQ(i ? expected_upper : expected_lower, returned_output)
					<< (i ? "upper" : "lower") << std::endl
					<< input << std::endl << buffer_free(output);
		}

		--node_count;
	}

	buffer_release(output);
}

TEST_F(TestStringUnit, string_trim)
{
	static const std::string all("all");
	static const std::string end("end");
	static const std::string start("start");

	for (const auto& node : nodes)
	{
		const char* input = node.node().select_node("input").node().child_value();
		const std::string mode(node.node().select_node("mode").node().child_value());
		//
		const std::string input_length_str(node.node().select_node("input_length").node().child_value());
		auto input_in_a_range = string_to_range(input_length_str);
		const auto input_length =
			int_parse(input_in_a_range.start, input_in_a_range.finish);
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		input_in_a_range.start = reinterpret_cast<const uint8_t*>(input);
		input_in_a_range.finish = input_in_a_range.start + input_length;
		uint8_t returned = 0;

		if (all == mode)
		{
			returned = string_trim(&input_in_a_range);
		}
		else if (end == mode)
		{
			returned = string_trim_end(&input_in_a_range);
		}
		else if (start == mode)
		{
			returned = string_trim_start(&input_in_a_range);
		}

		ASSERT_EQ(expected_return, returned) << "'" << input << "'" << std::endl << mode << std::endl;
		ASSERT_EQ(expected_output, range_to_string(input_in_a_range)) << "'" << input << "'" << std::endl << mode <<
				std::endl;
		//
		--node_count;
	}
}

TEST_F(TestStringUnit, string_un_quote)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		null_range_to_empty(input_in_a_range);
		const auto returned = string_un_quote(&input_in_a_range);
		//
		ASSERT_EQ(expected_return, returned) << input;
		ASSERT_EQ(expected_output, range_to_string(input_in_a_range)) << input;
		//
		--node_count;
	}
}
