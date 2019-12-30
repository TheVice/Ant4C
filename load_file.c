/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "load_file.h"
#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include <stdio.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

uint8_t load_file_to_buffer(const uint8_t* path, struct buffer* content, uint16_t encoding, uint8_t verbose)
{
	if (NULL == path ||
		NULL == content ||
		FILE_ENCODING_UNKNOWN == encoding ||
		((ASCII != encoding) && /*TODO:*/
		 (UTF8 != encoding) &&
		 (Unicode != encoding) &&
		 (UTF16LE != encoding) &&
		 (UTF32 != encoding) &&
		 (UTF32LE != encoding)))
	{
		return 0;
	}

	const ptrdiff_t size = file_get_length(path);

	if (!size)
	{
		return 1;
	}

	FILE* file_stream = NULL;

	if (!file_open(path, (const uint8_t*)"rb", (void**)&file_stream))
	{
		fclose(file_stream);
		return 0;
	}

	if (!buffer_resize(content, size))
	{
		fclose(file_stream);
		return 0;
	}

	if (Binary != encoding)
	{
		if (!buffer_append(content, NULL, 2 * sizeof(uint32_t)))
		{
			fclose(file_stream);
			return 0;
		}
	}

	uint8_t returned = 0;
	uint8_t* ptr = buffer_data(content, 0);

	if (Binary != encoding)
	{
		while (returned != 2 * sizeof(uint32_t))
		{
			ptr[returned++] += '\0';
		}

		ptr += returned;
	}

#if __STDC_SEC_API__ && defined(_MSC_VER)
	returned = (size == (ptrdiff_t)fread_s(ptr, size, sizeof(uint8_t), size, file_stream));
#else
	returned = (size == (ptrdiff_t)fread(ptr, sizeof(uint8_t), size, file_stream));
#endif

	if (!returned)
	{
		fclose(file_stream);
		return 0;
	}

	returned = (0 == fclose(file_stream));

	if (!returned)
	{
		return 0;
	}

	if (Binary == encoding)
	{
		return 1;
	}

	switch (text_encoding_get_data_encoding_by_BOM(ptr, size))
	{
		case ASCII:
			if (ASCII == encoding)
			{
				return 1;
			}

			break;

		/*case UTF7:
			TODO:*/
		case UTF8:
			ptr[0] = ptr[1] = ptr[2] = '\0';
			return 1;

		case BigEndianUnicode:
		case UTF16BE:
			/*ptr[0] = ptr[1] = '\0';
			TODO:*/
			return 0;

		case Unicode:
		case UTF16LE:
		{
			const uint16_t* start = (const uint16_t*)ptr;
			const uint16_t* finish = (const uint16_t*)(ptr + size);
			returned = (uint8_t)(ptr - buffer_data(content, 0)) + 2; /*NOTE: 2 - do not convert and do not left BOM.*/

			if (!buffer_append(content, NULL, 4 * (finish - start) + sizeof(uint32_t)))
			{
				return 0;
			}

			ptr = buffer_data(content, returned);
			start = (const uint16_t*)ptr;
			finish = (const uint16_t*)(ptr + size);
			return buffer_resize(content, 0) && text_encoding_UTF16LE_to_UTF8(start, finish, content);
		}

		case UTF32BE:
			/*ptr[0] = ptr[1] = ptr[2] = ptr[3] = ' ';
			TODO:*/
			return 0;

		case UTF32:
		case UTF32LE:
			return buffer_resize(content, 0) &&
				   text_encoding_encode_UTF8((const uint32_t*)(ptr + 4), (const uint32_t*)(ptr + size), content);

		default:
			break;
	}

	switch (encoding)
	{
		case ASCII:
		case UTF8:
			return 1;

		case BigEndianUnicode:
		case UTF16BE:
			/*TODO:*/
			return 0;

		case Unicode:
		case UTF16LE:
		{
			const uint16_t* start = (const uint16_t*)ptr;
			const uint16_t* finish = (const uint16_t*)(ptr + size);

			if (!buffer_append(content, NULL, 4 * (finish - start) + sizeof(uint32_t)))
			{
				return 0;
			}

			ptr = buffer_data(content, 2 * sizeof(uint32_t));
			start = (const uint16_t*)ptr;
			finish = (const uint16_t*)(ptr + size);
			return buffer_resize(content, 0) && text_encoding_UTF16LE_to_UTF8(start, finish, content);
		}

		case UTF32BE:
			/*TODO:*/
			return 0;

		case UTF32:
		case UTF32LE:
			return buffer_resize(content, 0) &&
				   text_encoding_encode_UTF8((const uint32_t*)ptr, (const uint32_t*)(ptr + size), content);

		case Windows_874:
		case Windows_1250:
		case Windows_1251:
		case Windows_1252:
		case Windows_1253:
		case Windows_1254:
		case Windows_1255:
		case Windows_1256:
		case Windows_1257:
		case Windows_1258:
		case ISO_8859_1:
		case ISO_8859_2:
		case ISO_8859_3:
		case ISO_8859_4:
		case ISO_8859_5:
		case ISO_8859_6:
		case ISO_8859_7:
		case ISO_8859_8:
		case ISO_8859_9:
			/*TODO:*/
			return 0;

		case Default:
		default:
			break;
	}

	(void)verbose;
	return 0;
}

uint8_t load_file(const uint8_t* path, void* the_property, uint16_t encoding, uint8_t verbose)
{
	if (NULL == path ||
		NULL == the_property)
	{
		return 0;
	}

	struct buffer content;

	SET_NULL_TO_BUFFER(content);

	if (!load_file_to_buffer(path, &content, encoding, verbose))
	{
		buffer_release(&content);
		return 0;
	}

	struct range input_in_range;

	input_in_range.start = buffer_data(&content, 0);

	if (NULL == input_in_range.start)
	{
		buffer_release(&content);
		return 1;
	}

	input_in_range.finish = input_in_range.start + buffer_size(&content);

	if (!string_trim(&input_in_range))
	{
		buffer_release(&content);
		return 0;
	}

	if (!property_set_by_pointer(the_property, input_in_range.start, range_size(&input_in_range),
								 property_value_is_byte_array/*property_value_is_file_content*/, 0, 0, verbose))
	{
		buffer_release(&content);
		return 0;
	}

	buffer_release(&content);
	return 1;
}

uint16_t load_file_get_file_encoding(const uint8_t* encoding_start, const uint8_t* encoding_finish)
{
	static const uint8_t* code_pages[] =
	{
		(const uint8_t*)"windows_874",
		(const uint8_t*)"windows_1250",
		(const uint8_t*)"windows_1251",
		(const uint8_t*)"windows_1252",
		(const uint8_t*)"windows_1253",
		(const uint8_t*)"windows_1254",
		(const uint8_t*)"windows_1255",
		(const uint8_t*)"windows_1256",
		(const uint8_t*)"windows_1257",
		(const uint8_t*)"windows_1258"
	};
	/**/
	static const uint8_t* file_encodings[] =
	{
		(const uint8_t*)"iso_8859_1",
		(const uint8_t*)"iso_8859_2",
		(const uint8_t*)"iso_8859_3",
		(const uint8_t*)"iso_8859_4",
		(const uint8_t*)"iso_8859_5",
		(const uint8_t*)"iso_8859_6",
		(const uint8_t*)"iso_8859_7",
		(const uint8_t*)"iso_8859_8",
		(const uint8_t*)"iso_8859_9"
	};
	/**/
	struct buffer enc;
	SET_NULL_TO_BUFFER(enc);

	if (!string_to_lower(encoding_start, encoding_finish, &enc))
	{
		buffer_release(&enc);
		return FILE_ENCODING_UNKNOWN;
	}

	encoding_start = buffer_data(&enc, 0);
	encoding_finish = encoding_start + buffer_size(&enc);
	uint16_t result = text_encoding_get_one(encoding_start, encoding_finish);

	if (TEXT_ENCODING_UNKNOWN != result)
	{
		buffer_release(&enc);
		return result;
	}

	result = common_string_to_enum(encoding_start, encoding_finish, code_pages, COUNT_OF(code_pages));

	if (COUNT_OF(code_pages) != result)
	{
		result += Windows_874;
		buffer_release(&enc);
		return result;
	}

	result = common_string_to_enum(encoding_start, encoding_finish, file_encodings, COUNT_OF(file_encodings));

	if (COUNT_OF(file_encodings) != result)
	{
		result += ISO_8859_1;
		buffer_release(&enc);
		return result;
	}

	buffer_release(&enc);
	return FILE_ENCODING_UNKNOWN;
}
