/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _STRING_UNIT_H_
#define _STRING_UNIT_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

uint8_t string_contains(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish);
uint8_t string_ends_with(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish);
const uint8_t* string_enumerate(
	const uint8_t* input_start, const uint8_t* input_finish,
	uint32_t* char_set);
uint8_t string_equal(
	const uint8_t* input_1_start, const uint8_t* input_1_finish,
	const uint8_t* input_2_start, const uint8_t* input_2_finish);
const uint8_t* string_find_any_symbol_like_or_not_like_that(
	const uint8_t* start, const uint8_t* finish,
	const uint8_t* that_start, const uint8_t* that_finish,
	uint8_t like, int8_t step);
ptrdiff_t string_get_length(
	const uint8_t* input_start, const uint8_t* input_finish);
ptrdiff_t string_index_of(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish);
ptrdiff_t string_index_of_any(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish);
ptrdiff_t string_index_of_value(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish, int8_t step);
ptrdiff_t string_last_index_of(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish);
ptrdiff_t string_last_index_of_any(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish);
uint8_t string_pad_left(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish,
	ptrdiff_t result_length, struct buffer* output);
uint8_t string_pad_right(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish,
	ptrdiff_t result_length, struct buffer* output);
uint8_t string_quote(
	const uint8_t* input_start, const uint8_t* input_finish,
	struct buffer* output);
uint8_t string_replace(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* to_be_replaced_start, const uint8_t* to_be_replaced_finish,
	const uint8_t* by_replacement_start, const uint8_t* by_replacement_finish,
	struct buffer* output);
uint8_t string_starts_with(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish);
uint8_t string_substring(
	const uint8_t* input_start, const uint8_t* input_finish,
	ptrdiff_t index, ptrdiff_t length, struct range* output);
uint32_t string_to_case(uint32_t input, uint8_t required_case);
uint8_t string_to_lower(
	const uint8_t* input_start, const uint8_t* input_finish,
	struct buffer* output);
uint8_t string_to_upper(
	const uint8_t* input_start, const uint8_t* input_finish,
	struct buffer* output);
uint8_t string_transform_to_case(
	const uint8_t* input_start, const uint8_t* input_finish,
	struct buffer* output, uint8_t required_case);
uint8_t string_trim(struct range* input_output);
uint8_t string_trim_any(
	struct range* input_output, uint8_t mode, const uint16_t* trim_symbols,
	uint8_t count_of_symbols);
uint8_t string_trim_end(struct range* input_output);
uint8_t string_trim_start(struct range* input_output);
uint8_t string_un_quote(struct range* input_output);

#endif
