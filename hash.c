/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "hash.h"
#include "buffer.h"
#include "common.h"
#include "range.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

uint8_t hash_algorithm_bytes_to_string(const uint8_t* start, const uint8_t* finish,
									   struct buffer* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 2 * (finish - start) + 2))
	{
		return 0;
	}

	const ptrdiff_t max_size = buffer_size(output) - 3;
	char* ptr = (char*)buffer_data(output, size);

	while (start < finish && size < max_size)
	{
		if ((*start) < 16)
		{
			(*ptr) = '0';
			++ptr;
			++size;
		}

#if __STDC_SEC_API__
		const int32_t sz = sprintf_s(ptr, 3, "%x", *start);
#else
		const int32_t sz = sprintf(ptr, "%x", *start);
#endif
		ptr += sz;
		size += sz;
		++start;
	}

	return buffer_resize(output, size);
}

static const uint8_t* hash_function_str[] =
{
	(const uint8_t*)"blake2b-160",
	(const uint8_t*)"blake2b-256",
	(const uint8_t*)"blake2b-384",
	(const uint8_t*)"blake2b-512",
	(const uint8_t*)"blake3-256",
	(const uint8_t*)"bytes-to-string",
	(const uint8_t*)"crc32",
	(const uint8_t*)"keccak-224",
	(const uint8_t*)"keccak-256",
	(const uint8_t*)"keccak-384",
	(const uint8_t*)"keccak-512",
	(const uint8_t*)"sha3-224",
	(const uint8_t*)"sha3-256",
	(const uint8_t*)"sha3-384",
	(const uint8_t*)"sha3-512"
};

enum hash_function
{
	blake2b_160, blake2b_256, blake2b_384, blake2b_512,
	blake3_256,
	bytes_to_string,
	crc32,
	keccak_224, keccak_256, keccak_384, keccak_512,
	sha3_224, sha3_256, sha3_384, sha3_512,
	UNKNOWN_HASH_FUNCTION
};

uint8_t hash_algorithm_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, hash_function_str, UNKNOWN_HASH_FUNCTION);
}

uint8_t hash_algorithm_exec_function(uint8_t function, const struct buffer* arguments,
									 uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_HASH_FUNCTION <= function ||
		NULL == arguments ||
		arguments_count < 1 || 2 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range argument1;

	struct range argument2;

	argument1.start = argument2.start = argument1.finish = argument2.finish = NULL;

	if (1 == arguments_count)
	{
		if (!common_get_one_argument(arguments, &argument1, 0))
		{
			argument1.start = argument1.finish = (const uint8_t*)&argument1;
		}
	}
	else if (2 == arguments_count)
	{
		if (!common_get_two_arguments(arguments, &argument1, &argument2, 0))
		{
			argument1.start = argument1.finish = (const uint8_t*)&argument1;
			arguments_count = 1;
		}
	}

	switch (function)
	{
		case blake2b_160:
			return 1 == arguments_count && hash_algorithm_blake2b_160(argument1.start, argument1.finish, output);

		case blake2b_256:
			return 1 == arguments_count && hash_algorithm_blake2b_256(argument1.start, argument1.finish, output);

		case blake2b_384:
			return 1 == arguments_count && hash_algorithm_blake2b_384(argument1.start, argument1.finish, output);

		case blake2b_512:
			return 1 == arguments_count && hash_algorithm_blake2b_512(argument1.start, argument1.finish, output);

		case blake3_256:
			return 1 == arguments_count && hash_algorithm_blake3_256(argument1.start, argument1.finish, output);

		case bytes_to_string:
			return 1 == arguments_count && hash_algorithm_bytes_to_string(argument1.start, argument1.finish, output);

		case crc32:
		{
			uint8_t order = 1;

			/* "decreasing" 0
			 * "increasing" 1 */
			if (2 == arguments_count)
			{
				if (10 != range_size(&argument2))
				{
					break;
				}

				order = (0 == memcmp("decreasing", argument2.start, 10) ? 0 :
						 (0 == memcmp("increasing", argument2.start, 10) ? 1 : 2));

				if (1 < order)
				{
					break;
				}
			}

			if (!buffer_push_back_uint32(output, 0))
			{
				break;
			}

			uint32_t* ptr = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));
			return hash_algorithm_crc32(argument1.start, argument1.finish, ptr, order);
		}

		case keccak_224:
			return 1 == arguments_count && hash_algorithm_keccak_224(argument1.start, argument1.finish, output);

		case keccak_256:
			return 1 == arguments_count && hash_algorithm_keccak_256(argument1.start, argument1.finish, output);

		case keccak_384:
			return 1 == arguments_count && hash_algorithm_keccak_384(argument1.start, argument1.finish, output);

		case keccak_512:
			return 1 == arguments_count && hash_algorithm_keccak_512(argument1.start, argument1.finish, output);

		case sha3_224:
			return 1 == arguments_count && hash_algorithm_sha3_224(argument1.start, argument1.finish, output);

		case sha3_256:
			return 1 == arguments_count && hash_algorithm_sha3_256(argument1.start, argument1.finish, output);

		case sha3_384:
			return 1 == arguments_count && hash_algorithm_sha3_384(argument1.start, argument1.finish, output);

		case sha3_512:
			return 1 == arguments_count && hash_algorithm_sha3_512(argument1.start, argument1.finish, output);

		case UNKNOWN_HASH_FUNCTION:
		default:
			break;
	}

	return 0;
}
