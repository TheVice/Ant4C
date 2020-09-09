/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _CONVERSION_H_
#define _CONVERSION_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

uint8_t bool_parse(const uint8_t* input, ptrdiff_t input_length, uint8_t* bool_value);
uint8_t bool_to_string(uint8_t bool_value, struct buffer* output_string);

double double_parse(const uint8_t* value);
uint8_t double_to_string(double double_value, struct buffer* output_string);

int32_t int_parse(const uint8_t* value);
uint8_t int_to_string(int32_t int_value, struct buffer* output_string);

long long_parse(const uint8_t* value);
uint8_t long_to_string(long long_value, struct buffer* output_string);

int64_t int64_parse(const uint8_t* value);
uint8_t int64_to_string(int64_t int_value, struct buffer* output_string);

void* pointer_parse(const uint8_t* value);
uint8_t pointer_to_string(const void* pointer_value, struct buffer* output_string);

uint8_t conversion_get_function(const uint8_t* name_start, const uint8_t* name_finish);
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
