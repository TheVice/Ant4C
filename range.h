/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _RANGE_H_
#define _RANGE_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

struct range
{
	const uint8_t* start;
	const uint8_t* finish;
};

ptrdiff_t range_size(const struct range* range);
uint8_t range_is_null_or_empty(const struct range* range);
uint8_t range_in_parts_is_null_or_empty(const uint8_t* range_start, const uint8_t* range_finish);

uint8_t range_from_string(const uint8_t* input, ptrdiff_t size, ptrdiff_t count_of_sub_strings,
						  struct buffer* output);

uint8_t buffer_append_data_from_range(struct buffer* storage, const struct range* data);

uint8_t buffer_append_range(struct buffer* ranges, const struct range* data, ptrdiff_t data_count);
struct range* buffer_range_data(const struct buffer* ranges, ptrdiff_t data_position);

#define BUFFER_TO_RANGE(R, B)				\
	(R).start = buffer_data((B), 0);		\
	(R).finish = (R).start + buffer_size(B);

#endif
