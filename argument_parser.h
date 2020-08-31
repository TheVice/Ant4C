/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _ARGUMENT_PARSER_H_
#define _ARGUMENT_PARSER_H_

#if defined(_WIN32)
#include <wchar.h>
#endif
#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

uint8_t argument_parser_get_verbose_char(int i, int argc, char** argv);
#if defined(_WIN32)
uint8_t argument_parser_get_verbose_wchar_t(int i, int argc, wchar_t** argv);
#endif

uint8_t argument_parser_char(int i, int argc, char** argv,
							 struct buffer* arguments, uint8_t verbose);
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv,
								struct buffer* arguments, uint8_t verbose);
#endif

uint8_t argument_append_arguments(
	const uint8_t* input_start, const uint8_t* input_finish, struct buffer* output);
uint8_t argument_create_arguments(struct buffer* output, int* argc, char*** argv);
uint8_t argument_from_char(const char* input_start, const char* input_finish,
						   struct buffer* output, int* argc, char*** argv);

uint8_t argument_parser_get_debug(const struct buffer* arguments, struct buffer* argument_value);
uint8_t argument_parser_get_program_help(const struct buffer* arguments, struct buffer* argument_value);
uint8_t argument_parser_get_indent(const struct buffer* arguments, struct buffer* argument_value);
uint8_t argument_parser_get_no_logo(const struct buffer* arguments, struct buffer* argument_value);
uint8_t argument_parser_get_pause(const struct buffer* arguments, struct buffer* argument_value);
uint8_t argument_parser_get_project_help(const struct buffer* arguments, struct buffer* argument_value);
uint8_t argument_parser_get_module_priority(const struct buffer* arguments, struct buffer* argument_value);
uint8_t argument_parser_get_quiet(const struct buffer* arguments, struct buffer* argument_value);
uint16_t argument_parser_get_encoding(const struct buffer* arguments, struct buffer* argument_value);

uint8_t argument_parser_get_properties(const struct buffer* arguments, struct buffer* properties,
									   uint8_t verbose);
const struct range* argument_parser_get_build_file(const struct buffer* arguments,
		struct buffer* argument_value, int index);
const struct range* argument_parser_get_log_file(const struct buffer* arguments,
		struct buffer* argument_value);
const struct range* argument_parser_get_target(const struct buffer* arguments,
		struct buffer* argument_value, int index);
const struct range* argument_parser_get_listener(const struct buffer* arguments,
		struct buffer* argument_value);

#endif
