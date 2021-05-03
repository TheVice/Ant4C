/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 TheVice
 *
 */

#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "load_file.h"
#include "math_unit.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include <string.h>

static const uint8_t equal_symbol = '=';
static const uint8_t quote_symbol = '"';
static const uint8_t space_symbol = ' ';
static const uint8_t zero_symbol = '\0';

static const uint8_t* properties_names[] =
{
	(const uint8_t*)"target",
	(const uint8_t*)"D",
	(const uint8_t*)"indent",
	/**/
	(const uint8_t*)"f",
	(const uint8_t*)"buildfile",
	/**/
	(const uint8_t*)"l",
	(const uint8_t*)"logfile",
	/**/
	(const uint8_t*)"pause",
	(const uint8_t*)"debug",
	/**/
	(const uint8_t*)"q",
	(const uint8_t*)"quiet",
	/**/
	(const uint8_t*)"h",
	(const uint8_t*)"help",
	/**/
	(const uint8_t*)"projecthelp",
	(const uint8_t*)"nologo",
	(const uint8_t*)"encoding",
	(const uint8_t*)"listener",
	(const uint8_t*)"modulepriority"
};

static const uint8_t properties_names_lengths[] =
{
	6, 1, 6,
	/**/
	1, 9,
	/**/
	1, 7,
	/**/
	5, 5,
	/**/
	1, 5,
	/**/
	1, 4,
	/**/
	11, 6, 8, 8, 14
};

#define TARGET_POSITION					0
#define PROPERTY_POSITION				1
#define INDENT_POSITION					2

#define BUILD_FILES_POSITION_1			3
#define BUILD_FILES_POSITION_2			4

#define LOG_FILE_POSITION_1				5
#define LOG_FILE_POSITION_2				6

#define PAUSE_POSITION					7
#define DEBUG_POSITION					8

#define QUIET_POSITION_1				9
#define QUIET_POSITION_2				10

#define HELP_POSITION_1					11
#define HELP_POSITION_2					12

#define PROJECT_HELP_POSITION			13
#define NO_LOGO_POSITION				14
#define ENCODING_POSITION				15
#define LISTENER_POSITION				16
#define MODULE_PRIORITY_POSITION		17

#define PROPERTIES_COUNT	(MODULE_PRIORITY_POSITION + 1)

uint8_t argument_parser_get_verbose_char(int i, int argc, char** argv)
{
	uint8_t verbose = 0;

	if (NULL == argv)
	{
		return verbose;
	}

	for (; i < argc; ++i)
	{
		if ('-' != argv[i][0])
		{
			continue;
		}

		const size_t length = strlen(argv[i]);

		if ((9 == length && (0 == memcmp(argv[i], "-verbose+", 9) ||
							 0 == memcmp(argv[i], "-verbose-", 9))) ||
			(8 == length && (0 == memcmp(argv[i], "-verbose", 8))) ||
			(3 == length && (0 == memcmp(argv[i], "-v+", 3) ||
							 0 == memcmp(argv[i], "-v-", 3))) ||
			(2 == length && (0 == memcmp(argv[i], "-v", 2))))
		{
			const char ch = argv[i][length - 1];

			if ('+' == ch)
			{
				verbose = 1;
			}
			else if ('-' == ch)
			{
				verbose = 0;
			}
			else
			{
				verbose = 1;
			}

			break;
		}
	}

	return verbose;
}
#if defined(_WIN32)
uint8_t argument_parser_get_verbose_wchar_t(int i, int argc, wchar_t** argv)
{
	uint8_t verbose = 0;

	if (NULL == argv)
	{
		return verbose;
	}

	for (; i < argc; ++i)
	{
		if (L'-' != argv[i][0])
		{
			continue;
		}

		const size_t length = wcslen(argv[i]);

		if ((9 == length && (0 == wmemcmp(argv[i], L"-verbose+", 9) ||
							 0 == wmemcmp(argv[i], L"-verbose-", 9))) ||
			(8 == length && (0 == wmemcmp(argv[i], L"-verbose", 8))) ||
			(3 == length && (0 == wmemcmp(argv[i], L"-v+", 3) ||
							 0 == wmemcmp(argv[i], L"-v-", 3))) ||
			(2 == length && (0 == wmemcmp(argv[i], L"-v", 2))))
		{
			const wchar_t ch = argv[i][length - 1];

			if (L'+' == ch)
			{
				verbose = 1;
			}
			else if (L'-' == ch)
			{
				verbose = 0;
			}
			else
			{
				verbose = 1;
			}

			break;
		}
	}

	return verbose;
}
#endif
uint8_t argument_get_key_and_value(
	const uint8_t* input_start, const uint8_t* input_finish,
	struct range* key, struct range* value)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == key || NULL == value)
	{
		return 0;
	}

	value->start = find_any_symbol_like_or_not_like_that(input_start, input_finish,
				   &equal_symbol, 1, 1, 1);

	if (input_finish == value->start)
	{
		return 0;
	}

	key->finish = value->start;
	value->start = find_any_symbol_like_or_not_like_that(value->start + 1, input_finish,
				   &quote_symbol, 1, 0, 1);
	value->finish = 1 + find_any_symbol_like_or_not_like_that(input_finish - 1, value->start,
					&quote_symbol, 1, 0, -1);
	/**/
	key->start = find_any_symbol_like_or_not_like_that(input_start, key->finish,
				 &quote_symbol, 1, 0, 1);
	key->finish = 1 + find_any_symbol_like_or_not_like_that(key->finish - 1, key->start,
				  &quote_symbol, 1, 0, -1);
	/**/
	return !range_is_null_or_empty(key) && string_trim(key) && value->start <= value->finish;
}

uint8_t argument_parser_buffer_to_properties(const struct buffer* input, struct buffer* properties,
		uint8_t verbose)
{
	const uint8_t* start = buffer_data(input, 0);
	const uint8_t* previous_pos = start;
	const uint8_t* finish = start + buffer_size(input);

	while ((start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 1, 1)) < finish)
	{
		struct range key;
		struct range value;

		if (!argument_get_key_and_value(
				previous_pos, start, &key, &value))
		{
			return 0;
		}

		if (!property_set_by_name(properties,
								  key.start, (uint8_t)range_size(&key),
								  value.start, range_size(&value),
								  property_value_is_byte_array,
								  1, 1, 1, verbose))
		{
			return 0;
		}

		start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 0, 1);
		previous_pos = start;
	}

	return 1;
}

uint8_t argument_parser_append_property(struct buffer* properties_, const uint8_t* name, uint8_t name_length,
										struct buffer* argument_value, uint8_t append_property_value, uint8_t verbose)
{
	void* the_property = NULL;
	ptrdiff_t size = buffer_size(argument_value);

	if (size)
	{
		if (!buffer_push_back(argument_value, 0))
		{
			return 0;
		}

		++size;
	}

	if (!property_exists(properties_, name, name_length, &the_property))
	{
		return property_set_by_name(properties_, name, name_length,
									size ? buffer_data(argument_value, 0) : (const uint8_t*)&size, size,
									property_value_is_byte_array, 0, 0, 0, verbose);
	}

	size = buffer_size(argument_value);

	if (!property_get_by_pointer(the_property, argument_value))
	{
		return 0;
	}

	ptrdiff_t new_size = buffer_size(argument_value);

	if (append_property_value && size < new_size)
	{
		if (!buffer_push_back(argument_value, 0))
		{
			return 0;
		}

		++new_size;

		if (!buffer_append(argument_value, NULL, size))
		{
			return 0;
		}

		const uint8_t* src = buffer_data(argument_value, 0);
		uint8_t* dst = buffer_data(argument_value, new_size);
		MEM_CPY(dst, src, size);
		/**/
		dst = buffer_data(argument_value, 0);
		src = buffer_data(argument_value, size);
		MEM_CPY(dst, src, new_size - size);
		/**/
		src = buffer_data(argument_value, new_size);
		dst = buffer_data(argument_value, new_size - size);
		MEM_CPY(dst, src, size);

		if (!buffer_resize(argument_value, new_size))
		{
			return 0;
		}
	}

	size = buffer_size(argument_value);
	return property_set_by_pointer(the_property,
								   size ? buffer_data(argument_value, 0) : (const uint8_t*)&size, size,
								   property_value_is_byte_array, 0, 0, verbose);
}

uint8_t argument_parser_char(int i, int argc, char** argv, struct buffer* arguments, uint8_t verbose)
{
	if ((i < argc && NULL == argv) ||
		NULL == arguments)
	{
		return 0;
	}

	struct buffer argument_value;

	SET_NULL_TO_BUFFER(argument_value);

	for (; i < argc; ++i)
	{
		const size_t length = strlen(argv[i]);

		if (!length || '@' == argv[i][0])
		{
			continue;
		}

		if (!buffer_resize(&argument_value, 0))
		{
			buffer_release(&argument_value);
			return 0;
		}

		if (1 < length && ('-' == argv[i][0] || '/' == argv[i][0]))
		{
			static const uint8_t colon_mark = ':';
			const uint8_t* start = (const uint8_t*)argv[i] + 1;
			const uint8_t* finish = (const uint8_t*)argv[i] + length;
			const uint8_t* middle = find_any_symbol_like_or_not_like_that(start, finish, &colon_mark, 1, 1, 1);

			if (middle != finish)
			{
				middle = find_any_symbol_like_or_not_like_that(middle, finish, &colon_mark, 1, 0, 1);

				if (middle == finish)
				{
					buffer_release(&argument_value);
					return 0;
				}

				const ptrdiff_t value_length = finish - middle;

				if (!buffer_append(&argument_value, middle, value_length))
				{
					buffer_release(&argument_value);
					return 0;
				}

				middle = find_any_symbol_like_or_not_like_that(middle, start, &colon_mark, 1, 1, -1);
				ptrdiff_t name_length = middle - start;

				if (UINT8_MAX < name_length)
				{
					buffer_release(&argument_value);
					return 0;
				}

				uint8_t property_number = common_string_to_enum(start, middle, properties_names + 1, PROPERTIES_COUNT - 1);

				if (PROPERTIES_COUNT - 1 == property_number)
				{
					continue;
				}

				property_number = (BUILD_FILES_POSITION_1 - 1 == property_number ||
								   BUILD_FILES_POSITION_2 - 1 == property_number);

				if (property_number)
				{
					start = properties_names[BUILD_FILES_POSITION_1];
					name_length = properties_names_lengths[BUILD_FILES_POSITION_1];
				}

				if (!argument_parser_append_property(arguments, start, (uint8_t)name_length, &argument_value, property_number,
													 verbose))
				{
					buffer_release(&argument_value);
					return 0;
				}
			}
			else
			{
				if (UINT8_MAX < length)
				{
					buffer_release(&argument_value);
					return 0;
				}

				uint8_t offset = 1;

				if ('-' == argv[i][length - 1])
				{
					if (!bool_to_string(0, &argument_value))
					{
						buffer_release(&argument_value);
						return 0;
					}

					++offset;
				}
				else
				{
					if (!bool_to_string(1, &argument_value))
					{
						buffer_release(&argument_value);
						return 0;
					}

					if ('+' == argv[i][length - 1])
					{
						++offset;
					}
				}

				if (PROPERTIES_COUNT - 1 == common_string_to_enum(start, start + (length - offset), properties_names + 1,
						PROPERTIES_COUNT - 1))
				{
					continue;
				}

				if (!argument_parser_append_property(arguments, start, (uint8_t)length - offset, &argument_value, 0, verbose))
				{
					buffer_release(&argument_value);
					return 0;
				}
			}

			continue;
		}

		if (!buffer_append_char(&argument_value, argv[i], length))
		{
			buffer_release(&argument_value);
			return 0;
		}

		if (!argument_parser_append_property(arguments, properties_names[TARGET_POSITION],
											 properties_names_lengths[TARGET_POSITION], &argument_value, 1, verbose))
		{
			buffer_release(&argument_value);
			return 0;
		}
	}

	buffer_release(&argument_value);
	return 1;
}
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv, struct buffer* arguments, uint8_t verbose)
{
	if ((i < argc && NULL == argv) ||
		NULL == arguments)
	{
		return 0;
	}

	const int argcA = argc - i;

	if (!argcA)
	{
		return 1;
	}

	struct buffer argumentA;

	SET_NULL_TO_BUFFER(argumentA);

	if (!buffer_append(&argumentA, NULL, argc * sizeof(char**)))
	{
		buffer_release(&argumentA);
		return 0;
	}

	const ptrdiff_t size = buffer_size(&argumentA);

	for (; i < argc; ++i)
	{
		const size_t length = wcslen(argv[i]);

		if (!text_encoding_UTF16LE_to_UTF8(argv[i], argv[i] + length, &argumentA) ||
			!buffer_push_back(&argumentA, 0))
		{
			buffer_release(&argumentA);
			return 0;
		}
	}

	const char* argument = (const char*)buffer_data(&argumentA, size);

	if (!buffer_resize(&argumentA, 0))
	{
		buffer_release(&argumentA);
		return 0;
	}

	for (i = 0; i < argcA; ++i)
	{
		if (!buffer_append(&argumentA, (const uint8_t*)&argument, sizeof(char**)))
		{
			buffer_release(&argumentA);
			return 0;
		}

		argument += strlen(argument) + 1;
	}

	char** argvA = (char**)buffer_data(&argumentA, 0);

	if (!argument_parser_char(0, argcA, argvA, arguments, verbose))
	{
		buffer_release(&argumentA);
		return 0;
	}

	buffer_release(&argumentA);
	return 1;
}
#endif

uint8_t argument_assign_un_quote(const uint8_t* previous_pos, const uint8_t* input_start,
								 struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(previous_pos, input_start) ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 1 + (input_start - previous_pos)) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	while (previous_pos < input_start)
	{
		if (quote_symbol != *previous_pos)
		{
			if (!buffer_push_back(output, *previous_pos))
			{
				return 0;
			}
		}

		++previous_pos;
	}

	return buffer_push_back(output, 0);
}

uint8_t argument_append_arguments(
	const uint8_t* input_start, const uint8_t* input_finish, struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == output)
	{
		return 0;
	}

	uint8_t inside_quote = 0;
	const uint8_t* previous_pos = input_start;

	while (input_start < input_finish)
	{
		if (quote_symbol == *input_start)
		{
			inside_quote = !inside_quote;
		}
		else if (space_symbol == *input_start)
		{
			if (!inside_quote)
			{
				if (!argument_assign_un_quote(previous_pos, input_start, output))
				{
					return 0;
				}

				input_start = find_any_symbol_like_or_not_like_that(input_start, input_finish, &space_symbol, 1, 0, 1);
				previous_pos = input_start;
				/**/
				continue;
			}
		}

		++input_start;
	}

	if (previous_pos < input_start &&
		!argument_assign_un_quote(previous_pos, input_start, output))
	{
		return 0;
	}

	return 1;
}

uint8_t argument_create_arguments(struct buffer* output, int* argc, char*** argv)
{
	if (NULL == output ||
		NULL == argc ||
		NULL == argv)
	{
		return 0;
	}

	const int index = *argc;
	const uint8_t* start = buffer_data(output, index);
	*argc = 0;

	if (NULL == start)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(const uint8_t*)))
		{
			return 0;
		}

		(*argv) = (char**)buffer_data(output, buffer_size(output) - sizeof(const uint8_t*));
		return 1;
	}

	const uint8_t* finish = (const uint8_t*)(buffer_data(output, 0) + buffer_size(output));

	while ((start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 1, 1)) < finish)
	{
		start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 0, 1);
		(*argc)++;
	}

	const ptrdiff_t size = buffer_size(output);

	if ((*argc) < 1)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(const uint8_t*)))
		{
			return 0;
		}

		(*argv) = (char**)buffer_data(output, size);
		return 1;
	}

	if (!buffer_append(output, NULL, ((ptrdiff_t)1 + (*argc)) * sizeof(const uint8_t*)))
	{
		return 0;
	}

	start = buffer_data(output, index);
	finish = buffer_data(output, size);

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	while ((start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 0, 1)) < finish)
	{
		if (!buffer_append(output, (const uint8_t*)&start, sizeof(const uint8_t*)))
		{
			return 0;
		}

		start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 1, 1);
	}

	start = NULL;

	if (!buffer_append(output, (const uint8_t*)&start, sizeof(const uint8_t*)))
	{
		return 0;
	}

	(*argv) = (char**)buffer_data(output, size);
	return 1;
}

uint8_t argument_from_char(const char* input_start, const char* input_finish,
						   struct buffer* output, int* argc, char*** argv)
{
	return argument_append_arguments((const uint8_t*)input_start, (const uint8_t*)input_finish, output) &&
		   argument_create_arguments(output, argc, argv);
}

const uint8_t* argument_parser_get_null_terminated_by_index(struct buffer* argument_value, int index,
		int* current_i)
{
	int i = 0;
	const uint8_t* start = buffer_data(argument_value, 0);
	const uint8_t* finish = start + buffer_size(argument_value);

	while ((start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 0, 1)) < finish &&
		   (i++) < index)
	{
		start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 1, 1);
	}

	if (NULL != current_i)
	{
		*current_i = i;
	}

	return finish != start ? start : NULL;
}

const struct range* argument_parser_get_value_by_index(
	const struct buffer* arguments, uint8_t a, uint8_t b, struct buffer* argument_value, int index)
{
	int i = 0;
	void* the_property = NULL;
	const uint8_t* ptr = NULL;

	for (uint8_t j = a; j < b + 1; ++j)
	{
		index -= i;

		if (property_exists(arguments, properties_names[j], properties_names_lengths[j], &the_property))
		{
			if (buffer_resize(argument_value, 0) &&
				property_get_by_pointer(the_property, argument_value))
			{
				if (NULL != (ptr = argument_parser_get_null_terminated_by_index(argument_value, index, &i)))
				{
					break;
				}
			}
		}
	}

	struct range* result = NULL;

	if (NULL != ptr)
	{
		index = (int)(ptr - buffer_data(argument_value, 0));
		const ptrdiff_t size = buffer_size(argument_value);

		if (!buffer_append_range(argument_value, NULL, 1))
		{
			return 0;
		}

		result = (struct range*)buffer_data(argument_value, size);
		result->start = buffer_data(argument_value, index);
		result->finish = find_any_symbol_like_or_not_like_that(result->start, buffer_data(argument_value, size) - 1,
						 &zero_symbol, 1, 1, 1);
	}

	return result;
}

uint8_t argument_parser_get_bool_value(
	const struct buffer* arguments, uint8_t a, uint8_t b, struct buffer* argument_value)
{
	const struct range* result = argument_parser_get_value_by_index(arguments, a, b, argument_value, 0);

	if (result && buffer_resize(argument_value, range_size(result)))
	{
		if (!bool_parse(buffer_data(argument_value, 0), buffer_size(argument_value), &a))
		{
			return 0;
		}

		return a;
	}

	return 0;
}

uint8_t argument_parser_get_debug(const struct buffer* arguments, struct buffer* argument_value)
{
	return argument_parser_get_bool_value(arguments, DEBUG_POSITION, DEBUG_POSITION, argument_value);
}

uint16_t argument_parser_get_encoding(const struct buffer* arguments, struct buffer* argument_value)
{
	const struct range* result = argument_parser_get_value_by_index(arguments,
								 ENCODING_POSITION, ENCODING_POSITION, argument_value, 0);

	if (!result || !buffer_resize(argument_value, range_size(result)))
	{
		return UTF8;
	}

	return load_file_get_encoding(argument_value);
}

uint8_t argument_parser_get_program_help(const struct buffer* arguments, struct buffer* argument_value)
{
	return argument_parser_get_bool_value(arguments, HELP_POSITION_1, HELP_POSITION_2, argument_value);
}

uint8_t argument_parser_get_indent(const struct buffer* arguments, struct buffer* argument_value)
{
	const struct range* result = argument_parser_get_value_by_index(arguments,
								 INDENT_POSITION, INDENT_POSITION, argument_value, 0);

	if (result)
	{
		return (uint8_t)math_abs(long_parse(result->start));
	}

	return 0;
}

uint8_t argument_parser_get_no_logo(const struct buffer* arguments, struct buffer* argument_value)
{
	return argument_parser_get_bool_value(arguments, NO_LOGO_POSITION, NO_LOGO_POSITION, argument_value);
}

uint8_t argument_parser_get_pause(const struct buffer* arguments, struct buffer* argument_value)
{
	return argument_parser_get_bool_value(arguments, PAUSE_POSITION, PAUSE_POSITION, argument_value);
}

uint8_t argument_parser_get_project_help(const struct buffer* arguments, struct buffer* argument_value)
{
	return argument_parser_get_bool_value(arguments, PROJECT_HELP_POSITION, PROJECT_HELP_POSITION,
										  argument_value);
}

uint8_t argument_parser_get_module_priority(const struct buffer* arguments, struct buffer* argument_value)
{
	return argument_parser_get_bool_value(arguments, MODULE_PRIORITY_POSITION, MODULE_PRIORITY_POSITION,
										  argument_value);
}

uint8_t argument_parser_get_quiet(const struct buffer* arguments, struct buffer* argument_value)
{
	return argument_parser_get_bool_value(arguments, QUIET_POSITION_1, QUIET_POSITION_2, argument_value);
}

uint8_t argument_parser_get_properties(const struct buffer* arguments, struct buffer* properties,
									   uint8_t verbose)
{
	void* the_property = NULL;

	if (property_exists(arguments, properties_names[PROPERTY_POSITION],
						properties_names_lengths[PROPERTY_POSITION],
						&the_property))
	{
		struct buffer argument_value;
		SET_NULL_TO_BUFFER(argument_value);

		if (!property_get_by_pointer(the_property, &argument_value) ||
			!argument_parser_buffer_to_properties(&argument_value, properties, verbose))
		{
			buffer_release(&argument_value);
			return 0;
		}

		buffer_release(&argument_value);
		return 1;
	}

	return 0;
}

const struct range* argument_parser_get_build_file(const struct buffer* arguments,
		struct buffer* argument_value, int index)
{
	return argument_parser_get_value_by_index(
			   arguments, BUILD_FILES_POSITION_1, BUILD_FILES_POSITION_1, argument_value, index);
}

const struct range* argument_parser_get_log_file(const struct buffer* arguments,
		struct buffer* argument_value)
{
	return argument_parser_get_value_by_index(
			   arguments, LOG_FILE_POSITION_1, LOG_FILE_POSITION_2, argument_value, 0);
}

const struct range* argument_parser_get_target(const struct buffer* arguments,
		struct buffer* argument_value, int index)
{
	return argument_parser_get_value_by_index(
			   arguments, TARGET_POSITION, TARGET_POSITION, argument_value, index);
}

const struct range* argument_parser_get_listener(const struct buffer* arguments,
		struct buffer* argument_value)
{
	return argument_parser_get_value_by_index(
			   arguments, LISTENER_POSITION, LISTENER_POSITION, argument_value, 0);
}
