/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019, 2021 - 2022 TheVice
 *
 */

#include "range.h"

#include "buffer.h"

ptrdiff_t range_size(const struct range* range)
{
	return range_is_null_or_empty(range) ? 0 : range->finish - range->start;
}

uint8_t range_is_null_or_empty(const struct range* range)
{
	return NULL == range ||
		   range_in_parts_is_null_or_empty(range->start, range->finish);
}

uint8_t range_in_parts_is_null_or_empty(
	const uint8_t* range_start, const uint8_t* range_finish)
{
	return NULL == range_start ||
		   NULL == range_finish ||
		   range_finish <= range_start;
}

uint8_t buffer_append_data_from_range(
	void* storage, const struct range* data)
{
	return NULL != data && NULL != data->start && NULL != data->finish &&
		   data->start <= data->finish &&
		   buffer_append(storage, data->start, range_size(data));
}

uint8_t buffer_append_range(
	void* ranges, const struct range* data, ptrdiff_t data_count)
{
	return buffer_append(
			   ranges, (const void*)data, sizeof(struct range) * data_count);
}

struct range* buffer_range_data(
	const void* ranges, ptrdiff_t data_position)
{
	return (struct range*)buffer_data(
			   ranges, sizeof(struct range) * data_position);
}
