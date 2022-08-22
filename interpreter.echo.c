/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include "echo.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "text_encoding.h"

#define APPEND_POSITION			0
#define ENCODING_POSITION		1
#define FILE_POSITION			2
#define LEVEL_POSITION			3
#define MESSAGE_POSITION		4

uint8_t echo_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, uint8_t* task_arguments)
{
	static const uint8_t* echo_attributes[] =
	{
		(const uint8_t*)"append",
		(const uint8_t*)"encoding",
		(const uint8_t*)"file",
		(const uint8_t*)"level",
		(const uint8_t*)"message"
	};
	/**/
	static const uint8_t echo_attributes_lengths[] = { 6, 8, 4, 5, 7 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   echo_attributes, echo_attributes_lengths,
			   COUNT_OF(echo_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t echo_get_level(const uint8_t* level_start, const uint8_t* level_finish)
{
	static const uint8_t* echo_levels[] =
	{
		(const uint8_t*)"Debug",
		(const uint8_t*)"Error",
		(const uint8_t*)"Info",
		(const uint8_t*)"None",
		(const uint8_t*)"Verbose",
		(const uint8_t*)"Warning",
		(const uint8_t*)"NoLevel"
	};
	/**/
	return common_string_to_enum(level_start, level_finish, echo_levels, ECHO_UNKNOWN_LEVEL);
}

uint8_t echo_evaluate_task(void* task_arguments, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	const void* append_in_a_buffer = buffer_buffer_data(task_arguments, APPEND_POSITION);
	const void* encoding_in_a_buffer = buffer_buffer_data(task_arguments, ENCODING_POSITION);
	void* file_path_in_a_buffer = buffer_buffer_data(task_arguments, FILE_POSITION);
	const void* message_in_a_buffer = buffer_buffer_data(task_arguments, MESSAGE_POSITION);
	/**/
	uint8_t append = (uint8_t)buffer_size(append_in_a_buffer);
	const uint8_t* value = buffer_uint8_t_data(append_in_a_buffer, 0);

	if (append && !bool_parse(value, value + append, &append))
	{
		return 0;
	}

	uint8_t encoding = (uint8_t)buffer_size(encoding_in_a_buffer);

	if (encoding)
	{
		value = buffer_uint8_t_data(encoding_in_a_buffer, 0);
		encoding = text_encoding_get_one(value, value + encoding);
	}
	else
	{
		encoding = UTF8;
	}

	const uint8_t* file = NULL;

	if (buffer_size(file_path_in_a_buffer))
	{
		if (!buffer_push_back(file_path_in_a_buffer, 0))
		{
			return 0;
		}

		file = buffer_uint8_t_data(file_path_in_a_buffer, 0);
	}

	uint8_t level = Info;

	if (!file)
	{
		const void* level_in_a_buffer = buffer_buffer_data(task_arguments, LEVEL_POSITION);
		level = (uint8_t)buffer_size(level_in_a_buffer);

		if (level)
		{
			value = buffer_uint8_t_data(level_in_a_buffer, 0);
			level = echo_get_level(value, value + level);
		}
		else
		{
			level = Info;
		}
	}

	value = buffer_uint8_t_data(message_in_a_buffer, 0);
	ptrdiff_t message_length = buffer_size(message_in_a_buffer);
	/*if (message_length)
	{
		TODO: string_trim for 'NULL == file'.
	}*/
	uint8_t new_line = 1;
	return echo(append, encoding, file, level, value, message_length, new_line, verbose);
}

