/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
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

uint8_t argument_parser_char(int i, int argc, char** argv);
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv);
#endif
void argument_parser_release();

uint8_t argument_append_arguments(
	const uint8_t* input_start, const uint8_t* input_finish, struct buffer* output);
uint8_t argument_create_arguments(struct buffer* output, int* argc, char*** argv);
uint8_t argument_from_char(const char* input_start, const char* input_finish,
						   struct buffer* output, int* argc, char*** argv);
#if defined(_WIN32)
uint8_t argument_append_arguments_wchar_t(
	const wchar_t* input_start, const wchar_t* input_finish, struct buffer* output);
uint8_t argument_create_arguments_wchar_t(struct buffer* output, int* argc, wchar_t*** argv);
uint8_t argument_from_wchar_t(const wchar_t* input_start, const wchar_t* input_finish,
							  struct buffer* output, int* argc, wchar_t*** argv);
#endif
uint8_t argument_parser_get_debug();
uint16_t argument_parser_get_encoding();
uint8_t argument_parser_get_help();
uint8_t argument_parser_get_indent();
uint8_t argument_parser_get_no_logo();
uint8_t argument_parser_get_pause();
uint8_t argument_parser_get_project_help();
uint8_t argument_parser_get_quiet();
uint8_t argument_parser_get_verbose();

const struct buffer* argument_parser_get_properties();
const struct range* argument_parser_get_build_file(int index);
const struct range* argument_parser_get_log_file();

#endif
