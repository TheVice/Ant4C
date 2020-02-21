/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "echo.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "range.h"
#include "text_encoding.h"

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#include <wchar.h>
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#if defined(_WIN32)
#define REQUIRED_UNICODE_CONSOLE_AT_WINDOWS(A) (Default != (A) && ASCII != (A))
#endif

static const uint8_t* echo_attributes[] =
{
	(const uint8_t*)"append",
	(const uint8_t*)"encoding",
	(const uint8_t*)"file",
	(const uint8_t*)"level",
	(const uint8_t*)"message"
};

static const uint8_t echo_attributes_lengths[] = { 6, 8, 4, 5, 7 };

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

static const uint8_t* echo_labels[] =
{
	(const uint8_t*)"[Debug]: ",
	(const uint8_t*)"[Error]: ",
	(const uint8_t*)"[Info]: ",
	(const uint8_t*)"",/*None*/
	(const uint8_t*)"[Verbose]: ",
	(const uint8_t*)"[Warning]: ",
	(const uint8_t*)""/*NoLevel*/
};

static const uint8_t echo_labels_lengths[] = { 9, 9, 8, 0, 11, 11, 0 };

uint8_t echo(uint8_t append, uint8_t encoding, const uint8_t* file,
			 uint8_t level, const uint8_t* message, ptrdiff_t message_length,
			 uint8_t new_line, uint8_t verbose)
{
	(void)verbose;/*TODO: */

	if (NoLevel < level)
	{
		return 0;
	}

	if (None == level)
	{
		return 1;
	}

	uint8_t result = 0;
	void* file_stream = NULL;

	if (NULL != file)
	{
		result = file_open(file, append ? (const uint8_t*)"ab" : (const uint8_t*)"wb", &file_stream);
	}
	else
	{
		file_stream = (Error != level) ? common_get_output_stream() : common_get_error_output_stream();

		if (NoLevel == level)
		{
			result = 1;
		}
		else
		{
			result = (echo_labels_lengths[level] == file_write(echo_labels[level], sizeof(uint8_t),
					  echo_labels_lengths[level], file_stream));
		}
	}

	if (!result)
	{
		return 0;
	}

	if (NULL != message && 0 < message_length)
	{
#if defined(_WIN32)
		struct buffer new_message;
		SET_NULL_TO_BUFFER(new_message);
		int mode = 0;

		if (!file && REQUIRED_UNICODE_CONSOLE_AT_WINDOWS(encoding))
		{
			if (!buffer_assing_to_uint16(&new_message, message, message_length))
			{
				return 0;
			}

			result = file_flush(file_stream);

			if (result)
			{
				mode = _setmode(_file_fileno(file_stream), _O_U8TEXT);
				message = buffer_data(&new_message, 0);
				message_length = buffer_size(&new_message);
			}
		}

#endif

		if (result)
		{
			result = (message_length == (ptrdiff_t)file_write(message, sizeof(uint8_t), message_length, file_stream));
		}

#if defined(_WIN32)

		if (0 < mode)
		{
			uint8_t previous_result = result;
			result = file_flush(file_stream);
			previous_result = previous_result & result;
#if defined(_MSC_VER)
			result = (_O_U8TEXT == _setmode(_file_fileno(file_stream), mode));
#else
			mode = _setmode(_file_fileno(file_stream), mode);
			result = (_O_U8TEXT == mode || -1 != mode);
#endif
			result = result & previous_result;
		}

		buffer_release(&new_message);
#endif
	}

	if (NULL != file)
	{
		const uint8_t previous_result = file_close(file_stream);
		result = result && previous_result;
	}
	else if (result && new_line)
	{
		result = result && (1 == file_write("\n", sizeof(uint8_t), 1, file_stream));
	}

	file_stream = NULL;
	return result;
}

#define ECHO_UNKNOWN_LEVEL (NoLevel + 1)

#define APPEND_POSITION			0
#define ENCODING_POSITION		1
#define FILE_POSITION			2
#define LEVEL_POSITION			3
#define MESSAGE_POSITION		4

uint8_t echo_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   echo_attributes, echo_attributes_lengths,
			   COUNT_OF(echo_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t echo_get_level(const uint8_t* level_start, const uint8_t* level_finish)
{
	return common_string_to_enum(level_start, level_finish, echo_levels, ECHO_UNKNOWN_LEVEL);
}

uint8_t echo_evaluate_task(struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	const struct buffer* append_in_buffer = buffer_buffer_data(task_arguments, APPEND_POSITION);
	const struct buffer* encoding_in_buffer = buffer_buffer_data(task_arguments, ENCODING_POSITION);
	struct buffer* file_path_in_buffer = buffer_buffer_data(task_arguments, FILE_POSITION);
	const struct buffer* level_in_buffer = buffer_buffer_data(task_arguments, LEVEL_POSITION);
	const struct buffer* message_in_buffer = buffer_buffer_data(task_arguments, MESSAGE_POSITION);
	/**/
	uint8_t append = 0;

	if (buffer_size(append_in_buffer) &&
		!bool_parse(buffer_data(append_in_buffer, 0), buffer_size(append_in_buffer), &append))
	{
		return 0;
	}

	uint8_t encoding = UTF8;

	if (buffer_size(encoding_in_buffer))
	{
		struct range value;
		value.start = buffer_data(encoding_in_buffer, 0);
		value.finish = value.start + buffer_size(encoding_in_buffer);
		/**/
		encoding = text_encoding_get_one(value.start, value.finish);
	}

	const uint8_t* file = NULL;

	if (buffer_size(file_path_in_buffer))
	{
		if (!buffer_push_back(file_path_in_buffer, 0))
		{
			return 0;
		}

		file = buffer_data(file_path_in_buffer, 0);
	}

	uint8_t level = Info;

	if (buffer_size(level_in_buffer))
	{
		struct range value;
		value.start = buffer_data(level_in_buffer, 0);
		value.finish = value.start + buffer_size(level_in_buffer);
		/**/
		level = echo_get_level(value.start, value.finish);
	}

	const uint8_t* message = NULL;
	ptrdiff_t message_length = 0;

	if (buffer_size(message_in_buffer))
	{
		/*TODO: string_trim for 'NULL == file'.*/
		message = buffer_data(message_in_buffer, 0);
		message_length = buffer_size(message_in_buffer);
	}

	uint8_t new_line = 1;
	return echo(append, encoding, file, level, message, message_length, new_line, verbose);
}
