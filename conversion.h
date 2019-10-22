/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _CONVERSION_H_
#define _CONVERSION_H_

#include <wchar.h>
#include <stdint.h>

struct buffer;

uint8_t bool_parse(const char* input_string_start, const char* input_string_finish, uint8_t* bool_value);
uint8_t bool_to_string(uint8_t bool_value, struct buffer* output_string);

double double_parse(const char* value);
uint8_t double_to_string(double double_value, struct buffer* output_string);

int32_t int_parse(const char* value);
uint8_t int_to_string(int32_t int_value, struct buffer* output_string);

long long_parse(const char* value);
long long_parse_wchar_t(const wchar_t* value);
uint8_t long_to_string(long long_value, struct buffer* output_string);

int64_t int64_parse(const char* value);
int64_t int64_parse_wchar_t(const wchar_t* value);
uint8_t int64_to_string(int64_t int_value, struct buffer* output_string);

uint8_t conversion_get_function(const char* name_start, const char* name_finish);
uint8_t bool_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						   struct buffer* output);
uint8_t double_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							 struct buffer* output);
uint8_t int_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						  struct buffer* output);
uint8_t long_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						   struct buffer* output);
uint8_t int64_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							struct buffer* output);

#endif
