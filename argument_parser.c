/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
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

#include <stdlib.h>
#include <string.h>

static const uint8_t equal_symbol = '=';
static const uint8_t quote_symbol = '"';
static const uint8_t space_symbol = ' ';
static const uint8_t zero_symbol = '\0';

struct Parameters
{
	struct buffer build_file;
	struct buffer listener;
	struct buffer log_file;
	struct buffer properties;
	struct buffer target;
	uint16_t encoding;
	uint8_t debug;
	uint8_t indent;
	uint8_t module_priority;
	uint8_t no_logo;
	uint8_t pause;
	uint8_t program_help;
	uint8_t project_help;
	uint8_t quiet;
	uint8_t verbose;
};

static struct Parameters parameters_;
static uint8_t is_init = 0;

static const uint8_t* arguments_0[] =
{
	(const uint8_t*)"-debug",
	(const uint8_t*)"-h",
	(const uint8_t*)"-help",
	(const uint8_t*)"-modulepriority",
	(const uint8_t*)"-nologo",
	(const uint8_t*)"-pause",
	(const uint8_t*)"-projecthelp",
	(const uint8_t*)"-q",
	(const uint8_t*)"-quiet",
	(const uint8_t*)"-verbose",
	(const uint8_t*)"-v",
	/**/
	(const uint8_t*)"-debug+",
	(const uint8_t*)"-h+",
	(const uint8_t*)"-help+",
	(const uint8_t*)"-modulepriority+",
	(const uint8_t*)"-nologo+",
	(const uint8_t*)"-pause+",
	(const uint8_t*)"-projecthelp+",
	(const uint8_t*)"-q+",
	(const uint8_t*)"-quiet+",
	(const uint8_t*)"-verbose+",
	(const uint8_t*)"-v+",
	/**/
	(const uint8_t*)"-debug-",
	(const uint8_t*)"-h-",
	(const uint8_t*)"-help-",
	(const uint8_t*)"-modulepriority-",
	(const uint8_t*)"-nologo-",
	(const uint8_t*)"-pause-",
	(const uint8_t*)"-projecthelp-",
	(const uint8_t*)"-q-",
	(const uint8_t*)"-quiet-",
	(const uint8_t*)"-verbose-",
	(const uint8_t*)"-v-",
};

typedef void(*P_0)(struct Parameters* parameters);

void set_debug(struct Parameters* parameters)
{
	parameters->debug = 1;
}

void unset_debug(struct Parameters* parameters)
{
	parameters->debug = 0;
}

void set_help(struct Parameters* parameters)
{
	parameters->program_help = 1;
}

void unset_help(struct Parameters* parameters)
{
	parameters->program_help = 0;
}

void set_modulepriority(struct Parameters* parameters)
{
	parameters->module_priority = 1;
}

void unset_modulepriority(struct Parameters* parameters)
{
	parameters->module_priority = 0;
}

void set_nologo(struct Parameters* parameters)
{
	parameters->no_logo = 1;
}

void unset_nologo(struct Parameters* parameters)
{
	parameters->no_logo = 0;
}

void set_pause(struct Parameters* parameters)
{
	parameters->pause = 1;
}

void unset_pause(struct Parameters* parameters)
{
	parameters->pause = 0;
}

void set_projecthelp(struct Parameters* parameters)
{
	parameters->project_help = 1;
}

void unset_projecthelp(struct Parameters* parameters)
{
	parameters->project_help = 0;
}

void set_quiet(struct Parameters* parameters)
{
	parameters->quiet = 1;
}

void unset_quiet(struct Parameters* parameters)
{
	parameters->quiet = 0;
}

void set_verbose(struct Parameters* parameters)
{
	parameters->verbose = 1;
}

void unset_verbose(struct Parameters* parameters)
{
	parameters->verbose = 0;
}

static const P_0 functions_0[] =
{
	set_debug,
	set_help,
	set_help,
	set_modulepriority,
	set_nologo,
	set_pause,
	set_projecthelp,
	set_quiet,
	set_quiet,
	set_verbose,
	set_verbose,
	/**/
	set_debug,
	set_help,
	set_help,
	set_modulepriority,
	set_nologo,
	set_pause,
	set_projecthelp,
	set_quiet,
	set_quiet,
	set_verbose,
	set_verbose,
	/**/
	unset_debug,
	unset_help,
	unset_help,
	unset_modulepriority,
	unset_nologo,
	unset_pause,
	unset_projecthelp,
	unset_quiet,
	unset_quiet,
	unset_verbose,
	unset_verbose,
};

static const uint8_t* arguments_1[] =
{
	(const uint8_t*)"-buildfile:",
	(const uint8_t*)"-D:",
	(const uint8_t*)"-encoding:",
	(const uint8_t*)"/f:",
	(const uint8_t*)"-indent:",
	(const uint8_t*)"-l:",
	(const uint8_t*)"-listener:",
	(const uint8_t*)"-logfile:",
};

typedef uint8_t(*P_1)(struct Parameters* parameters, const char* argument, ptrdiff_t i, ptrdiff_t length);

uint8_t set_buildfile(struct Parameters* parameters, const char* argument, ptrdiff_t i, ptrdiff_t length)
{
	return buffer_append_char(&(parameters->build_file), argument + i, length - i) &&
		   buffer_push_back(&(parameters->build_file), zero_symbol);
}

uint8_t argument_parser_argument_to_property(
	const uint8_t* input_start, const uint8_t* input_finish,
	struct buffer* properties, uint8_t verbose);

uint8_t set_properties(struct Parameters* parameters, const char* argument, ptrdiff_t i, ptrdiff_t length)
{
	return argument_parser_argument_to_property(
			   (const uint8_t*)(argument + i), (const uint8_t*)(argument + length),
			   &(parameters->properties), parameters->verbose);
}

uint8_t set_encoding(struct Parameters* parameters, const char* argument, ptrdiff_t i, ptrdiff_t length)
{
	struct buffer encoding_name;
	SET_NULL_TO_BUFFER(encoding_name);

	if (!buffer_append_char(&encoding_name, argument + i, length - i))
	{
		buffer_release(&encoding_name);
		return 0;
	}

	parameters->encoding = load_file_get_encoding(&encoding_name);
	buffer_release(&encoding_name);
	return 1;
}

uint8_t set_indent(struct Parameters* parameters, const char* argument, ptrdiff_t i, ptrdiff_t length)
{
	parameters->indent = (uint8_t)math_abs(int_parse((const uint8_t*)(argument + i),
										   (const uint8_t*)(argument + length)));
	return 1;
}

uint8_t set_listener(struct Parameters* parameters, const char* argument, ptrdiff_t i, ptrdiff_t length)
{
	return buffer_resize(&(parameters->listener), 0) &&
		   buffer_append_char(&(parameters->listener), argument + i, length - i) &&
		   buffer_push_back(&(parameters->listener), zero_symbol);
}

uint8_t set_logfile(struct Parameters* parameters, const char* argument, ptrdiff_t i, ptrdiff_t length)
{
	return buffer_resize(&(parameters->log_file), 0) &&
		   buffer_append_char(&(parameters->log_file), argument + i, length - i) &&
		   buffer_push_back(&(parameters->log_file), zero_symbol);
}

static const P_1 functions_1[] =
{
	set_buildfile,
	set_properties,
	set_encoding,
	set_buildfile,
	set_indent,
	set_logfile,
	set_listener,
	set_logfile,
};

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

uint8_t argument_parser_argument_to_property(
	const uint8_t* input_start, const uint8_t* input_finish,
	struct buffer* properties, uint8_t verbose)
{
	const uint8_t* pos = input_start;

	while (pos < (input_start = string_find_any_symbol_like_or_not_like_that(
									input_start, input_finish,
									&zero_symbol, &zero_symbol + 1, 1, 1)))
	{
		struct range key;
		struct range value;

		if (!argument_get_key_and_value(pos, input_start, &key, &value))
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

		input_start = string_find_any_symbol_like_or_not_like_that(
						  input_start, input_finish, &zero_symbol, &zero_symbol + 1, 0, 1);
		pos = input_start;
	}

	return 1;
}

uint8_t argument_parser_char_(int i, int argc, char** argv, struct Parameters* parameters)
{
	if ((i < argc && NULL == argv) ||
		NULL == parameters)
	{
		return 0;
	}

	if (!is_init &&
		!argument_parser_init())
	{
		return 0;
	}

	for (; i < argc; ++i)
	{
		if (NULL == argv[i])
		{
			continue;
		}

		const ptrdiff_t length = common_count_bytes_until((const uint8_t*)(argv[i]), 0);

		if (length < 1)
		{
			continue;
		}

		uint8_t pass = 0;

		for (uint8_t j = 0, count = sizeof(arguments_0) / sizeof(*arguments_0); j < count; ++j)
		{
			const ptrdiff_t argument_length = common_count_bytes_until(arguments_0[j], 0);

			if (length == argument_length &&
				0 == memcmp(argv[i], arguments_0[j], length))
			{
				(functions_0[j])(parameters);
				pass = 1;
				break;
			}
		}

		if (pass)
		{
			continue;
		}

		for (uint8_t j = 0, count = sizeof(arguments_1) / sizeof(*arguments_1); j < count; ++j)
		{
			const ptrdiff_t argument_length = common_count_bytes_until(arguments_1[j], 0);

			if (string_starts_with(
					(const uint8_t*)(argv[i]), (const uint8_t*)(argv[i] + length),
					arguments_1[j], arguments_1[j] + argument_length))
			{
				if (length <= argument_length ||
					!(functions_1[j])(parameters, argv[i], argument_length, length))
				{
					return 0;
				}

				pass = 1;
				break;
			}
		}

		if (pass)
		{
			continue;
		}

		if (string_starts_with(
				(const uint8_t*)(argv[i]), (const uint8_t*)(argv[i] + length),
				arguments_1[0], arguments_1[0] + 1))
		{
			continue;
		}

		if (!buffer_append_char(&(parameters->target), argv[i], length) ||
			!buffer_push_back(&(parameters->target), zero_symbol))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t argument_parser_char(int i, int argc, char** argv)
{
	return argument_parser_char_(i, argc, argv, &parameters_);
}
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv)
{
	if (i < argc && NULL == argv)
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
		if (NULL == argv[i])
		{
			continue;
		}

		const size_t length = wcslen(argv[i]);

		if (0 == length)
		{
			continue;
		}

		if (!text_encoding_UTF16LE_to_UTF8(
				(const uint16_t*)(argv[i]),
				(const uint16_t*)(argv[i] + length),
				&argumentA) ||
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

	if (!argument_parser_char(0, argcA, argvA))
	{
		buffer_release(&argumentA);
		return 0;
	}

	buffer_release(&argumentA);
	return 1;
}
#endif
uint8_t argument_parser_init()
{
	if (!is_init)
	{
		memset(&parameters_, 0, sizeof(struct Parameters));
		is_init = 1;
	}
	else
	{
		if (!buffer_resize(&(parameters_.build_file), 0))
		{
			return 0;
		}

		if (!buffer_resize(&(parameters_.listener), 0))
		{
			return 0;
		}

		if (!buffer_resize(&(parameters_.log_file), 0))
		{
			return 0;
		}

		if (!buffer_resize(&(parameters_.target), 0))
		{
			return 0;
		}

		property_release_inner(&(parameters_.properties));

		if (!buffer_resize(&(parameters_.properties), 0))
		{
			return 0;
		}
	}

	parameters_.debug = 0;
	parameters_.encoding = UTF8;
	parameters_.indent = 0;
	parameters_.module_priority = 0;
	parameters_.no_logo = 0;
	parameters_.pause = 0;
	parameters_.program_help = 0;
	parameters_.project_help = 0;
	parameters_.quiet = 0;
	parameters_.verbose = 0;
	/**/
	return 1;
}

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
	struct buffer* argument_value, int index)
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

	return finish != start ? start : NULL;
}

uint8_t argument_parser_get_debug()
{
	return parameters_.debug;
}

uint8_t argument_parser_get_indent()
{
	return parameters_.indent;
}

uint8_t argument_parser_get_module_priority()
{
	return parameters_.module_priority;
}

uint8_t argument_parser_get_no_logo()
{
	return parameters_.no_logo;
}

uint8_t argument_parser_get_pause()
{
	return parameters_.pause;
}

uint8_t argument_parser_get_program_help()
{
	return parameters_.program_help;
}

uint8_t argument_parser_get_project_help()
{
	return parameters_.project_help;
}

uint8_t argument_parser_get_quiet()
{
	return parameters_.quiet;
}

uint8_t argument_parser_get_verbose()
{
	return parameters_.verbose;
}

uint16_t argument_parser_get_encoding()
{
	return parameters_.encoding;
}

const struct buffer* argument_parser_get_properties()
{
	return &(parameters_.properties);
}

const uint8_t* argument_parser_get_build_file(int index)
{
	return argument_parser_get_null_terminated_by_index(&(parameters_.build_file), index);
}

const uint8_t* argument_parser_get_log_file()
{
	return buffer_data(&(parameters_.log_file), 0);
}

const uint8_t* argument_parser_get_target(int index)
{
	return argument_parser_get_null_terminated_by_index(&(parameters_.target), index);
}

const uint8_t* argument_parser_get_listener()
{
	return buffer_data(&(parameters_.listener), 0);
}

void argument_parser_release()
{
	if (is_init)
	{
		buffer_release(&(parameters_.build_file));
		buffer_release(&(parameters_.listener));
		buffer_release(&(parameters_.log_file));
		buffer_release(&(parameters_.target));
		property_release(&(parameters_.properties));
	}
}
