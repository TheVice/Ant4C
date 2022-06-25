/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

/*
 * As reference used:
 * RFC 7693 - BLAKE2 Cryptographic Hash and MAC.
 * Date Published - November 2015.
 * https://tools.ietf.org/html/rfc7693
 */

#include "stdc_secure_api.h"

#include "hash.h"
#include "buffer.h"
#include "range.h"

#include <string.h>

static const uint64_t IV[] =
{
	0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
	0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};

#define ROTATE_RIGHT_UINT64_T(VALUE, OFFSET)			\
	((VALUE) >> (OFFSET)) | (VALUE) << (64 - (OFFSET))

#define BLAKE2b_MIX(VA, VB, VC, VD, X, Y)				\
	(VA) += (VB) + (X);									\
	(VD) = (VD) ^ (VA);									\
	(VD) = ROTATE_RIGHT_UINT64_T((VD), 32);				\
	(VC) += (VD);										\
	(VB) = (VB) ^ (VC);									\
	(VB) = ROTATE_RIGHT_UINT64_T((VB), 24);				\
	(VA) += (VB) + (Y);									\
	(VD) = (VD) ^ (VA);									\
	(VD) = ROTATE_RIGHT_UINT64_T((VD), 16);				\
	(VC) += (VD);										\
	(VB) = (VB) ^ (VC);									\
	(VB) = ROTATE_RIGHT_UINT64_T((VB), 63);

uint8_t BLAKE2b_compress(uint64_t* h, const uint64_t* chunk, const uint64_t* t, uint8_t isLastBlock)
{
	static const uint8_t SIGMA[10][16] =
	{
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
		{ 14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
		{ 11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
		{ 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
		{ 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
		{ 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 },
		{ 12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 },
		{ 13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 },
		{ 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 },
		{ 10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0 }
	};

	if (NULL == h ||
		NULL == chunk ||
		NULL == t)
	{
		return 0;
	}

	uint64_t V[16];

	for (uint8_t i = 0; i < 16; ++i)
	{
		V[i] = (i < sizeof(uint64_t)) ? h[i] : IV[i - sizeof(uint64_t)];
	}

	V[12] = V[12] ^ t[0]; /*Lo*/
	V[13] = V[13] ^ t[1]; /*Hi*/

	if (isLastBlock)
	{
		V[14] = V[14] ^ UINT64_MAX;
	}

	for (uint8_t i = 0; i < 12; ++i)
	{
		const uint8_t* S = SIGMA[i % 10];
		BLAKE2b_MIX(V[0], V[4], V[8], V[12], chunk[S[0]], chunk[S[1]]);
		BLAKE2b_MIX(V[1], V[5], V[9], V[13], chunk[S[2]], chunk[S[3]]);
		BLAKE2b_MIX(V[2], V[6], V[10], V[14], chunk[S[4]], chunk[S[5]]);
		BLAKE2b_MIX(V[3], V[7], V[11], V[15], chunk[S[6]], chunk[S[7]]);
		BLAKE2b_MIX(V[0], V[5], V[10], V[15], chunk[S[8]], chunk[S[9]]);
		BLAKE2b_MIX(V[1], V[6], V[11], V[12], chunk[S[10]], chunk[S[11]]);
		BLAKE2b_MIX(V[2], V[7], V[8], V[13], chunk[S[12]], chunk[S[13]]);
		BLAKE2b_MIX(V[3], V[4], V[9], V[14], chunk[S[14]], chunk[S[15]]);
	}

	for (uint8_t i = 0, j = sizeof(uint64_t); i < sizeof(uint64_t); ++i, ++j)
	{
		h[i] = h[i] ^ V[i];
		h[i] = h[i] ^ V[j];
	}

	return 1;
}

uint8_t BLAKE2b_init(uint8_t hash_length, uint64_t* output)
{
	if (NULL == output)
	{
		return 0;
	}

	for (uint8_t i = 1; i < sizeof(uint64_t); ++i)
	{
		output[i] = IV[i];
	}

	output[0] = 0x01010000;
	/*output[0] += (key_size << 2);*/
	output[0] += hash_length;
	output[0] = IV[0] ^ output[0];
	/**/
	return 1;
}

uint8_t BLAKE2b_core(const uint8_t* start, const uint8_t* finish, ptrdiff_t* bytes_compressed,
					 uint64_t* output)
{
	if (range_in_parts_is_null_or_empty(start, finish) ||
		0 != ((finish - start) % 128) ||
		NULL == bytes_compressed ||
		NULL == output)
	{
		return 0;
	}

	uint64_t t[2];
	t[1] = 0;/*NOTE: using of t[1] not implemented.*/
	uint64_t start_uint64_t[16];

	while (start < finish)
	{
		*bytes_compressed += 128;
		t[0] = *bytes_compressed;

		if (!hash_algorithm_uint8_t_array_to_uint64_t_array(start, 128, start_uint64_t) ||
			!BLAKE2b_compress(output, start_uint64_t, t, 0))
		{
			return 0;
		}

		start += 128;
	}

	return 1;
}

uint8_t BLAKE2b_final(const uint8_t* start, ptrdiff_t* bytes_compressed, uint8_t bytes_remaining,
					  uint64_t* output)
{
	if (NULL == start ||
		NULL == bytes_compressed ||
		128 < bytes_remaining ||
		NULL == output)
	{
		return 0;
	}

	uint64_t t[2];
	t[0] = t[1] = 0;/*NOTE: using of t[1] not implemented.*/
	uint8_t chunk[128];

	if (0 < bytes_remaining)
	{
#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(chunk, 128, start, bytes_remaining))
		{
			return 0;
		}

#else
		memcpy(chunk, start, bytes_remaining);
#endif
		*bytes_compressed += bytes_remaining;
	}

	t[0] = *bytes_compressed;

	if (128 != bytes_remaining)
	{
		memset(chunk + bytes_remaining, 0, 128 - bytes_remaining);
	}

	uint64_t chunk_uint64_t[16];

	if (!hash_algorithm_uint8_t_array_to_uint64_t_array(chunk, 128, chunk_uint64_t))
	{
		return 0;
	}

	return BLAKE2b_compress(output, chunk_uint64_t, t, 1);
}

uint8_t BLAKE2b(const uint8_t* start, const uint8_t* finish,
				uint8_t hash_length, uint64_t* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	if (!BLAKE2b_init(hash_length, output))
	{
		return 0;
	}

	ptrdiff_t bytes_compressed = 0;

	while (start + 128 < finish)
	{
		if (!BLAKE2b_core(start, start + 128, &bytes_compressed, output))
		{
			return 0;
		}

		start += 128;
	}

	return BLAKE2b_final(start, &bytes_compressed, (uint8_t)(finish - start), output);
}

uint8_t hash_algorithm_blake2b(
	const uint8_t* start, const uint8_t* finish,
	uint16_t hash_length, struct buffer* output)
{
	uint64_t out[16];

	if (hash_length < sizeof(uint64_t) ||
		1024 < hash_length ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 128))
	{
		return 0;
	}

	hash_length /= sizeof(uint64_t);

	if (!BLAKE2b(start, finish, (uint8_t)hash_length, out))
	{
		return 0;
	}

	return hash_algorithm_uint64_t_array_to_uint8_t_array(out, out + 16, buffer_data(output, size)) &&
		   buffer_resize(output, buffer_size(output) - (128 - hash_length));
}
