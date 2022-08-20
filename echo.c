/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "echo.h"

#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(_WIN32)
#define REQUIRED_UNICODE_CONSOLE_AT_WINDOWS(A) (Default != (A) && ASCII != (A))
#endif

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
	if (ECHO_UNKNOWN_LEVEL <= level)
	{
		return INVALID_HANDLE_VALUE;
	}

	if (Error != level)
	{
		static HANDLE output_stream = INVALID_HANDLE_VALUE;

		if (INVALID_HANDLE_VALUE == output_stream)
		{
			output_stream = GetStdHandle(STD_OUTPUT_HANDLE);
		}

		return output_stream;
	}

	static HANDLE error_output_stream = INVALID_HANDLE_VALUE;

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
uint8_t echo(
	uint8_t append, uint8_t encoding, const uint8_t* file, uint8_t level,
	const uint8_t* message, ptrdiff_t message_length, uint8_t new_line, uint8_t verbose)
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

		if (!result)
		{
			return 0;
		}
	}
	else
	{
		file_stream = (Error != level) ? common_get_output_stream() : common_get_error_output_stream();
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

		uint8_t new_message_buffer[BUFFER_SIZE_OF];
		void* new_message = (void*)new_message_buffer;

		if (!buffer_init(new_message, BUFFER_SIZE_OF))
		{
			return 0;
		}

		if (!file && is_output_standard && REQUIRED_UNICODE_CONSOLE_AT_WINDOWS(encoding))
		{
			if (!text_encoding_UTF8_to_UTF16LE(message, message + message_length, new_message))
			{
				return 0;
			}

			message_length = string_get_length(message, message + message_length);
			message = buffer_data(new_message, 0);
			result = file_flush(file_stream);
		}

		if (result)
#endif
		{
			if (NULL != file)
			{
				struct range message_in_a_range;
				message_in_a_range.start = message;
				message_in_a_range.finish = message + message_length;
				/**/
				result = file_write_with_encoding(&message_in_a_range, encoding, file_stream);
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
		buffer_release(new_message);
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
