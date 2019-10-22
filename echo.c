/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "echo.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "interpreter.h"
#include "range.h"
#include "xml.h"

#include <stdio.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#define INFO_LABEL				"[Info]: "
#define DEBUG_LABEL				"[Debug]: "
#define ERROR_LABEL				"[Error]: "
#define VERBOSE_LABEL			"[Verbose]: "
#define WARNING_LABEL			"[Warning]: "

#define INFO_LENGTH				8
#define DEBUG_LENGTH			9
#define ERROR_LENGTH			9
#define VERBOSE_LENGTH			11
#define WARNING_LENGTH			11

uint8_t echo(uint8_t append, uint8_t encoding, const char* file,
			 uint8_t level, const char* message, ptrdiff_t message_length,
			 uint8_t new_line, uint8_t verbose)
{
	uint8_t result = 0;
	FILE* file_stream = NULL;
	/**/
	(void)encoding;
	(void)verbose;/*TODO: */

	if (None == level)
	{
		return 1;
	}

	if (NULL != file)
	{
#if __STDC_SEC_API__
		result = (0 == fopen_s(&file_stream, file, append ? "ab" : "wb") && NULL != file_stream);
#else
		file_stream = fopen(file, append ? "ab" : "wb");
		result = (NULL != file_stream);
#endif
	}
	else
	{
		size_t was_written = 0;
		file_stream = (Error != level) ? stdout : stderr;

		switch (level)
		{
			case Error:
				was_written = fwrite(ERROR_LABEL, sizeof(char), ERROR_LENGTH, file_stream);
				result = (ERROR_LENGTH == was_written);
				break;

			case Debug:
				was_written = fwrite(DEBUG_LABEL, sizeof(char), DEBUG_LENGTH, file_stream);
				result = (DEBUG_LENGTH == was_written);
				break;

			case Info:
				was_written = fwrite(INFO_LABEL, sizeof(char), INFO_LENGTH, file_stream);
				result = (INFO_LENGTH == was_written);
				break;

			case Verbose:
				was_written = fwrite(VERBOSE_LABEL, sizeof(char), VERBOSE_LENGTH, file_stream);
				result = (VERBOSE_LENGTH == was_written);
				break;

			case Warning:
				was_written = fwrite(WARNING_LABEL, sizeof(char), WARNING_LENGTH, file_stream);
				result = (WARNING_LENGTH == was_written);
				break;

			case NoLevel:
				result = 1;
				break;

			default:
				return 0;
		}
	}

	if (result)
	{
		if (NULL != message &&
			0 < message_length)
		{
			result = (message_length == (ptrdiff_t)fwrite(message, sizeof(char), message_length, file_stream));
		}
	}
	else
	{
		return 0;
	}

	if (NULL != file)
	{
		result = result && (0 == fclose(file_stream));
	}
	else if (result && new_line)
	{
		result = result && (1 == fwrite("\n", sizeof(char), 1, file_stream));
	}

	file_stream = NULL;
	return result;
}

#define APPEND_POSITION			0
#define ENCODING_POSITION		1
#define FILE_POSITION			2
#define LEVEL_POSITION			3
#define MESSAGE_POSITION		4
#define FAIL_ON_ERROR_POSITION	5
#define VERBOSE_POSITION		6

uint8_t echo_get_arguments_for_task(
	const void* project,
	const void* target,
	const char* attributes_start,
	const char* attributes_finish,
	const char* element_finish,
	struct buffer* arguments)
{
	if (range_in_parts_is_null_or_empty(attributes_start, attributes_finish) ||
		NULL == element_finish || element_finish < attributes_finish ||
		NULL == arguments)
	{
		return 0;
	}

	static const char* attributes[] = { "append", "encoding", "file", "level", "message", "failonerror", "verbose" };
	static const uint8_t attributes_lengths[] = { 6, 8, 4, 5, 7, 11, 7 };
	/**/
	static const uint8_t default_append_value = 0;
	static const char* default_encoding_value = "UTF8";
	static const uint8_t default_encoding_value_length = 4;
	static const char* default_file_value = NULL;
	static const char* defalut_level_value = "Info";
	static const uint8_t defalut_level_value_length = 4;
	static const char* default_message_value = NULL;
	static const uint8_t default_fail_on_error_value = 1;
	static const uint8_t default_verbose_value = 0;
	/**/
	struct buffer argument;
	SET_NULL_TO_BUFFER(argument);

	if (!bool_to_string(default_append_value, &argument) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_encoding_value, default_encoding_value_length) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_file_value, 0) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, defalut_level_value, defalut_level_value_length) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_message_value, 0) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!bool_to_string(default_fail_on_error_value, &argument) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!bool_to_string(default_verbose_value, &argument) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	if (!interpreter_get_arguments_from_xml_tag_record(project, target, attributes_start, attributes_finish,
			attributes, attributes_lengths, sizeof(attributes_lengths) / sizeof(*attributes_lengths), arguments))
	{
		return 0;
	}

	struct buffer* message = buffer_buffer_data(arguments, MESSAGE_POSITION);

	if (NULL == message)
	{
		return 0;
	}

	if (default_message_value == buffer_char_data(message, 0))
	{
		struct range value;

		if (!xml_get_element_value_from_parts(attributes_finish, element_finish, &value))
		{
			return 0;
		}

		if ((value.start < value.finish) && (!buffer_resize(message, 0) ||
											 !buffer_append_data_from_range(message, &value)))
		{
			return 0;
		}
	}

	return 1;
}

static const char* echo_encoding_str[] = { "UTF7", "BigEndianUnicode", "Unicode", "Default", "ASCII", "UTF8", "UTF32" };
#define ECHO_UNKNOWN_ENCODING (UTF32 + 1)

uint8_t echo_get_encoding(const char* encoding_start, const char* encoding_finish)
{
	return common_string_to_enum(encoding_start, encoding_finish, echo_encoding_str, ECHO_UNKNOWN_ENCODING);
}

static const char* echo_level_str[] = { "Debug", "Error", "Info", "None", "Verbose", "Warning", "NoLevel" };
#define ECHO_UNKNOWN_LEVEL (NoLevel + 1)

uint8_t echo_get_level(const char* level_start, const char* level_finish)
{
	return common_string_to_enum(level_start, level_finish, echo_level_str, ECHO_UNKNOWN_LEVEL);
}

uint8_t echo_evaluate_task(const void* project, const void* target,
						   const char* attributes_start, const char* attributes_finish,
						   const char* element_finish)
{
	struct buffer arguments;
	SET_NULL_TO_BUFFER(arguments);

	if (!buffer_resize(&arguments, UINT8_MAX) || !buffer_resize(&arguments, 0))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	if (!echo_get_arguments_for_task(project, target,
									 attributes_start, attributes_finish, element_finish, &arguments))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	uint8_t append = 0;

	if (!common_unbox_bool_data(&arguments, APPEND_POSITION, 0, &append))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	struct range value;

	value.start = value.finish = NULL;

	if (!common_unbox_char_data(&arguments, ENCODING_POSITION, 0, &value, 0))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	const uint8_t encoding = echo_get_encoding(value.start, value.finish);

	if (ECHO_UNKNOWN_ENCODING == encoding)
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	const char* file = NULL;

	if (common_unbox_char_data(&arguments, FILE_POSITION, 0, &value, 1))
	{
		file = value.start;
	}

	if (!common_unbox_char_data(&arguments, LEVEL_POSITION, 0, &value, 0))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	const uint8_t level = echo_get_level(value.start, value.finish);

	if (ECHO_UNKNOWN_LEVEL == level)
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	if (!common_unbox_char_data(&arguments, MESSAGE_POSITION, 0, &value, 0))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	const char* message = value.start;
	const ptrdiff_t message_length = range_size(&value);
	/**/
	uint8_t fail_on_error = 1;
	uint8_t new_line = 1;
	uint8_t verbose = 0;

	if (!common_unbox_bool_data(&arguments, FAIL_ON_ERROR_POSITION, 0, &fail_on_error) ||
		!common_unbox_bool_data(&arguments, VERBOSE_POSITION, 0, &verbose))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	new_line = echo(append, encoding, file, level, message, message_length, new_line, verbose);
	buffer_release_with_inner_buffers(&arguments);
	/*TODO: comment about fail_on_error, if verbose mode,
	and manipulate of return value.
	return fail_on_error ? new_line : 1;*/
	return new_line;
}
