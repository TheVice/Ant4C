/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#ifndef _INTERPRETER_STRING_UNIT_H_
#define _INTERPRETER_STRING_UNIT_H_

#include <stdint.h>

uint8_t string_get_id_of_to_lower_function();
uint8_t string_get_id_of_to_upper_function();
uint8_t string_get_id_of_trim_function();
uint8_t string_get_id_of_trim_end_function();
uint8_t string_get_id_of_trim_start_function();
uint8_t string_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t string_exec_function(
	uint8_t function, const void* arguments,
	uint8_t arguments_count, void* output);

#endif
