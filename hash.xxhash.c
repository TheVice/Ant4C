/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 https://github.com/TheVice/
 *
 */

/*
 * As reference used:
 * xxHash fast digest algorithm. Version 0.1.1 (10/10/18)
 * https://github.com/Cyan4973/xxHash/blob/ff5df558b7bad19bc060d756f4dbd528b202c820/doc/xxhash_spec.md
 */

#include "hash.h"

#include <stddef.h>
#include <stdint.h>

#define ROTATE_LEFT_UINT32_T(VALUE, OFFSET)				\
	(((VALUE) << (OFFSET)) | (VALUE) >> (32 - (OFFSET)))

uint32_t hash_algorithm_XXH32(const uint8_t* input, uint32_t length, uint32_t seed)
{
	static const uint32_t PRIME32_1 = 0x9E3779B1U;
	static const uint32_t PRIME32_2 = 0x85EBCA77U;
	static const uint32_t PRIME32_3 = 0xC2B2AE3DU;
	static const uint32_t PRIME32_4 = 0x27D4EB2FU;
	static const uint32_t PRIME32_5 = 0x165667B1U;

	if (length && !input)
	{
		return 0;
	}

	uint32_t accumulators[5];

	if (length < 16)
	{
		accumulators[4] = seed + PRIME32_5;
		/**/
		accumulators[4] += length;
	}
	else
	{
		accumulators[0] = seed + PRIME32_1 + PRIME32_2;
		accumulators[1] = seed + PRIME32_2;
		accumulators[2] = seed;
		accumulators[3] = seed - PRIME32_1;
		/**/
		accumulators[4] = length;

		while (16 <= length)
		{
			for (uint8_t i = 0; i < 4; ++i, input += 4, length -= 4)
			{
				if (!hash_algorithm_uint8_t_array_to_uint32_t(input, input + 4, &seed))
				{
					return 0;
				}

				accumulators[i] += seed * PRIME32_2;
				accumulators[i] = ROTATE_LEFT_UINT32_T(accumulators[i], 13);
				accumulators[i] *= PRIME32_1;
			}
		}

		accumulators[4] += ROTATE_LEFT_UINT32_T(accumulators[0], 1) +
						   ROTATE_LEFT_UINT32_T(accumulators[1], 7) +
						   ROTATE_LEFT_UINT32_T(accumulators[2], 12) +
						   ROTATE_LEFT_UINT32_T(accumulators[3], 18);
	}

	for (; 4 <= length; input += 4, length -= 4)
	{
		if (!hash_algorithm_uint8_t_array_to_uint32_t(input, input + 4, &seed))
		{
			return 0;
		}

		accumulators[4] += seed * PRIME32_3;
		accumulators[4] = ROTATE_LEFT_UINT32_T(accumulators[4], 17) * PRIME32_4;
	}

	for (; 1 <= length; ++input, --length)
	{
		accumulators[4] += ((uint32_t)(*input)) * PRIME32_5;
		accumulators[4] = ROTATE_LEFT_UINT32_T(accumulators[4], 11) * PRIME32_1;
	}

	accumulators[4] = accumulators[4] ^ (accumulators[4] >> 15);
	accumulators[4] = accumulators[4] * PRIME32_2;
	accumulators[4] = accumulators[4] ^ (accumulators[4] >> 13);
	accumulators[4] = accumulators[4] * PRIME32_3;
	accumulators[4] = accumulators[4] ^ (accumulators[4] >> 16);
	/**/
	return accumulators[4];
}

#define ROTATE_LEFT_UINT64_T(VALUE, OFFSET)				\
	(((VALUE) << (OFFSET)) | (VALUE) >> (64 - (OFFSET)))

#define XXH64_ROUND(OUT, IN, LINE)				\
	(OUT) = (IN) + (LINE) * PRIME64_2;			\
	(OUT) = ROTATE_LEFT_UINT64_T((OUT), 31);	\
	(OUT) *= PRIME64_1;

#define XXH64_MERGE(OUT, SUB_OUT, IN)			\
	XXH64_ROUND((SUB_OUT), 0, (IN));			\
	(OUT) ^= (SUB_OUT);							\
	(OUT) *= PRIME64_1;							\
	(OUT) += PRIME64_4;

uint64_t hash_algorithm_XXH64(const uint8_t* input, uint64_t length, uint64_t seed)
{
	static const uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;
	static const uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
	static const uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
	static const uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
	static const uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;

	if (length && !input)
	{
		return 0;
	}

	const uint64_t length_ = length;
	uint64_t accumulators[5];

	if (length < 32)
	{
		accumulators[4] = seed + PRIME64_5;
	}
	else
	{
		accumulators[0] = seed + PRIME64_1 + PRIME64_2;
		accumulators[1] = seed + PRIME64_2;
		accumulators[2] = seed;
		accumulators[3] = seed - PRIME64_1;
		/**/
		accumulators[4] = 0;

		while (32 <= length)
		{
			for (uint8_t i = 0; i < 4; ++i, input += 8, length -= 8)
			{
				if (!hash_algorithm_uint8_t_array_to_uint64_t(input, input + 8, &seed))
				{
					return 0;
				}

				XXH64_ROUND(accumulators[i], accumulators[i], seed);
			}
		}

		accumulators[4] += ROTATE_LEFT_UINT64_T(accumulators[0], 1) +
						   ROTATE_LEFT_UINT64_T(accumulators[1], 7) +
						   ROTATE_LEFT_UINT64_T(accumulators[2], 12) +
						   ROTATE_LEFT_UINT64_T(accumulators[3], 18);

		for (uint8_t i = 0; i < 4; ++i)
		{
			XXH64_MERGE(accumulators[4], seed, accumulators[i]);
		}
	}

	accumulators[4] += length_;

	for (; 8 <= length; input += 8, length -= 8)
	{
		if (!hash_algorithm_uint8_t_array_to_uint64_t(input, input + 8, &seed))
		{
			return 0;
		}

		XXH64_ROUND(accumulators[3], 0, seed);
		accumulators[4] = accumulators[4] ^ accumulators[3];
		accumulators[4] = ROTATE_LEFT_UINT64_T(accumulators[4], 27) * PRIME64_1;
		accumulators[4] = accumulators[4] + PRIME64_4;
	}

	for (; 4 <= length; input += 4, length -= 4)
	{
		uint32_t* ptr = (uint32_t*)&seed;

		if (!hash_algorithm_uint8_t_array_to_uint32_t(input, input + 4, ptr))
		{
			return 0;
		}

		accumulators[4] ^= (uint64_t)(*ptr) * PRIME64_1;
		accumulators[4] = ROTATE_LEFT_UINT64_T(accumulators[4], 23) * PRIME64_2;
		accumulators[4] = accumulators[4] + PRIME64_3;
	}

	for (; 1 <= length; ++input, --length)
	{
		accumulators[4] ^= ((uint64_t)(*input)) * PRIME64_5;
		accumulators[4] = ROTATE_LEFT_UINT64_T(accumulators[4], 11) * PRIME64_1;
	}

	accumulators[4] ^= accumulators[4] >> 33;
	accumulators[4] *= PRIME64_2;
	accumulators[4] ^= accumulators[4] >> 29;
	accumulators[4] *= PRIME64_3;
	accumulators[4] ^= accumulators[4] >> 32;
	/**/
	return accumulators[4];
}
