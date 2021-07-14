/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "arguments.h"

#include "buffer.h"
#include "file_system.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

uint8_t values_to_arguments(
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count, struct buffer* output, const type_of_element*** argv)
{
	if (NULL == values ||
		NULL == values_lengths ||
		!output ||
		!argv)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (values_count)
	{
		if (!buffer_append(output, NULL, ((ptrdiff_t)1 + values_count) * sizeof(ptrdiff_t)))
		{
			return 0;
		}
	}

	for (uint16_t i = 0; i < values_count; ++i)
	{
		ptrdiff_t* positions = (ptrdiff_t*)buffer_data(output, size);
		positions[i] = buffer_size(output);
#if defined(_WIN32)

		if (values_lengths[i] &&
			!text_encoding_UTF8_to_UTF16LE(values[i], values[i] + values_lengths[i], output))
		{
			return 0;
		}

		if (!buffer_push_back_uint16(output, 0))
		{
			return 0;
		}

#else

		if (values_lengths[i] &&
			!buffer_append(output, values[i], values_lengths[i]))
		{
			return 0;
		}

		if (!buffer_push_back(output, 0))
		{
			return 0;
		}

#endif
	}

	if (values_count)
	{
		const ptrdiff_t sz = buffer_size(output);

		if (!buffer_append(output, NULL, ((ptrdiff_t)1 + values_count) * sizeof(type_of_element*)) ||
			!buffer_resize(output, sz))
		{
			return 0;
		}

		const ptrdiff_t* positions = (const ptrdiff_t*)buffer_data(output, size);

		for (uint16_t i = 0; i < values_count; ++i)
		{
			const type_of_element* data_at_position = (const type_of_element*)buffer_data(output, positions[i]);

			if (!buffer_append(output, (const uint8_t*)&data_at_position, sizeof(type_of_element*)))
			{
				return 0;
			}
		}

		*argv = (const type_of_element**)buffer_data(output, sz);
	}
	else
	{
		*argv = NULL;
	}

	return 1;
}

uint8_t value_to_system_path(const uint8_t* input, ptrdiff_t length, struct buffer* output)
{
#if defined(_WIN32)
	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (ptrdiff_t)4 * length + INT8_MAX) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

#endif

	if (!buffer_append(output, input, length) ||
		!buffer_push_back(output, 0))
	{
		return 0;
	}

#if defined(_WIN32)

	if (!file_system_path_to_pathW(buffer_data(output, size), output))
	{
		return 0;
	}

#endif
	return 1;
}

uint8_t values_to_system_paths(
	const uint8_t** values, const uint16_t* values_lengths,
	ptrdiff_t* positions, uint8_t values_count, struct buffer* output)
{
	if (!values ||
		!values_lengths ||
		!positions ||
		!output)
	{
		return 0;
	}

	for (uint8_t i = 0; i < values_count; ++i)
	{
		positions[i] = PTRDIFF_MAX;

		if (!values_lengths[i])
		{
			continue;
		}

		positions[i] = buffer_size(output);

		if (!value_to_system_path(values[i], values_lengths[i], output))
		{
			return 0;
		}

#if defined(_WIN32)
		positions[i] += values_lengths[i];
		++positions[i];
#endif
	}

	return 1;
}

uint8_t values_to_system_paths_(
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count,
	struct buffer* output, const type_of_element*** argv)
{
	if (!values ||
		!values_lengths ||
		!output ||
		!argv)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (values_count)
	{
		if (!buffer_append(output, NULL, ((ptrdiff_t)1 + values_count) * sizeof(ptrdiff_t)))
		{
			return 0;
		}
	}

	for (uint8_t i = 0; i < values_count; ++i)
	{
		ptrdiff_t* positions = (ptrdiff_t*)buffer_data(output, size);
		positions[i] = buffer_size(output);

		if (values_lengths[i])
		{
			if (!value_to_system_path(values[i], values_lengths[i], output))
			{
				return 0;
			}
		}
		else
		{
			if (!buffer_push_back_uint32(output, 0))
			{
				return 0;
			}
		}

#if defined(_WIN32)
		positions = (ptrdiff_t*)buffer_data(output, size);
		positions[i] += values_lengths[i];
		++positions[i];
#endif
	}

	if (values_count)
	{
		const ptrdiff_t sz = buffer_size(output);

		if (!buffer_append(output, NULL, ((ptrdiff_t)1 + values_count) * sizeof(type_of_element*)) ||
			!buffer_resize(output, sz))
		{
			return 0;
		}

		const ptrdiff_t* positions = (const ptrdiff_t*)buffer_data(output, size);

		for (uint16_t i = 0; i < values_count; ++i)
		{
			const type_of_element* data_at_position = (const type_of_element*)buffer_data(output, positions[i]);

			if (!buffer_append(output, (const uint8_t*)&data_at_position, sizeof(type_of_element*)))
			{
				return 0;
			}
		}

		*argv = (const type_of_element**)buffer_data(output, sz);
	}
	else
	{
		*argv = NULL;
	}

	return 1;
}
