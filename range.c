/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 TheVice
 *
 */

#include "range.h"
#include "buffer.h"
#include "common.h"

ptrdiff_t range_size(const struct range* range)
{
	return range_is_null_or_empty(range) ? 0 : range->finish - range->start;
}

uint8_t range_is_null_or_empty(const struct range* range)
{
	return NULL == range ||
		   range_in_parts_is_null_or_empty(range->start, range->finish);
}

uint8_t range_in_parts_is_null_or_empty(const uint8_t* range_start, const uint8_t* range_finish)
{
	return NULL == range_start ||
		   NULL == range_finish ||
		   range_finish <= range_start;
}

uint8_t range_from_string(const uint8_t* input, ptrdiff_t size, ptrdiff_t count_of_sub_strings,
						  struct buffer* output)
{
	if (NULL == input ||
		size < 1 ||
		count_of_sub_strings < 1 ||
		NULL == output)
	{
		return 0;
	}

	if (!buffer_resize(output, 0) ||
		!buffer_append_range(output, NULL, count_of_sub_strings) ||
		!buffer_append(output, input, size))
	{
		return 0;
	}

	const uint8_t* max_ptr = buffer_data(output, 0) + buffer_size(output);
	const uint8_t* ptr = (const uint8_t*)buffer_range_data(output, count_of_sub_strings);

	if (NULL == ptr ||
		!buffer_resize(output, count_of_sub_strings * sizeof(struct range)))
	{
		return 0;
	}

	for (size = 0; size < count_of_sub_strings; ++size)
	{
		struct range* element = buffer_range_data(output, size);

		if (NULL == element)
		{
			return 0;
		}

		element->start = ptr;
		element->finish = element->start + common_count_bytes_until(ptr, 0);
		ptr = element->finish + 1;

		if (max_ptr < ptr)
		{
			return 0;
		}
	}

	return 1;
}

uint8_t buffer_append_data_from_range(struct buffer* storage, const struct range* data)
{
	return NULL != data && NULL != data->start && NULL != data->finish &&
		   data->start <= data->finish &&
		   buffer_append(storage, data->start, range_size(data));
}

uint8_t buffer_append_range(struct buffer* ranges, const struct range* data, ptrdiff_t data_count)
{
	return buffer_append(ranges, (const uint8_t*)data, sizeof(struct range) * data_count);
}

struct range* buffer_range_data(const struct buffer* ranges, ptrdiff_t data_position)
{
	return (struct range*)buffer_data(ranges, sizeof(struct range) * data_position);
}
