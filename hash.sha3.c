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

static const uint8_t w_array[] = { 1, 2, 4, 8, 16, 32, 64 };

#define INSTANCE_NUMBER		6

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
		D[i] = C[(i + 4) % 5] ^ ROT(C[(i + 1) % 5], 1, w_array[INSTANCE_NUMBER]);
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
					r[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)], w_array[INSTANCE_NUMBER]);
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
	static const uint8_t n_array[] = { 12, 14, 16, 18, 20, 22, 24 };
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

	for (uint8_t i = 0; i < n_array[INSTANCE_NUMBER]; i++)
	{
		Round(A, RC[i]);
	}
}

uint8_t Padding(const uint8_t* input, ptrdiff_t length, uint8_t is_sha3, uint16_t rate, uint8_t w,
				struct buffer* P)
{
	if (NULL == input ||
		NULL == P)
	{
		return 0;
	}

	ptrdiff_t size = length;

	do
	{
		++size;
	}
	while (0 != (size * 8) % rate);

	const uint8_t delta = (uint8_t)(size - length);

	if (180 < delta)
	{
		return 0;
	}

	uint8_t addition[192];

	if (1 == delta)
	{
		addition[0] = (128 + (is_sha3 ? 6 : 1));
	}
	else
	{
		addition[0] = (is_sha3 ? 6 : 1);
		memset(addition + 1, 0, delta - 2);
		addition[delta - 1] = 128;
	}

	ptrdiff_t s = (size * 8) / rate;
	s = sizeof(uint64_t) * (s * (1600 / w));

	if (!buffer_resize(P, s))
	{
		return 0;
	}

	uint64_t* ptr = (uint64_t*)buffer_data(P, 0);
	memset(ptr, 0, s);
	s = w / 8;
	uint8_t* part = (addition + 180);

	for (ptrdiff_t i = 0, j = 0, xF = 0; xF < size; xF++)
	{
		if ((ptrdiff_t)((uint64_t)rate / w - 1) < j)
		{
			j = 0;
			i++;
		}

		ptrdiff_t count = xF + 1;

		if (0 == (count * 8 % w))
		{
			const ptrdiff_t index = count - s;

			if (length <= index || length <= index + s)
			{
				if (index < length)
				{
					const ptrdiff_t to_copy = length - index;
#if __STDC_SEC_API__

					if (0 != memcpy_s(part, to_copy, input + index, to_copy))
					{
						return 0;
					}

					if (0 != memcpy_s(part + to_copy, s - to_copy, addition, s - to_copy))
					{
						return 0;
					}

#else
					memcpy(part, input + index, to_copy);
					memcpy(part + to_copy, addition, s - to_copy);
#endif
				}
				else
				{
#if __STDC_SEC_API__

					if (0 != memcpy_s(part, s, addition + index - length, s))
					{
						return 0;
					}

#else
					memcpy(part, addition + index - length, s);
#endif
				}
			}
			else
			{
#if __STDC_SEC_API__

				if (0 != memcpy_s(part, s, input + index, s))
				{
					return 0;
				}

#else
				memcpy(part, input + index, s);
#endif
			}

			uint64_t* Pi = ptr + ((1600 / w) * i + j);

			if (!hash_algorithm_uint8_t_array_to_uint64_t(part, part + 8, Pi))
			{
				return 0;
			}

			j++;
		}
	}

	return 1;
}

uint8_t Keccak(const uint8_t* input, const ptrdiff_t length, uint8_t is_sha3,
			   uint16_t rate, uint16_t capacity, uint8_t* output)
{
	if (NULL == input ||
		length < 0 ||
		1 < is_sha3 ||
		NULL == output)
	{
		return 0;
	}

	static const uint8_t count = 5;
	/*Padding*/
	struct buffer P;
	SET_NULL_TO_BUFFER(P);

	if (!Padding(input, length, is_sha3, rate, w_array[INSTANCE_NUMBER], &P))
	{
		buffer_release(&P);
		return 0;
	}

	uint64_t* ptr = (uint64_t*)buffer_data(&P, 0);
	/*Initialization*/
	uint64_t S[] =
	{
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};

	/*Absorption phase*/
	for (ptrdiff_t xF = 0, i = 0, j = 0,
		 size = (buffer_size(&P) / sizeof(uint64_t)) / (1600 / w_array[INSTANCE_NUMBER]); xF < size; xF++)
	{
		for (i = 0; i < count; i++)
		{
			for (j = 0; j < count; j++)
			{
				if ((i + j * count) < (rate / w_array[INSTANCE_NUMBER]))
				{
					S[TWO_DIMENSION_TO_ONE_INDEX(i, j, count)] =
						S[TWO_DIMENSION_TO_ONE_INDEX(i, j, count)] ^ ptr[(1600 / w_array[INSTANCE_NUMBER]) * xF + i + j * 5];
				}
			}
		}

		Keccak_f(S);
	}

	buffer_release(&P);
	/**/
	const uint8_t d_max = (uint8_t)(capacity / (2 * 8));

	/*Squeezing phase*/
	for (is_sha3 = 0; ;)
	{
		for (uint8_t i = 0; i < count; i++)
		{
			for (uint8_t j = 0; j < count; j++)
			{
				if ((count * i + j) < (rate / w_array[INSTANCE_NUMBER]))
				{
					ptr = &(S[TWO_DIMENSION_TO_ONE_INDEX(j, i, count)]);
#if __STDC_SEC_API__

					if (0 != memcpy_s(output + is_sha3, sizeof(uint64_t), ptr, sizeof(uint64_t)))
					{
						return 0;
					}

#else
					memcpy(output + is_sha3, ptr, sizeof(uint64_t));
#endif
					is_sha3 += sizeof(uint64_t);

					if (d_max <= is_sha3)
					{
						i = j = count;
					}
				}
			}
		}

		if (d_max <= is_sha3)
		{
			break;
		}

		Keccak_f(S);
	}

	return 1;
}

enum HashType { Hash512 = 0, Hash384, Hash256 = 3, Hash224 };
static const uint16_t rate_array[] = { 576, 832, 1024, 1088, 1152, 1216, 1280, 1344, 1408 };
static const uint16_t capacity_array[] = { 1024, 768, 576, 512, 448, 384, 320, 256, 192 };

int8_t hash_length_to_type(uint16_t hash_length)
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

	return -1;
}

uint8_t hash_algorithm_keccak(const uint8_t* start, const uint8_t* finish, uint16_t hash_length,
							  struct buffer* output)
{
	const int8_t hash_type = hash_length_to_type(hash_length);

	if (-1 == hash_type)
	{
		return 0;
	}

	hash_length = hash_length / 8;
	/**/
	return buffer_append(output, NULL, UINT8_MAX) &&
		   Keccak(start, finish - start, 0, rate_array[hash_type], capacity_array[hash_type],
				  (buffer_data(output, 0) + buffer_size(output) - UINT8_MAX)) &&
		   buffer_resize(output, buffer_size(output) - (UINT8_MAX - hash_length));
}

uint8_t hash_algorithm_sha3(const uint8_t* start, const uint8_t* finish, uint16_t hash_length,
							struct buffer* output)
{
	const int8_t hash_type = hash_length_to_type(hash_length);

	if (-1 == hash_type)
	{
		return 0;
	}

	hash_length = hash_length / 8;
	/**/
	return buffer_append(output, NULL, UINT8_MAX) &&
		   Keccak(start, finish - start, 1, rate_array[hash_type], capacity_array[hash_type],
				  (buffer_data(output, 0) + buffer_size(output) - UINT8_MAX)) &&
		   buffer_resize(output, buffer_size(output) - (UINT8_MAX - hash_length));
}
