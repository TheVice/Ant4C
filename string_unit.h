/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _STRING_UNIT_H_
#define _STRING_UNIT_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

uint8_t string_contains(const char* input_start, const char* input_finish,
						const char* value_start, const char* value_finish);
uint8_t string_ends_with(const char* input_start, const char* input_finish,
						 const char* value_start, const char* value_finish);
ptrdiff_t string_get_length(const char* input_start, const char* input_finish);
ptrdiff_t string_index_of(const char* input_start, const char* input_finish,
						  const char* value_start, const char* value_finish);
ptrdiff_t string_last_index_of(const char* input_start, const char* input_finish,
							   const char* value_start, const char* value_finish);
/*TODO: string_pad_left
string_pad_right*/
#if 0
uint8_t string_replace_in_buffer(struct buffer* input_output,
								 const char* to_be_replaced, ptrdiff_t to_be_replaced_length,
								 const char* by_replacement, ptrdiff_t by_replacement_length);
#endif
uint8_t string_replace(const char* input_start, const char* input_finish,
					   const char* to_be_replaced_start, const char* to_be_replaced_finish,
					   const char* by_replacement_start, const char* by_replacement_finish,
					   struct buffer* output);
uint8_t string_starts_with(const char* input_start, const char* input_finish,
						   const char* value_start, const char* value_finish);
uint8_t string_substring(const char* input, ptrdiff_t input_length,
						 ptrdiff_t index, ptrdiff_t length, struct buffer* output);
uint8_t string_to_lower(const char* input_start, const char* input_finish, char* output);
uint8_t string_to_upper(const char* input_start, const char* input_finish, char* output);
uint8_t string_trim(struct range* input_output);
uint8_t string_trim_end(struct range* input_output);
uint8_t string_trim_start(struct range* input_output);
uint8_t string_quote(const char* input_start, const char* input_finish,
					 struct buffer* output);
uint8_t string_un_quote(struct range* input_output);
uint8_t string_equal(const char* input_1_start, const char* input_1_finish,
					 const char* input_2_start, const char* input_2_finish);

uint8_t string_get_function(const char* name_start, const char* name_finish);
uint8_t string_exec_function(uint8_t function, const struct buffer* arguments,
							 uint8_t arguments_count, struct buffer* output);

#endif
