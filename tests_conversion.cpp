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
};

#include <cfloat>
#include <string>
#include <climits>

class TestConversion : public TestsBaseXml
{
};

TEST_F(TestConversion, bool_parse)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const uint8_t expected_output = (uint8_t)int_parse(
											(uint8_t*)node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											(uint8_t*)node.node().select_node("return").node().child_value());
		//
		const range input_in_range(string_to_range(input));
		uint8_t output = 0;
		//
		const uint8_t returned = bool_parse(input_in_range.start, range_size(&input_in_range), &output);
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
		const uint8_t input = (uint8_t)int_parse((uint8_t*)node.node().select_node("input").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											(uint8_t*)node.node().select_node("return").node().child_value());
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
	const uint8_t* input[] =
	{
		(const uint8_t*)"-1",
		(const uint8_t*)"0",
		(const uint8_t*)"1",
		(const uint8_t*)"2.2250738585072014e-308",
		(const uint8_t*)"1.6976931348623157e+308",
		(const uint8_t*)"-1.6976931348623157e+308",
		(const uint8_t*)"3.1415926535897931",
		(const uint8_t*)"2.7182818284590451"
	};
	//
	const double expected_output[] = { -1, 0, 1, DBL_MIN, 1.6976931348623157e+308, -1.6976931348623157e+308, 3.1415926535897931, 2.7182818284590451 };

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
	{
		const double epsilon = (1.0e+308 < expected_output[i] ||
								expected_output[i] < -1.0e+308) ? 0.0000000000001158e+308 : 0.00000001;
		const double returned = double_parse(input[i]);
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

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
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
		(const uint8_t*)"-1",
		(const uint8_t*)"0",
		(const uint8_t*)"1",
		(const uint8_t*)"2147483647",
		(const uint8_t*)"-2147483648"
	};
	//
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
	const uint8_t* input[] =
	{
		(const uint8_t*)"-1",
		(const uint8_t*)"0",
		(const uint8_t*)"1",
		(const uint8_t*)"9223372036854775807L",
		(const uint8_t*)"-9223372036854775808L"
	};
#if !defined(_WIN32)
	const long expected_output[] = { -1, 0, 1, LONG_MAX, LONG_MIN };
#else
	const int64_t expected_output[] = { -1, 0, 1, INT64_MAX, INT64_MIN };
#endif

	for (uint8_t i = 0,
		 count = sizeof(input) / sizeof(input[0]); i < count; ++i)
	{
#if !defined(_WIN32)
		ASSERT_EQ(expected_output[i], long_parse(input[i]));
#endif
		ASSERT_EQ(expected_output[i], int64_parse(input[i]));
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

TEST(TestConversion_, pointer_to_string_and_parse)
{
	buffer value;
	SET_NULL_TO_BUFFER(value);
	//
	buffer* pointer_to_buffer = &value;
	ASSERT_TRUE(pointer_to_string(pointer_to_buffer, pointer_to_buffer)) << buffer_free(pointer_to_buffer);
	//
	const void* returned_pointer = pointer_parse(buffer_data(pointer_to_buffer, 0));
	const buffer* returned_pointer_to_buffer = static_cast<const buffer*>(returned_pointer);
	//
	ASSERT_EQ(pointer_to_buffer, returned_pointer_to_buffer) << buffer_free(pointer_to_buffer);
	//
	buffer_release(pointer_to_buffer);
}
