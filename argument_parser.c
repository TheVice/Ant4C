/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
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
static uint16_t argument_parser_encoding = UTF8;
static uint8_t argument_parser_help = 0;
static uint8_t argument_parser_indent = 0;
static uint8_t argument_parser_no_logo = 0;
static uint8_t argument_parser_pause = 0;
static uint8_t argument_parser_project_help = 0;
static uint8_t argument_parser_quiet = 0;
static uint8_t argument_parser_verbose = UINT8_MAX;

static struct buffer build_files;
static struct buffer properties;
static struct buffer log_file;

static ptrdiff_t build_files_size = 0;
static ptrdiff_t log_file_size = 0;

static const uint8_t zero_symbol = '\0';
static const uint8_t quote_symbol = '"';
static const uint8_t equal_symbol = '=';
static const uint8_t space_symbol = ' ';

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
	if (NULL == argument || argument_name_length < 1 || argument_length < 1 ||
		argument_length < argument_name_length || NULL == output)
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

uint8_t argument_parser_char(int i, int argc, char** argv)
{
	argument_parser_get_verbose_char(argc, argv);

	if (!is_argument_init)
	{
		SET_NULL_TO_BUFFER(build_files);
		SET_NULL_TO_BUFFER(properties);
		SET_NULL_TO_BUFFER(log_file);
		is_argument_init = 1;
	}

	for (; i < argc; ++i)
	{
		if ('-' != argv[i][0] &&
			'@' != argv[i][0] &&
			'/' != argv[i][0])
		{
			continue;
		}

		const size_t length = strlen(argv[i]);

		if (length < 2)
		{
			continue;
		}
		else if (7 < length && 0 == memcmp(argv[i], "-indent:", 8))
		{
			if (length < 9)
			{
				return 0;
			}

			argument_parser_indent = (uint8_t)math_abs(long_parse((const uint8_t*)(argv[i] + 8)));
		}
		else if ((10 < length && 0 == memcmp(argv[i], "-buildfile:", 11)) ||
				 (2 < length && 0 == memcmp(argv[i], "/f:", 3)))
		{
			if (length < (size_t)('/' == argv[i][0] ? 4 : 12))
			{
				buffer_release(&build_files);
				return 0;
			}

			if (!argument_get_file_path((const uint8_t*)argv[i], ('/' == argv[i][0] ? 3 : 11), length, &build_files))
			{
				buffer_release(&build_files);
				return 0;
			}
		}
		else if ((8 < length && 0 == memcmp(argv[i], "-logfile:", 9)) ||
				 (2 < length && 0 == memcmp(argv[i], "-l:", 3)))
		{
			if (length < (size_t)(':' == argv[i][2] ? 4 : 10))
			{
				buffer_release(&log_file);
				return 0;
			}

			if (!buffer_resize(&log_file, 0) ||
				!argument_get_file_path((const uint8_t*)argv[i], (':' == argv[i][2] ? 3 : 9), length, &log_file))
			{
				buffer_release(&log_file);
				return 0;
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
			argument_parser_encoding = load_file_get_file_encoding((const uint8_t*)(argv[i] + 10),
									   (const uint8_t*)(argv[i] + length));
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
				property_clear(&properties);
				return 0;
			}

			struct range key;

			struct range value;

			if (!argument_get_key_and_value(
					(const uint8_t*)(argv[i] + 3),
					(const uint8_t*)(argv[i] + length),
					&key,
					&value))
			{
				property_clear(&properties);
				return 0;
			}

			if (!property_set_by_name(&properties,
									  key.start, (uint8_t)range_size(&key),
									  value.start, range_size(&value),
									  property_value_is_byte_array,
									  1, 1, 1, argument_parser_get_verbose()))
			{
				property_clear(&properties);
				return 0;
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
	}

	build_files_size = build_files_size < 0 ? -1 : buffer_size(&build_files);

	if (0 < build_files_size)
	{
		const uint8_t* start = buffer_data(&build_files, 0);
		const uint8_t* finish = buffer_data(&build_files, 0) + build_files_size;
		int count = 1;

		while (finish != (start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 1, 1)))
		{
			start = find_any_symbol_like_or_not_like_that(start + 1, finish, &zero_symbol, 1, 0, 1);
			++count;
		}

		if (!buffer_append(&build_files, NULL, count * sizeof(struct range)) ||
			!buffer_resize(&build_files, build_files_size))
		{
			buffer_release(&build_files);
			return 0;
		}

		start = buffer_data(&build_files, 0);
		finish = buffer_data(&build_files, 0) + build_files_size;
		const uint8_t* start_ = start;

		while (finish != (start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 1, 1)))
		{
			struct range build_file;
			build_file.start = start_;
			build_file.finish = start;
			start = find_any_symbol_like_or_not_like_that(start + 1, finish, &zero_symbol, 1, 0, 1);
			start_ = start;

			if (!buffer_append_range(&build_files, &build_file, 1))
			{
				buffer_release(&build_files);
				return 0;
			}
		}
	}

	log_file_size = log_file_size < 0 ? -1 : buffer_size(&log_file);

	if (0 < log_file_size)
	{
		if (!buffer_append_range(&log_file, NULL, 1))
		{
			buffer_release(&log_file);
			return 0;
		}

		struct range* log_file_range = (struct range*)buffer_data(&log_file, log_file_size);

		log_file_range->start = buffer_data(&log_file, 0);

		log_file_range->finish = buffer_data(&log_file, log_file_size - 1);
	}

	return 1;
}
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv)
{
	argument_parser_get_verbose_wchar_t(argc, argv);
	/**/
	build_files_size = -1;
	log_file_size = -1;
	/**/
	struct buffer argumentA;
	SET_NULL_TO_BUFFER(argumentA);

	for (; i < argc; ++i)
	{
		const size_t length = wcslen(argv[i]);

		if (!text_encoding_UTF16LE_to_UTF8(argv[i], argv[i] + length, &argumentA) ||
			!buffer_push_back(&argumentA, 0))
		{
			buffer_release(&argumentA);
			return 0;
		}

		if (i == argc - 1)
		{
			build_files_size = 0;
			log_file_size = 0;
		}

		char* argvA[1];
		argvA[0] = buffer_char_data(&argumentA, 0);

		if (!argument_parser_char(0, 1, argvA) ||
			!buffer_resize(&argumentA, 0))
		{
			buffer_release(&argumentA);
			return 0;
		}
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
	build_files_size = 0;
	log_file_size = 0;

	if (is_argument_init)
	{
		buffer_release(&build_files);
		property_clear(&properties);
		buffer_release(&log_file);
	}
	else
	{
		SET_NULL_TO_BUFFER(build_files);
		SET_NULL_TO_BUFFER(properties);
		SET_NULL_TO_BUFFER(log_file);
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
	if (!is_argument_init || !build_files_size  || !buffer_size(&build_files))
	{
		return NULL;
	}

	return (const struct range*)buffer_data(&build_files, build_files_size + index * sizeof(struct range));
}

const struct range* argument_parser_get_log_file()
{
	ptrdiff_t size = 0;

	if (!is_argument_init ||
		0 == (size = buffer_size(&log_file)) ||
		size < (ptrdiff_t)sizeof(struct range))
	{
		return NULL;
	}

	return (const struct range*)buffer_data(&log_file, size - sizeof(struct range));
}
