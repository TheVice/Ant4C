/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "buffer.h"
#include "hash.h"

#include <math.h>
#include <time.h>
#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <stdbool.h>
#include <inttypes.h>

#ifdef _WIN32
#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#if defined(_MSC_VER)
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
#ifdef _WIN32
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif
#if HASH
	struct buffer input;
	SET_NULL_TO_BUFFER(input);

	if (!buffer_resize(&input, UINT8_MAX + 1 + 2 * (UINT8_MAX + 1) + 2) ||
		!buffer_resize(&input, 0))
	{
		buffer_release(&input);
		return EXIT_FAILURE;
	}

	for (uint16_t i = 0, count = UINT8_MAX + 1; i < count; ++i)
	{
		if (!buffer_push_back(&input, (uint8_t)i))
		{
			buffer_release(&input);
			return EXIT_FAILURE;
		}
	}

	const ptrdiff_t size = buffer_size(&input);
	const uint8_t* ptr = buffer_data(&input, 0);

	if (!hash_algorithm_bytes_to_string(ptr, ptr + size, &input))
	{
		buffer_release(&input);
		return EXIT_FAILURE;
	}

	ptr = buffer_data(&input, size);
	printf("%s\n", ptr);
	buffer_release(&input);
#elif BLAKE2
	typedef uint8_t(*BLAKE2b)(const uint8_t*, const uint8_t*, struct buffer*);
	static const BLAKE2b functions[] =
	{
		&hash_algorithm_blake2b_160, &hash_algorithm_blake2b_256,
		&hash_algorithm_blake2b_384, &hash_algorithm_blake2b_512
	};
	/**/
	const uint8_t* input[] =
	{
		(const uint8_t*)"",
		(const uint8_t*)"The quick brown fox jumps over the lazy dog",
		(const uint8_t*)"BLAKE and BLAKE2 are hash functions based on the ChaCha stream cipher, and was one of the finalists in the NIST competition for SHA-3"
	};
	const uint8_t lengths[] = { 0, 43, 133 };
	const char* expected_output[] =
	{
		"3345524abf6bbe1809449224b5972c41790b6cf2",
		"0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8",
		"b32811423377f52d7862286ee1a72ee540524380fda1724a6f25d7978c6fd3244a6caf0498812673c5e05ef583825100",
		"786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce",
		/**/
		"3c523ed102ab45a37d54f5610d5a983162fde84f",
		"01718cec35cd3d796dd00020e0bfecb473ad23457d063b75eff29c0ffa2e58a9",
		"b7c81b228b6bd912930e8f0b5387989691c1cee1e65aade4da3b86a3c9f678fc8018f6ed9e2906720c8d2a3aeda9c03d",
		"a8add4bdddfd93e4877d2746e62817b116364a1fa7bc148d95090bc7333b3673f82401cf7aa2e4cb1ecd90296e3f14cb5413f8ed77be73045b13914cdcd6a918",
		/**/
		"5e92b21f7cb0b46d06416c314f41c474e2feaf30",
		"40a9706556c5b70ef10daae4c519c4fcfdee519b3ded9bc6801730b23f2c1d40",
		"1441c65e58b2e9656200df99c9bd1e188a3a4e18e80d0ed9f69d4ce67bde16ef8f726e54800554dda7b29f485e50d0e9",
		"f22f00d4570b502efe801674f6588ad6926b54ef3503d266224b65c257d1422683bfd4a12470ba3d5cb3c73d5016a5a6f1fb96a13b7f0a84d03777bb18e6e859"
	};
	/**/
	struct buffer str_output;
	SET_NULL_TO_BUFFER(str_output);

	for (uint8_t i = 0, count = sizeof(lengths) / sizeof(*lengths), o = 0; i < count; ++i)
	{
		for (uint8_t j = 0; j < 4; ++j, ++o)
		{
			if (!buffer_resize(&str_output, 0) ||
				!functions[j](input[i], input[i] + lengths[i], &str_output))
			{
				buffer_release(&str_output);
				printf("%s\n", "FAILURE");
				return EXIT_FAILURE;
			}

			const ptrdiff_t size = buffer_size(&str_output);

			if (!buffer_append(&str_output, NULL, 2 * size + 2) ||
				!buffer_resize(&str_output, size) ||
				!hash_algorithm_bytes_to_string(buffer_data(&str_output, 0), buffer_data(&str_output, 0), &str_output))
			{
				buffer_release(&str_output);
				printf("%s\n", "FAILURE");
				return EXIT_FAILURE;
			}

			if (0 != memcmp(expected_output[o], buffer_data(&str_output, size), buffer_size(&str_output) - size))
			{
				buffer_release(&str_output);
				printf("%s\n", "FAILURE");
				return EXIT_FAILURE;
			}
		}
	}

	buffer_release(&str_output);
	printf("%s\n", "SUCCESS");
#endif
	/**/
	(void)argc;
	(void)argv;
	return EXIT_SUCCESS;
}
