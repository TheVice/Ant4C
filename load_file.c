/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "load_file.h"
#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

uint8_t load_file_to_buffer(const uint8_t* path, uint16_t encoding, struct buffer* output, uint8_t verbose)
{
	(void)encoding;
	(void)verbose;
	return buffer_resize(output, 0) && file_read_all_bytes(path, output);
#if 0

	if (NULL == path ||
		NULL == output ||
		FILE_ENCODING_UNKNOWN == encoding ||
		((ASCII != encoding) && /*TODO:*/
		 (UTF8 != encoding) &&
		 (Unicode != encoding) &&
		 (UTF16LE != encoding) &&
		 (UTF32 != encoding) &&
		 (UTF32LE != encoding) &&
		 (Default != encoding)))
	{
		return 0;
	}

	const ptrdiff_t size = (ptrdiff_t)file_get_length(path);

	if (!size)
	{
		return buffer_resize(output, 0);
	}

	void* file_stream = NULL;

	if (!file_open(path, (const uint8_t*)"rb", &file_stream))
	{
		file_close(file_stream);
		return 0;
	}

	if (!buffer_resize(output, size))
	{
		file_close(file_stream);
		return 0;
	}

	if (Default != encoding)
	{
		if (!buffer_append(output, NULL, 2 * sizeof(uint32_t)))
		{
			file_close(file_stream);
			return 0;
		}
	}

	uint8_t returned = 0;
	uint8_t* ptr = buffer_data(output, 0);

	if (Default != encoding)
	{
		while (returned != 2 * sizeof(uint32_t))
		{
			ptr[returned++] += '\0';
		}

		ptr += returned;
	}

	returned = (size == (ptrdiff_t)file_read(ptr, sizeof(uint8_t), size, file_stream));

	if (!returned)
	{
		file_close(file_stream);
		return 0;
	}

	returned = file_close(file_stream);

	if (!returned)
	{
		return 0;
	}

	if (Default == encoding)
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
			returned = (uint8_t)(ptr - buffer_data(output, 0)) + 2; /*NOTE: 2 - do not convert and do not left BOM.*/

			if (!buffer_append(output, NULL, 4 * (finish - start) + sizeof(uint32_t)))
			{
				return 0;
			}

			ptr = buffer_data(output, returned);
			start = (const uint16_t*)ptr;
			finish = (const uint16_t*)(ptr + size);
			return buffer_resize(output, 0) && text_encoding_UTF16LE_to_UTF8(start, finish, output);
		}

		case UTF32BE:
			/*ptr[0] = ptr[1] = ptr[2] = ptr[3] = ' ';
			TODO:*/
			return 0;

		case UTF32:
		case UTF32LE:
			return buffer_resize(output, 0) &&
				   text_encoding_encode_UTF8((const uint32_t*)(ptr + 4), (const uint32_t*)(ptr + size), output);

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

			if (!buffer_append(output, NULL, 4 * (finish - start) + sizeof(uint32_t)))
			{
				return 0;
			}

			ptr = buffer_data(output, 2 * sizeof(uint32_t));
			start = (const uint16_t*)ptr;
			finish = (const uint16_t*)(ptr + size);
			return buffer_resize(output, 0) && text_encoding_UTF16LE_to_UTF8(start, finish, output);
		}

		case UTF32BE:
			/*TODO:*/
			return 0;

		case UTF32:
		case UTF32LE:
			return buffer_resize(output, 0) &&
				   text_encoding_encode_UTF8((const uint32_t*)ptr, (const uint32_t*)(ptr + size), output);

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
#endif
}

uint16_t load_file_get_encoding(struct buffer* encoding_name)
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

	if (NULL == encoding_name)
	{
		return 0;
	}

	uint16_t result = (uint16_t)buffer_size(encoding_name);

	if (!buffer_append(encoding_name, NULL, (ptrdiff_t)2 * result + sizeof(uint32_t)) ||
		!buffer_resize(encoding_name, result))
	{
		return 0;
	}

	const uint8_t* start = buffer_data(encoding_name, 0);
	const uint8_t* finish = start + result;

	if (!string_to_lower(start, finish, encoding_name))
	{
		return FILE_ENCODING_UNKNOWN;
	}

	finish = start + buffer_size(encoding_name);
	start = buffer_data(encoding_name, result);
	text_encoding_get_one(start, finish);

	if (TEXT_ENCODING_UNKNOWN != result)
	{
		return result;
	}

	result = common_string_to_enum(start, finish, code_pages, COUNT_OF(code_pages));

	if (COUNT_OF(code_pages) != result)
	{
		result += Windows_874;
		return result;
	}

	result = common_string_to_enum(start, finish, file_encodings, COUNT_OF(file_encodings));

	if (COUNT_OF(file_encodings) != result)
	{
		result += ISO_8859_1;
		return result;
	}

	return FILE_ENCODING_UNKNOWN;
}

#define FILE_POSITION		0
#define PROPERTY_POSITION	1
#define ENCODING_POSITION	2

static const uint8_t* attributes[] =
{
	(const uint8_t*)"file",
	(const uint8_t*)"property",
	(const uint8_t*)"encoding"
};

static const uint8_t attributes_lengths[] = { 4, 8, 8 };

uint8_t load_file_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   attributes, attributes_lengths,
			   COUNT_OF(attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t load_file_evaluate_task(void* project, struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == project ||
		NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* file_path_in_a_buffer = buffer_buffer_data(task_arguments, FILE_POSITION);

	const struct buffer* property_in_a_buffer = buffer_buffer_data(task_arguments, PROPERTY_POSITION);

	struct buffer* encoding_name_in_a_buffer = buffer_buffer_data(task_arguments, ENCODING_POSITION);

	uint8_t property_name_length = 0;

	if (!buffer_size(file_path_in_a_buffer) ||
		0 == (property_name_length = (uint8_t)buffer_size(property_in_a_buffer)))
	{
		return 0;
	}

	const uint8_t* file = NULL;

	if (!buffer_push_back(file_path_in_a_buffer, 0))
	{
		return 0;
	}

	file = buffer_data(file_path_in_a_buffer, 0);
	const uint8_t* property_name = buffer_data(property_in_a_buffer, 0);
	uint16_t encoding = ASCII;

	if (buffer_size(encoding_name_in_a_buffer))
	{
		encoding = load_file_get_encoding(encoding_name_in_a_buffer);

		if (FILE_ENCODING_UNKNOWN == encoding)
		{
			return 0;
		}
	}

	uint8_t dynamic = 0;
	uint8_t read_only = 0;
	/**/
	void* the_property = NULL;

	if (!project_property_set_value(project, property_name,
									property_name_length,
									(const uint8_t*)&the_property, 0, dynamic, 1, read_only, verbose) ||
		!project_property_exists(project, property_name,
								 property_name_length, &the_property, verbose))
	{
		return 0;
	}

	return property_set_from_file(the_property, file, encoding, dynamic, read_only, verbose);
}
