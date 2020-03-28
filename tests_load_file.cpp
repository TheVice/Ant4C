/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "echo.h"
#include "load_file.h"
#include "path.h"
#include "text_encoding.h"
}

#include <string>
#include <vector>
#include <cstdint>
#include <ostream>

TEST(TestLoadFile, load_file_to_buffer)
{
	static const std::vector<uint8_t> byte_order_marks[] =
	{
		{ 0xEF, 0xBB, 0xBF },
		{ 0xFE, 0xFF },
		{ 0xFF, 0xFE },
		{ 0x00, 0x00, 0xFE, 0xFF },
		{ 0xFF, 0xFE, 0x00, 0x00 },
		{ }, { }, { }, { }, { },
		{ }, { }, { },
		{ }, { }, { }, { },
		{ }, { }, { }, { },
		{ }, { }, { }, { },
		{ }, { }, { }, { },
		{ }, { }
	};
	//
	static const uint16_t encodings[] =
	{
		UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE,
		UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE,
		BigEndianUnicode, Unicode, UTF32,
		Windows_874, ISO_8859_11, Windows_1250, ISO_8859_2,
		Windows_1251, Windows_1255, Windows_1256, Windows_1258,
		Windows_1252, ISO_8859_1, Windows_1253, ISO_8859_7,
		Windows_1254, ISO_8859_9, Windows_1257, ISO_8859_13,
		ASCII, Default
	};
	//
	ASSERT_EQ(COUNT_OF(byte_order_marks), COUNT_OF(encodings));
	//
	std::vector<uint8_t> file_content(33792, 0);
	//
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_file_name(&path)) << buffer_free(&path);
	const auto path_str(buffer_to_string(&path));

	for (uint8_t i = 0, count = COUNT_OF(encodings); i < count; ++i)
	{
		ASSERT_TRUE(echo(0, Default,
						 (const uint8_t*)path_str.c_str(), NoLevel,
						 byte_order_marks[i].data(), (ptrdiff_t)byte_order_marks[i].size(), 0, 0))
				<< buffer_free(&path);
		//
		file_content.clear();

		switch (encodings[i])
		{
			case UTF8:
			default:
				file_content.assign(8192, ' ');

				for (uint8_t j = 0; j < INT8_MAX; ++j)
				{
					file_content.push_back(j);
				}

				break;

			case BigEndianUnicode:
			case UTF16BE:
				for (uint16_t j = 0; j < 8192; ++j)
				{
					file_content.push_back(0);
					file_content.push_back(' ');
				}

				for (uint8_t j = 0; j < INT8_MAX; ++j)
				{
					file_content.push_back(0);
					file_content.push_back(j);
				}

				break;

			case Unicode:
			case UTF16LE:
				for (uint16_t j = 0; j < 8192; ++j)
				{
					file_content.push_back(' ');
					file_content.push_back(0);
				}

				for (uint8_t j = 0; j < INT8_MAX; ++j)
				{
					file_content.push_back(j);
					file_content.push_back(0);
				}

				break;

			case UTF32BE:
				for (uint16_t j = 0; j < 8192; ++j)
				{
					file_content.push_back(0);
					file_content.push_back(0);
					file_content.push_back(0);
					file_content.push_back(' ');
				}

				for (uint8_t j = 0; j < INT8_MAX; ++j)
				{
					file_content.push_back(0);
					file_content.push_back(0);
					file_content.push_back(0);
					file_content.push_back(j);
				}

				break;

			case UTF32:
			case UTF32LE:
				for (uint16_t j = 0; j < 8192; ++j)
				{
					file_content.push_back(' ');
					file_content.push_back(0);
					file_content.push_back(0);
					file_content.push_back(0);
				}

				for (uint8_t j = 0; j < INT8_MAX; ++j)
				{
					file_content.push_back(j);
					file_content.push_back(0);
					file_content.push_back(0);
					file_content.push_back(0);
				}

				break;
		}

		ASSERT_TRUE(echo(1, Default,
						 (const uint8_t*)path_str.c_str(), NoLevel, file_content.data(),
						 (ptrdiff_t)file_content.size(), 0, 0))
				<< buffer_free(&path);
		//
		ASSERT_TRUE(load_file_to_buffer((const uint8_t*)path_str.c_str(), encodings[i], &path, 0))
				<< buffer_free(&path);
		//
		file_content.clear();
		file_content.assign(8192, ' ');

		for (uint8_t j = 0; j < INT8_MAX; ++j)
		{
			file_content.push_back(j);
		}

		ASSERT_EQ((ptrdiff_t)file_content.size(), buffer_size(&path)) <<
				(int)i << std::endl << buffer_free(&path);
		//
		ASSERT_EQ(std::string((const char*)file_content.data(), file_content.size()),
				  buffer_to_string(&path)) << (int)i << std::endl << buffer_free(&path);
	}

	buffer_release(&path);
}

TEST(TestLoadFile, load_file_get_encoding)
{
	static const std::string encodings[] =
	{
		"ASCII",
		"UTF7",
		"UTF8",
		"UTF16BE",
		"UTF16LE",
		"UTF32BE",
		"UTF32LE",
		"BigEndianUnicode",
		"Unicode",
		"UTF32",
		"Default",
		//
		"Windows_874",
		"Windows_1250",
		"Windows_1251",
		"Windows_1252",
		"Windows_1253",
		"Windows_1254",
		"Windows_1255",
		"Windows_1256",
		"Windows_1257",
		"Windows_1258",
		//
		"ISO_8859_1",
		"ISO_8859_2",
		/*"ISO_8859_3",
		"ISO_8859_4",
		"ISO_8859_5",
		"ISO_8859_6",*/
		"ISO_8859_7",
		/*"ISO_8859_8",*/
		"ISO_8859_9",
		"ISO_8859_11",
		"ISO_8859_13"
	};
	//
	static const uint16_t expected_output[] =
	{
		ASCII,
		UTF7,
		UTF8,
		UTF16BE,
		UTF16LE,
		UTF32BE,
		UTF32LE,
		BigEndianUnicode,
		Unicode,
		UTF32,
		Default,
		//
		Windows_874,
		Windows_1250,
		Windows_1251,
		Windows_1252,
		Windows_1253,
		Windows_1254,
		Windows_1255,
		Windows_1256,
		Windows_1257,
		Windows_1258,
		//
		ISO_8859_1,
		ISO_8859_2,
		/*ISO_8859_3,
		ISO_8859_4,
		ISO_8859_5,
		ISO_8859_6,*/
		ISO_8859_7,
		/*ISO_8859_8,*/
		ISO_8859_9,
		ISO_8859_11,
		ISO_8859_13
	};
	//
	buffer encoding;
	SET_NULL_TO_BUFFER(encoding);

	for (uint8_t i = 0, count = COUNT_OF(expected_output); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&encoding, 0)) << buffer_free(&encoding);
		ASSERT_TRUE(string_to_buffer(encodings[i], &encoding)) << buffer_free(&encoding);
		ASSERT_EQ(expected_output[i], load_file_get_encoding(&encoding)) << (int)i << std::endl << buffer_free(
					&encoding);
	}

	buffer_release(&encoding);
}
