/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
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

static uint8_t is_argument_init = 0;

static uint8_t argument_parser_debug = 0;
static uint8_t argument_parser_help = 0;
static uint8_t argument_parser_indent = 0;
static uint8_t argument_parser_no_logo = 0;
static uint8_t argument_parser_pause = 0;
static uint8_t argument_parser_project_help = 0;
static uint8_t argument_parser_quiet = 0;
static uint8_t argument_parser_verbose = UINT8_MAX;
static uint16_t argument_parser_encoding = UTF8;

static struct buffer build_files;
static struct buffer log_file;
static struct buffer properties;
static struct buffer targets;

static const uint8_t equal_symbol = '=';
static const uint8_t quote_symbol = '"';
static const uint8_t space_symbol = ' ';
static const uint8_t zero_symbol = '\0';

uint8_t argument_parser_get_bool_value(char* argument, size_t length)
{
	if (length < 1)
	{
		return 0;
	}

	const char ch = argument[length - 1];

	if ('+' == ch)
	{
		return 1;
	}
	else if ('-' == ch)
	{
		return 0;
	}

	return 1;
}

void argument_parser_get_verbose_char(int argc, char** argv)
{
	if (argument_parser_verbose < 2)
	{
		return;
	}

	for (int i = 0; i < argc; ++i)
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
			argument_parser_verbose = argument_parser_get_bool_value(argv[i], length);
			break;
		}
	}
}
#if defined(_WIN32)
void argument_parser_get_verbose_wchar_t(int argc, wchar_t** argv)
{
	if (argument_parser_verbose < 2)
	{
		return;
	}

	for (int i = 0; i < argc; ++i)
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
				argument_parser_verbose = 1;
			}
			else if (L'-' == ch)
			{
				argument_parser_verbose = 0;
			}
			else
			{
				argument_parser_verbose = 1;
			}

			break;
		}
	}
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

uint8_t argument_get_file_path(const uint8_t* argument,
							   ptrdiff_t argument_name_length, ptrdiff_t argument_length, struct buffer* output)
{
	if (NULL == argument ||
		argument_name_length < 0 ||
		argument_length < 1 ||
		argument_length < argument_name_length ||
		NULL == output)
	{
		return 0;
	}

	struct range path;

	path.start = argument + argument_name_length;

	path.finish = argument + argument_length;

	path.start = find_any_symbol_like_or_not_like_that(path.start, path.finish, &quote_symbol, 1, 0, 1);

	path.finish = 1 + find_any_symbol_like_or_not_like_that(path.finish - 1, path.start, &quote_symbol, 1, 0, -1);

	return !range_is_null_or_empty(&path) &&
		   buffer_append_data_from_range(output, &path) &&
		   buffer_push_back(output, 0);
}

uint16_t argument_parser_get_encoding_from_name(const char* start, const char* finish)
{
	struct buffer encoding_name;
	SET_NULL_TO_BUFFER(encoding_name);

	if (!buffer_append_char(&encoding_name, start, finish - start))
	{
		buffer_release(&encoding_name);
		return FILE_ENCODING_UNKNOWN;
	}

	const uint16_t result = load_file_get_encoding(&encoding_name);
	buffer_release(&encoding_name);
	return result;
}

struct buffer* argument_parser_get_build_files()
{
	return &build_files;
}

uint8_t argument_parser_fill_ranges_at_storage(struct buffer* storage, ptrdiff_t max_size)
{
	if (NULL == storage)
	{
		return 0;
	}

	if (max_size < buffer_size(storage))
	{
		const uint8_t* start = buffer_data(storage, max_size);
		const uint8_t* finish = buffer_data(storage, 0) + buffer_size(storage);

		if (!buffer_resize(storage, max_size))
		{
			return 0;
		}

		ptrdiff_t i = 0;
		struct range* r = NULL;

		while (NULL != (r = buffer_range_data(storage, i++)) && start < finish)
		{
			r->start = start;
			r->finish = start + common_count_bytes_until(start, 0);
			start = r->finish + 1;
		}

		i = (i - 1) * (ptrdiff_t)sizeof(struct range);

		if (max_size < i)
		{
			return 0;
		}

		return buffer_resize(storage, i);
	}

	return buffer_resize(storage, 0);
}

uint8_t argument_parser_char(int i, int argc, char** argv)
{
	argument_parser_get_verbose_char(argc, argv);

	if (!is_argument_init)
	{
		SET_NULL_TO_BUFFER(build_files);
		SET_NULL_TO_BUFFER(properties);
		SET_NULL_TO_BUFFER(log_file);
		SET_NULL_TO_BUFFER(targets);
		is_argument_init = 1;
	}

	const ptrdiff_t max_size = argc * sizeof(struct range);

	if (!buffer_append(&build_files, NULL, max_size) ||
		!buffer_append(&log_file, NULL, sizeof(struct range)) ||
		!buffer_append(&targets, NULL, max_size))
	{
		return 0;
	}

	for (; i < argc; ++i)
	{
		const size_t length = strlen(argv[i]);

		if (length < 2)
		{
			continue;
		}
		else if (7 < length && 0 == memcmp(argv[i], "-indent:", 8))
		{
			if (length < 9)
			{
				i = argc + 1;
				break;
			}

			argument_parser_indent = (uint8_t)math_abs(long_parse((const uint8_t*)(argv[i] + 8)));
		}
		else if ((10 < length && 0 == memcmp(argv[i], "-buildfile:", 11)) ||
				 (2 < length && 0 == memcmp(argv[i], "/f:", 3)))
		{
			if (length < (size_t)('/' == argv[i][0] ? 4 : 12))
			{
				i = argc + 1;
				break;
			}

			if (!argument_get_file_path((const uint8_t*)argv[i], ('/' == argv[i][0] ? 3 : 11), length, &build_files))
			{
				i = argc + 1;
				break;
			}
		}
		else if ((8 < length && 0 == memcmp(argv[i], "-logfile:", 9)) ||
				 (2 < length && 0 == memcmp(argv[i], "-l:", 3)))
		{
			if (length < (size_t)(':' == argv[i][2] ? 4 : 10))
			{
				i = argc + 1;
				break;
			}

			if (!buffer_resize(&log_file, sizeof(struct range)) ||
				!argument_get_file_path((const uint8_t*)argv[i], (':' == argv[i][2] ? 3 : 9), length, &log_file))
			{
				i = argc + 1;
				break;
			}
		}
		else if ((7 == length && (0 == memcmp(argv[i], "-pause+", 7) ||
								  0 == memcmp(argv[i], "-pause-", 7))) ||
				 (6 == length && 0 == memcmp(argv[i], "-pause", 6)))
		{
			argument_parser_pause = argument_parser_get_bool_value(argv[i], length);
		}
		else if ((7 == length && (0 == memcmp(argv[i], "-debug+", 7) ||
								  0 == memcmp(argv[i], "-debug-", 7))) ||
				 (6 == length && 0 == memcmp(argv[i], "-debug", 6)))
		{
			argument_parser_debug = argument_parser_get_bool_value(argv[i], length);
		}
		else if (10 < length && 0 == memcmp(argv[i], "-encoding:", 10))
		{
			argument_parser_encoding = argument_parser_get_encoding_from_name(argv[i] + 10, argv[i] + length);
		}
		else if ((7 == length && (0 == memcmp(argv[i], "-quiet+", 7) ||
								  0 == memcmp(argv[i], "-quiet-", 7))) ||
				 (6 == length && 0 == memcmp(argv[i], "-quiet", 6)) ||
				 (3 == length && (0 == memcmp(argv[i], "-q+", 3) ||
								  0 == memcmp(argv[i], "-q-", 3))) ||
				 (2 == length && 0 == memcmp(argv[i], "-q", 2)))
		{
			argument_parser_quiet = argument_parser_get_bool_value(argv[i], length);
		}
		else if (2 < length && 0 == memcmp(argv[i], "-D:", 3))
		{
			if (length < 4)
			{
				i = argc + 1;
				break;
			}

			struct range key;

			struct range value;

			if (!argument_get_key_and_value(
					(const uint8_t*)(argv[i] + 3),
					(const uint8_t*)(argv[i] + length),
					&key,
					&value))
			{
				i = argc + 1;
				break;
			}

			if (!property_set_by_name(&properties,
									  key.start, (uint8_t)range_size(&key),
									  value.start, range_size(&value),
									  property_value_is_byte_array,
									  1, 1, 1, argument_parser_get_verbose()))
			{
				i = argc + 1;
				break;
			}
		}
		else if ((13 == length && (0 == memcmp(argv[i], "-projecthelp+", 13) ||
								   0 == memcmp(argv[i], "-projecthelp-", 13))) ||
				 (12 == length && 0 == memcmp(argv[i], "-projecthelp", 12)))
		{
			argument_parser_project_help = argument_parser_get_bool_value(argv[i], length);
		}
		else if ((8 == length && (0 == memcmp(argv[i], "-nologo+", 8) ||
								  0 == memcmp(argv[i], "-nologo-", 8))) ||
				 (7 == length && 0 == memcmp(argv[i], "-nologo", 7)))
		{
			argument_parser_no_logo = argument_parser_get_bool_value(argv[i], length);
		}
		else if ((6 == length && (0 == memcmp(argv[i], "-help+", 6) ||
								  0 == memcmp(argv[i], "-help-", 6))) ||
				 (5 == length && 0 == memcmp(argv[i], "-help", 5)) ||
				 (3 == length && (0 == memcmp(argv[i], "-h+", 3) ||
								  0 == memcmp(argv[i], "-h-", 3))) ||
				 (2 == length && 0 == memcmp(argv[i], "-h", 2)))
		{
			argument_parser_help = argument_parser_get_bool_value(argv[i], length);
		}
		else if ('-' != argv[i][0] &&
				 '@' != argv[i][0] &&
				 '/' != argv[i][0])
		{
			if (!argument_get_file_path((const uint8_t*)argv[i], 0, length, &targets))
			{
				i = argc + 1;
				break;
			}
		}
	}

	if (argc < i)
	{
		buffer_resize(&build_files, 0);
		buffer_resize(&log_file, 0);
		buffer_resize(&targets, 0);
		/**/
		return 0;
	}

	if (!argument_parser_fill_ranges_at_storage(&build_files, max_size) ||
		!argument_parser_fill_ranges_at_storage(&log_file, sizeof(struct range)) ||
		!argument_parser_fill_ranges_at_storage(&targets, max_size))
	{
		return 0;
	}

	return 1;
}
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv)
{
	argument_parser_get_verbose_wchar_t(argc, argv);

	if (!is_argument_init)
	{
		SET_NULL_TO_BUFFER(build_files);
		SET_NULL_TO_BUFFER(properties);
		SET_NULL_TO_BUFFER(log_file);
		SET_NULL_TO_BUFFER(targets);
		is_argument_init = 1;
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

	if (!argument_parser_char(0, argcA, argvA))
	{
		buffer_release(&argumentA);
		return 0;
	}

	buffer_release(&argumentA);
	return 1;
}
#endif
void argument_parser_release()
{
	argument_parser_debug = 0;
	argument_parser_help = 0;
	argument_parser_indent = 0;
	argument_parser_no_logo = 0;
	argument_parser_pause = 0;
	argument_parser_project_help = 0;
	argument_parser_quiet = 0;
	argument_parser_verbose = UINT8_MAX;
	argument_parser_encoding = UTF8;

	if (is_argument_init)
	{
		buffer_release(&build_files);
		property_release(&properties);
		buffer_release(&log_file);
		buffer_release(&targets);
	}
	else
	{
		SET_NULL_TO_BUFFER(build_files);
		SET_NULL_TO_BUFFER(properties);
		SET_NULL_TO_BUFFER(log_file);
		SET_NULL_TO_BUFFER(targets);
		is_argument_init = 1;
	}
}

uint8_t argument_append_arguments(
	const uint8_t* input_start, const uint8_t* input_finish, struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == output)
	{
		return 0;
	}

	for (; input_start < input_finish; ++input_start)
	{
		const uint8_t* finish = NULL;

		if (finish == input_start)
		{
			break;
		}

		if (quote_symbol == *input_start)
		{
			input_start = find_any_symbol_like_or_not_like_that(
							  input_start + 1, input_finish, &quote_symbol, 1, 0, 1);
			finish = find_any_symbol_like_or_not_like_that(
						 input_start + 1, input_finish, &quote_symbol, 1, 1, 1);
		}
		else if (space_symbol == *input_start)
		{
			continue;
		}
		else
		{
			finish = find_any_symbol_like_or_not_like_that(
						 input_start + 1, input_finish, &space_symbol, 1, 1, 1);
		}

		if (input_start < finish)
		{
			if (!buffer_append(output, input_start, finish - input_start) ||
				!buffer_push_back(output, 0))
			{
				return 0;
			}
		}

		input_start = finish;
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
		++start;
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
#if defined(_WIN32)
uint8_t argument_append_arguments_wchar_t(
	const wchar_t* input_start, const wchar_t* input_finish, struct buffer* output)
{
	if (NULL == input_start ||
		NULL == input_finish ||
		input_finish <= input_start ||
		NULL == output)
	{
		return 0;
	}

	for (; input_start < input_finish; ++input_start)
	{
		const wchar_t* finish = NULL;

		if (finish == input_start)
		{
			break;
		}

		if (L'\"' == *input_start)
		{
			input_start = find_any_symbol_like_or_not_like_that_wchar_t(input_start + 1, input_finish, L"\"", 1, 0, 1);
			finish = find_any_symbol_like_or_not_like_that_wchar_t(input_start + 1, input_finish, L"\"", 1, 1, 1);
		}
		else if (L' ' == *input_start)
		{
			continue;
		}
		else
		{
			finish = find_any_symbol_like_or_not_like_that_wchar_t(input_start + 1, input_finish, L" ", 1, 1, 1);
		}

		if (input_start < finish)
		{
			if (!buffer_append_wchar_t(output, input_start, finish - input_start) ||
				!buffer_push_back_uint16(output, L'\0'))
			{
				return 0;
			}
		}

		input_start = finish;
	}

	return 1;
}

uint8_t argument_create_arguments_wchar_t(struct buffer* output, int* argc, wchar_t*** argv)
{
	if (NULL == output ||
		NULL == argc ||
		NULL == argv)
	{
		return 0;
	}

	const int index = *argc;
	const wchar_t* start = (const wchar_t*)buffer_data(output, index);
	*argc = 0;

	if (NULL == start)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(wchar_t*)))
		{
			return 0;
		}

		(*argv) = (wchar_t**)buffer_data(output, buffer_size(output) - sizeof(wchar_t*));
		return 1;
	}

	const wchar_t* finish = (const wchar_t*)(buffer_data(output, 0) + buffer_size(output));

	while ((start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 1, 1)) < finish)
	{
		++start;
		(*argc)++;
	}

	const ptrdiff_t size = buffer_size(output);

	if ((*argc) < 1)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(wchar_t*)))
		{
			return 0;
		}

		(*argv) = (wchar_t**)buffer_data(output, size);
		return 1;
	}

	if (!buffer_append(output, NULL, ((ptrdiff_t)2 + (*argc)) * sizeof(wchar_t*)))
	{
		return 0;
	}

	start = (const wchar_t*)buffer_data(output, index);
	finish = (const wchar_t*)buffer_data(output, size);

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	while ((start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 0, 1)) < finish)
	{
		if (!buffer_append(output, (const uint8_t*)&start, sizeof(wchar_t*)))
		{
			return 0;
		}

		start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 1, 1);
	}

	start = NULL;

	if (!buffer_append(output, (const uint8_t*)&start, sizeof(wchar_t*)))
	{
		return 0;
	}

	(*argv) = (wchar_t**)buffer_data(output, size);
	return 1;
}

uint8_t argument_from_wchar_t(const wchar_t* input_start, const wchar_t* input_finish,
							  struct buffer* output, int* argc, wchar_t*** argv)
{
	return argument_append_arguments_wchar_t(input_start, input_finish, output) &&
		   argument_create_arguments_wchar_t(output, argc, argv);
}
#endif
uint8_t argument_parser_get_debug()
{
	return argument_parser_debug;
}

uint16_t argument_parser_get_encoding()
{
	return argument_parser_encoding;
}

uint8_t argument_parser_get_help()
{
	return argument_parser_help;
}

uint8_t argument_parser_get_indent()
{
	return argument_parser_indent;
}

uint8_t argument_parser_get_no_logo()
{
	return argument_parser_no_logo;
}

uint8_t argument_parser_get_pause()
{
	return argument_parser_pause;
}

uint8_t argument_parser_get_project_help()
{
	return argument_parser_project_help;
}

uint8_t argument_parser_get_quiet()
{
	return argument_parser_quiet;
}

uint8_t argument_parser_get_verbose()
{
	return argument_parser_verbose < 2 ? argument_parser_verbose : 0;
}

const struct buffer* argument_parser_get_properties()
{
	return is_argument_init ? &properties : NULL;
}

const struct range* argument_parser_get_build_file(int index)
{
	if (!is_argument_init)
	{
		return NULL;
	}

	return buffer_range_data(&build_files, index);
}

const struct range* argument_parser_get_log_file()
{
	if (!is_argument_init)
	{
		return NULL;
	}

	return buffer_range_data(&log_file, 0);
}

const struct range* argument_parser_get_target(int index)
{
	if (!is_argument_init)
	{
		return NULL;
	}

	return buffer_range_data(&targets, index);
}
