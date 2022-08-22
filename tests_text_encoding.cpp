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
#include "text_encoding.h"
}

#include <limits>
#include <string>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ostream>

class TestTextEncoding : public TestsBaseXml
{
};

TEST(TestTextEncoding_, text_encoding_get_BOM)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));
	//
	ASSERT_TRUE(text_encoding_get_BOM(ASCII, output)) << buffer_free(output);
	ASSERT_FALSE(text_encoding_get_BOM(UTF7, output)) << buffer_free(output); //TODO:
	//
	ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF8, output)) << buffer_free(output);
	ASSERT_FALSE(buffer_size(output)) << buffer_free(output);
	//
	ASSERT_TRUE(text_encoding_get_BOM(BigEndianUnicode, output)) << buffer_free(output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF16BE, output)) << buffer_free(output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF16LE, output)) << buffer_free(output);
	ASSERT_TRUE(text_encoding_get_BOM(Unicode, output)) << buffer_free(output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF32BE, output)) << buffer_free(output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF32, output)) << buffer_free(output);
	ASSERT_TRUE(text_encoding_get_BOM(UTF32LE, output)) << buffer_free(output);
	//
	buffer_release(output);
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

	for (uint8_t i = 0, count = COUNT_OF(data); i < count; ++i)
	{
		const uint8_t returned_encoding = text_encoding_get_one_of_data_by_BOM(data[i], 4);
		ASSERT_EQ(expected_encoding[i], returned_encoding) << static_cast<int>(i);
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
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));
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

		const auto input_in_a_range(string_to_range(input));
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		//
		ASSERT_TRUE(
			text_encoding_UTF_to_ASCII(
				input_in_a_range.start, input_in_a_range.finish, encoding, output))
				<< buffer_free(output);
		//
		ASSERT_EQ(expected_output, buffer_to_string(output))
				<< static_cast<int>(encoding) << std::endl << buffer_free(output);
	}

	buffer_release(output);
}

TEST(TestTextEncoding_, text_encoding_UTF_from_ASCII)
{
	std::string input_buffer(buffer_size_of(), 0);
	auto input = reinterpret_cast<void*>(&input_buffer[0]);
	ASSERT_TRUE(buffer_init(input, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_append(input, nullptr, UINT8_MAX + 1)) << buffer_free(input);
	ASSERT_TRUE(buffer_resize(input, 0)) << buffer_free(input);

	for (uint16_t i = 0; ; ++i)
	{
		if (UINT8_MAX < i)
		{
			ASSERT_TRUE(buffer_push_back(input, 0)) << buffer_free(input);
			break;
		}

		ASSERT_TRUE(buffer_push_back(input, static_cast<uint8_t>(i))) << buffer_free(input);
	}

	const auto input_in_a_range(buffer_to_range(input));
	const auto expected_size = static_cast<ptrdiff_t>(sizeof(uint16_t) * (UINT8_MAX + sizeof(uint16_t)));
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (uint8_t a = 0; a < 2; ++a)
	{
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output) << buffer_free(input);

		if (a)
		{
			ASSERT_TRUE(
				text_encoding_UTF_from_ASCII(input_in_a_range.start,
											 input_in_a_range.finish,
											 UTF16BE,
											 output))
					<< buffer_free(output) << buffer_free(input);
		}
		else
		{
			ASSERT_TRUE(
				text_encoding_UTF_from_ASCII(input_in_a_range.start,
											 input_in_a_range.finish,
											 UTF16LE,
											 output))
					<< buffer_free(output) << buffer_free(input);
		}

		ASSERT_EQ(expected_size, buffer_size(output)) << buffer_free(output) << buffer_free(input);
		const auto* ptr = reinterpret_cast<const uint16_t*>(buffer_data(output, a));
		ASSERT_NE(nullptr, ptr) << buffer_free(output) << buffer_free(input);

		for (uint8_t i = 0; i < INT8_MAX + 1; ++i)
		{
			ASSERT_EQ(input_in_a_range.start[i], ptr[i])
					<< buffer_free(output) << buffer_free(input);
		}

		for (uint16_t i = INT8_MAX + 1; i < UINT8_MAX + 1; ++i)
		{
			ASSERT_EQ('?', ptr[i])
					<< buffer_free(output) << buffer_free(input);
		}

		/*ASSERT_EQ(0, ptr[UINT8_MAX + 1 // - a])
				<< buffer_free(output) << buffer_free(input);*/
	}

	buffer_release(output);
	buffer_release(input);
}

template<typename T>
void string_hex_parse(const char* input, T& output)
{
	if (nullptr == input)
	{
		return;
	}

	const typename T::value_type max = std::numeric_limits<typename T::value_type>::max();
	//
	const auto size = output.size();
	output.reserve(size + std::strlen(input));

	while (nullptr != (input = std::strstr(input, "0x")))
	{
		char* pos;
		const auto value = std::strtol(input, &pos, 16);
		input += pos - input;

		if (value < 0 || max < value)
		{
			break;
		}

		output.push_back(static_cast<typename T::value_type>(value));
	}
}

TEST_F(TestTextEncoding, text_encoding_UTF16LE_from_code_page)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));
	//
	std::vector<uint8_t> input;
	std::vector<uint16_t> expected_output;

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		//
		const auto input_hex(get_data_from_nodes(the_node, "input"));
		input.clear();
		string_hex_parse(input_hex.c_str(), input);
		//
		const std::string code_page_str(
			the_node.select_node("code_page").node().child_value());
		const auto code_page_in_a_range(string_to_range(code_page_str));
		//
		auto code_page =
			static_cast<uint16_t>(int_parse(
									  code_page_in_a_range.start, code_page_in_a_range.finish));
		//
		const auto output_hex(get_data_from_nodes(the_node, "output"));
		expected_output.clear();
		string_hex_parse(output_hex.c_str(), expected_output);
		//
		ASSERT_TRUE(text_encoding_UTF16LE_from_code_page(
						input.data(),
						input.data() + input.size(),
						code_page,
						output)) << buffer_free(output);
		//
		code_page = 0;
		const uint16_t* returned_code;

		for (const auto& expected_code : expected_output)
		{
			returned_code = buffer_uint16_t_data(output, code_page);
			ASSERT_NE(nullptr, returned_code) << buffer_free(output);
			//
			ASSERT_EQ(expected_code, *returned_code)
					<< code_page << std::endl
					<< buffer_free(output);
			//
			++code_page;
		}

		returned_code = buffer_uint16_t_data(output, code_page);
		ASSERT_EQ(nullptr, returned_code) << buffer_free(output);
		//
		--node_count;
	}

	buffer_release(output);
}

TEST(TestTextEncoding_, text_encoding_encode_decode_UTF8)
{
	uint8_t output[6];

	for (uint32_t i = 0, o = 0; i < 0x10001; ++i)
	{
		const auto stored = text_encoding_encode_UTF8_single(
								i, output);
		ASSERT_LT(0, stored);

		if (i < 0x10000 && (i < 0xD800 || 0xDFFF < i))
		{
			const auto stored_o = text_encoding_decode_UTF8_single(
									  output, output + stored, &o);
			ASSERT_EQ(stored, stored_o);
			ASSERT_EQ(i, o);
		}
		else
		{
			ASSERT_EQ(3, stored);
			ASSERT_EQ(0xEF, output[0]);
			ASSERT_EQ(0xBF, output[1]);
			ASSERT_EQ(0xBD, output[2]);
		}
	}
}

TEST(TestTextEncoding_, text_encoding_encode_UTF8)
{
	std::string input_buffer(buffer_size_of(), 0);
	auto input = reinterpret_cast<void*>(&input_buffer[0]);
	ASSERT_TRUE(buffer_init(input, buffer_size_of()));
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));
	//
	std::string output_buffer_(buffer_size_of(), 0);
	auto output_ = reinterpret_cast<void*>(&output_buffer_[0]);
	ASSERT_TRUE(buffer_init(output_, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(input, 4096))
			<< buffer_free(input) << buffer_free(output) << buffer_free(output_);
	ASSERT_TRUE(buffer_resize(output, 4096))
			<< buffer_free(input) << buffer_free(output) << buffer_free(output_);
	ASSERT_TRUE(buffer_resize(output_, 4096))
			<< buffer_free(input) << buffer_free(output) << buffer_free(output_);

	for (uint32_t i = 0; i < UINT16_MAX + 1;)
	{
		const auto range_max = i + 1024;
		//
		ASSERT_TRUE(buffer_resize(input, 0))
				<< buffer_free(input) << buffer_free(output) << buffer_free(output_);
		ASSERT_TRUE(buffer_resize(output, 0))
				<< buffer_free(input) << buffer_free(output) << buffer_free(output_);
		ASSERT_TRUE(buffer_resize(output_, 0))
				<< buffer_free(input) << buffer_free(output) << buffer_free(output_);

		while (i < range_max)
		{
			ASSERT_TRUE(buffer_push_back_uint32_t(input, i++))
					<< buffer_free(input) << buffer_free(output) << buffer_free(output_);
		}

		const auto* start = reinterpret_cast<const uint32_t*>(buffer_data(input, 0));
		const auto* finish = start + buffer_size(input) / sizeof(uint32_t);
		ASSERT_TRUE(text_encoding_encode_UTF8(start, finish, output))
				<< buffer_free(input) << buffer_free(output) << buffer_free(output_);

		if (0xD7FF < i - 1024 && i - 1024 < 0xE000)
		{
			uint32_t* ptr;
			ptrdiff_t j = 0;

			while (nullptr != (ptr = buffer_uint32_t_data(input, j++)))
			{
				const uint32_t input_code = *ptr;

				if (0xD7FF < input_code && input_code < 0xE000)
				{
					*ptr = 0xFFFD;
				}
			}
		}

		const auto* start_ = buffer_uint8_t_data(output, 0);
		const auto* finish_ = start_ + buffer_size(output);
		ASSERT_TRUE(text_encoding_decode_UTF8(start_, finish_, output_))
				<< buffer_free(input) << buffer_free(output) << buffer_free(output_);
		ASSERT_EQ(buffer_to_string(input), buffer_to_string(output_)) << range_max
				<< buffer_free(input) << buffer_free(output) << buffer_free(output_);
	}

	buffer_release(output_);
	buffer_release(output);
	buffer_release(input);
}

TEST(TestTextEncoding_, text_encoding_decode_UTF8)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_append(output, nullptr, 4 * sizeof(uint32_t))) << buffer_free(output);

	for (uint8_t i = 0; i < 0x80; ++i)
	{
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		ASSERT_TRUE(text_encoding_decode_UTF8(&i, &i + 1, output)) << buffer_free(output);
		auto ptr = buffer_uint32_t_data(output, 0);
		ASSERT_NE(nullptr, ptr) << buffer_free(output);
		ASSERT_EQ(i, *ptr) << buffer_free(output);
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
				ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
				ASSERT_TRUE(text_encoding_decode_UTF8(code, code + 2, output)) << buffer_free(output);
				auto ptr = buffer_uint32_t_data(output, 0);
				ASSERT_NE(nullptr, ptr) << buffer_free(output);
				ASSERT_EQ(i++, *ptr) <<
									 static_cast<int>(a) << " " <<
									 static_cast<int>(b) << std::endl <<
									 buffer_free(output);
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
					ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
					ASSERT_TRUE(text_encoding_decode_UTF8(code, code + 3, output)) << buffer_free(output);
					auto ptr = buffer_uint32_t_data(output, 0);
					ASSERT_NE(nullptr, ptr) << buffer_free(output);
					ASSERT_EQ(i++, *ptr) <<
										 static_cast<int>(a) << " " <<
										 static_cast<int>(b) << " " <<
										 static_cast<int>(c) << std::endl <<
										 buffer_free(output);
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
					ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
					ASSERT_TRUE(text_encoding_decode_UTF8(code, code + 3, output)) << buffer_free(output);
					auto ptr = buffer_uint32_t_data(output, 0);
					ASSERT_NE(nullptr, ptr) << buffer_free(output);
					ASSERT_EQ(i++, *ptr) <<
										 static_cast<int>(a) << " " <<
										 static_cast<int>(b) << " " <<
										 static_cast<int>(c) << std::endl <<
										 buffer_free(output);
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
						ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
						ASSERT_TRUE(text_encoding_decode_UTF8(code, code + 3,
															  output)) << (int)a << " " << (int)b << " " << (int)c << " " << (int)d << std::endl << buffer_free(output);
						uint32_t* ptr = buffer_uint32_data(output, 0);
						ASSERT_NE(nullptr, ptr) << buffer_free(output);
						ASSERT_EQ(i++, *ptr) << (int)a << " " << (int)b << " " << (int)c << " " << (int)d << std::endl << buffer_free(
												 output);
					}
				}
			}
		}
	}

#endif
	buffer_release(output);
}

TEST(TestTextEncoding_, text_encoding_encode_UTF16LE)
{
	std::string input_buffer(buffer_size_of(), 0);
	auto input = reinterpret_cast<void*>(&input_buffer[0]);
	ASSERT_TRUE(buffer_init(input, buffer_size_of()));
	//
	std::string encode_output_buffer(buffer_size_of(), 0);
	auto encode_output = reinterpret_cast<void*>(&encode_output_buffer[0]);
	ASSERT_TRUE(buffer_init(encode_output, buffer_size_of()));
	//
	std::string decode_output_buffer(buffer_size_of(), 0);
	auto decode_output = reinterpret_cast<void*>(&decode_output_buffer[0]);
	ASSERT_TRUE(buffer_init(decode_output, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(input, 4096))
			<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);
	ASSERT_TRUE(buffer_resize(encode_output, 4096))
			<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);
	ASSERT_TRUE(buffer_resize(decode_output, 4096))
			<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);

	for (uint32_t i = 0; i < UINT16_MAX + 1;)
	{
		const auto range_max = i + 1024;
		//
		ASSERT_TRUE(buffer_resize(input, 0))
				<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);
		ASSERT_TRUE(buffer_resize(encode_output, 0))
				<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);
		ASSERT_TRUE(buffer_resize(decode_output, 0))
				<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);

		while (i < range_max)
		{
			ASSERT_TRUE(buffer_push_back_uint32_t(input, i++))
					<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);
		}

		const auto* start = reinterpret_cast<const uint32_t*>(buffer_data(input, 0));
		const auto* finish = start + buffer_size(input) / sizeof(uint32_t);
		ASSERT_TRUE(text_encoding_encode_UTF16(start, finish, 0, encode_output))
				<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);

		if (0xD7FF < i - 1024 && i - 1024 < 0xE000)
		{
			uint32_t* ptr;
			ptrdiff_t j = 0;

			while (nullptr != (ptr = buffer_uint32_t_data(input, j++)))
			{
				const auto input_code = *ptr;

				if (0xD7FF < input_code && input_code < 0xE000)
				{
					*ptr = 0xFFFD;
				}
			}
		}

		const auto* start_for_decode = reinterpret_cast<const uint16_t*>(buffer_data(encode_output, 0));
		const auto* finish_for_decode = start_for_decode + buffer_size(encode_output) / sizeof(uint16_t);
		ASSERT_TRUE(text_encoding_decode_UTF16(start_for_decode, finish_for_decode, 0, decode_output))
				<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);
		//
		ASSERT_EQ(buffer_to_string(input), buffer_to_string(decode_output)) << range_max
				<< buffer_free(input) << buffer_free(encode_output) << buffer_free(decode_output);
	}

	buffer_release(decode_output);
	buffer_release(encode_output);
	buffer_release(input);
}

TEST(TestTextEncoding_, text_encoding_UTF8_to_UTF16LE)
{
	std::string input_UTF8_buffer(buffer_size_of(), 0);
	auto input_UTF8 = reinterpret_cast<void*>(&input_UTF8_buffer[0]);
	ASSERT_TRUE(buffer_init(input_UTF8, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(input_UTF8, sizeof(uint32_t) * (0xFFFF + 1))) << buffer_free(input_UTF8);
	ASSERT_TRUE(buffer_resize(input_UTF8, 0)) << buffer_free(input_UTF8);

	for (uint8_t a = 0; a < 0x80; ++a)
	{
		ASSERT_TRUE(buffer_push_back(input_UTF8, a)) << buffer_free(input_UTF8);
	}

	for (uint8_t a = 0xC2; a < 0xE0; ++a)
	{
		for (uint8_t b = 0x80; b < 0xC0; ++b)
		{
			ASSERT_TRUE(buffer_push_back(input_UTF8, a)) << buffer_free(input_UTF8);
			ASSERT_TRUE(buffer_push_back(input_UTF8, b)) << buffer_free(input_UTF8);
		}
	}

	for (uint8_t a = 0xE0; a < 0xE1; ++a)
	{
		for (uint8_t b = 0xA0; b < 0xC0; ++b)
		{
			for (uint8_t c = 0x80; c < 0xC0; ++c)
			{
				ASSERT_TRUE(buffer_push_back(input_UTF8, a)) << buffer_free(input_UTF8);
				ASSERT_TRUE(buffer_push_back(input_UTF8, b)) << buffer_free(input_UTF8);
				ASSERT_TRUE(buffer_push_back(input_UTF8, c)) << buffer_free(input_UTF8);
			}
		}
	}

	for (uint8_t a = 0xE1; a < 0xF0; ++a)
	{
		for (uint8_t b = 0x80; b < 0xC0; ++b)
		{
			for (uint8_t c = 0x80; c < 0xC0; ++c)
			{
				ASSERT_TRUE(buffer_push_back(input_UTF8, a)) << buffer_free(input_UTF8);
				ASSERT_TRUE(buffer_push_back(input_UTF8, b)) << buffer_free(input_UTF8);
				ASSERT_TRUE(buffer_push_back(input_UTF8, c)) << buffer_free(input_UTF8);
			}
		}
	}

	std::string returned_UTF16LE_buffer(buffer_size_of(), 0);
	auto returned_UTF16LE = reinterpret_cast<void*>(&returned_UTF16LE_buffer[0]);
	ASSERT_TRUE(buffer_init(returned_UTF16LE, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(returned_UTF16LE,
							  sizeof(uint32_t) * (0xFFFF + 1))) << buffer_free(input_UTF8) << buffer_free(returned_UTF16LE);
	ASSERT_TRUE(buffer_resize(returned_UTF16LE,
							  0)) << buffer_free(input_UTF8) << buffer_free(returned_UTF16LE);
	//
	ASSERT_TRUE(text_encoding_UTF8_to_UTF16LE(buffer_uint8_t_data(input_UTF8, 0), buffer_uint8_t_data(input_UTF8,
				0) + buffer_size(input_UTF8), returned_UTF16LE)) << buffer_free(input_UTF8) << buffer_free(
							returned_UTF16LE);
	//
	std::string decoded_UTF8_buffer(buffer_size_of(), 0);
	auto decoded_UTF8 = reinterpret_cast<void*>(&decoded_UTF8_buffer[0]);
	ASSERT_TRUE(buffer_init(decoded_UTF8, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(decoded_UTF8,
							  sizeof(uint32_t) * (0xFFFF + 1))) << buffer_free(input_UTF8) << buffer_free(
										  returned_UTF16LE) << buffer_free(decoded_UTF8);
	ASSERT_TRUE(buffer_resize(decoded_UTF8,
							  0)) << buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) << buffer_free(decoded_UTF8);
	//
	ASSERT_TRUE(text_encoding_decode_UTF8(buffer_uint8_t_data(input_UTF8, 0), buffer_uint8_t_data(input_UTF8,
										  0) + buffer_size(input_UTF8), decoded_UTF8)) << buffer_free(input_UTF8) << buffer_free(
												  returned_UTF16LE) << buffer_free(decoded_UTF8);
	//
	std::string decoded_UTF16LE_buffer(buffer_size_of(), 0);
	auto decoded_UTF16LE = reinterpret_cast<void*>(&decoded_UTF16LE_buffer[0]);
	ASSERT_TRUE(buffer_init(decoded_UTF16LE, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(decoded_UTF16LE,
							  sizeof(uint32_t) * (0xFFFF + 1))) << buffer_free(input_UTF8) << buffer_free(
										  returned_UTF16LE) << buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);
	ASSERT_TRUE(buffer_resize(decoded_UTF16LE,
							  0)) << buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) << buffer_free(
									  decoded_UTF8) << buffer_free(decoded_UTF16LE);
	//
	ASSERT_TRUE(text_encoding_decode_UTF16(buffer_uint16_t_data(returned_UTF16LE, 0),
										   reinterpret_cast<const uint16_t*>(buffer_uint8_t_data(returned_UTF16LE, 0) + buffer_size(returned_UTF16LE)),
										   0, decoded_UTF16LE)) << buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) << buffer_free(
												   decoded_UTF8) << buffer_free(decoded_UTF16LE);
	uint32_t expected_return = 0x0;

	for (; expected_return < 0x10000; ++expected_return)
	{
		const auto* returned_UTF16LE_value = buffer_uint32_t_data(decoded_UTF16LE, expected_return);
		ASSERT_NE(nullptr, returned_UTF16LE_value) <<
				buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
				buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);

		if (0xD7FF < expected_return && expected_return < 0xE000)
		{
			ASSERT_EQ(static_cast<uint32_t>(0xFFFD), *returned_UTF16LE_value) <<
					buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
					buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);
		}
		else
		{
			ASSERT_EQ(expected_return, *returned_UTF16LE_value) <<
					buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
					buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);
		}

		const auto* returned_UTF8_value = buffer_uint32_t_data(decoded_UTF16LE, expected_return);
		ASSERT_NE(nullptr, returned_UTF8_value) <<
												buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
												buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);

		if (0xD7FF < expected_return && expected_return < 0xE000)
		{
			ASSERT_EQ(static_cast<uint32_t>(0xFFFD), *returned_UTF8_value) <<
					buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
					buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);
		}
		else
		{
			ASSERT_EQ(expected_return, *returned_UTF8_value) <<
					buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
					buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);
		}
	}

	ASSERT_EQ(nullptr, buffer_uint32_t_data(decoded_UTF16LE, expected_return)) <<
			buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
			buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);
	ASSERT_EQ(nullptr, buffer_uint32_t_data(decoded_UTF16LE, expected_return)) <<
			buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
			buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE);
	//
#if defined(_WIN32)
	std::string returned_UTF8_buffer(buffer_size_of(), 0);
	auto returned_UTF8 = reinterpret_cast<void*>(&returned_UTF8_buffer[0]);
	ASSERT_TRUE(buffer_init(returned_UTF8, buffer_size_of()));
	//
	ASSERT_TRUE(buffer_resize(returned_UTF8, sizeof(uint32_t) * (0xFFFF + 1))) <<
			buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
			buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE) << buffer_free(returned_UTF8);
	ASSERT_TRUE(buffer_resize(returned_UTF8, 0)) <<
			buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
			buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE) << buffer_free(returned_UTF8);
	//
	ASSERT_TRUE(text_encoding_UTF16LE_to_UTF8(buffer_uint16_t_data(returned_UTF16LE, 0),
				reinterpret_cast<const uint16_t*>(buffer_uint8_t_data(returned_UTF16LE, 0) + buffer_size(decoded_UTF16LE)),
				returned_UTF8)) <<
								buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
								buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE) << buffer_free(returned_UTF8);
	//
	uint32_t returned_result = 0;

	for (expected_return = 0x0; expected_return < static_cast<uint32_t>(buffer_size(input_UTF8));
		 ++expected_return, ++returned_result)
	{
		const auto* expected = buffer_uint8_t_data(input_UTF8, expected_return);
		ASSERT_NE(nullptr, expected) <<
									 buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
									 buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE) << buffer_free(returned_UTF8);
		const auto* returned = buffer_uint8_t_data(returned_UTF8, returned_result);
		ASSERT_NE(nullptr, returned) <<
									 buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
									 buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE) << buffer_free(returned_UTF8);

		if (163711 < expected_return) //TODO: && expected_return < 163711 + 3 * (0xE000 - 0xD7FF))
		{
			/*TODO: expected_return += 2;
			returned_result += 2;*/
			continue;
		}
		else
		{
			ASSERT_EQ(*expected, *returned) <<
											buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
											buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE) << buffer_free(returned_UTF8);
		}
	}

	ASSERT_EQ(nullptr, buffer_data(input_UTF8, expected_return)) <<
			buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
			buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE) << buffer_free(returned_UTF8);
	/*ASSERT_EQ(nullptr, buffer_data(returned_UTF8, returned_result)) <<
		buffer_free(input_UTF8) << buffer_free(returned_UTF16LE) <<
		buffer_free(decoded_UTF8) << buffer_free(decoded_UTF16LE) << buffer_free(returned_UTF8);*/
	buffer_release(returned_UTF8);
#endif
	//
	buffer_release(decoded_UTF16LE);
	buffer_release(decoded_UTF8);
	buffer_release(returned_UTF16LE);
	buffer_release(input_UTF8);
}

TEST_F(TestTextEncoding, text_encoding_UTF8_to_code_page)
{
	std::vector<uint16_t> input_utf16le;
	//
	std::string input_buffer(buffer_size_of(), 0);
	auto input = reinterpret_cast<void*>(&input_buffer[0]);
	ASSERT_TRUE(buffer_init(input, buffer_size_of()));
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		input_utf16le.resize(0);
		const auto input_hex(get_data_from_nodes(the_node, "input"));
		string_hex_parse(input_hex.c_str(), input_utf16le);
		//
		ASSERT_TRUE(buffer_resize(input, 0)) << buffer_free(output) << buffer_free(input);
		ASSERT_TRUE(
			text_encoding_UTF16LE_to_UTF8(input_utf16le.data(), input_utf16le.data() + input_utf16le.size(), input))
				<< buffer_free(output) << buffer_free(input);
		ASSERT_FALSE(the_node.select_node("output").node().empty())
				<< buffer_free(output) << buffer_free(input);
		const auto output_nodes = the_node.select_nodes("output");

		for (const auto& output_node : output_nodes)
		{
			const auto the_output_node = output_node.node();
			const auto code_page = static_cast<uint16_t>(the_output_node.attribute("code").as_int());
			auto expected_return = the_output_node.attribute("return").as_int();
			ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output) << buffer_free(input);
			//
			ASSERT_EQ(expected_return,
					  text_encoding_UTF8_to_code_page(
						  buffer_uint8_t_data(input, 0),
						  buffer_uint8_t_data(input, 0) + buffer_size(input),
						  code_page, output)) << buffer_free(output) << buffer_free(input);
			//
			input_utf16le.resize(0);
			string_hex_parse(the_output_node.child_value(), input_utf16le);
			//
			ASSERT_EQ(static_cast<ptrdiff_t>(input_utf16le.size()), buffer_size(output))
					<< buffer_free(output) << buffer_free(input);
			const auto* returned = buffer_uint8_t_data(output, 0);
			expected_return = 0;

			for (const auto& expected_code : input_utf16le)
			{
				ASSERT_EQ(expected_code, *returned)
						<< expected_return << std::endl
						<< buffer_free(output) << buffer_free(input);
				++returned;
				++expected_return;
			}
		}

		--node_count;
	}

	buffer_release(output);
	buffer_release(input);
}

TEST_F(TestTextEncoding, text_encoding_UTF8_from_code_page)
{
	std::vector<uint8_t> input_ascii;
	std::vector<uint8_t> expected_output;
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		input_ascii.resize(0);
		const auto input_hex(get_data_from_nodes(the_node, "input"));
		string_hex_parse(input_hex.c_str(), input_ascii);
		//
		ASSERT_FALSE(the_node.select_node("output").node().empty())
				<< buffer_free(output);
		const auto output_nodes = the_node.select_nodes("output");

		for (const auto& output_node : output_nodes)
		{
			const auto the_output_node = output_node.node();
			const auto code_page = static_cast<uint16_t>(the_output_node.attribute("code").as_int());
			auto expected_return = the_output_node.attribute("return").as_int();
			//
			ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
			ASSERT_EQ(expected_return,
					  text_encoding_UTF8_from_code_page(input_ascii.data(),
							  input_ascii.data() + input_ascii.size(), code_page, output)) << buffer_free(output);
			//
			expected_output.clear();
			string_hex_parse(the_output_node.child_value(), expected_output);
			//
			ASSERT_EQ(static_cast<ptrdiff_t>(expected_output.size()), buffer_size(output))
					<< "Code page: " << code_page << std::endl
					<< buffer_free(output);
			const auto* returned = buffer_uint8_t_data(output, 0);
			expected_return = 0;

			for (const auto& expected_code : expected_output)
			{
				ASSERT_EQ(expected_code, *returned)
						<< "Position at input: " << expected_return << std::endl
						<< "Code page: " << code_page << std::endl
						<< buffer_free(output);
				++returned;
				++expected_return;
			}
		}

		--node_count;
	}

	buffer_release(output);
}

TEST_F(TestTextEncoding, text_encoding_encode_UTF16)
{
	uint32_t input_digit;
	const auto start = &input_digit;
	const auto finish = start + 1;
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		const std::string expected_size_str(the_node.select_node("expected_size").node().child_value());
		auto input_in_a_range = string_to_range(expected_size_str);
		const auto expected_size =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string endian_str(the_node.select_node("endian").node().child_value());
		input_in_a_range = string_to_range(endian_str);
		const auto endian = static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string input(the_node.select_node("input").node().child_value());
		input_in_a_range = string_to_range(input);
		input_digit = static_cast<uint32_t>(uint64_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		ASSERT_TRUE(buffer_resize(output, 0))
				<< input << std::endl
				<< static_cast<uint32_t>(endian) << std::endl
				<< buffer_free(output);
		//
		ASSERT_TRUE(text_encoding_encode_UTF16(start, finish, endian, output))
				<< input << std::endl
				<< static_cast<uint32_t>(endian) << std::endl
				<< buffer_free(output);
		//
		ASSERT_EQ(expected_size, buffer_size(output))
				<< input << std::endl
				<< static_cast<uint32_t>(endian) << std::endl
				<< buffer_free(output);
		//
		const auto expected_outputs = the_node.select_nodes("output");
		ASSERT_EQ(expected_size / sizeof(uint16_t), expected_outputs.size())
				<< input << std::endl
				<< static_cast<uint32_t>(endian) << std::endl
				<< buffer_free(output);
		//
		input_digit = 0;

		for (const auto& out : expected_outputs)
		{
			const auto returned_output = buffer_uint16_t_data(output, input_digit);
			ASSERT_NE(nullptr, returned_output);
			//
			const std::string output_str(out.node().child_value());
			input_in_a_range = string_to_range(output_str);
			const auto expected_output =
				static_cast<uint16_t>(
					int_parse(input_in_a_range.start, input_in_a_range.finish));
			//
			ASSERT_EQ(expected_output, *returned_output)
					<< input << std::endl
					<< input_digit << std::endl
					<< static_cast<uint32_t>(endian) << std::endl
					<< buffer_free(output);
			//
			++input_digit;
		}

		--node_count;
	}

	buffer_release(output);
}

uint16_t fill_uint16_t(uint16_t max, uint16_t* result)
{
	if (nullptr == result)
	{
		return max;
	}

	for (uint16_t i = 0, j = 0; i < max; ++i)
	{
		if (127 < j)
		{
			j = 0;
		}

		result[i] = j++;
	}

	return max;
}

TEST(TestTextEncoding_, text_encoding_UTF16LE_to_UTF8)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (uint16_t i = 1, count = UINT8_MAX + 1; i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(output, 0))
				<< buffer_free(output);
		//
		const auto length = fill_uint16_t(i, nullptr);
		const auto size = buffer_size(output);
		//
		ASSERT_TRUE(buffer_append(
						output, nullptr, static_cast<ptrdiff_t>(4) * length + sizeof(uint32_t)))
				<< buffer_free(output);
		//
		uint16_t* dataW = reinterpret_cast<uint16_t*>(buffer_data(
							  output,
							  buffer_size(output) - sizeof(uint32_t) - sizeof(uint16_t) * length));
		ASSERT_NE(nullptr, dataW) << buffer_free(output);
		//
		ASSERT_EQ(length, fill_uint16_t(length, dataW))
				<< buffer_free(output);
		//
		const auto startW = reinterpret_cast<const uint16_t*>(dataW);
		const auto finishW = reinterpret_cast<const uint16_t*>(dataW) + length;
		//
		ASSERT_TRUE(buffer_resize(output, size))
				<< buffer_free(output);
		ASSERT_TRUE(text_encoding_UTF16LE_to_UTF8(startW, finishW, output))
				<< buffer_free(output);
		//
		const auto data = buffer_uint8_t_data(output, size);

		for (uint16_t a = 0, b = 0; a < length; ++a)
		{
			if (127 < b)
			{
				b = 0;
			}

			ASSERT_EQ(b++, data[a])
					<< a << std::endl
					<< buffer_free(output);
		}

		buffer_release(output);
	}
}

/*
text_encoding_UTF8_to_UTF16BE
text_encoding_UTF16BE_to_UTF8
text_encoding_UTF8_to_UTF32BE
text_encoding_UTF32BE_to_UTF8
text_encoding_get_one
*/