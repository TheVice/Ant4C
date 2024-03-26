/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022, 2024 TheVice
 *
 */

#include "hash.h"

#include "bit_converter.h"
#include "buffer.h"
#include "conversion.h"

uint8_t hash_algorithm_bytes_to_string(
	const uint8_t* start, const uint8_t* finish, void* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	if (finish == start)
	{
		return 1;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 2 * (finish - start) + 2) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	for (; start < finish; ++start)
	{
		if ((*start) < 16)
		{
			if (!buffer_push_back(output, '0'))
			{
				return 0;
			}
		}

		if (!int_to_hex(*start, output))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t hash_algorithm_uint32_t_array_to_uint8_t_array(
	const uint32_t* start, const uint32_t* finish, uint8_t* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish <= start ||
		NULL == output)
	{
		return 0;
	}

	while (start < finish)
	{
		if (!bit_converter_get_bytes_from_uint32_t(*start, output))
		{
			return 0;
		}

		output += sizeof(uint32_t);
		++start;
	}

	return 1;
}

uint8_t hash_algorithm_uint64_t_array_to_uint8_t_array(
	const uint64_t* start, const uint64_t* finish, uint8_t* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish <= start ||
		NULL == output)
	{
		return 0;
	}

	while (start < finish)
	{
		if (!bit_converter_get_bytes_from_uint64_t(*start, output))
		{
			return 0;
		}

		output += sizeof(uint64_t);
		++start;
	}

	return 1;
}

uint8_t hash_algorithm_uint8_t_array_to_uint32_t_array(
	const uint8_t* input, ptrdiff_t input_size, uint32_t* output)
{
	if (NULL == input ||
		0 != (input_size % sizeof(uint32_t)) ||
		NULL == output)
	{
		return 0;
	}

	for (ptrdiff_t i = 0, j = 0; i < input_size; i += sizeof(uint32_t), ++j)
	{
		if (!bit_converter_to_uint32_t(input + i, input + i + sizeof(uint32_t), output + j))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t hash_algorithm_uint8_t_array_to_uint64_t_array(
	const uint8_t* input, ptrdiff_t input_size, uint64_t* output)
{
	if (NULL == input ||
		0 != (input_size % sizeof(uint64_t)) ||
		NULL == output)
	{
		return 0;
	}

	for (ptrdiff_t i = 0, j = 0; i < input_size; i += sizeof(uint64_t), ++j)
	{
		if (!bit_converter_to_uint64_t(input + i, input + i + sizeof(uint64_t), output + j))
		{
			return 0;
		}
	}

	return 1;
}
