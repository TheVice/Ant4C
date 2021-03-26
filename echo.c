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
#include "string_unit.h"
#include "text_encoding.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#if defined(_WIN32)
#define REQUIRED_UNICODE_CONSOLE_AT_WINDOWS(A) (Default != (A) && ASCII != (A))
#endif

#define ECHO_UNKNOWN_LEVEL (Warning + 1)

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

static uint8_t levels[] = { 0, 1, 1, 0, 0, 1 };

void echo_set_level(uint8_t level, uint8_t enable)
{
	if (COUNT_OF(levels) <= level)
	{
		return;
	}

	levels[level] = 0 < enable;
}

uint8_t echo_is_level_enabled(uint8_t level)
{
	if (COUNT_OF(levels) <= level)
	{
		return 0;
	}

	return levels[level];
}

#if defined(_WIN32)
HANDLE echo_get_win32_console_output(uint8_t level)
{
	static HANDLE output_stream = INVALID_HANDLE_VALUE;
	static HANDLE error_output_stream = INVALID_HANDLE_VALUE;

	if (ECHO_UNKNOWN_LEVEL <= level)
	{
		return INVALID_HANDLE_VALUE;
	}

	if (Error != level)
	{
		if (INVALID_HANDLE_VALUE == output_stream)
		{
			output_stream = GetStdHandle(STD_OUTPUT_HANDLE);
		}

		return output_stream;
	}

	if (INVALID_HANDLE_VALUE == error_output_stream)
	{
		error_output_stream = GetStdHandle(STD_ERROR_HANDLE);
	}

	return error_output_stream;
}

uint8_t echo_win32(HANDLE output, const uint16_t* message, uint16_t count_of_chars)
{
	if (INVALID_HANDLE_VALUE == output)
	{
		return 0;
	}

	BOOL result1 = 0;
	DWORD chars_that_was_written = 0;
	DWORD mode = 0;

	if (GetConsoleMode(output, &mode))
	{
		result1 = WriteConsoleW(output, message, count_of_chars, &chars_that_was_written, NULL);
	}
	else
	{
		result1 = WriteFile(output, message, sizeof(uint16_t) * count_of_chars, &chars_that_was_written, NULL);
		chars_that_was_written /= sizeof(uint16_t);
	}

	const uint8_t result2 = count_of_chars == chars_that_was_written;
	return result1 && result2;
}
#endif
uint8_t echo(uint8_t append, uint8_t encoding, const uint8_t* file,
			 uint8_t level, const uint8_t* message, ptrdiff_t message_length,
			 uint8_t new_line, uint8_t verbose)
{
	(void)verbose;/*TODO: */

	if (!file)
	{
		if (ECHO_UNKNOWN_LEVEL <= level)
		{
			return 0;
		}

		if (None == level ||
			!echo_is_level_enabled(level))
		{
			return 1;
		}
	}

	uint8_t result = 1;
	void* file_stream = NULL;

	if (NULL != file)
	{
		result = file_open(file, append ? (const uint8_t*)"ab" : (const uint8_t*)"wb", &file_stream);
	}
	else
	{
		file_stream = (Error != level) ? common_get_output_stream() : common_get_error_output_stream();
	}

	if (!result)
	{
		return 0;
	}

	if (NULL != message && 0 < message_length)
	{
#if defined(_WIN32)
		uint8_t is_output_standard = 1;

		if (!file)
		{
			is_output_standard = (Error != level) ? common_is_output_stream_standard() :
								 common_is_error_output_stream_standard();
		}

		struct buffer new_message;

		SET_NULL_TO_BUFFER(new_message);

		if (!file && is_output_standard && REQUIRED_UNICODE_CONSOLE_AT_WINDOWS(encoding))
		{
			if (!text_encoding_UTF8_to_UTF16LE(message, message + message_length, &new_message))
			{
				return 0;
			}

			/*message_length = buffer_size(&new_message) / sizeof(uint16_t);*/
			message_length = string_get_length(message, message + message_length);
			message = buffer_data(&new_message, 0);
			result = file_flush(file_stream);
		}

		if (result)
#endif
		{
			if (NULL != file)
			{
				struct range message_in_range;
				message_in_range.start = message;
				message_in_range.finish = message + message_length;
				/**/
				result = file_write_with_encoding(&message_in_range, encoding, file_stream);
			}
			else
			{
#if defined(_WIN32)
				result = (is_output_standard && REQUIRED_UNICODE_CONSOLE_AT_WINDOWS(encoding)) ?
						 echo_win32(echo_get_win32_console_output(level), (const uint16_t*)message, (uint16_t)message_length) :
						 (message_length == (ptrdiff_t)file_write(message, sizeof(uint8_t), message_length, file_stream));
#else
				result = (message_length == (ptrdiff_t)file_write(message, sizeof(uint8_t), message_length, file_stream));
#endif
			}
		}

#if defined(_WIN32)
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
		result = result && (1 == file_write((const uint8_t*)"\n", sizeof(uint8_t), 1, file_stream));
	}

	file_stream = NULL;
	return result;
}

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

	if (!file)
	{
		const struct buffer* level_in_buffer = buffer_buffer_data(task_arguments, LEVEL_POSITION);

		if (buffer_size(level_in_buffer))
		{
			struct range value;
			value.start = buffer_data(level_in_buffer, 0);
			value.finish = value.start + buffer_size(level_in_buffer);
			/**/
			level = echo_get_level(value.start, value.finish);
		}
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
