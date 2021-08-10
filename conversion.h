/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _CONVERSION_H_
#define _CONVERSION_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

uint8_t bool_parse(const uint8_t* input, ptrdiff_t length, uint8_t* output);
uint8_t bool_to_string(uint8_t input, struct buffer* output);

double double_parse(const uint8_t* value);
uint8_t double_to_string(double input, struct buffer* output);

int32_t int_parse(const uint8_t* value);
uint8_t int_to_string(int32_t input, struct buffer* output);

long long_parse(const uint8_t* value);
uint8_t long_to_string(long input, struct buffer* output);

int64_t int64_parse(const uint8_t* input_start, const uint8_t* input_finish);
uint8_t int64_to_string(int64_t input, struct buffer* output);

uint64_t uint64_parse(const uint8_t* input_start, const uint8_t* input_finish);
uint8_t uint64_to_string(uint64_t input, struct buffer* output);

void* pointer_parse(const uint8_t* value);
uint8_t pointer_to_string(const void* input, struct buffer* output);

#endif
