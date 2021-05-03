/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2021 TheVice
 *
 */

/*
 * As reference used:
 * BLAKE3 one function, fast everywhere. Version 20200109113900.
 * https://github.com/BLAKE3-team/BLAKE3-specs/raw/master/blake3.pdf
 */

#include "stdc_secure_api.h"

#include "hash.h"
#include "buffer.h"

#include <stddef.h>
#include <string.h>

static const uint32_t IV[] =
{
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

enum BLAKE3_DOMAIN_FLAGS
{
	CHUNK_START = 1, CHUNK_END = 2, PARENT = 4, ROOT = 8/*,
	KEYED_HASH = 16, DERIVE_KEY_CONTEXT = 32, DERIVE_KEY_MATERIAL = 64*/
};

#define BLAKE3_STACK_LENGTH (uint8_t)32
#define BLAKE3_BLOCK_LENGTH (uint8_t)(16 * sizeof(uint32_t))
#define BLAKE3_CHUNK_LENGTH (uint16_t)1024
#define BLAKE3_OUTPUT_LENGTH (uint8_t)(8 * sizeof(uint32_t))
#define BLAKE3_MAXIMUM_CHUNKS_COUNT (uint8_t)24

#define ROTATE_RIGHT_UINT32_T(VALUE, OFFSET)			\
	((VALUE) >> (OFFSET)) | (VALUE) << (32 - (OFFSET))

#define BLAKE3_MIX(VA, VB, VC, VD, X, Y)				\
	(VA) = (VA) + (VB) + (X);							\
	(VD) = ROTATE_RIGHT_UINT32_T((VD) ^ (VA), 16);		\
	(VC) = (VC) + (VD);									\
	(VB) = ROTATE_RIGHT_UINT32_T((VB) ^ (VC), 12);		\
	(VA) = (VA) + (VB) + (Y);							\
	(VD) = ROTATE_RIGHT_UINT32_T((VD) ^ (VA), 8);		\
	(VC) = (VC) + (VD);									\
	(VB) = ROTATE_RIGHT_UINT32_T((VB) ^ (VC), 7);

#define BLAKE3_ROUND(V, M, S)													\
	BLAKE3_MIX((V)[0], (V)[4], (V)[8], (V)[12], (M)[(S)[0]], (M)[(S)[1]]);		\
	BLAKE3_MIX((V)[1], (V)[5], (V)[9], (V)[13], (M)[(S)[2]], (M)[(S)[3]]);		\
	BLAKE3_MIX((V)[2], (V)[6], (V)[10], (V)[14], (M)[(S)[4]], (M)[(S)[5]]);		\
	BLAKE3_MIX((V)[3], (V)[7], (V)[11], (V)[15], (M)[(S)[6]], (M)[(S)[7]]);		\
	BLAKE3_MIX((V)[0], (V)[5], (V)[10], (V)[15], (M)[(S)[8]], (M)[(S)[9]]);		\
	BLAKE3_MIX((V)[1], (V)[6], (V)[11], (V)[12], (M)[(S)[10]], (M)[(S)[11]]);	\
	BLAKE3_MIX((V)[2], (V)[7], (V)[8], (V)[13], (M)[(S)[12]], (M)[(S)[13]]);	\
	BLAKE3_MIX((V)[3], (V)[4], (V)[9], (V)[14], (M)[(S)[14]], (M)[(S)[15]]);

uint8_t BLAKE3_compress(const uint32_t* h, const uint32_t* m, const uint32_t* t,
						uint32_t b, uint32_t d, uint32_t* V)
{
	static const uint8_t SIGMA[7][16] =
	{
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
		{ 2, 6, 3, 10, 7, 0, 4, 13, 1, 11, 12, 5, 9, 14, 15, 8 },
		{ 3, 4, 10, 12, 13, 2, 7, 14, 6, 5, 9, 0, 11, 15, 8, 1 },
		{ 10, 7, 12, 9, 14, 3, 13, 15, 4, 0, 11, 2, 5, 8, 1, 6 },
		{ 12, 13, 9, 11, 15, 10, 14, 8, 7, 2, 5, 3, 0, 1, 6, 4 },
		{ 9, 14, 11, 5, 8, 12, 15, 1, 13, 3, 0, 10, 2, 6, 4, 7 },
		{ 11, 15, 5, 0, 1, 9, 8, 6, 14, 10, 2, 12, 3, 4, 7, 13 }
	};

	if (NULL == h ||
		NULL == m ||
		NULL == t ||
		NULL == V)
	{
		return 0;
	}

	for (uint8_t i = 0; i < 8; ++i)
	{
		V[i] = h[i];
	}

	for (uint8_t i = 8; i < 12; ++i)
	{
		V[i] = IV[i - 8];
	}

	V[12] = t[0]; /*Lo*/
	V[13] = t[1]; /*Hi*/
	V[14] = b;
	V[15] = d;

	for (uint8_t i = 0; i < 7; ++i)
	{
		const uint8_t* S = SIGMA[i];
		BLAKE3_ROUND(V, m, S);
	}

	return 1;
}

uint8_t BLAKE3_compress_XOF(uint32_t* h, const uint32_t* m, const uint32_t* t, uint32_t b, uint8_t d,
							uint32_t* output)
{
	uint32_t V[16];

	if (NULL == h ||
		!BLAKE3_compress(h, m, t, b, d, V))
	{
		return 0;
	}

	if (NULL != output)
	{
		for (uint8_t i = 0, count = 16, middle = 8; i < count; ++i)
		{
			output[i] = V[i] ^ ((i < middle) ? V[middle + i] : h[i - middle]);
		}
	}
	else
	{
		for (uint8_t i = 0, count = 8; i < count; ++i)
		{
			h[i] = V[i] ^ V[count + i];
		}
	}

	return 1;
}

uint8_t BLAKE3_hash_input(uint8_t d, const uint8_t* input, const uint32_t* t, uint8_t* h)
{
#if __STDC_LIB_EXT1__

	if (0 != memcpy_s(h, BLAKE3_OUTPUT_LENGTH, IV, BLAKE3_OUTPUT_LENGTH))
	{
		return 0;
	}

#else
	memcpy(h, IV, BLAKE3_OUTPUT_LENGTH);
#endif
	uint16_t count_of_blocks = BLAKE3_CHUNK_LENGTH / BLAKE3_BLOCK_LENGTH;
	uint8_t domain_flags = d | CHUNK_START;

	while (0 < count_of_blocks)
	{
		if (1 == count_of_blocks)
		{
			domain_flags |= CHUNK_END;
		}

		if (!BLAKE3_compress_XOF((uint32_t*)h, (const uint32_t*)input, t, BLAKE3_BLOCK_LENGTH, domain_flags,
								 NULL))
		{
			return 0;
		}

		input += BLAKE3_BLOCK_LENGTH;
		count_of_blocks -= 1;
		domain_flags = d;
	}

	return 1;
}

uint8_t BLAKE3_hash_inputs(uint8_t d, const uint8_t** inputs, uint8_t count_of_inputs, const uint32_t* t,
						   uint8_t* output)
{
	uint64_t index = 0;
	uint32_t counter[2];
	counter[0] = t[0];
	counter[1] = t[1];

	while (0 < count_of_inputs)
	{
		if (!BLAKE3_hash_input(d, inputs[index], counter, output + index * BLAKE3_OUTPUT_LENGTH))
		{
			return 0;
		}

		counter[0] += 1;
		++index;
		--count_of_inputs;
	}

	return 1;
}

#define GET_DATA_FOR_CHUNK_SEC(INPUT, LENGTH, OUTPUT, L, PROCESSED)			\
	(PROCESSED) = (BLAKE3_BLOCK_LENGTH) - (L);								\
	\
	if ((LENGTH) < (PROCESSED))												\
	{																		\
		(PROCESSED) = (uint8_t)(LENGTH);									\
	}																		\
	\
	if (0 < (PROCESSED))													\
	{																		\
		if (0 != memcpy_s((OUTPUT)+(L), (PROCESSED), (INPUT), (PROCESSED)))	\
		{																	\
			return 0;														\
		}																	\
		\
		(L) += (PROCESSED);													\
	}

#define GET_DATA_FOR_CHUNK(INPUT, LENGTH, OUTPUT, L, PROCESSED)				\
	(PROCESSED) = (BLAKE3_BLOCK_LENGTH) - (L);								\
	\
	if ((LENGTH) < (PROCESSED))												\
	{																		\
		(PROCESSED) = (uint8_t)(LENGTH);									\
	}																		\
	\
	if (0 < (PROCESSED))													\
	{																		\
		memcpy((OUTPUT) + (L), (INPUT), (PROCESSED));						\
		(L) += (PROCESSED);													\
	}

uint8_t BLAKE3_update_chunk_data(
	const uint8_t* input, uint64_t length, uint32_t* m, uint8_t* l, uint32_t* h,
	uint8_t* compressed, const uint32_t* t, uint8_t d)
{
	if (NULL == input ||
		NULL == m ||
		NULL == l ||
		NULL == h ||
		NULL == compressed ||
		NULL == t)
	{
		return 0;
	}

	uint16_t index = 0;
	uint8_t processed = 0;

	if (0 < (*l))
	{
#if __STDC_LIB_EXT1__
		GET_DATA_FOR_CHUNK_SEC(input, length, m, (*l), processed);
#else
		GET_DATA_FOR_CHUNK(input, length, m, (*l), processed);
#endif
		index += processed;
		length -= processed;

		if (0 < length)
		{
			if (!BLAKE3_compress_XOF(h, m, t, BLAKE3_BLOCK_LENGTH,
									 d | (0 < (*compressed) ? 0 : CHUNK_START), NULL))
			{
				return 0;
			}

			(*compressed) += 1;
			(*l) = 0;
			memset(m, 0, BLAKE3_BLOCK_LENGTH);
		}
	}

	while (BLAKE3_BLOCK_LENGTH < length)
	{
		if (!BLAKE3_compress_XOF(h, (const uint32_t*)(input + index), t, BLAKE3_BLOCK_LENGTH,
								 d | (0 < (*compressed) ? 0 : CHUNK_START), NULL))
		{
			return 0;
		}

		(*compressed) += 1;
		index += BLAKE3_BLOCK_LENGTH;
		length -= BLAKE3_BLOCK_LENGTH;
	}

#if __STDC_LIB_EXT1__
	GET_DATA_FOR_CHUNK_SEC(input + index, length, m, (*l), processed);
#else
	GET_DATA_FOR_CHUNK(input + index, length, m, (*l), processed);
#endif
	return processed == length;
}

#define PUSH_CHUNK_TO_STACK_SEC(INPUT, STACK, STACK_LENGTH)							\
	\
	if (0 != memcpy_s((STACK) + (uint64_t)(STACK_LENGTH) * (BLAKE3_OUTPUT_LENGTH),	\
					  (BLAKE3_OUTPUT_LENGTH), (INPUT), (BLAKE3_OUTPUT_LENGTH)))		\
	{																				\
		return 0;																	\
	}																				\
	\
	++(STACK_LENGTH);

#define PUSH_CHUNK_TO_STACK(INPUT, STACK, STACK_LENGTH)								\
	\
	memcpy((STACK) + (uint64_t)(STACK_LENGTH) * (BLAKE3_OUTPUT_LENGTH),				\
		   (INPUT), (BLAKE3_OUTPUT_LENGTH));										\
	\
	++(STACK_LENGTH);

uint8_t population_count(const uint8_t* input, uint8_t size)
{
	uint8_t result = 0;

	for (uint8_t i = 0; i < size; ++i)
	{
		uint8_t in = input[i];

		do
		{
			result += 1 == in % 2;
		}
		while (1 < (in /= 2));

		result += 1 == in;
	}

	return result;
}

uint8_t MERGE(uint8_t* stack, uint8_t* stack_length, const uint32_t* t, uint8_t d)
{
	const uint8_t population = population_count((const uint8_t*)t, sizeof(uint32_t));
	/*uint32_t* h = (uint32_t*)(stack + ((BLAKE3_STACK_LENGTH - 1) * BLAKE3_OUTPUT_LENGTH));*/
	uint32_t h[BLAKE3_OUTPUT_LENGTH / sizeof(uint32_t)];

	while ((*stack_length) > population)
	{
		static const uint32_t local_t[] = { 0, 0 };
		uint32_t* m = (uint32_t*)(stack + ((uint64_t)((*stack_length) - 2) * BLAKE3_OUTPUT_LENGTH));
#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(h, BLAKE3_OUTPUT_LENGTH, IV, BLAKE3_OUTPUT_LENGTH))
		{
			return 0;
		}

#else
		memcpy(h, IV, BLAKE3_OUTPUT_LENGTH);
#endif

		if (!BLAKE3_compress_XOF(h, m, local_t, BLAKE3_BLOCK_LENGTH, d | PARENT, NULL))
		{
			return 0;
		}

#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(m, BLAKE3_OUTPUT_LENGTH, h, BLAKE3_OUTPUT_LENGTH))
		{
			return 0;
		}

#else
		memcpy(m, h, BLAKE3_OUTPUT_LENGTH);
#endif
		--(*stack_length);
	}

	return 1;
}

uint8_t BLAKE3_core(const uint8_t* input, uint64_t length, uint32_t* m, uint8_t* l, uint32_t* h,
					uint8_t* compressed, uint32_t* t, uint8_t* stack, uint8_t* stack_length, uint8_t d)
{
	uint16_t to_process = (uint64_t)(BLAKE3_BLOCK_LENGTH) * (*compressed) + (*l);

	if ((0 == t[0] && BLAKE3_CHUNK_LENGTH == length) || 0 < to_process)
	{
		to_process = BLAKE3_CHUNK_LENGTH - to_process;

		if (length < to_process)
		{
			to_process = (uint16_t)length;
		}

		if (!BLAKE3_update_chunk_data(input, to_process, m, l, h, compressed, t, d))
		{
			return 0;
		}

		length -= to_process;
		return 0 == length;
#if 0

		/*NOTE: Requested test case for commented code, if such even exists.*/
		if (0 == length)
		{
			return 1;
		}

		input += to_process;
		const uint8_t domain_flags = d | (0 < (*compressed) ? 0 : CHUNK_START) | CHUNK_END;

		if (!BLAKE3_compress(h, m, t, (*l), domain_flags, NULL))
		{
			return 0;
		}

		if (!MERGE(stack, stack_length, t, d))
		{
			return 0;
		}

		if ((BLAKE3_STACK_LENGTH) == *stack_length)
		{
			return 0;
		}

#if __STDC_LIB_EXT1__
		PUSH_CHUNK_TO_STACK_SEC(h, stack, *stack_length);
#else
		PUSH_CHUNK_TO_STACK(h, stack, *stack_length);
#endif
#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(h, BLAKE3_OUTPUT_LENGTH, IV, BLAKE3_OUTPUT_LENGTH))
		{
			return 0;
		}

#else
		memcpy(h, IV, BLAKE3_OUTPUT_LENGTH);
#endif
		t[0] += 1;
		(*compressed) = 0;
		memset(m, 0, BLAKE3_BLOCK_LENGTH);
		(*l) = 0;
#endif
	}

	uint8_t number_of_chunks = 0;
	const uint8_t* chunks[BLAKE3_MAXIMUM_CHUNKS_COUNT];
	uint8_t output[BLAKE3_OUTPUT_LENGTH * BLAKE3_MAXIMUM_CHUNKS_COUNT];

	while (BLAKE3_CHUNK_LENGTH <= length)
	{
		while (BLAKE3_CHUNK_LENGTH <= length && number_of_chunks < BLAKE3_MAXIMUM_CHUNKS_COUNT)
		{
			chunks[number_of_chunks] = input;
			input += BLAKE3_CHUNK_LENGTH;
			length -= BLAKE3_CHUNK_LENGTH;
			++number_of_chunks;
		}

		if (!BLAKE3_hash_inputs(d, chunks, number_of_chunks, t, output))
		{
			return 0;
		}

		for (uint64_t index = 0; index < number_of_chunks; ++index)
		{
			if (!MERGE(stack, stack_length, t, d))
			{
				return 0;
			}

			if ((BLAKE3_STACK_LENGTH) == *stack_length)
			{
				return 0;
			}

#if __STDC_LIB_EXT1__
			PUSH_CHUNK_TO_STACK_SEC(output + index * BLAKE3_OUTPUT_LENGTH, stack, *stack_length);
#else
			PUSH_CHUNK_TO_STACK(output + index * BLAKE3_OUTPUT_LENGTH, stack, *stack_length);
#endif
			t[0] += 1;
		}

		number_of_chunks = 0;
	}

	if (0 < length)
	{
		if (!MERGE(stack, stack_length, t, d))
		{
			return 0;
		}

		if (!BLAKE3_update_chunk_data(input, length, m, l, h, compressed, t, d))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t BLAKE3_get_bytes_from_root_chunk(
	uint8_t hash_length, uint32_t* h, const uint32_t* m,
	uint8_t l, uint8_t d, uint8_t* output)
{
	if (NULL == output)
	{
		return 0;
	}

	uint8_t index = 0;
	uint32_t chunk_counter[2];
	chunk_counter[0] = chunk_counter[1] = 0;

	while (0 < hash_length)
	{
		if (!BLAKE3_compress_XOF(h, m, chunk_counter, l, d, (uint32_t*)(output + index)))
		{
			return 0;
		}

		const uint8_t processed = BLAKE3_BLOCK_LENGTH < hash_length ? BLAKE3_BLOCK_LENGTH : hash_length;
		index += processed;
		hash_length -= processed;
		chunk_counter[0] += 1;
	}

	return 1;
}

uint8_t BLAKE3_final(const uint8_t* stack, uint8_t stack_length,
					 uint8_t compressed, uint32_t* t, uint32_t* h, uint32_t* m,
					 uint8_t l, uint8_t d, uint8_t hash_length, uint8_t* output)
{
	uint8_t domain_flags = d;

	if (0 == stack_length)
	{
		domain_flags |= (0 < compressed ? 0 : CHUNK_START) | CHUNK_END | ROOT;
		return BLAKE3_get_bytes_from_root_chunk(hash_length, h, m, l, domain_flags, output);
	}

	if (0 < BLAKE3_BLOCK_LENGTH * compressed + l)
	{
		domain_flags |= (0 < compressed ? 0 : CHUNK_START) | CHUNK_END;
	}
	else
	{
		stack_length -= 2;
#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(m, BLAKE3_BLOCK_LENGTH, stack + (uint64_t)stack_length * BLAKE3_OUTPUT_LENGTH,
						  BLAKE3_BLOCK_LENGTH))
		{
			return 0;
		}

#else
		memcpy(m, stack + (uint64_t)stack_length * BLAKE3_OUTPUT_LENGTH, BLAKE3_BLOCK_LENGTH);
#endif
		domain_flags |= PARENT;
		l = BLAKE3_BLOCK_LENGTH;
		t[0] = t[1] = 0;
	}

	while (0 < stack_length)
	{
		--stack_length;
		uint8_t block[BLAKE3_BLOCK_LENGTH];
#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(block, BLAKE3_OUTPUT_LENGTH,
						  stack + (uint64_t)stack_length * BLAKE3_OUTPUT_LENGTH, BLAKE3_OUTPUT_LENGTH))
		{
			return 0;
		}

		if (0 != memcpy_s(block + BLAKE3_OUTPUT_LENGTH, BLAKE3_OUTPUT_LENGTH, h, BLAKE3_OUTPUT_LENGTH))
		{
			return 0;
		}

#else
		memcpy(block, stack + (uint64_t)stack_length * BLAKE3_OUTPUT_LENGTH, BLAKE3_OUTPUT_LENGTH);
		memcpy(block + BLAKE3_OUTPUT_LENGTH, h, BLAKE3_OUTPUT_LENGTH);
#endif

		if (!BLAKE3_compress_XOF((uint32_t*)(block + BLAKE3_OUTPUT_LENGTH), m, t, l, domain_flags, NULL))
		{
			return 0;
		}

#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(m, BLAKE3_BLOCK_LENGTH, block, BLAKE3_BLOCK_LENGTH))
		{
			return 0;
		}

#else
		memcpy(m, block, BLAKE3_BLOCK_LENGTH);
#endif
		domain_flags = d | PARENT;
		l = BLAKE3_BLOCK_LENGTH;
		t[0] = t[1] = 0;
	}

	domain_flags |= ROOT;
	return BLAKE3_get_bytes_from_root_chunk(hash_length, h, m, l, domain_flags, output);
}

uint8_t BLAKE3_init(uint32_t* h, uint8_t h_length,
					uint32_t* m, uint8_t m_length,
					uint16_t stack_length)
{
	if (NULL == h ||
		h_length < BLAKE3_OUTPUT_LENGTH / sizeof(uint32_t) ||
		NULL == m ||
		m_length < BLAKE3_BLOCK_LENGTH / sizeof(uint32_t) ||
		stack_length < (uint16_t)BLAKE3_STACK_LENGTH * BLAKE3_OUTPUT_LENGTH)
	{
		return 0;
	}

#if __STDC_LIB_EXT1__

	if (0 != memcpy_s(h, BLAKE3_OUTPUT_LENGTH, IV, BLAKE3_OUTPUT_LENGTH))
	{
		return 0;
	}

#else
	memcpy(h, IV, BLAKE3_OUTPUT_LENGTH);
#endif
	memset(m, 0, BLAKE3_BLOCK_LENGTH);
	return 1;
}

uint8_t BLAKE3(const uint8_t* start, const uint8_t* finish, uint8_t hash_length, uint8_t* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		hash_length < 1 ||
		NULL == output)
	{
		return 0;
	}

	static const uint8_t d = 0;
	/**/
	const ptrdiff_t length = finish - start;
	uint32_t m[BLAKE3_BLOCK_LENGTH / sizeof(uint32_t)];
	uint32_t h[BLAKE3_OUTPUT_LENGTH / sizeof(uint32_t)];

	if (!BLAKE3_init(h, BLAKE3_OUTPUT_LENGTH / sizeof(uint32_t),
					 m, BLAKE3_BLOCK_LENGTH / sizeof(uint32_t),
					 (uint16_t)BLAKE3_STACK_LENGTH * BLAKE3_OUTPUT_LENGTH))
	{
		return 0;
	}

	uint8_t l = 0;
	uint32_t t[2];
	t[0] = t[1] = 0;
	uint8_t compressed = 0;
	uint8_t stack[(uint16_t)BLAKE3_STACK_LENGTH * BLAKE3_OUTPUT_LENGTH];
	uint8_t stack_length = 0;

	if (0 < length && !BLAKE3_core(start, (uint64_t)length, m, &l, h, &compressed, t, stack, &stack_length, d))
	{
		return 0;
	}

	return BLAKE3_final(stack, stack_length, compressed, t, h, m, l, d, hash_length, output);
}

uint8_t hash_algorithm_blake3(const uint8_t* start, const uint8_t* finish, uint16_t hash_length,
							  struct buffer* output)
{
	if (hash_length < 8 || 1024 < hash_length)
	{
		return 0;
	}

	hash_length = hash_length / 8;
	/**/
	return buffer_append(output, NULL, UINT8_MAX) &&
		   BLAKE3(start, finish, (uint8_t)hash_length, (buffer_data(output, 0) + buffer_size(output) - UINT8_MAX)) &&
		   buffer_resize(output, buffer_size(output) - (UINT8_MAX - hash_length));
}
