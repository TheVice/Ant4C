/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#if defined(_WIN32)
#include <wchar.h>
#endif
#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

const uint8_t* find_any_symbol_like_or_not_like_that(const uint8_t* start, const uint8_t* finish,
		const uint8_t* that, ptrdiff_t that_length, uint8_t like, int8_t step);
const uint32_t* find_any_symbol_like_or_not_like_that_UTF32LE(
	const uint32_t* start, const uint32_t* finish,
	const uint16_t* that, ptrdiff_t that_length, uint8_t like, int8_t step);
#if defined(_WIN32)
const wchar_t* find_any_symbol_like_or_not_like_that_wchar_t(const wchar_t* start, const wchar_t* finish,
		const wchar_t* that, ptrdiff_t that_length, uint8_t like, int8_t step);
#endif
uint8_t common_replace_double_byte_by_single(uint8_t* input, ptrdiff_t* size, uint8_t to_be_replaced);

ptrdiff_t common_count_bytes_until(const uint8_t* bytes, uint8_t until);

uint8_t common_string_to_enum(const uint8_t* string_start, const uint8_t* string_finish,
							  const uint8_t** reference_strings, uint8_t max_enum_value);

uint8_t common_append_string_to_buffer(const uint8_t* input, struct buffer* output);

uint8_t common_get_arguments(const struct buffer* boxed_arguments,
							 uint8_t arguments_count, struct range* arguments, uint8_t terminate);

uint8_t common_get_attributes_and_arguments_for_task(
	const uint8_t** input_task_attributes,
	const uint8_t* input_task_attributes_lengths,
	uint8_t input_task_attributes_count,
	const uint8_t*** task_attributes,
	const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count,
	struct buffer* task_arguments);

void common_set_output_stream(void* stream);
void common_set_error_output_stream(void* stream);
void* common_get_output_stream();
void* common_get_error_output_stream();
uint8_t common_is_output_stream_standard();
uint8_t common_is_error_output_stream_standard();

void common_set_module_priority(uint8_t priority);
uint8_t common_get_module_priority();

#define XCHG(A, Z)	\
	(A) -= (Z);		\
	(Z) += (A);		\
	(A) = (Z) - (A);

#define COUNT_OF(A) (sizeof(A) / sizeof(*(A)))

#define MEM_CPY(DST, SRC, LENGTH)								\
	for (ptrdiff_t counter = 0; counter < LENGTH; ++counter)	\
	{															\
		(*(DST)) = (*(SRC));									\
		++(DST);												\
		++(SRC);												\
	}

#if !defined(MAX)
#define MAX(A, B) ((A) < (B) ? (B) : (A))
#endif

#if !defined(MIN)
#define MIN(A, B) ((A) > (B) ? (B) : (A))
#endif

#define FAIL_WITH_OUT_ERROR					2
#define ATTEMPT_TO_WRITE_READ_ONLY_PROPERTY	3

#endif
