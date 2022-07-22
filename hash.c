/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "hash.h"
#include "buffer.h"
#include "conversion.h"

uint8_t hash_algorithm_bytes_to_string(
	const uint8_t* start, const uint8_t* finish, struct buffer* output)
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
		if (!hash_algorithm_uint32_t_to_uint8_t_array(*start, output))
		{
			return 0;
		}

		output += sizeof(uint32_t);
		++start;
	}

	return 1;
}

uint8_t hash_algorithm_uint32_t_to_uint8_t_array(
	uint32_t input, uint8_t* output)
{
	if (NULL == output)
	{
		return 0;
	}

	uint8_t i = 0;

	do
	{
		output[i] = (uint8_t)(input % 16);
		input /= 16;
		output[i++] += (uint8_t)((input % 16) * 16);
	}
	while (15 < (input /= 16));

	while (i < sizeof(uint32_t))
	{
		output[i++] = (uint8_t)input;
		input = 0;
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
		if (!hash_algorithm_uint64_t_to_uint8_t_array(*start, output))
		{
			return 0;
		}

		output += sizeof(uint64_t);
		++start;
	}

	return 1;
}

uint8_t hash_algorithm_uint64_t_to_uint8_t_array(uint64_t input, uint8_t* output)
{
	if (NULL == output)
	{
		return 0;
	}

	uint8_t i = 0;

	do
	{
		output[i] = (uint8_t)(input % 16);
		input /= 16;
		output[i++] += (uint8_t)((input % 16) * 16);
	}
	while (15 < (input /= 16));

	while (i < sizeof(uint64_t))
	{
		output[i++] = (uint8_t)input;
		input = 0;
	}

	return 1;
}

uint8_t hash_algorithm_uint8_t_array_to_uint32_t(
	const uint8_t* start, const uint8_t* finish, uint32_t* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	uint8_t j = 0;
	*output = 0;

	while ((--finish) > (start - 1) && j < 8)
	{
		*output += (uint32_t)(((*finish) & 0xF0) >> 4) * ((uint32_t)1 << (4 * (7 - (j++))));
		*output += (uint32_t)((*finish) & 0x0F) * ((uint32_t)1 << (4 * (7 - (j++))));
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
		if (!hash_algorithm_uint8_t_array_to_uint32_t(input + i, input + i + sizeof(uint32_t), output + j))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t hash_algorithm_uint8_t_array_to_uint64_t(
	const uint8_t* start, const uint8_t* finish, uint64_t* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	uint8_t j = 0;
	*output = 0;

	while ((--finish) > (start - 1) && j < 16)
	{
		*output += (uint64_t)(((*finish) & 0xF0) >> 4) * ((uint64_t)1 << (4 * (15 - (j++))));
		*output += (uint64_t)((*finish) & 0x0F) * ((uint64_t)1 << (4 * (15 - (j++))));
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
		if (!hash_algorithm_uint8_t_array_to_uint64_t(input + i, input + i + sizeof(uint64_t), output + j))
		{
			return 0;
		}
	}

	return 1;
}
