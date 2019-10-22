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
};

#include <cfloat>

class TestConversion : public TestsBaseXml
{
};

TEST_F(TestConversion, bool_parse)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const uint8_t expected_output = (uint8_t)int_parse(
											node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const range input_in_range(string_to_range(input));
		uint8_t output = 0;
		//
		const uint8_t returned = bool_parse(input_in_range.start, input_in_range.finish, &output);
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
		const uint8_t input = (uint8_t)int_parse(node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		const uint8_t returned = bool_to_string(input, &output);
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
	const char* input[] = { "-1", "0", "1", "2.2250738585072014e-308", "1.7976931348623157e+308",
							"-1.7976931348623157e+308", "3.1415926535897931", "2.7182818284590451"
						  };
	const double expected_output[] = { -1, 0, 1, DBL_MIN, DBL_MAX,
									   -DBL_MAX, 3.1415926535897931, 2.7182818284590451
									 };

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
	{
		const double returned = double_parse(input[i]);
		ASSERT_NEAR(expected_output[i], returned, 50 * DBL_EPSILON);
	}
}

TEST(TestConversion_, double_to_string)
{
	const double input[] = { -1, 0, 1, /*DBL_MIN,*/ DBL_MAX,
							 -DBL_MAX, 3.1415926535897931, 2.7182818284590451
						   };
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(double_to_string(input[i], &output)) << buffer_free(&output);
		ASSERT_LT(0, output.size) << buffer_free(&output);
		ASSERT_NEAR(input[i], double_parse(buffer_char_data(&output, 0)), 50 * DBL_EPSILON) << buffer_free(&output);
	}

	buffer_release(&output);
}

TEST(TestConversion_, int_parse)
{
	const char* input[] = { "-1", "0", "1", "2147483647", "-2147483648" };
	const int32_t expected_output[] = { -1, 0, 1, INT32_MAX, INT32_MIN };

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
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

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(int_to_string(input[i], &output)) << buffer_free(&output);
		ASSERT_STREQ(expected_output[i], buffer_to_string(&output).c_str()) << buffer_free(&output);
	}

	buffer_release(&output);
}

TEST(TestConversion_, long_parse)
{
	const std::string input[] = { "-1", "0", "1", "9223372036854775807L", "-9223372036854775808L" };
#if !defined(_WIN32)
	const long expected_output[] = { -1, 0, 1, LONG_MAX, LONG_MIN };
#else
	const int64_t expected_output[] = { -1, 0, 1, INT64_MAX, INT64_MIN };
#endif

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
	{
#if !defined(_WIN32)
		ASSERT_EQ(expected_output[i], long_parse(input[i].c_str()));
		std::wstring inputW;
		inputW.assign(input[i].cbegin(), input[i].cend());
		ASSERT_EQ(expected_output[i], long_parse_wchar_t(inputW.c_str()));
#endif
		ASSERT_EQ(expected_output[i], int64_parse(input[i].c_str()));
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

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
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
