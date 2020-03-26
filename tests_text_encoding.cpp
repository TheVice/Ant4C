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
#include "text_encoding.h"
}

#include <limits>
#include <string>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <ostream>

class TestTextEncoding : public TestsBaseXml
{
};

TEST(TestTextEncoding_, text_encoding_get_BOM)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_TRUE(text_encoding_get_BOM(ASCII, &output)) << buffer_free(&output);
	ASSERT_FALSE(text_encoding_get_BOM(UTF7, &output)) << buffer_free(&output); //TODO:
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF8, &output)) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	ASSERT_TRUE(text_encoding_get_BOM(BigEndianUnicode, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF16BE, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF16LE, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(Unicode, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF32BE, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF32, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF32LE, &output)) << buffer_free(&output);
	//
	buffer_release(&output);
}

TEST(TestTextEncoding_, text_encoding_get_one_of_data_by_BOM)
{
	const uint8_t data[][4] =
	{
		{ 0x1, 0x2, 0x3, 0x4 },
		{ 0xEF, 0xBB, 0xBF, 0x5 },
		{ 0xFE, 0xFF, 0x6, 0x7 },
		{ 0xFF, 0xFE, 0x8, 0x9 },
		{ 0x00, 0x00, 0xFE, 0xFF },
		{ 0xFF, 0xFE, 0x00, 0x00 }
	};
	//
	const uint8_t expected_encoding[] =
	{
		ASCII, UTF8,
		UTF16BE, UTF16LE,
		UTF32BE, UTF32LE
	};

	for (uint8_t i = 0, count = sizeof(data) / sizeof(*data); i < count; ++i)
	{
		const uint8_t returned_encoding = text_encoding_get_one_of_data_by_BOM(data[i], 4);
		ASSERT_EQ(expected_encoding[i], returned_encoding) << (int)i;
	}
}

TEST(TestTextEncoding_, text_encoding_UTF_to_ASCII)
{
	static const uint8_t encodings[] =
	{
		UTF8,
		UTF16BE,
		UTF16LE,
		UTF32BE,
		UTF32LE
	};
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	std::string input;
	std::string expected_output;

	for (uint8_t i = 0; i < UINT8_MAX; ++i)
	{
		if (INT8_MAX < i)
		{
			expected_output.push_back(63);
		}
		else
		{
			expected_output.push_back(i);
		}
	}

	for (const auto& encoding : encodings)
	{
		input.clear();

		switch (encoding)
		{
			case UTF8:
				for (uint8_t i = 0; i < UINT8_MAX; ++i)
				{
					input.push_back(i);
				}

				break;

			case UTF16BE:
				for (uint8_t i = 0; i < UINT8_MAX; ++i)
				{
					input.push_back(0);
					input.push_back(i);
				}

				break;

			case UTF16LE:
				for (uint8_t i = 0; i < UINT8_MAX; ++i)
				{
					input.push_back(i);
					input.push_back(0);
				}

				break;

			case UTF32BE:
				for (uint8_t i = 0; i < UINT8_MAX; ++i)
				{
					input.push_back(0);
					input.push_back(0);
					input.push_back(0);
					input.push_back(i);
				}

				break;

			case UTF32LE:
				for (uint8_t i = 0; i < UINT8_MAX; ++i)
				{
					input.push_back(i);
					input.push_back(0);
					input.push_back(0);
					input.push_back(0);
				}

				break;

			default:
				break;
		}

		const auto input_in_range(string_to_range(input));
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		ASSERT_TRUE(
			text_encoding_UTF_to_ASCII(
				input_in_range.start, input_in_range.finish, encoding, &output))
				<< buffer_free(&output);
		//
		ASSERT_EQ(expected_output, buffer_to_string(&output)) << (int)encoding << std::endl <<
				buffer_free(&output);
	}

	buffer_release(&output);
}

TEST(TestTextEncoding_, text_encoding_UTF_from_ASCII)
{
	buffer input;
	SET_NULL_TO_BUFFER(input);
	//
	ASSERT_TRUE(buffer_append(&input, NULL, UINT8_MAX + 1)) << buffer_free(&input);
	ASSERT_TRUE(buffer_resize(&input, 0)) << buffer_free(&input);

	for (uint16_t i = 0; ; ++i)
	{
		if (UINT8_MAX < i)
		{
			ASSERT_TRUE(buffer_push_back(&input, 0)) << buffer_free(&input);
			break;
		}

		ASSERT_TRUE(buffer_push_back(&input, (uint8_t)i)) << buffer_free(&input);
	}

	const range input_in_range = buffer_to_range(&input);
	const ptrdiff_t expected_size = (ptrdiff_t)(sizeof(uint16_t) * (UINT8_MAX + sizeof(uint16_t)));
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (uint8_t a = 0; a < 2; ++a)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output) << buffer_free(&input);

		if (a)
		{
			ASSERT_TRUE(
				text_encoding_UTF_from_ASCII(input_in_range.start,
											 input_in_range.finish,
											 UTF16BE,
											 &output))
					<< buffer_free(&output) << buffer_free(&input);
		}
		else
		{
			ASSERT_TRUE(
				text_encoding_UTF_from_ASCII(input_in_range.start,
											 input_in_range.finish,
											 UTF16LE,
											 &output))
					<< buffer_free(&output) << buffer_free(&input);
		}

		ASSERT_EQ(expected_size, buffer_size(&output)) << buffer_free(&output) << buffer_free(&input);
		const uint16_t* ptr = (const uint16_t*)buffer_data(&output, a);
		ASSERT_NE(nullptr, ptr) << buffer_free(&output) << buffer_free(&input);

		for (uint8_t i = 0; i < INT8_MAX + 1; ++i)
		{
			ASSERT_EQ(input_in_range.start[i], ptr[i])
					<< buffer_free(&output) << buffer_free(&input);
		}

		for (uint16_t i = INT8_MAX + 1; i < UINT8_MAX + 1; ++i)
		{
			ASSERT_EQ('?', ptr[i])
					<< buffer_free(&output) << buffer_free(&input);
		}

		/*ASSERT_EQ(0, ptr[UINT8_MAX + 1 // - a])
				<< buffer_free(&output) << buffer_free(&input);*/
	}

	buffer_release(&output);
	buffer_release(&input);
}

template<typename T>
void string_hex_parse(const char* input, T& output)
{
	if (NULL == input)
	{
		return;
	}

	const typename T::value_type max = std::numeric_limits<typename T::value_type>::max();
	//
	const auto size = output.size();
	output.reserve(size + strlen(input));

	while (NULL != (input = strstr(input, "0x")))
	{
		char* pos = NULL;
		const auto value = strtol(input, &pos, 16);
		input += pos - input;

		if (value < 0 || max < value)
		{
			break;
		}

		output.push_back((typename T::value_type)value);
	}
}

TEST_F(TestTextEncoding, text_encoding_UTF16LE_from_code_page)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		const auto input_hex(get_data_from_nodes(node, "input"));
		std::vector<uint8_t> input;
		string_hex_parse(input_hex.c_str(), input);
		//
		auto code_page = (uint16_t)INT_PARSE(
							 node.node().select_node("code_page").node().child_value());
		//
		const auto output_hex(get_data_from_nodes(node, "output"));
		std::vector<uint16_t> expected_output;
		string_hex_parse(output_hex.c_str(), expected_output);
		//
		ASSERT_TRUE(text_encoding_UTF16LE_from_code_page(
						input.data(),
						input.data() + input.size(),
						code_page,
						&output)) << buffer_free(&output);
		//
		code_page = 0;
		const uint16_t* ptr = NULL;

		for (const auto& expected_code : expected_output)
		{
			ptr = buffer_uint16_data(&output, code_page);
			ASSERT_NE(nullptr, ptr) << buffer_free(&output);
			//
			ASSERT_EQ(expected_code, (*ptr))
					<< code_page << std::endl
					<< buffer_free(&output);
			//
			++code_page;
		}

		ptr = buffer_uint16_data(&output, code_page);
		ASSERT_EQ(nullptr, ptr) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}
/*
text_encoding_encode_UTF8_single
text_encoding_decode_UTF8_single
*/
TEST(TestTextEncoding_, text_encoding_encode_UTF8)
{
	buffer input;
	SET_NULL_TO_BUFFER(input);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	buffer output_;
	SET_NULL_TO_BUFFER(output_);
	//
	ASSERT_TRUE(buffer_resize(&input, 4096))
			<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);
	ASSERT_TRUE(buffer_resize(&output, 4096))
			<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);
	ASSERT_TRUE(buffer_resize(&output_, 4096))
			<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);

	for (uint32_t i = 0; i < UINT16_MAX + 1;)
	{
		const uint32_t range_max = i + 1024;
		//
		ASSERT_TRUE(buffer_resize(&input, 0))
				<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);
		ASSERT_TRUE(buffer_resize(&output, 0))
				<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);
		ASSERT_TRUE(buffer_resize(&output_, 0))
				<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);

		while (i < range_max)
		{
			ASSERT_TRUE(buffer_push_back_uint32(&input, i++))
					<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);
		}

		const uint32_t* start = (const uint32_t*)buffer_data(&input, 0);
		const uint32_t* finish = start + buffer_size(&input) / sizeof(uint32_t);
		ASSERT_TRUE(text_encoding_encode_UTF8(start, finish, &output))
				<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);

		if (0xD7FF < i - 1024 && i - 1024 < 0xE000)
		{
			uint32_t* ptr = NULL;
			ptrdiff_t j = 0;

			while (NULL != (ptr = buffer_uint32_data(&input, j++)))
			{
				const uint32_t input_code = *ptr;

				if (0xD7FF < input_code && input_code < 0xE000)
				{
					*ptr = 0xFFFD;
				}
			}
		}

		const uint8_t* start_ = buffer_data(&output, 0);
		const uint8_t* finish_ = start_ + buffer_size(&output);
		ASSERT_TRUE(text_encoding_decode_UTF8(start_, finish_, &output_))
				<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);
		ASSERT_EQ(buffer_to_string(&input), buffer_to_string(&output_)) << range_max
				<< buffer_free(&input) << buffer_free(&output) << buffer_free(&output_);
	}

	buffer_release(&output_);
	buffer_release(&output);
	buffer_release(&input);
}

TEST(TestTextEncoding_, text_encoding_decode_UTF8)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
	ASSERT_TRUE(buffer_append(&output, NULL, 4 * sizeof(uint32_t))) << buffer_free(&output);

	for (uint8_t i = 0; i < 0x80; ++i)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		ASSERT_TRUE(text_encoding_decode_UTF8(&i, &i + 1, &output)) << buffer_free(&output);
		uint32_t* ptr = buffer_uint32_data(&output, 0);
		ASSERT_NE(nullptr, ptr) << buffer_free(&output);
		ASSERT_EQ(i, *ptr) << buffer_free(&output);
	}

	for (uint16_t i = 0x80; i < 0x800;)
	{
		for (uint8_t a = 0xC2; a < 0xE0; ++a)
		{
			for (uint8_t b = 0x80; b < 0xC0; ++b)
			{
				uint8_t code[2];
				code[0] = a;
				code[1] = b;
				ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
				ASSERT_TRUE(text_encoding_decode_UTF8(code, code + 2, &output)) << buffer_free(&output);
				uint32_t* ptr = buffer_uint32_data(&output, 0);
				ASSERT_NE(nullptr, ptr) << buffer_free(&output);
				ASSERT_EQ(i++, *ptr) << (int)a << " " << (int)b << std::endl << buffer_free(&output);
			}
		}
	}

	for (uint16_t i = 0x800; i < 0x1000;)
	{
		for (uint8_t a = 0xE0; a < 0xE1; ++a)
		{
			for (uint8_t b = 0xA0; b < 0xC0; ++b)
			{
				for (uint8_t c = 0x80; c < 0xC0; ++c)
				{
					uint8_t code[3];
					code[0] = a;
					code[1] = b;
					code[2] = c;
					//
					ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
					ASSERT_TRUE(text_encoding_decode_UTF8(code, code + 3, &output)) << buffer_free(&output);
					uint32_t* ptr = buffer_uint32_data(&output, 0);
					ASSERT_NE(nullptr, ptr) << buffer_free(&output);
					ASSERT_EQ(i++, *ptr) << (int)a << " " << (int)b << " " << (int)c << std::endl << buffer_free(&output);
				}
			}
		}
	}

	for (uint32_t i = 0x1000; i < 0x10000;)
	{
		for (uint8_t a = 0xE1; a < 0xF0; ++a)
		{
			for (uint8_t b = 0x80; b < 0xC0; ++b)
			{
				for (uint8_t c = 0x80; c < 0xC0; ++c)
				{
					uint8_t code[3];
					code[0] = a;
					code[1] = b;
					code[2] = c;
					//
					ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
					ASSERT_TRUE(text_encoding_decode_UTF8(code, code + 3, &output)) << buffer_free(&output);
					uint32_t* ptr = buffer_uint32_data(&output, 0);
					ASSERT_NE(nullptr, ptr) << buffer_free(&output);
					ASSERT_EQ(i++, *ptr) << (int)a << " " << (int)b << " " << (int)c << std::endl << buffer_free(&output);
				}
			}
		}
	}

#if 0

	for (uint32_t i = 0x10000; i < 0x2F900;)
	{
		for (uint8_t a = 0xF0; a < 0xF1; ++a)
		{
			for (uint8_t b = 0x90; b < 0xC0; ++b)
			{
				for (uint8_t c = 0x80; c < 0xC0; ++c)
				{
					for (uint8_t d = 0x80; d < 0xC0; ++d)
					{
						uint8_t code[3];
						code[0] = a;
						code[1] = b;
						code[2] = c;
						code[3] = d;
						//
						ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
						ASSERT_TRUE(text_encoding_decode_UTF8(code, code + 3,
															  &output)) << (int)a << " " << (int)b << " " << (int)c << " " << (int)d << std::endl << buffer_free(&output);
						uint32_t* ptr = buffer_uint32_data(&output, 0);
						ASSERT_NE(nullptr, ptr) << buffer_free(&output);
						ASSERT_EQ(i++, *ptr) << (int)a << " " << (int)b << " " << (int)c << " " << (int)d << std::endl << buffer_free(
												 &output);
					}
				}
			}
		}
	}

#endif
	buffer_release(&output);
}

TEST(TestTextEncoding_, text_encoding_encode_UTF16LE)
{
	buffer input;
	SET_NULL_TO_BUFFER(input);
	//
	buffer encode_output;
	SET_NULL_TO_BUFFER(encode_output);
	//
	buffer decode_output;
	SET_NULL_TO_BUFFER(decode_output);
	//
	ASSERT_TRUE(buffer_resize(&input, 4096))
			<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
	ASSERT_TRUE(buffer_resize(&encode_output, 4096))
			<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
	ASSERT_TRUE(buffer_resize(&decode_output, 4096))
			<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);

	for (uint32_t i = 0; i < UINT16_MAX + 1;)
	{
		const uint32_t range_max = i + 1024;
		//
		ASSERT_TRUE(buffer_resize(&input, 0))
				<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
		ASSERT_TRUE(buffer_resize(&encode_output, 0))
				<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
		ASSERT_TRUE(buffer_resize(&decode_output, 0))
				<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);

		while (i < range_max)
		{
			ASSERT_TRUE(buffer_push_back_uint32(&input, i++))
					<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
		}

		const uint32_t* start = (const uint32_t*)buffer_data(&input, 0);
		const uint32_t* finish = start + buffer_size(&input) / sizeof(uint32_t);
		ASSERT_TRUE(text_encoding_encode_UTF16(start, finish, 0, &encode_output))
				<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);

		if (0xD7FF < i - 1024 && i - 1024 < 0xE000)
		{
			uint32_t* ptr = NULL;
			ptrdiff_t j = 0;

			while (NULL != (ptr = buffer_uint32_data(&input, j++)))
			{
				const uint32_t input_code = *ptr;

				if (0xD7FF < input_code && input_code < 0xE000)
				{
					*ptr = 0xFFFD;
				}
			}
		}

		const uint16_t* start_for_decode = (const uint16_t*)buffer_data(&encode_output, 0);
		const uint16_t* finish_for_decode = start_for_decode + buffer_size(&encode_output) / sizeof(uint16_t);
		ASSERT_TRUE(text_encoding_decode_UTF16(start_for_decode, finish_for_decode, 0, &decode_output))
				<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
		//
		ASSERT_EQ(buffer_to_string(&input), buffer_to_string(&decode_output)) << range_max
				<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
	}

	buffer_release(&decode_output);
	buffer_release(&encode_output);
	buffer_release(&input);
}

TEST(TestTextEncoding_, text_encoding_UTF8_to_UTF16LE)
{
	buffer input_UTF8;
	SET_NULL_TO_BUFFER(input_UTF8);
	//
	ASSERT_TRUE(buffer_resize(&input_UTF8, sizeof(uint32_t) * (0xFFFF + 1))) << buffer_free(&input_UTF8);
	ASSERT_TRUE(buffer_resize(&input_UTF8, 0)) << buffer_free(&input_UTF8);

	for (uint8_t a = 0; a < 0x80; ++a)
	{
		ASSERT_TRUE(buffer_push_back(&input_UTF8, a)) << buffer_free(&input_UTF8);
	}

	for (uint8_t a = 0xC2; a < 0xE0; ++a)
	{
		for (uint8_t b = 0x80; b < 0xC0; ++b)
		{
			ASSERT_TRUE(buffer_push_back(&input_UTF8, a)) << buffer_free(&input_UTF8);
			ASSERT_TRUE(buffer_push_back(&input_UTF8, b)) << buffer_free(&input_UTF8);
		}
	}

	for (uint8_t a = 0xE0; a < 0xE1; ++a)
	{
		for (uint8_t b = 0xA0; b < 0xC0; ++b)
		{
			for (uint8_t c = 0x80; c < 0xC0; ++c)
			{
				ASSERT_TRUE(buffer_push_back(&input_UTF8, a)) << buffer_free(&input_UTF8);
				ASSERT_TRUE(buffer_push_back(&input_UTF8, b)) << buffer_free(&input_UTF8);
				ASSERT_TRUE(buffer_push_back(&input_UTF8, c)) << buffer_free(&input_UTF8);
			}
		}
	}

	for (uint8_t a = 0xE1; a < 0xF0; ++a)
	{
		for (uint8_t b = 0x80; b < 0xC0; ++b)
		{
			for (uint8_t c = 0x80; c < 0xC0; ++c)
			{
				ASSERT_TRUE(buffer_push_back(&input_UTF8, a)) << buffer_free(&input_UTF8);
				ASSERT_TRUE(buffer_push_back(&input_UTF8, b)) << buffer_free(&input_UTF8);
				ASSERT_TRUE(buffer_push_back(&input_UTF8, c)) << buffer_free(&input_UTF8);
			}
		}
	}

	buffer returned_UTF16LE;
	SET_NULL_TO_BUFFER(returned_UTF16LE);
	//
	ASSERT_TRUE(buffer_resize(&returned_UTF16LE,
							  sizeof(uint32_t) * (0xFFFF + 1))) << buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE);
	ASSERT_TRUE(buffer_resize(&returned_UTF16LE,
							  0)) << buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE);
	//
	ASSERT_TRUE(text_encoding_UTF8_to_UTF16LE(buffer_data(&input_UTF8, 0), buffer_data(&input_UTF8,
				0) + buffer_size(&input_UTF8), &returned_UTF16LE)) << buffer_free(&input_UTF8) << buffer_free(
							&returned_UTF16LE);
	//
	buffer decoded_UTF8;
	SET_NULL_TO_BUFFER(decoded_UTF8);
	//
	ASSERT_TRUE(buffer_resize(&decoded_UTF8,
							  sizeof(uint32_t) * (0xFFFF + 1))) << buffer_free(&input_UTF8) << buffer_free(
										  &returned_UTF16LE) << buffer_free(&decoded_UTF8);
	ASSERT_TRUE(buffer_resize(&decoded_UTF8,
							  0)) << buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) << buffer_free(&decoded_UTF8);
	//
	ASSERT_TRUE(text_encoding_decode_UTF8(buffer_data(&input_UTF8, 0), buffer_data(&input_UTF8,
										  0) + buffer_size(&input_UTF8), &decoded_UTF8)) << buffer_free(&input_UTF8) << buffer_free(
												  &returned_UTF16LE) << buffer_free(&decoded_UTF8);
	//
	buffer decoded_UTF16LE;
	SET_NULL_TO_BUFFER(decoded_UTF16LE);
	//
	ASSERT_TRUE(buffer_resize(&decoded_UTF16LE,
							  sizeof(uint32_t) * (0xFFFF + 1))) << buffer_free(&input_UTF8) << buffer_free(
										  &returned_UTF16LE) << buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);
	ASSERT_TRUE(buffer_resize(&decoded_UTF16LE,
							  0)) << buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) << buffer_free(
									  &decoded_UTF8) << buffer_free(&decoded_UTF16LE);
	//
	ASSERT_TRUE(text_encoding_decode_UTF16(buffer_uint16_data(&returned_UTF16LE, 0),
										   (const uint16_t*)(buffer_data(&returned_UTF16LE, 0) + buffer_size(&returned_UTF16LE)),
										   0, &decoded_UTF16LE)) << buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) << buffer_free(
												   &decoded_UTF8) << buffer_free(&decoded_UTF16LE);
	uint32_t expected_return = 0x0;

	for (; expected_return < 0x10000; ++expected_return)
	{
		const uint32_t* returned_UTF16LE_value = buffer_uint32_data(&decoded_UTF16LE, expected_return);
		ASSERT_NE(nullptr, returned_UTF16LE_value) <<
				buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
				buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);

		if (0xD7FF < expected_return && expected_return < 0xE000)
		{
			ASSERT_EQ((uint32_t)0xFFFD, (*returned_UTF16LE_value)) <<
					buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
					buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);
		}
		else
		{
			ASSERT_EQ(expected_return, (*returned_UTF16LE_value)) <<
					buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
					buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);
		}

		const uint32_t* returned_UTF8_value = buffer_uint32_data(&decoded_UTF16LE, expected_return);
		ASSERT_NE(nullptr, returned_UTF8_value) <<
												buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
												buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);

		if (0xD7FF < expected_return && expected_return < 0xE000)
		{
			ASSERT_EQ((uint32_t)0xFFFD, (*returned_UTF8_value)) <<
					buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
					buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);
		}
		else
		{
			ASSERT_EQ(expected_return, (*returned_UTF8_value)) <<
					buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
					buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);
		}
	}

	ASSERT_EQ(nullptr, buffer_uint32_data(&decoded_UTF16LE, expected_return)) <<
			buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
			buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);
	ASSERT_EQ(nullptr, buffer_uint32_data(&decoded_UTF16LE, expected_return)) <<
			buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
			buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE);
	//
#if defined(_WIN32)
	buffer returned_UTF8;
	SET_NULL_TO_BUFFER(returned_UTF8);
	//
	ASSERT_TRUE(buffer_resize(&returned_UTF8, sizeof(uint32_t) * (0xFFFF + 1))) <<
			buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
			buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE) << buffer_free(&returned_UTF8);
	ASSERT_TRUE(buffer_resize(&returned_UTF8, 0)) <<
			buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
			buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE) << buffer_free(&returned_UTF8);
	//
	ASSERT_TRUE(text_encoding_UTF16LE_to_UTF8(buffer_uint16_data(&returned_UTF16LE, 0),
				(const uint16_t*)(buffer_data(&returned_UTF16LE, 0) + buffer_size(&decoded_UTF16LE)),
				&returned_UTF8)) <<
								 buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
								 buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE) << buffer_free(&returned_UTF8);
	//
	uint32_t returned_result = 0;

	for (expected_return = 0x0; expected_return < (uint32_t)buffer_size(&input_UTF8);
		 ++expected_return, ++returned_result)
	{
		const uint8_t* expected = buffer_data(&input_UTF8, expected_return);
		ASSERT_NE(nullptr, expected) <<
									 buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
									 buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE) << buffer_free(&returned_UTF8);
		const uint8_t* returned = buffer_data(&returned_UTF8, returned_result);
		ASSERT_NE(nullptr, returned) <<
									 buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
									 buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE) << buffer_free(&returned_UTF8);

		if (163711 < expected_return) //TODO: && expected_return < 163711 + 3 * (0xE000 - 0xD7FF))
		{
			/*TODO: expected_return += 2;
			returned_result += 2;*/
			continue;
		}
		else
		{
			ASSERT_EQ(*expected, *returned) <<
											buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
											buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE) << buffer_free(&returned_UTF8);
		}
	}

	ASSERT_EQ(nullptr, buffer_data(&input_UTF8, expected_return)) <<
			buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
			buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE) << buffer_free(&returned_UTF8);
	/*ASSERT_EQ(nullptr, buffer_data(&returned_UTF8, returned_result)) <<
		buffer_free(&input_UTF8) << buffer_free(&returned_UTF16LE) <<
		buffer_free(&decoded_UTF8) << buffer_free(&decoded_UTF16LE) << buffer_free(&returned_UTF8);*/
	buffer_release(&returned_UTF8);
#endif
	//
	buffer_release(&decoded_UTF16LE);
	buffer_release(&decoded_UTF8);
	buffer_release(&returned_UTF16LE);
	buffer_release(&input_UTF8);
}

TEST_F(TestTextEncoding, text_encoding_UTF8_to_code_page)
{
	std::vector<uint16_t> input_utf16le;
	//
	buffer input;
	SET_NULL_TO_BUFFER(input);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		input_utf16le.resize(0);
		const auto input_hex(get_data_from_nodes(node, "input"));
		string_hex_parse(input_hex.c_str(), input_utf16le);
		//
		ASSERT_TRUE(buffer_resize(&input, 0)) << buffer_free(&output) << buffer_free(&input);
		ASSERT_TRUE(
			text_encoding_UTF16LE_to_UTF8(input_utf16le.data(), input_utf16le.data() + input_utf16le.size(), &input))
				<< buffer_free(&output) << buffer_free(&input);
		ASSERT_FALSE(node.node().select_node("output").node().empty())
				<< buffer_free(&output) << buffer_free(&input);

		for (const auto& output_node : node.node().select_nodes("output"))
		{
			const auto code_page = (uint16_t)output_node.node().attribute("code").as_int();
			auto expected_return = output_node.node().attribute("return").as_int();
			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output) << buffer_free(&input);
			//
			ASSERT_EQ(expected_return,
					  text_encoding_UTF8_to_code_page(
						  buffer_data(&input, 0),
						  buffer_data(&input, 0) + buffer_size(&input),
						  code_page, &output)) << buffer_free(&output) << buffer_free(&input);
			//
			input_utf16le.resize(0);
			string_hex_parse(output_node.node().child_value(), input_utf16le);
			//
			ASSERT_EQ((ptrdiff_t)input_utf16le.size(), buffer_size(&output))
					<< buffer_free(&output) << buffer_free(&input);
			const uint8_t* ptr = buffer_data(&output, 0);
			expected_return = 0;

			for (const auto& expected_code : input_utf16le)
			{
				ASSERT_EQ(expected_code, (*ptr))
						<< expected_return << std::endl
						<< buffer_free(&output) << buffer_free(&input);
				++ptr;
				++expected_return;
			}
		}

		--node_count;
	}

	buffer_release(&output);
	buffer_release(&input);
}

TEST_F(TestTextEncoding, text_encoding_UTF8_from_code_page)
{
	std::vector<uint8_t> input_ascii;
	std::vector<uint8_t> expected_output;
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		input_ascii.resize(0);
		const auto input_hex(get_data_from_nodes(node, "input"));
		string_hex_parse(input_hex.c_str(), input_ascii);
		//
		ASSERT_FALSE(node.node().select_node("output").node().empty())
				<< buffer_free(&output);

		for (const auto& output_node : node.node().select_nodes("output"))
		{
			const auto code_page = (uint16_t)output_node.node().attribute("code").as_int();
			auto expected_return = output_node.node().attribute("return").as_int();
			//
			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
			ASSERT_EQ(expected_return,
					  text_encoding_UTF8_from_code_page(input_ascii.data(),
							  input_ascii.data() + input_ascii.size(), code_page, &output)) << buffer_free(&output);
			//
			expected_output.clear();
			string_hex_parse(output_node.node().child_value(), expected_output);
			//
			ASSERT_EQ((ptrdiff_t)expected_output.size(), buffer_size(&output))
					<< buffer_free(&output);
			const uint8_t* ptr = buffer_data(&output, 0);
			expected_return = 0;

			for (const auto& expected_code : expected_output)
			{
				ASSERT_EQ(expected_code, (*ptr))
						<< expected_return << std::endl
						<< buffer_free(&output);
				++ptr;
				++expected_return;
			}
		}

		--node_count;
	}

	buffer_release(&output);
}
/*
text_encoding_UTF8_to_UTF16BE
text_encoding_UTF16BE_to_UTF8
text_encoding_UTF8_to_UTF32BE
text_encoding_UTF32BE_to_UTF8
text_encoding_get_one
*/