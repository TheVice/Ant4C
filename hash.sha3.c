/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2012 - 2021, 2024 TheVice
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

#include "stdc_secure_api.h"

#include "hash.h"
#include "bit_converter.h"
#include "buffer.h"
#include "common.h"

#if defined(__clang__) && defined(NDEBUG)
#include "echo.h"
#endif

#include <stddef.h>
#include <string.h>

static const uint8_t w = 64;

#define TWO_DIMENSION_TO_ONE_INDEX(X, Y, Y_MAX)	\
	(X) * (Y_MAX) + (Y)

#define ROT(X, N, W)	\
	(((X) << ((N) % (W))) | ((X) >> ((W) - ((N) % (W)))))

void theta(uint64_t* A, uint64_t* C, uint64_t* D)
{
	for (uint8_t i = 0; i < 5; ++i)
	{
		C[i] = A[TWO_DIMENSION_TO_ONE_INDEX(i, 0, 5)] ^
			   A[TWO_DIMENSION_TO_ONE_INDEX(i, 1, 5)] ^
			   A[TWO_DIMENSION_TO_ONE_INDEX(i, 2, 5)] ^
			   A[TWO_DIMENSION_TO_ONE_INDEX(i, 3, 5)] ^
			   A[TWO_DIMENSION_TO_ONE_INDEX(i, 4, 5)];
	}

	for (uint8_t i = 0; i < 5; ++i)
	{
		D[i] = C[(i + 4) % 5] ^ ROT(C[(i + 1) % 5], 1, w);
	}

	for (uint8_t i = 0; i < 5; ++i)
	{
		for (uint8_t j = 0; j < 5; ++j)
		{
			A[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] ^= D[i];
		}
	}
}

void rho_pi(const uint64_t* A, uint64_t* B)
{
	static const uint8_t r[] =
	{
		0, 36, 3, 41, 18,
		1, 44, 10, 45, 2,
		62, 6, 43, 15, 61,
		28, 55, 25, 21, 56,
		27, 20, 39, 8, 14
	};

	for (uint8_t i = 0; i < 5; ++i)
	{
		for (uint8_t j = 0; j < 5; ++j)
		{
#if defined(__clang__) && defined(NDEBUG)
			const uint8_t index_1 = TWO_DIMENSION_TO_ONE_INDEX(j, (2 * i + 3 * j) % 5, 5);
			const uint8_t index_2 = TWO_DIMENSION_TO_ONE_INDEX(i, j, 5);
			B[index_1] = ROT(A[index_2], r[index_2], w);
			echo(0, 0, NULL, None, (const uint8_t*)&B[index_1], sizeof(uint64_t), 0, 0);
#else
			B[TWO_DIMENSION_TO_ONE_INDEX(j, (2 * i + 3 * j) % 5, 5)] =
				ROT(A[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)],
					r[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)], w);
#endif
		}
	}
}

void chi(uint64_t* A, const uint64_t* B)
{
	for (uint8_t i = 0; i < 5; ++i)
	{
		for (uint8_t j = 0; j < 5; ++j)
		{
			A[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] =
				B[TWO_DIMENSION_TO_ONE_INDEX(i, j, 5)] ^
				((~B[TWO_DIMENSION_TO_ONE_INDEX((i + 1) % 5, j, 5)]) &
				 B[TWO_DIMENSION_TO_ONE_INDEX((i + 2) % 5, j, 5)]);
		}
	}
}

void iota(uint64_t* A, uint64_t RC_i)
{
	A[TWO_DIMENSION_TO_ONE_INDEX(0, 0, 5)] ^= RC_i;
}

void Keccak_f(uint64_t* A)
{
	/*l -> log(w) / log(2)
	number of permutation -> 12 + 2 * l
	static const uint8_t n = 12, 14, 16, 18, 20, 22, 24*/
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
	/**/
	uint64_t B[25];
	uint64_t* C = B;
	uint64_t* D = B + 5;

	for (uint8_t i = 0; i < 24; ++i)
	{
		theta(A, C, D);
		rho_pi(A, B);
		chi(A, B);
		iota(A, RC[i]);
	}
}

void Keccak_absorption(const uint64_t* data, uint64_t* S, uint8_t rate_on_w)
{
	/*Absorption phase*/
	for (uint8_t i = 0; i < 5; ++i)
	{
		for (uint8_t j = 0; j < 5; ++j)
		{
			const uint8_t source_index = TWO_DIMENSION_TO_ONE_INDEX(j, i, 5);

			if (source_index < rate_on_w)
			{
				const uint8_t destination_index = TWO_DIMENSION_TO_ONE_INDEX(i, j, 5);
				S[destination_index] ^= data[source_index];
			}
		}
	}

	Keccak_f(S);
}

uint8_t Keccak_absorption_bytes(const uint8_t* data, uint64_t* S, uint8_t rate_on_w)
{
	if (!data ||
		!S ||
		18 < rate_on_w)
	{
		return 0;
	}

	uint64_t data_[18];

	for (uint8_t i = 0; i < rate_on_w; ++i)
	{
		const uint8_t* start = data + 8 * i;
		const uint8_t* finish = start + 8;

		if (!bit_converter_to_uint64_t(start, finish, &data_[i]))
		{
			return 0;
		}
	}

	Keccak_absorption(data_, S, rate_on_w);
	return 1;
}

uint8_t Keccak_squeezing(uint64_t* S, uint8_t rate_on_w, uint8_t d_max, uint8_t* output)
{
	/*Squeezing phase*/
	for (uint8_t xF = 0; xF < d_max;)
	{
		for (uint8_t i = 0; i < 5; ++i)
		{
			for (uint8_t j = 0; j < 5; ++j)
			{
				if (TWO_DIMENSION_TO_ONE_INDEX(i, j, 5) < rate_on_w)
				{
					const uint64_t* s = &(S[TWO_DIMENSION_TO_ONE_INDEX(j, i, 5)]);
#if defined(__STDC_LIB_EXT1__)

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

		Keccak_f(S);
	}

	return 0;
}

uint8_t hash_algorithm_sha3_init(
	uint16_t hash_length,
	uint8_t* rate_on_w,
	uint8_t* maximum_delta)
{
	static const uint16_t permutation_width = 1600;
	static const uint16_t capacity_array[] = { 1024, 768, 512, 448 };
	static const uint16_t hash_lengths[] = { 512, 384, 256, 224 };

	if (NULL == rate_on_w ||
		NULL == maximum_delta)
	{
		return 0;
	}

	uint8_t i = 0;

	do
	{
		if (hash_lengths[i] == hash_length)
		{
			break;
		}

		++i;
	}
	while (i < 4);

	if (4 == i)
	{
		return 0;
	}

	const uint16_t capacity = capacity_array[i];
	const uint16_t rate = permutation_width - capacity;
	/**/
	*rate_on_w = (uint8_t)(rate / w);
	*maximum_delta = (*rate_on_w) * 8;
	/**/
	return 1;
}

uint8_t hash_algorithm_sha3_core(
	const uint8_t* start, const uint8_t* finish,
	uint8_t* queue, uint8_t* queue_size, uint8_t maximum_delta,
	uint64_t* S, uint8_t rate_on_w)
{
	if (!start ||
		!finish ||
		!queue ||
		!queue_size ||
		!S)
	{
		return 0;
	}

	if (0 < *queue_size)
	{
		const uint8_t length = (uint8_t)MIN(finish - start, maximum_delta - *queue_size);
#if defined(__STDC_LIB_EXT1__)

		if (0 != memcpy_s(queue + *queue_size, maximum_delta - *queue_size, start, length))
		{
			return 0;
		}

#else
		memcpy(queue + *queue_size, start, length);
#endif
		start += length;
		*queue_size += length;
	}

	if (maximum_delta == *queue_size)
	{
		if (!Keccak_absorption_bytes(queue, S, rate_on_w))
		{
			return 0;
		}

		*queue_size = 0;
	}

	while (start + maximum_delta <= finish)
	{
		if (!Keccak_absorption_bytes(start, S, rate_on_w))
		{
			return 0;
		}

		start += maximum_delta;
	}

	if (start < finish)
	{
		*queue_size = (uint8_t)(finish - start);
#if defined(__STDC_LIB_EXT1__)

		if (0 != memcpy_s(queue, maximum_delta, start, *queue_size))
		{
			return 0;
		}

#else
		memcpy(queue, start, *queue_size);
#endif
	}

	return 1;
}

uint8_t hash_algorithm_sha3_final(
	uint8_t is_sha3,
	uint8_t* queue, uint8_t queue_size, uint8_t maximum_delta,
	uint64_t* S, uint8_t rate_on_w, uint8_t d_max,
	uint8_t* output)
{
	const uint8_t delta = maximum_delta - queue_size;
	memset(queue + queue_size, 0, delta);
	queue[queue_size] = is_sha3 ? 6 : 1;

	if (1 == delta)
	{
		queue[maximum_delta - 1] = (is_sha3 ? 6 : 1) + 128;
	}
	else
	{
		queue[maximum_delta - 1] = 128;
	}

	if (!Keccak_absorption_bytes(queue, S, rate_on_w))
	{
		return 0;
	}

	return Keccak_squeezing(S, rate_on_w, d_max, output);
}

uint8_t Keccak(const uint8_t* input, ptrdiff_t length, uint8_t is_sha3,
			   uint16_t hash_length, uint8_t* output)
{
	if (NULL == input ||
		length < 0 ||
		1 < is_sha3 ||
		NULL == output)
	{
		return 0;
	}

	uint8_t rate_on_w;
	uint8_t maximum_delta;

	if (!hash_algorithm_sha3_init(hash_length, &rate_on_w, &maximum_delta))
	{
		return 0;
	}

	uint8_t queue[192];
	uint8_t queue_size = 0;
	uint64_t S[] =
	{
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};

	if (!hash_algorithm_sha3_core(input, input + length, queue, &queue_size, maximum_delta, S, rate_on_w))
	{
		return 0;
	}

	hash_length = hash_length / 8;
	return hash_algorithm_sha3_final(
			   is_sha3, queue, queue_size, maximum_delta, S, rate_on_w,
			   (uint8_t)hash_length, output);
}

uint8_t hash_algorithm_keccak(const uint8_t* start, const uint8_t* finish, uint16_t hash_length,
							  void* output)
{
	return buffer_append(output, NULL, UINT8_MAX) &&
		   Keccak(start, finish - start, 0, hash_length,
				  (buffer_uint8_t_data(output, 0) + buffer_size(output) - UINT8_MAX)) &&
		   buffer_resize(output, buffer_size(output) - ((ptrdiff_t)UINT8_MAX - hash_length / 8));
}

uint8_t hash_algorithm_sha3(const uint8_t* start, const uint8_t* finish, uint16_t hash_length,
							void* output)
{
	return buffer_append(output, NULL, UINT8_MAX) &&
		   Keccak(start, finish - start, 1, hash_length,
				  (buffer_uint8_t_data(output, 0) + buffer_size(output) - UINT8_MAX)) &&
		   buffer_resize(output, buffer_size(output) - ((ptrdiff_t)UINT8_MAX - hash_length / 8));
}
