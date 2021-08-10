/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"
};

#include <cfloat>
#include <string>
#include <climits>
#include <cstring>

class TestConversion : public TestsBaseXml
{
};

TEST_F(TestConversion, bool_parse)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const auto expected_output = static_cast<uint8_t>(INT_PARSE(
										 node.node().select_node("output").node().child_value()));
		const auto expected_return = static_cast<uint8_t>(INT_PARSE(
										 node.node().select_node("return").node().child_value()));
		//
		const auto input_in_range(string_to_range(input));
		uint8_t output = 0;
		//
		const auto returned = bool_parse(input_in_range.start, range_size(&input_in_range), &output);
		//
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_output, output);
		//
		--node_count;
	}
}

TEST_F(TestConversion, bool_to_string)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const auto input = static_cast<uint8_t>(INT_PARSE(node.node().select_node("input").node().child_value()));
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const auto expected_return = static_cast<uint8_t>(INT_PARSE(
										 node.node().select_node("return").node().child_value()));
		//
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		const auto returned = bool_to_string(input, &output);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&output);
		ASSERT_EQ(expected_output, buffer_to_string(&output)) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}

TEST(TestConversion_, double_parse)
{
	const uint8_t* input[] =
	{
		reinterpret_cast<const uint8_t*>("-1"),
		reinterpret_cast<const uint8_t*>("0"),
		reinterpret_cast<const uint8_t*>("1"),
		reinterpret_cast<const uint8_t*>("2.2250738585072014e-308"),
		reinterpret_cast<const uint8_t*>("1.6976931348623157e+308"),
		reinterpret_cast<const uint8_t*>("-1.6976931348623157e+308"),
		reinterpret_cast<const uint8_t*>("3.1415926535897931"),
		reinterpret_cast<const uint8_t*>("2.7182818284590451")
	};
	//
	const double expected_output[] = { -1, 0, 1, DBL_MIN, 1.6976931348623157e+308, -1.6976931348623157e+308, 3.1415926535897931, 2.7182818284590451 };

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
		const double epsilon = (1.0e+308 < expected_output[i] ||
								expected_output[i] < -1.0e+308) ? 0.0000000000001158e+308 : 0.00000001;
		const auto returned = double_parse(input[i]);
		ASSERT_NEAR(expected_output[i], returned, epsilon) << input[i];
	}
}

TEST(TestConversion_, double_to_string)
{
	const double input[] = { -1, 0, 1, DBL_MIN, DBL_MAX, -DBL_MAX,
							 3.1415926535897931, 2.7182818284590451
						   };
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(double_to_string(input[i], &output)) << buffer_free(&output);
		ASSERT_LT(0, buffer_size(&output)) << buffer_free(&output);
	}

	buffer_release(&output);
}

TEST(TestConversion_, int_parse)
{
	const uint8_t* input[] =
	{
		reinterpret_cast<const uint8_t*>("-1"),
		reinterpret_cast<const uint8_t*>("0"),
		reinterpret_cast<const uint8_t*>("1"),
		reinterpret_cast<const uint8_t*>("2147483647"),
		reinterpret_cast<const uint8_t*>("-2147483648")
	};
	//
	const int32_t expected_output[] = { -1, 0, 1, INT32_MAX, INT32_MIN };

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
		ASSERT_EQ(expected_output[i], int_parse(input[i]));
	}
}

TEST(TestConversion_, int_to_string)
{
	const int32_t input[] = { -1, 0, 1, INT32_MAX, INT32_MIN };
	const char* expected_output[] = { "-1", "0", "1", "2147483647", "-2147483648" };
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(int_to_string(input[i], &output)) << buffer_free(&output);
		ASSERT_STREQ(expected_output[i], buffer_to_string(&output).c_str()) << buffer_free(&output);
	}

	buffer_release(&output);
}

TEST(TestConversion_, long_parse)
{
	const uint8_t* input[] =
	{
		reinterpret_cast<const uint8_t*>("-1"),
		reinterpret_cast<const uint8_t*>("0"),
		reinterpret_cast<const uint8_t*>("1"),
		reinterpret_cast<const uint8_t*>("9223372036854775807L"),
		reinterpret_cast<const uint8_t*>("-9223372036854775808L")
	};
#if !defined(_WIN32)
	const long expected_output[] = { -1, 0, 1, LONG_MAX, LONG_MIN };
#else
	const int64_t expected_output[] = { -1, 0, 1, INT64_MAX, INT64_MIN };
#endif

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
#if !defined(_WIN32)
		ASSERT_EQ(expected_output[i], long_parse(input[i]));
#endif
		ASSERT_EQ(expected_output[i], int64_parse(input[i], input[i] + common_count_bytes_until(input[i], 0)));
	}
}

TEST(TestConversion_, long_to_string)
{
#if !defined(_WIN32)
	const long input[] = { -1, 0, 1, LONG_MAX, LONG_MIN };
#else
	const int64_t input[] = { -1, 0, 1, INT64_MAX, INT64_MIN };
#endif
	const char* expected_output[] = { "-1", "0", "1", "9223372036854775807", "-9223372036854775808" };
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
#if !defined(_WIN32)
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(long_to_string(input[i], &output)) << buffer_free(&output);
		ASSERT_STREQ(expected_output[i], buffer_to_string(&output).c_str()) << buffer_free(&output);
#endif
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(int64_to_string(input[i], &output)) << buffer_free(&output);
		ASSERT_STREQ(expected_output[i], buffer_to_string(&output).c_str()) << buffer_free(&output);
	}

	buffer_release(&output);
}

TEST(TestConversion_, uint64_parse)
{
	const uint8_t* input[] =
	{
		reinterpret_cast<const uint8_t*>("0"),
		reinterpret_cast<const uint8_t*>(" 1"),
		reinterpret_cast<const uint8_t*>("  9223372036854775807"),
		reinterpret_cast<const uint8_t*>("  18446744073709551615   "),
		reinterpret_cast<const uint8_t*>("  18446744073709551616  "),
		reinterpret_cast<const uint8_t*>("00000000000000000018446744073709551614   "),
		reinterpret_cast<const uint8_t*>("100000000000000000000"),
	};
	//
	const uint64_t expected_output[] =
	{
		0, 1, INT64_MAX, UINT64_MAX,
		UINT64_MAX, UINT64_MAX - 1, UINT64_MAX
	};

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
		range input_in_range;
		input_in_range.start = input[i];
		input_in_range.finish = input[i] + common_count_bytes_until(input[i], 0);
		//
		const auto returned = uint64_parse(input_in_range.start, input_in_range.finish);
		//
		ASSERT_EQ(expected_output[i], returned) << input[i];
	}
}

TEST(TestConversion_, uint64_to_string)
{
	const uint64_t input[] =
	{
		0, INT8_MAX, UINT8_MAX, INT16_MAX, UINT16_MAX,
		INT32_MAX, UINT32_MAX, INT64_MAX, UINT64_MAX,
		UINT64_MAX - 1, UINT64_MAX - 2, UINT64_MAX - 3, UINT64_MAX - 4
	};
	//
	const char* expected_output[] =
	{
		"0", "127", "255", "32767", "65535",
		"2147483647", "4294967295", "9223372036854775807", "18446744073709551615",
		"18446744073709551614", "18446744073709551613", "18446744073709551612", "18446744073709551611"
	};
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(uint64_to_string(input[i], &output)) << buffer_free(&output);
		ASSERT_STREQ(expected_output[i], buffer_to_string(&output).c_str()) << buffer_free(&output);
	}

	buffer_release(&output);
}

TEST(TestConversion_, pointer_to_string_and_parse)
{
	buffer value;
	SET_NULL_TO_BUFFER(value);
	//
	auto pointer_to_buffer = &value;
	ASSERT_TRUE(pointer_to_string(pointer_to_buffer, pointer_to_buffer)) << buffer_free(pointer_to_buffer);
	//
	const auto returned_pointer = pointer_parse(buffer_data(pointer_to_buffer, 0));
	const auto returned_pointer_to_buffer = reinterpret_cast<const buffer*>(returned_pointer);
	//
	ASSERT_EQ(pointer_to_buffer, returned_pointer_to_buffer) << buffer_free(pointer_to_buffer);
	//
	buffer_release(pointer_to_buffer);
}
