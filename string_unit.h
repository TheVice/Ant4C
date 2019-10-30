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

uint8_t string_contains(const uint8_t* input_start, const uint8_t* input_finish,
						const uint8_t* value_start, const uint8_t* value_finish);
uint8_t string_ends_with(const uint8_t* input_start, const uint8_t* input_finish,
						 const uint8_t* value_start, const uint8_t* value_finish);
ptrdiff_t string_get_length(const uint8_t* input_start, const uint8_t* input_finish);
ptrdiff_t string_index_of(const uint8_t* input_start, const uint8_t* input_finish,
						  const uint8_t* value_start, const uint8_t* value_finish);
ptrdiff_t string_last_index_of(const uint8_t* input_start, const uint8_t* input_finish,
							   const uint8_t* value_start, const uint8_t* value_finish);
/*TODO: string_pad_left
string_pad_right*/
uint8_t string_replace(const uint8_t* input_start, const uint8_t* input_finish,
					   const uint8_t* to_be_replaced_start, const uint8_t* to_be_replaced_finish,
					   const uint8_t* by_replacement_start, const uint8_t* by_replacement_finish,
					   struct buffer* output);
uint8_t string_starts_with(const uint8_t* input_start, const uint8_t* input_finish,
						   const uint8_t* value_start, const uint8_t* value_finish);
uint8_t string_substring(const uint8_t* input, ptrdiff_t input_length,
						 ptrdiff_t index, ptrdiff_t length, struct buffer* output);
uint8_t string_to_lower(const uint8_t* input_start, const uint8_t* input_finish, uint8_t* output);
uint8_t string_to_upper(const uint8_t* input_start, const uint8_t* input_finish, uint8_t* output);
uint8_t string_trim(struct range* input_output);
uint8_t string_trim_end(struct range* input_output);
uint8_t string_trim_start(struct range* input_output);
uint8_t string_quote(const uint8_t* input_start, const uint8_t* input_finish,
					 struct buffer* output);
uint8_t string_un_quote(struct range* input_output);
uint8_t string_equal(const uint8_t* input_1_start, const uint8_t* input_1_finish,
					 const uint8_t* input_2_start, const uint8_t* input_2_finish);

uint8_t string_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t string_exec_function(uint8_t function, const struct buffer* arguments,
							 uint8_t arguments_count, struct buffer* output);

#endif
