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
#include "text_encoding.h"
}

#include <string>
#include <cstdint>
#include <cstdlib>

class TestTextEncoding : public TestsBaseXml
{
};

TEST(TestTextEncoding_, text_encoding_get_BOM)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_FALSE(text_encoding_get_BOM(UTF7, &output)) << buffer_free(&output); //TODO:
	ASSERT_TRUE(text_encoding_get_BOM(UTF8, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF16BE, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF16LE, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF32BE, &output)) << buffer_free(&output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF32LE, &output)) << buffer_free(&output);
	//
	buffer_release(&output);
}

TEST(TestTextEncoding_, text_encoding_get_data_encoding_by_BOM)
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
		const uint8_t returned_encoding = text_encoding_get_data_encoding_by_BOM(data[i], 4);
		ASSERT_EQ(expected_encoding[i], returned_encoding) << (int)i;
	}
}

TEST(TestTextEncoding_, text_encoding_UTF16_from_ASCII)
{
	buffer input;
	SET_NULL_TO_BUFFER(input);
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
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (uint8_t a = 0; a < 2; ++a)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output) << buffer_free(&input);

		if (a)
		{
			ASSERT_TRUE(
				text_encoding_UTF16BE_from_ASCII(input_in_range.start,
												 input_in_range.finish,
												 &output))
					<< buffer_free(&output) << buffer_free(&input);
		}
		else
		{
			ASSERT_TRUE(
				text_encoding_UTF16LE_from_ASCII(input_in_range.start,
												 input_in_range.finish,
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

std::string string_hex_to_string(const char* str)
{
	std::string output;

	if (NULL == str || '\0' == *str)
	{
		return output;
	}

	while (NULL != (str = strstr(str, "0x")))
	{
		char* pos = NULL;
		const long value = strtol(str, &pos, 16);
		str += pos - str;

		if (value < 0 || UINT8_MAX < value)
		{
			break;
		}

		output.push_back((char)value);
	}

	return output;
}

std::wstring string_hex_to_wstring(const char* str)
{
	std::wstring output;

	if (NULL == str || '\0' == *str)
	{
		return output;
	}

	while (NULL != (str = strstr(str, "0x")))
	{
		char* pos = NULL;
		const long value = strtol(str, &pos, 16);
		str += pos - str;

		if (value < 0 || UINT16_MAX < value)
		{
			break;
		}

		output.push_back((uint16_t)value);
	}

	return output;
}

TEST_F(TestTextEncoding, text_encoding_UTF16LE_from_code_page)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		const std::string input_hex(get_data_from_nodes(node, "input"));
		const std::string input(string_hex_to_string(input_hex.c_str()));
		const uint16_t code_page = (uint16_t)INT_PARSE(
									   node.node().select_node("code_page").node().child_value());
		const std::string output_hex(get_data_from_nodes(node, "output"));
		const std::wstring expected_output(string_hex_to_wstring(output_hex.c_str()));
		//
		const range input_in_range = string_to_range(input);
		//
		ASSERT_TRUE(text_encoding_UTF16LE_from_code_page(
						input_in_range.start,
						input_in_range.finish,
						code_page,
						&output)) << buffer_free(&output);
		//
		ASSERT_EQ(expected_output, buffer_to_u16string(&output)) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}

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
	struct buffer output;
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
		ASSERT_TRUE(text_encoding_encode_UTF16LE(start, finish, &encode_output))
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

		/*mbstate_t state;
		memset(&state, 0, sizeof(mbstate_t));
		//
		ptrdiff_t j = 0;
		uint8_t* ptr = NULL;
		uint32_t expected_out = i - 1024;
		const uint8_t* finish_ = (buffer_data(&encode_output, 0) + buffer_size(&encode_output));

		while (NULL != (ptr = buffer_data(&encode_output, j)))
		{
			uint32_t out = 0;
			size_t rc = mbrtoc16((char16_t*)&out, (const char*)&ptr, finish_ - ptr, &state);
			ASSERT_LT(0, rc) << buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
			ASSERT_LE(rc, sizeof(uint32_t)) << buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
			ASSERT_EQ(expected_out, out) <<  buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
			j += rc;
			++expected_out;
		}*/
		const uint16_t* start_for_decode = (const uint16_t*)buffer_data(&encode_output, 0);
		const uint16_t* finish_for_decode = start_for_decode + buffer_size(&encode_output) / sizeof(uint16_t);
		ASSERT_TRUE(text_encoding_decode_UTF16LE(start_for_decode, finish_for_decode, &decode_output))
				<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
		//
		ASSERT_EQ(buffer_to_string(&input), buffer_to_string(&decode_output)) << range_max
				<< buffer_free(&input) << buffer_free(&encode_output) << buffer_free(&decode_output);
	}

	buffer_release(&decode_output);
	buffer_release(&encode_output);
	buffer_release(&input);
}
