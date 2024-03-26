/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 TheVice
 *
 */

#include "bit_converter.h"

#include <stddef.h>

#define X_TO_BYTES(count, input, output)			\
	if (NULL == (output))							\
	{												\
		return 0;									\
	}												\
	\
	for (uint8_t i = 0; i < (count); ++i)			\
	{												\
		(output)[i] = (uint8_t)((input) & 0xFF);	\
		(input) >>= 8;								\
	}												\
	\
	return 1;

#define BYTES_TO_X(count, start, finish, output)									\
	if (NULL == (start) ||															\
		NULL == (finish) ||															\
		(finish) < (start) ||														\
		NULL == (output))															\
	{																				\
		return 0;																	\
	}																				\
	\
	*(output) = 0;																	\
	uint8_t j = 0;																	\
	\
	for (const uint8_t* i = (finish) - 1; i >= (start) && j < (count); --i, ++j)	\
	{																				\
		*(output) <<= 8;															\
		*(output) += *i;															\
	}																				\
	\
	return 1;

uint8_t bit_converter_is_little_endian()
{
	union endian_union
	{
		uint8_t bytes[2];
		uint16_t data;
	};
	//
	union endian_union endian;
	endian.data = 4128;
	return 32 == endian.bytes[0];
}

uint8_t bit_converter_get_bytes_from_uint16_t(uint16_t input, uint8_t* output)
{
	X_TO_BYTES(2, input, output);
}

uint8_t bit_converter_get_bytes_from_uint32_t(uint32_t input, uint8_t* output)
{
	X_TO_BYTES(4, input, output);
}

uint8_t bit_converter_get_bytes_from_uint64_t(uint64_t input, uint8_t* output)
{
	X_TO_BYTES(8, input, output);
}

uint8_t bit_converter_to_uint16_t(const uint8_t* start, const uint8_t* finish, uint16_t* output)
{
	BYTES_TO_X(2, start, finish, output);
}

uint8_t bit_converter_to_uint32_t(const uint8_t* start, const uint8_t* finish, uint32_t* output)
{
	BYTES_TO_X(4, start, finish, output);
}

uint8_t bit_converter_to_uint64_t(const uint8_t* start, const uint8_t* finish, uint64_t* output)
{
	BYTES_TO_X(8, start, finish, output);
}
