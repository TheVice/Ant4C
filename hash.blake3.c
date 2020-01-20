/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

/*
 * As reference used:
 * BLAKE3 one function, fast everywhere. Version 20200109113900.
 * https://github.com/BLAKE3-team/BLAKE3-specs/raw/master/blake3.pdf
 */

#include "hash.h"
/*#include "buffer.h"*/

#include <stddef.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

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
		!BLAKE3_compress(h, m, t, b, d, V) ||
		NULL == output)
	{
		return 0;
	}

	for (uint8_t i = 0, count = 16, middle = 8; i < count; ++i)
	{
		output[i] = V[i] ^ ((i < middle) ? V[middle + i] : h[i - middle]);
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

		const uint8_t processed = 16 * sizeof(uint32_t) < hash_length ? 16 * sizeof(uint32_t) : hash_length;
		index += processed;
		hash_length -= processed;
		chunk_counter[0] += 1;
	}

	return 1;
}

uint8_t BLAKE3_final(uint8_t chunk_stack_length, uint8_t chunk_compressed,
					 uint32_t* h, const uint32_t* m, uint8_t l, uint8_t d,
					 uint8_t hash_length, uint8_t* output)
{
	if (0 == chunk_stack_length)
	{
		const uint8_t domain_flag = d | (0 < chunk_compressed ? 0 : CHUNK_START) | CHUNK_END | ROOT;
		return BLAKE3_get_bytes_from_root_chunk(hash_length, h, m, l, domain_flag, output);
	}

	/*TODO:*/
	return 0;
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

	/*TODO:*/
	uint8_t chunk_stack_length = 0;
	uint8_t chunk_compressed = 0;
	uint32_t h[8];
#if __STDC_SEC_API__

	if (0 != memcpy_s(h, 8 * sizeof(uint32_t), IV, 8 * sizeof(uint32_t)))
	{
		return 0;
	}

#else
	memcpy(h, IV, 8 * sizeof(uint32_t));
#endif
	uint32_t m[16];
	memset(m, 0, 16 * sizeof(uint32_t));
	/*uint32_t t[2];
	t[0] = t[1] = 0;*/
	uint8_t l = 0;
	uint8_t d = 0;
	/**/
	return BLAKE3_final(chunk_stack_length, chunk_compressed, h, m, l, d, hash_length, output);
}

/*uint8_t hash_algorithm_blake3_256(const uint8_t* start, const uint8_t* finish, struct buffer* output)
{
	return buffer_append(output, NULL, 256) &&
		   BLAKE3(start, finish, 32, (buffer_data(output, 0) + buffer_size(output) - 256)) &&
		   buffer_resize(output, buffer_size(output) - 224);
}*/
