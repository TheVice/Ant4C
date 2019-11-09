/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _TEXT_ENCODING_H_
#define _TEXT_ENCODING_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

enum TextEncoding { ASCII, UTF7, UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE, BigEndianUnicode, Unicode, UTF32, Default };

#define TEXT_ENCODING_UNKNOWN (Default + 1)

enum CodePageID
{
	windows_874 = 874,
	windows_1250 = 1250,
	windows_1251 = 1251,
	windows_1252 = 1252,
	windows_1253 = 1253,
	windows_1254 = 1254,
	windows_1255 = 1255,
	windows_1256 = 1256,
	windows_1257 = 1257,
	windows_1258 = 1258
};

uint8_t text_encoding_get_BOM(
	uint8_t encoding, struct buffer* output);

uint8_t text_encoding_get_data_encoding_by_BOM(
	const uint8_t* data, ptrdiff_t data_length);

uint8_t text_encoding_UTF16LE_from_ASCII(
	const uint8_t* data_start, const uint8_t* data_finish,
	struct buffer* output);
uint8_t text_encoding_UTF16BE_from_ASCII(
	const uint8_t* data_start, const uint8_t* data_finish,
	struct buffer* output);

uint8_t text_encoding_UTF16LE_from_code_page(
	const uint8_t* data_start, const uint8_t* data_finish,
	uint16_t code_page, struct buffer* output);

uint8_t text_encoding_encode_UTF8(
	const uint32_t* data_start, const uint32_t* data_finish,
	struct buffer* output);
uint8_t text_encoding_decode_UTF8(
	const uint8_t* data_start, const uint8_t* data_finish,
	struct buffer* output);

uint8_t text_encoding_encode_UTF16LE(
	const uint32_t* data_start, const uint32_t* data_finish,
	struct buffer* output);
uint8_t text_encoding_decode_UTF16LE(
	const uint16_t* data_start, const uint16_t* data_finish,
	struct buffer* output);

uint8_t text_encoding_UTF8_to_UTF16LE(const uint8_t* data_start, const uint8_t* data_finish,
									  struct buffer* output);
#if defined(_WIN32)
uint8_t text_encoding_UTF16LE_to_UTF8(const uint16_t* data_start, const uint16_t* data_finish,
									  struct buffer* output);
#endif

uint8_t text_encoding_get_one(const uint8_t* encoding_start, const uint8_t* encoding_finish);

#endif
