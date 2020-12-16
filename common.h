/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <wchar.h>
#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

const char* find_any_symbol_like_or_not_like_that(const char* start, const char* finish,
		const char* that, ptrdiff_t that_length, uint8_t like, int8_t step);
const wchar_t* find_any_symbol_like_or_not_like_that_wchar_t(const wchar_t* start, const wchar_t* finish,
		const wchar_t* that, ptrdiff_t that_length, uint8_t like, int8_t step);
void replace_double_char_by_single(char* string, ptrdiff_t* length, char to_be_replaced);

uint8_t common_string_to_enum(const char* string_start, const char* string_finish,
							  const char** reference_strings, uint8_t max_enum_value);

uint8_t common_append_string_to_buffer(const char* input, struct buffer* output);

uint8_t common_unbox_char_data(const struct buffer* box_with_data, uint8_t i, uint8_t j,
							   struct range* data, uint8_t terminate);
uint8_t common_get_one_argument(const struct buffer* arguments, struct range* argument, uint8_t terminate);
uint8_t common_get_two_arguments(const struct buffer* arguments, struct range* argument1,
								 struct range* argument2, uint8_t terminate);
uint8_t common_get_three_arguments(const struct buffer* arguments, struct range* argument1,
								   struct range* argument2, struct range* argument3, uint8_t terminate);
uint8_t common_unbox_bool_data(const struct buffer* box_with_data, uint8_t i, uint8_t j, uint8_t* data);
int64_t common_unbox_int64_data(const struct buffer* box_with_data, uint8_t i, uint8_t j);

uint8_t read_file(const char* file_path, struct buffer* content);

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

#if defined(_WIN32)

#define WIDE2MULTI(W, M, COUNT)	\
	(COUNT) = (0 < WideCharToMultiByte(CP_UTF8, 0, (W), (COUNT), (M), (COUNT), NULL, NULL));
#endif

#define MULTI2WIDE(M, W, COUNT)	\
	(COUNT) = (0 < MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (M), (COUNT), (W), (COUNT)));

#endif
