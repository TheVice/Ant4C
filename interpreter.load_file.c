/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include "load_file.h"

#include "buffer.h"
#include "common.h"
#include "project.h"
#include "property.h"
#include "string_unit.h"
#include "text_encoding.h"

uint16_t load_file_get_encoding(void* encoding_name)
{
	static const uint8_t* code_pages[] =
	{
		(const uint8_t*)"WINDOWS_874",
		(const uint8_t*)"WINDOWS_1250",
		(const uint8_t*)"WINDOWS_1251",
		(const uint8_t*)"WINDOWS_1252",
		(const uint8_t*)"WINDOWS_1253",
		(const uint8_t*)"WINDOWS_1254",
		(const uint8_t*)"WINDOWS_1255",
		(const uint8_t*)"WINDOWS_1256",
		(const uint8_t*)"WINDOWS_1257",
		(const uint8_t*)"WINDOWS_1258"
	};
	/**/
	static const uint8_t* file_encodings[] =
	{
		(const uint8_t*)"ISO_8859_1",
		(const uint8_t*)"ISO_8859_2",
		/*(const uint8_t*)"ISO_8859_3",
		(const uint8_t*)"ISO_8859_4",
		(const uint8_t*)"ISO_8859_5",
		(const uint8_t*)"ISO_8859_6",*/
		(const uint8_t*)"ISO_8859_7",
		/*(const uint8_t*)"ISO_8859_8",*/
		(const uint8_t*)"ISO_8859_9",
		(const uint8_t*)"ISO_8859_11",
		(const uint8_t*)"ISO_8859_13"
	};

	if (NULL == encoding_name)
	{
		return FILE_ENCODING_UNKNOWN;
	}

	uint16_t result = (uint16_t)buffer_size(encoding_name);

	if (!buffer_append(encoding_name, NULL, (ptrdiff_t)2 * result + sizeof(uint32_t)) ||
		!buffer_resize(encoding_name, result))
	{
		return FILE_ENCODING_UNKNOWN;
	}

	const uint8_t* start = buffer_uint8_t_data(encoding_name, 0);
	const uint8_t* finish = start + result;

	if (!string_to_upper(start, finish, encoding_name))
	{
		return FILE_ENCODING_UNKNOWN;
	}

	finish = buffer_uint8_t_data(encoding_name, 0) + buffer_size(encoding_name);
	start = buffer_uint8_t_data(encoding_name, result);
	result = text_encoding_get_one(start, finish);

	if (TEXT_ENCODING_UNKNOWN != result)
	{
		return result;
	}

	static const uint8_t* encodings[] = { (const uint8_t*)"BIGENDIANUNICODE", (const uint8_t*)"UNICODE", (const uint8_t*)"DEFAULT" };
	result = common_string_to_enum(start, finish, encodings, 3);

	if (result < 3)
	{
		return (uint16_t)(0 == result ? BigEndianUnicode : (1 == result ? Unicode : Default));
	}

	result = common_string_to_enum(start, finish, code_pages, COUNT_OF(code_pages));

	if (COUNT_OF(code_pages) != result)
	{
		if (0 == result)
		{
			result += Windows_874;
		}
		else
		{
			result += Windows_1250 - 1;
		}

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

#define FILE_POSITION   0
#define PROPERTY_POSITION 1
#define ENCODING_POSITION 2

uint8_t load_file_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	static const uint8_t* attributes[] =
	{
		(const uint8_t*)"file",
		(const uint8_t*)"property",
		(const uint8_t*)"encoding"
	};
	/**/
	static const uint8_t attributes_lengths[] = { 4, 8, 8 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   attributes, attributes_lengths,
			   COUNT_OF(attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}
#if 0
uint8_t load_file_evaluate_task(void* project, void* task_arguments, uint8_t verbose)
{
	if (NULL == project ||
		NULL == task_arguments)
	{
		return 0;
	}

	void* file_path_in_a_buffer = buffer_buffer_data(task_arguments, FILE_POSITION);
	const void* property_in_a_buffer = buffer_buffer_data(task_arguments, PROPERTY_POSITION);
	void* encoding_name_in_a_buffer = buffer_buffer_data(task_arguments, ENCODING_POSITION);
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

	file = buffer_uint8_t_data(file_path_in_a_buffer, 0);
	const uint8_t* property_name = buffer_uint8_t_data(property_in_a_buffer, 0);
	uint16_t encoding = Default;

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
#endif