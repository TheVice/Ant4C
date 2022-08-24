/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020, 2022 TheVice
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

uint8_t argument_parser_init();

uint8_t argument_parser_get_debug();
uint8_t argument_parser_get_indent();
uint8_t argument_parser_get_module_priority();
uint8_t argument_parser_get_no_logo();
uint8_t argument_parser_get_pause();
uint8_t argument_parser_get_program_help();
uint8_t argument_parser_get_project_help();
uint8_t argument_parser_get_quiet();
uint8_t argument_parser_get_verbose();
uint16_t argument_parser_get_encoding();

const void* argument_parser_get_properties();

const uint8_t* argument_parser_get_build_file(int index);
const uint8_t* argument_parser_get_log_file();
const uint8_t* argument_parser_get_target(int index);
const uint8_t* argument_parser_get_listener();

void argument_parser_release();

#endif
