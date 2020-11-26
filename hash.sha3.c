/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2012 - 2020 https://github.com/TheVice/
 *
 */

/*
 * As reference used:
 *
 * NIST Selects Winner of Secure Hash Algorithm (SHA-3) Competition.
 * Date: October 02, 2012.
 * https://www.nist.gov/news-events/news/2012/10/nist-selects-winner-secure-hash-algorithm-sha-3-competition (original link http://www.nist.gov/itl/csd/sha-100212.cfm)
 *
 * Keccak, the new standard for hashing data.
 * Author: NeverWalkAloner (https://github.com/NeverWalkAloner)
 * Date: November 17, 2012.
 * https://habr.com/ru/post/159073/
 *
 * FIPS 202.
 * SHA-3 Standard: Permutation-Based Hash and Extendable-Output Functions.
 * Date Published: August 2015.
 * https://csrc.nist.gov/publications/detail/fips/202/final
 */

#include "hash.h"
#include "buffer.h"

#include <stddef.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

enum HashType { Hash512 = 0, Hash384, HashUnknown, Hash256, Hash224 };

static const uint16_t permutation_width = 1600;/*25, 50, 100, 200, 400, 800, 1600*/
static const uint8_t w = 64;/*permutation_width / 25;*/
/*static const uint8_t width_on_w = 25;*/

#define TWO_DIMENSION_TO_ONE_INDEX(X, Y, Y_MAX)	\
	(X) * (Y_MAX) + Y

#define ROT(X, N, W)	\
	(((X) << ((N) % (W))) | ((X) >> ((W) - ((N) % (W)))))

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
	(*output) = 0;

	while ((--finish) > (start - 1) && j < 16)
	{
		(*output) += (uint64_t)(((*finish) & 0xF0) >> 4) * ((uint64_t)1 << (4 * (15 - (j++))));
		(*output) += (uint64_t)((*finish) & 0x0F) * ((uint64_t)1 << (4 * (15 - (j++))));
	}

	return 1;
}

void Round(uint64_t* A, uint64_t RC_i)
{
	static const uint8_t r[] =
	{
		0, 36, 3, 41, 18,
		1, 44, 10, 45, 2,
		62, 6, 43, 15, 61,
		28, 55, 25, 21, 56,
		27, 20, 39, 8, 14
	};
	/**/
	uint64_t B[] =
	{
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};
	/**/
	uint64_t C[] = { 0, 0, 0, 0, 0 };
	/**/
	uint64_t D[] = { 0, 0, 0, 0, 0 };

	/*theta step*/
	for (uint8_t i = 0; i < 5; i++)
	{
		C[i] = A[TWO_DIMENSION_TO_ONE_INDEX(i, 0, 5)] ^
			   A[TWO_DIMENSION_TO_ONE_INDEX(i, 1, 5)] ^
			   A[TWO_DIMENSION_TO_ONE_INDEX(i, 2, 5)] ^
			   A[TWO_DIMENSION_TO_ONE_INDEX(i, 3, 5)] ^
			   A[TWO_DIMENSION_TO_ONE_INDEX(i, 4, 5)];
	}

	for (uint8_t i = 0; i < 5; i++)
	{
		D[i] = C[(i + 4) % 5] ^ ROT(C[(i + 1) % 5], 1, w);
	}

	for (uint8_t i = 0; i < 5; i++)
	{
		for (uint8_t j = 0; j < 5; j++)
		{
			A[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] =
				A[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] ^ D[i];
		}
	}

	/*rho and pi steps*/
	for (uint8_t i = 0; i < 5; i++)
	{
		for (uint8_t j = 0; j < 5; j++)
		{
			B[TWO_DIMENSION_TO_ONE_INDEX(j, (2 * i + 3 * j) % 5, 5)] =
				ROT(A[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)],
					r[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)], w);
		}
	}

	/*chi step*/
	for (uint8_t i = 0; i < 5; i++)
	{
		for (uint8_t j = 0; j < 5; j++)
		{
			A[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] =
				B[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] ^
				((~B[TWO_DIMENSION_TO_ONE_INDEX((i + 1) % 5, j, 5)]) &
				 B[TWO_DIMENSION_TO_ONE_INDEX((i + 2) % 5, j, 5)]);
		}
	}

	/*iota step*/
	A[TWO_DIMENSION_TO_ONE_INDEX(0, 0, 5)] =
		A[TWO_DIMENSION_TO_ONE_INDEX(0, 0, 5)] ^ RC_i;
}

void Keccak_f(uint64_t* A)
{
	/*l -> log(w) / log(2)
	number of permutation -> 12 + 2 * l*/
	static const uint8_t n = 24;/*12, 14, 16, 18, 20, 22, 24*/
	/**/
	static const uint64_t RC[] =
	{
		0x0000000000000001,
		0x0000000000008082,
		0x800000000000808A,
		0x8000000080008000,
		0x000000000000808B,
		0x0000000080000001,
		0x8000000080008081,
		0x8000000000008009,
		0x000000000000008A,
		0x0000000000000088,
		0x0000000080008009,
		0x000000008000000A,
		0x000000008000808B,
		0x800000000000008B,
		0x8000000000008089,
		0x8000000000008003,
		0x8000000000008002,
		0x8000000000000080,
		0x000000000000800A,
		0x800000008000000A,
		0x8000000080008081,
		0x8000000000008080,
		0x0000000080000001,
		0x8000000080008008
	};

	for (uint8_t i = 0; i < n; i++)
	{
		Round(A, RC[i]);
	}
}

void Keccak_absorption(uint64_t* S, const uint64_t* data, uint8_t rate_on_w)
{
	/*Absorption phase*/
	for (uint8_t i = 0, j = 0; i < 5; ++i)
	{
		for (j = 0; j < 5; ++j)
		{
			const uint8_t index = i + j * 5;

			if (index < rate_on_w)
			{
				S[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] =
					S[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] ^ data[index];
			}
		}
	}

	Keccak_f(S);
}

uint8_t Keccak_squeezing(uint8_t d_max, uint64_t* S, uint8_t rate_on_w, uint8_t* output)
{
	/*Squeezing phase*/
	for (uint8_t xF = 0; ;)
	{
		for (uint8_t i = 0; i < 5; i++)
		{
			for (uint8_t j = 0; j < 5; j++)
			{
				if ((5 * i + j) < rate_on_w)
				{
					const uint64_t* s = &(S[TWO_DIMENSION_TO_ONE_INDEX(j, i, 5)]);
#if __STDC_SEC_API__

					if (0 != memcpy_s(output + xF, sizeof(uint64_t), s, sizeof(uint64_t)))
					{
						return 0;
					}

#else
					memcpy(output + xF, s, sizeof(uint64_t));
#endif
					xF += sizeof(uint64_t);

					if (d_max <= xF)
					{
						return 1;
					}
				}
			}
		}

		if (d_max <= xF)
		{
			break;
		}

		Keccak_f(S);
	}

	return 1;
}

uint8_t get_hash_type(uint16_t hash_length)
{
	switch (hash_length)
	{
		case 224:
			return Hash224;

		case 256:
			return Hash256;

		case 384:
			return Hash384;

		case 512:
			return Hash512;

		default:
			break;
	}

	return HashUnknown;
}

uint8_t GetDelta(ptrdiff_t length, uint8_t rate_in_bytes)
{
	const uint8_t remainder = (uint8_t)((length + 1) % rate_in_bytes);
	return 0 != remainder ? (uint8_t)(rate_in_bytes - (remainder - 1)) : (uint8_t)1;
}

uint8_t Keccak(const uint8_t* input, const ptrdiff_t length, uint8_t is_sha3,
			   uint16_t hash_length, uint8_t* output)
{
	static const uint16_t capacity_array[] = { 1024, 768, 576, 512, 448/*, 384, 320, 256, 192*/ };
	const uint8_t hash_type = get_hash_type(hash_length);

	if (NULL == input ||
		length < 0 ||
		1 < is_sha3 ||
		HashUnknown == hash_type ||
		NULL == output)
	{
		return 0;
	}

	const uint16_t capacity = capacity_array[hash_type];
	const uint16_t rate = permutation_width - capacity;
	const uint8_t rate_on_w = (uint8_t)(rate / w);
	/*Padding*/
	const uint8_t delta = GetDelta(length, (uint8_t)(rate / 8));

	if (144 < delta)
	{
		return 0;
	}

	uint8_t addition[192];
	addition[0] = is_sha3 ? 6 : 1;

	if (1 == delta)
	{
		addition[0] += 128;
	}
	else
	{
		memset(addition + 1, 0, delta - 2);
		addition[delta - 1] = 128;
	}

	is_sha3 = 0;
	/*Initialization*/
	uint64_t S[] =
	{
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};
	/**/
	uint64_t data[] =
	{
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0
	};
	/**/
	uint8_t xF = 0;
	uint8_t* part = (addition + 180);
	const uint8_t* finish = input + length;

	while (input < finish || is_sha3 < delta)
	{
		if (input + 8 < finish)
		{
			if (!hash_algorithm_uint8_t_array_to_uint64_t(input, input + 8, &data[xF++]))
			{
				return 0;
			}

			input += 8;
		}
		else
		{
			const uint8_t i = (uint8_t)(finish - input);

			if (i)
			{
#if __STDC_SEC_API__

				if (0 != memcpy_s(part, i, input, i))
				{
					return 0;
				}

#else
				memcpy(part, input, i);
#endif
				input += i;
			}

#if __STDC_SEC_API__

			if (0 != memcpy_s(part + i, 8 - i, addition + is_sha3, 8 - i))
			{
				return 0;
			}

#else
			memcpy(part + i, addition + is_sha3, 8 - i);
#endif
			is_sha3 += 8 - i;

			if (!hash_algorithm_uint8_t_array_to_uint64_t(part, part + 8, &data[xF++]))
			{
				return 0;
			}
		}

		if (xF == rate_on_w)
		{
			Keccak_absorption(S, data, rate_on_w);
			memset(data, 0, sizeof(data));
			xF = 0;
		}
	}

	return Keccak_squeezing((uint8_t)(hash_length / 8), S, rate_on_w, output);
}

uint8_t hash_algorithm_keccak(const uint8_t* start, const uint8_t* finish, uint16_t hash_length,
							  struct buffer* output)
{
	return buffer_append(output, NULL, UINT8_MAX) &&
		   Keccak(start, finish - start, 0, hash_length,
				  (buffer_data(output, 0) + buffer_size(output) - UINT8_MAX)) &&
		   buffer_resize(output, buffer_size(output) - ((ptrdiff_t)UINT8_MAX - hash_length / 8));
}

uint8_t hash_algorithm_sha3(const uint8_t* start, const uint8_t* finish, uint16_t hash_length,
							struct buffer* output)
{
	return buffer_append(output, NULL, UINT8_MAX) &&
		   Keccak(start, finish - start, 1, hash_length,
				  (buffer_data(output, 0) + buffer_size(output) - UINT8_MAX)) &&
		   buffer_resize(output, buffer_size(output) - ((ptrdiff_t)UINT8_MAX - hash_length / 8));
}
