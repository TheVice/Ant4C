/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020, 2022, 2024 TheVice
 *
 */

#ifndef _ARGUMENT_PARSER_H_
#define _ARGUMENT_PARSER_H_

#if defined(_WIN32)
#include <wchar.h>
#endif
#include <stddef.h>
#include <stdint.h>

struct range;

uint8_t argument_parser_char(int i, int argc, char** argv);
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv);
#endif

uint8_t argument_append_arguments(
	const uint8_t* input_start, const uint8_t* input_finish, void* output);
uint8_t argument_create_arguments(void* output, int* argc, char*** argv);
uint8_t argument_from_char(
	const char* input_start, const char* input_finish,
	void* output, int* argc, char*** argv);

uint8_t argument_parser_init(void);

uint8_t argument_parser_get_debug(void);
uint8_t argument_parser_get_indent(void);
uint8_t argument_parser_get_module_priority(void);
uint8_t argument_parser_get_no_logo(void);
uint8_t argument_parser_get_pause(void);
uint8_t argument_parser_get_program_help(void);
uint8_t argument_parser_get_project_help(void);
uint8_t argument_parser_get_quiet(void);
uint8_t argument_parser_get_verbose(void);
uint16_t argument_parser_get_encoding(void);

const void* argument_parser_get_properties(void);

const uint8_t* argument_parser_get_build_file(int index);
const uint8_t* argument_parser_get_log_file(void);
const uint8_t* argument_parser_get_target(int index);
const uint8_t* argument_parser_get_listener(void);

void argument_parser_release(void);

#endif
