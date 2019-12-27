/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

/*
 * As reference used:
 * FIPS 202.
 * SHA-3 Standard: Permutation-Based Hash and Extendable-Output Functions.
 * Date Published: August 2015.
 * https://csrc.nist.gov/publications/detail/fips/202/final
*/

#include "hash.h"
#include "buffer.h"
#include "math_unit.h"

#include <stddef.h>

static const uint8_t w_array[] = { 1, 2, 4, 8, 16, 32, 64 };

#define INSTANCE_NUMBER		6
#define MATRIX_SIZE			5 * 5

#define TWO_DIMENSION_TO_ONE_INDEX(X, Y, Y_MAX)	\
	(X) * (Y_MAX) + Y

#define ROT(X, N, W)	\
	(((X) << ((N) % (W))) | ((X) >> ((W) - ((N) % (W)))))

uint8_t hash_algorithm_uint8_t_array_to_uint64_t(
	const uint8_t* start, const uint8_t* finish, uint64_t* output/*, uint8_t order*/)
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

	/*(void)order;*/
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

uint8_t Keccak(const uint8_t* start, const uint8_t* finish, uint8_t start_byte,
			   uint16_t rate, uint16_t capacity, struct buffer* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		(0x06 != start_byte && 0x01 != start_byte) ||
		NULL == output)
	{
		return 0;
	}

	struct buffer message;

	SET_NULL_TO_BUFFER(message);

	/*TODO: Only pad part should be placed in this buffer.*/
	if (!buffer_append(&message, start, finish - start))
	{
		buffer_release(&message);
		return 0;
	}

	/*const ptrdiff_t real_input_size = finish - start;
	ptrdiff_t input_size = real_input_size;*/
	/*Padding*/
	/*++input_size;*/

	if (!buffer_push_back(&message, start_byte))
	{
		buffer_release(&message);
		return 0;
	}

	ptrdiff_t input_size = buffer_size(&message);
	/**/
	const uint16_t min = (uint16_t)((rate - 8) / 8);
	const uint16_t n = (uint16_t)math_truncate((double)(input_size / min));
	uint32_t message_full_count = 0;

	if (n < 2)
	{
		message_full_count = min;
	}
	else
	{
		message_full_count = n;
		message_full_count *= min;
		message_full_count += n - 1;
	}

	message_full_count -= (uint32_t)input_size;

	while (0 < (message_full_count--))
	{
		if (!buffer_push_back(&message, 0x00))
		{
			buffer_release(&message);
			return 0;
		}

		++input_size;
	}

	if (((input_size * 8) % rate) != (rate - 8))
	{
		buffer_release(&message);
		return 0;
	}

	if (!buffer_push_back(&message, 0x80))
	{
		buffer_release(&message);
		return 0;
	}

	++input_size;
	/**/
	const ptrdiff_t size = (input_size * 8) / rate;
	/*message_full_count = (uint32_t)(input_size - real_input_size + 8);*/
	struct buffer P;
	SET_NULL_TO_BUFFER(P);

	if (!buffer_append(&P, NULL, sizeof(uint64_t) * size * MATRIX_SIZE/* + message_full_count*/))
	{
		buffer_release(&message);
		buffer_release(&P);
		return 0;
	}

	start = buffer_data(&message, 0);
	finish = start + buffer_size(&message);
	/**/
	uint64_t* ptr = (uint64_t*)buffer_data(&P, 0);
	/*uint8_t* ptr8_last_one = buffer_data(&P, 0) + (buffer_size(&P) - 1);
	uint8_t* ptr8 = buffer_data(&P, 0) + (buffer_size(&P) - message_full_count);*/

	for (ptrdiff_t xF = 0, count = 0, i = 0, j = 0;
		 xF < input_size; xF++)
	{
		if (j > ((ptrdiff_t)(rate / w_array[INSTANCE_NUMBER]) - 1))
		{
			j = 0;
			i++;
		}

		count++;

		if (((count * 8) % w_array[INSTANCE_NUMBER]) == 0)
		{
			const uint8_t* start_ = start + (count - w_array[INSTANCE_NUMBER] / 8);
			const uint8_t* finish_ = start_ + 8;
#if 0

			if (finish <= start_ ||
				finish <= finish_)
			{
				(*ptr8_last_one) = 0x80;

				for (message_full_count = 0; ptr8_last_one != (ptr8 + message_full_count);)
				{
					while (start_ < finish && ptr8_last_one == (ptr8 + message_full_count))
					{
						ptr8[message_full_count++] = (*start_);
						++start_;
					}

					if (ptr8_last_one == (ptr8 + message_full_count))
					{
						break;
					}

					ptr8[message_full_count++] = 0x01;

					if (ptr8_last_one == (ptr8 + message_full_count))
					{
						break;
					}

					while (ptr8_last_one != (ptr8 + message_full_count))
					{
						ptr8[message_full_count++] = 0x00;
					}
				}

				start_ = ptr8;

				if (finish == start)
				{
					start_ += count - w_array[INSTANCE_NUMBER] / 8;
				}

				finish_ = start_ + 8;
			}

#endif

			if (!hash_algorithm_uint8_t_array_to_uint64_t(start_, finish_, ptr + (size * i + j)))
			{
				buffer_release(&message);
				buffer_release(&P);
				return 0;
			}

			j++;
		}
	}

	buffer_release(&message);
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
	for (ptrdiff_t xF = 0, i = 0, count = 5, j = 0; xF < size; xF++)
	{
		for (i = 0; i < count; i++)
		{
			for (j = 0; j < count; j++)
			{
				if ((i + j * count) < (rate / w_array[INSTANCE_NUMBER]))
				{
					S[TWO_DIMENSION_TO_ONE_INDEX(i, j, count)] =
						S[TWO_DIMENSION_TO_ONE_INDEX(i, j, count)] ^ ptr[size * xF + i + j * count];
				}
			}
		}

		Keccak_f(S);
	}

	buffer_release(&P);
	/**/
	input_size = buffer_size(output);
	const uint8_t d_max = (uint8_t)(capacity / (2 * 8));

	/*Squeezing phase*/
	for (message_full_count = 0; ;)
	{
		static const uint8_t count = 5;

		for (uint8_t i = 0; i < count; i++)
		{
			for (uint8_t j = 0; j < count; j++)
			{
				if ((count * i + j) < (rate / w_array[INSTANCE_NUMBER]))
				{
					if (message_full_count >= d_max)
					{
						i = j = 5;
					}
					else
					{
						ptr = &(S[TWO_DIMENSION_TO_ONE_INDEX(j, i, count)]);

						if (!buffer_append(output, (const uint8_t*)ptr, sizeof(uint64_t)))
						{
							return 0;
						}

						message_full_count = (uint32_t)(buffer_size(output) - input_size);
					}
				}
			}
		}

		if (message_full_count >= d_max)
		{
			break;
		}

		Keccak_f(S);
	}

	return buffer_resize(output, input_size + d_max);
}

enum HashType { Hash512 = 0, Hash384, Hash256 = 3, Hash224 };
static const uint16_t rate_array[] = { 576, 832, 1024, 1088, 1152, 1216, 1280, 1344, 1408 };
static const uint16_t capacity_array[] = { 1024, 768, 576, 512, 448, 384, 320, 256, 192 };

uint8_t hash_algorithm_keccak_224(
	const uint8_t* start, const uint8_t* finish,  struct buffer* output)
{
	return Keccak(start, finish, 0x01, rate_array[Hash224], capacity_array[Hash224], output);
}

uint8_t hash_algorithm_keccak_256(
	const uint8_t* start, const uint8_t* finish,  struct buffer* output)
{
	return Keccak(start, finish, 0x01, rate_array[Hash256], capacity_array[Hash256], output);
}

uint8_t hash_algorithm_keccak_384(
	const uint8_t* start, const uint8_t* finish,  struct buffer* output)
{
	return Keccak(start, finish, 0x01, rate_array[Hash384], capacity_array[Hash384], output);
}

uint8_t hash_algorithm_keccak_512(
	const uint8_t* start, const uint8_t* finish,  struct buffer* output)
{
	return Keccak(start, finish, 0x01, rate_array[Hash512], capacity_array[Hash512], output);
}

uint8_t hash_algorithm_sha3_224(
	const uint8_t* start, const uint8_t* finish, struct buffer* output)
{
	return Keccak(start, finish, 0x06, rate_array[Hash224], capacity_array[Hash224], output);
}

uint8_t hash_algorithm_sha3_256(
	const uint8_t* start, const uint8_t* finish, struct buffer* output)
{
	return Keccak(start, finish, 0x06, rate_array[Hash256], capacity_array[Hash256], output);
}

uint8_t hash_algorithm_sha3_384(
	const uint8_t* start, const uint8_t* finish, struct buffer* output)
{
	return Keccak(start, finish, 0x06, rate_array[Hash384], capacity_array[Hash384], output);
}

uint8_t hash_algorithm_sha3_512(
	const uint8_t* start, const uint8_t* finish, struct buffer* output)
{
	return Keccak(start, finish, 0x06, rate_array[Hash512], capacity_array[Hash512], output);
}
