/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
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

static const uint8_t equal_symbol = '=';
static const uint8_t quote_symbol = '"';
static const uint8_t space_symbol = ' ';
static const uint8_t zero_symbol = '\0';
static const uint8_t plus = '+';
static const uint8_t minus = '-';

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
		const uint8_t* start = (const uint8_t*)argv[i];
		const uint8_t* finish =
			start + common_count_bytes_until(start, 0);
		const ptrdiff_t length = string_get_length(start, finish);

		if (!length)
		{
			continue;
		}

		static const uint8_t* verbose_str[] =
		{
			(const uint8_t*)"-verbose",
			(const uint8_t*)"-v"
		};
		/**/
		static const uint8_t verbose_lengths[] = { 8, 2 };

		for (uint8_t j = 0; j < 2;)
		{
			if (string_starts_with(
					start, finish,
					verbose_str[j], verbose_str[j] + verbose_lengths[j]))
			{
				if (length == verbose_lengths[j])
				{
					verbose = 1;
					break;
				}

				if (length == verbose_lengths[j] + 1)
				{
					struct range char_set_in_a_range;

					if (!string_substring(
							start, finish, length - 1, 1,
							&char_set_in_a_range))
					{
						break;
					}

					uint32_t char_set;
					char_set_in_a_range.start = string_enumerate(
													char_set_in_a_range.start,
													char_set_in_a_range.finish, &char_set);

					if (char_set_in_a_range.start != char_set_in_a_range.finish)
					{
						break;
					}

					if (plus == char_set)
					{
						verbose = 1;
						break;
					}
					else if (minus == char_set)
					{
						verbose = 0;
						break;
					}
				}
			}

			++j;
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
		if (!argv[i])
		{
			continue;
		}

		const size_t length = wcslen(argv[i]);

		if (!length ||
			L'-' != argv[i][0])
		{
			continue;
		}

		static const wchar_t* verbose_str[] =
		{
			L"-verbose",
			L"-v"
		};
		/**/
		static const uint8_t verbose_lengths[] = { 8, 2 };

		for (uint8_t j = 0; j < 2;)
		{
			if (verbose_lengths[j] <= length &&
				0 == wmemcmp(argv[i], verbose_str[j], verbose_lengths[j]))
			{
				if ((uint8_t)length == verbose_lengths[j])
				{
					verbose = 1;
					break;
				}

				if ((uint8_t)length == verbose_lengths[j] + 1)
				{
					const wchar_t ch = argv[i][length - 1];

					if (L'+' == ch)
					{
						verbose = 1;
						break;
					}
					else if (L'-' == ch)
					{
						verbose = 0;
						break;
					}
				}
			}

			++j;
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

	value->start = string_find_any_symbol_like_or_not_like_that(
					   input_start, input_finish,
					   &equal_symbol, &equal_symbol + 1, 1, 1);

	if (input_finish == value->start)
	{
		return 0;
	}

	key->finish = value->start;
	value->start = string_enumerate(value->start, input_finish, NULL);
	value->start = string_find_any_symbol_like_or_not_like_that(
					   value->start, input_finish,
					   &quote_symbol, &quote_symbol + 1, 0, 1);
	value->finish = string_find_any_symbol_like_or_not_like_that(
						input_finish, value->start,
						&quote_symbol, &quote_symbol + 1, 0, -1);
	value->finish = string_enumerate(value->finish, input_finish, NULL);
	/**/
	key->start = string_find_any_symbol_like_or_not_like_that(
					 input_start, key->finish,
					 &quote_symbol, &quote_symbol + 1, 0, 1);
	key->finish = string_find_any_symbol_like_or_not_like_that(
					  key->finish, key->start,
					  &quote_symbol, &quote_symbol + 1, 0, -1);
	key->finish = string_enumerate(key->finish, input_finish, NULL);
	/**/
	return !range_is_null_or_empty(key) && string_trim(key) &&
		   value->start <= value->finish;
}

uint8_t argument_parser_buffer_to_properties(
	const struct buffer* input, struct buffer* properties, uint8_t verbose)
{
	const uint8_t* start = buffer_data(input, 0);
	const uint8_t* finish = start + buffer_size(input);
	const uint8_t* pos = start;

	while ((start = string_find_any_symbol_like_or_not_like_that(
						start, finish,
						&zero_symbol, &zero_symbol + 1, 1, 1)) < finish)
	{
		struct range key;
		struct range value;

		if (!argument_get_key_and_value(pos, start, &key, &value))
		{
			return 0;
		}

		if (!property_set_by_name(
				properties,
				key.start, (uint8_t)range_size(&key),
				value.start, range_size(&value),
				property_value_is_byte_array,
				1, 1, 1, verbose))
		{
			return 0;
		}

		start = string_find_any_symbol_like_or_not_like_that(
					start, finish, &zero_symbol, &zero_symbol + 1, 0, 1);
		pos = start;
	}

	return 1;
}

uint8_t argument_parser_append_property(
	struct buffer* properties_,
	const uint8_t* name, uint8_t name_length,
	struct buffer* argument_value, uint8_t append_property_value,
	uint8_t verbose)
{
	void* the_property;
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
		return property_set_by_name(
				   properties_, name, name_length,
				   size ? buffer_data(argument_value, 0) : (const uint8_t*)&size,
				   size, property_value_is_byte_array, 0, 0, 0, verbose);
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
	return property_set_by_pointer(
			   the_property,
			   size ? buffer_data(argument_value, 0) : (const uint8_t*)&size,
			   size, property_value_is_byte_array, 0, 0, verbose);
}

uint8_t argument_parser_char(int i, int argc, char** argv,
							 struct buffer* arguments, uint8_t verbose)
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
		if (!argv[i])
		{
			continue;
		}

		const uint8_t* start = (const uint8_t*)argv[i];
		const uint8_t* finish =
			start + common_count_bytes_until(start, 0);
		const ptrdiff_t length = string_get_length(start, finish);

		if (!length)
		{
			continue;
		}

		uint32_t char_set;

		if (NULL == (start = string_enumerate(start, finish, &char_set)))
		{
			continue;
		}

		if ('@' == char_set)
		{
			continue;
		}

		if (!buffer_resize(&argument_value, 0))
		{
			buffer_release(&argument_value);
			return 0;
		}

		if (1 < length && (minus == char_set || '/' == char_set))
		{
			static const uint8_t colon_mark = ':';
			/**/
			const uint8_t* middle =
				string_find_any_symbol_like_or_not_like_that(
					start, finish, &colon_mark, &colon_mark + 1, 1, 1);

			if (middle != finish)
			{
				middle = string_find_any_symbol_like_or_not_like_that(
							 middle, finish,
							 &colon_mark, &colon_mark + 1, 0, 1);

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

				middle = string_find_any_symbol_like_or_not_like_that(
							 middle, start, &colon_mark, &colon_mark + 1,
							 1, -1);
				ptrdiff_t name_length = middle - start;

				if (UINT8_MAX < name_length)
				{
					buffer_release(&argument_value);
					return 0;
				}

				uint8_t property_number = common_string_to_enum(
											  start, middle, properties_names + 1,
											  PROPERTIES_COUNT - 1);

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

				if (!argument_parser_append_property(
						arguments, start, (uint8_t)name_length,
						&argument_value, property_number, verbose))
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

				struct range char_set_in_a_range;

				if (!string_substring(
						(const uint8_t*)argv[i], finish, length - 1, 1,
						&char_set_in_a_range))
				{
					continue;
				}

				char_set_in_a_range.start =
					string_enumerate(char_set_in_a_range.start, char_set_in_a_range.finish, &char_set);

				if (char_set_in_a_range.start != char_set_in_a_range.finish)
				{
					continue;
				}

				if (!bool_to_string(minus == char_set ? 0 : 1, &argument_value))
				{
					buffer_release(&argument_value);
					return 0;
				}

				ptrdiff_t length_ = string_get_length(start, finish);

				if (minus == char_set || plus == char_set)
				{
					--length_;
				}

				if (!string_substring(start, finish, 0, length_, &char_set_in_a_range))
				{
					continue;
				}

				if (PROPERTIES_COUNT - 1 == common_string_to_enum(
						start, char_set_in_a_range.finish,
						properties_names + 1, PROPERTIES_COUNT - 1))
				{
					continue;
				}

				if (!argument_parser_append_property(
						arguments, start,
						(uint8_t)(char_set_in_a_range.finish - start),
						&argument_value, 0, verbose))
				{
					buffer_release(&argument_value);
					return 0;
				}
			}

			continue;
		}

		start = (const uint8_t*)argv[i];

		if (!buffer_append(&argument_value, start, finish - start))
		{
			buffer_release(&argument_value);
			return 0;
		}

		if (!argument_parser_append_property(
				arguments, properties_names[TARGET_POSITION],
				properties_names_lengths[TARGET_POSITION],
				&argument_value, 1, verbose))
		{
			buffer_release(&argument_value);
			return 0;
		}
	}

	buffer_release(&argument_value);
	return 1;
}
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv,
								struct buffer* arguments, uint8_t verbose)
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

	const uint8_t* argument = buffer_data(&argumentA, size);
	const uint8_t* finish = buffer_data(&argumentA, 0) + buffer_size(&argumentA);

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

		argument = string_find_any_symbol_like_or_not_like_that(
					   argument, finish, &zero_symbol, &zero_symbol + 1, 1, 1);
		argument = string_find_any_symbol_like_or_not_like_that(
					   argument, finish, &zero_symbol, &zero_symbol + 1, 0, 1);
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
uint8_t argument_assign_un_quote(const uint8_t* pos, const uint8_t* input_start,
								 struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(pos, input_start) ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 1 + (input_start - pos)) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	uint32_t char_set;
	const uint8_t* previous_pos = pos;

	while (NULL != (pos = string_enumerate(pos, input_start, &char_set)))
	{
		if (quote_symbol != char_set &&
			!buffer_append(output, previous_pos, pos - previous_pos))
		{
			return 0;
		}

		previous_pos = pos;
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
	const uint8_t* pos = input_start;

	while (input_start < input_finish)
	{
		uint32_t char_set;

		if (!string_enumerate(input_start, input_finish, &char_set))
		{
			return 0;
		}

		if (quote_symbol == char_set)
		{
			inside_quote = !inside_quote;
		}
		else if (space_symbol == char_set)
		{
			if (!inside_quote)
			{
				if (!argument_assign_un_quote(pos, input_start, output))
				{
					return 0;
				}

				input_start = string_find_any_symbol_like_or_not_like_that(
								  input_start, input_finish,
								  &space_symbol, &space_symbol + 1, 0, 1);
				pos = input_start;
				continue;
			}
		}

		input_start = string_enumerate(input_start, input_finish, NULL);
	}

	if (pos < input_start &&
		!argument_assign_un_quote(pos, input_start, output))
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
		if (!buffer_append(output, (const uint8_t*)&start, sizeof(const uint8_t*)))
		{
			return 0;
		}

		*argv = (char**)buffer_data(output, buffer_size(output) - sizeof(const uint8_t*));
		return 1;
	}

	const uint8_t* finish = buffer_data(output, 0) + buffer_size(output);

	while ((start = string_find_any_symbol_like_or_not_like_that(
						start, finish, &zero_symbol, &zero_symbol + 1, 1, 1)) < finish)
	{
		start = string_find_any_symbol_like_or_not_like_that(
					start, finish, &zero_symbol, &zero_symbol + 1, 0, 1);
		(*argc)++;
	}

	const ptrdiff_t size = buffer_size(output);

	if (*argc < 1)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(const uint8_t*)))
		{
			return 0;
		}

		*argv = (char**)buffer_data(output, size);
		return 1;
	}

	if (!buffer_append(output, NULL, ((ptrdiff_t)1 + *argc) * sizeof(const uint8_t*)))
	{
		return 0;
	}

	start = buffer_data(output, index);
	finish = buffer_data(output, size);

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	while ((start = string_find_any_symbol_like_or_not_like_that(
						start, finish, &zero_symbol, &zero_symbol + 1, 0, 1)) < finish)
	{
		if (!buffer_append(output, (const uint8_t*)&start, sizeof(const uint8_t*)))
		{
			return 0;
		}

		start = string_find_any_symbol_like_or_not_like_that(
					start, finish, &zero_symbol, &zero_symbol + 1, 1, 1);
	}

	start = NULL;

	if (!buffer_append(output, (const uint8_t*)&start, sizeof(const uint8_t*)))
	{
		return 0;
	}

	*argv = (char**)buffer_data(output, size);
	return 1;
}

uint8_t argument_from_char(const char* input_start, const char* input_finish,
						   struct buffer* output, int* argc, char*** argv)
{
	return argument_append_arguments((const uint8_t*)input_start, (const uint8_t*)input_finish, output) &&
		   argument_create_arguments(output, argc, argv);
}

const uint8_t* argument_parser_get_null_terminated_by_index(
	struct buffer* argument_value, int index, int* current_i)
{
	int i = 0;
	const uint8_t* start = buffer_data(argument_value, 0);
	const uint8_t* finish = start + buffer_size(argument_value);

	while ((start = string_find_any_symbol_like_or_not_like_that(
						start, finish, &zero_symbol, &zero_symbol + 1, 0, 1)) < finish &&
		   (i++) < index)
	{
		start = string_find_any_symbol_like_or_not_like_that(
					start, finish, &zero_symbol, &zero_symbol + 1, 1, 1);
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
	const uint8_t* ptr = NULL;

	for (uint8_t j = a; j < b + 1; ++j)
	{
		index -= i;
		void* the_property;

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
		/**/
		const uint8_t* finish = (const uint8_t*)result;
		const ptrdiff_t length = string_get_length(
									 buffer_data(argument_value, 0), finish);

		if (!string_substring(
				buffer_data(argument_value, 0), finish, 0, length - 1, result))
		{
			return 0;
		}

		result->start = buffer_data(argument_value, index);
		result->finish = string_find_any_symbol_like_or_not_like_that(
							 result->start, result->finish,
							 &zero_symbol, &zero_symbol + 1, 1, 1);
	}

	return result;
}

uint8_t argument_parser_get_bool_value(
	const struct buffer* arguments, uint8_t a, uint8_t b, struct buffer* argument_value)
{
	const struct range* result = argument_parser_get_value_by_index(
									 arguments, a, b, argument_value, 0);
	const ptrdiff_t size = range_size(result);

	if (result && buffer_resize(argument_value, size))
	{
		const uint8_t* value = buffer_data(argument_value, 0);

		if (!bool_parse(value, value + size, &a))
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
		return (uint8_t)math_abs(int_parse(result->start, result->finish));
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
	void* the_property;

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
