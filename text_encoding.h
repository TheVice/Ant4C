/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _TEXT_ENCODING_H_
#define _TEXT_ENCODING_H_

#include <stddef.h>
#include <stdint.h>

#ifndef NO_BUFFER_UNIT
struct buffer;
#endif

enum TextEncoding { ASCII, UTF7, UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE, BigEndianUnicode, Unicode, UTF32, Default };

#define TEXT_ENCODING_UNKNOWN (Default + 1)

#ifndef NO_BUFFER_UNIT
enum CodePageID
{
	Windows_874 = 874,
	Windows_1250 = 1250,
	Windows_1251 = 1251,
	Windows_1252 = 1252,
	Windows_1253 = 1253,
	Windows_1254 = 1254,
	Windows_1255 = 1255,
	Windows_1256 = 1256,
	Windows_1257 = 1257,
	Windows_1258 = 1258
};

uint8_t text_encoding_get_BOM(
	uint8_t encoding, struct buffer* output);
#endif

uint8_t text_encoding_get_one_of_data_by_BOM(
	const uint8_t* data, ptrdiff_t data_length);

#ifndef NO_BUFFER_UNIT
uint8_t text_encoding_UTF_to_ASCII(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint8_t encoding, struct buffer* output);
uint8_t text_encoding_UTF_from_ASCII(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint8_t encoding, struct buffer* output);
#ifndef NO_COMMON_UNIT
uint8_t text_encoding_UTF16LE_from_code_page(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint16_t code_page, struct buffer* output);
#endif
#endif

uint8_t text_encoding_encode_UTF8_single(uint32_t input, uint8_t* output);
uint8_t text_encoding_decode_UTF8_single(
	const uint8_t* input_start, const uint8_t* input_finish,
	uint32_t* output);

#ifndef NO_BUFFER_UNIT
uint8_t text_encoding_encode_UTF8(
	const uint32_t* data_start, const uint32_t* data_finish,
	struct buffer* output);
uint8_t text_encoding_decode_UTF8(
	const uint8_t* data_start, const uint8_t* data_finish,
	struct buffer* output);

uint8_t text_encoding_encode_UTF16(
	const uint32_t* data_start, const uint32_t* data_finish,
	uint8_t endian, struct buffer* output);
uint8_t text_encoding_decode_UTF16(
	const uint16_t* data_start, const uint16_t* data_finish,
	uint8_t endian, struct buffer* output);

uint8_t text_encoding_UTF8_to_code_page(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint16_t code_page, struct buffer* output);
uint8_t text_encoding_UTF8_from_code_page(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint16_t code_page, struct buffer* output);

uint8_t text_encoding_UTF8_to_UTF16BE(const uint8_t* data_start, const uint8_t* data_finish,
									  struct buffer* output);
uint8_t text_encoding_UTF8_to_UTF16LE(const uint8_t* data_start, const uint8_t* data_finish,
									  struct buffer* output);

uint8_t text_encoding_UTF16BE_to_UTF8(const uint16_t* data_start, const uint16_t* data_finish,
									  struct buffer* output);
uint8_t text_encoding_UTF16LE_to_UTF8(const uint16_t* data_start, const uint16_t* data_finish,
									  struct buffer* output);

uint8_t text_encoding_UTF8_to_UTF32BE(const uint8_t* data_start, const uint8_t* data_finish,
									  struct buffer* output);

uint8_t text_encoding_UTF32BE_to_UTF8(const uint32_t* data_start, const uint32_t* data_finish,
									  struct buffer* output);
#endif

#ifndef NO_COMMON_UNIT
uint8_t text_encoding_get_one(const uint8_t* encoding_start, const uint8_t* encoding_finish);
#endif

#endif
