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

#define UTF8_TO_UTF16LE(DATA_START, DATA_FINISH, OUTPUT)											\
	{																								\
		if (NULL == (DATA_START) ||																	\
			NULL == (DATA_FINISH) ||																\
			(DATA_FINISH) <= (DATA_START) ||														\
			NULL == (OUTPUT))																		\
		{																							\
			return 0;																				\
		}																							\
		\
		const ptrdiff_t size = buffer_size((OUTPUT));												\
		\
		if (!buffer_append((OUTPUT), NULL, sizeof(uint32_t) * ((DATA_FINISH) - (DATA_START))) ||	\
			!buffer_resize((OUTPUT), size) ||														\
			!text_encoding_decode_UTF8((DATA_START), (DATA_FINISH), (OUTPUT)))						\
		{																							\
			return 0;																				\
		}																							\
		\
		const ptrdiff_t size_ = buffer_size((OUTPUT));												\
		const uint32_t* start = (const uint32_t*)buffer_data((OUTPUT), size);						\
		const uint32_t* finish = (const uint32_t*)(buffer_data((OUTPUT), 0) + size_);				\
		\
		if (!text_encoding_encode_UTF16LE(start, finish, (OUTPUT)))									\
		{																							\
			return 0;																				\
		}																							\
		\
		const ptrdiff_t size__ = buffer_size((OUTPUT)) - size_;										\
		uint8_t* dst = buffer_data((OUTPUT), size);													\
		const uint8_t* src = buffer_data((OUTPUT), size_);											\
		MEM_CPY(dst, src, size__);																	\
		return buffer_resize((OUTPUT), size + size__);												\
	}

#define UTF16LE_TO_UTF8(DATA_START, DATA_FINISH, OUTPUT)											\
	{																								\
		if (NULL == (DATA_START) ||																	\
			NULL == (DATA_FINISH) ||																\
			(DATA_FINISH) <= (DATA_START) ||														\
			NULL == (OUTPUT))																		\
		{																							\
			return 0;																				\
		}																							\
		\
		const ptrdiff_t size = buffer_size((OUTPUT));												\
		\
		if (!buffer_append((OUTPUT), NULL, sizeof(uint32_t) * ((DATA_FINISH) - (DATA_START))) ||	\
			!buffer_resize((OUTPUT), size) ||														\
			!text_encoding_decode_UTF16LE((DATA_START), (DATA_FINISH), (OUTPUT)))					\
		{																							\
			return 0;																				\
		}																							\
		\
		const ptrdiff_t size_ = buffer_size((OUTPUT));												\
		const uint32_t* start = (const uint32_t*)buffer_data((OUTPUT), size);						\
		const uint32_t* finish = (const uint32_t*)(buffer_data((OUTPUT), 0) + size_);				\
		\
		if (!text_encoding_encode_UTF8(start, finish, (OUTPUT)))									\
		{																							\
			return 0;																				\
		}																							\
		\
		const ptrdiff_t size__ = buffer_size((OUTPUT)) - size_;										\
		uint8_t* dst = buffer_data((OUTPUT), size);													\
		const uint8_t* src = buffer_data((OUTPUT), size_);											\
		MEM_CPY(dst, src, size__);																	\
		return buffer_resize((OUTPUT), size + size__);												\
	}

uint8_t text_encoding_get_one(const uint8_t* encoding_start, const uint8_t* encoding_finish);

#endif
