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
#elif CRC32 || BLAKE2 || SHA3
#if CRC32
	const uint8_t* input[] = { (const uint8_t*)"", (const uint8_t*)"The quick brown fox jumps over the lazy dog" };
	const uint8_t lengths[] = { 0, 43 };
	const char* expected_output[] = { "00000000", "00000000", "414fa339", "39a34f41" };
#define FUNCTION_COUNT 2
#elif BLAKE2
	typedef uint8_t(*BLAKE2b)(const uint8_t*, const uint8_t*, struct buffer*);
	static const BLAKE2b functions[] =
	{
		&hash_algorithm_blake2b_160, &hash_algorithm_blake2b_256,
		&hash_algorithm_blake2b_384, &hash_algorithm_blake2b_512
	};
#define FUNCTION_COUNT 4
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
#elif SHA3
	typedef uint8_t(*Keccak)(const uint8_t*, const uint8_t*, struct buffer*);
	/**/
	static const Keccak functions[] =
	{
		&hash_algorithm_keccak_224, &hash_algorithm_keccak_256,
		&hash_algorithm_keccak_384, &hash_algorithm_keccak_512,
		/**/
		&hash_algorithm_sha3_224, &hash_algorithm_sha3_256,
		&hash_algorithm_sha3_384, &hash_algorithm_sha3_512
	};
#define FUNCTION_COUNT 8
	/**/
	const uint8_t* input[] =
	{
		(const uint8_t*)"",
		(const uint8_t*)"The quick brown fox jumps over the lazy dog"
	};
	const uint8_t lengths[] = { 0, 43 };
	const char* expected_output[] =
	{
		"f71837502ba8e10837bdd8d365adb85591895602fc552b48b7390abd",
		"c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470",
		"2c23146a63a29acf99e73b88f8c24eaa7dc60aa771780ccc006afbfa8fe2479b2dd2b21362337441ac12b515911957ff",
		"0eab42de4c3ceb9235fc91acffe746b29c29a8c366b7c60e4e67c466f36a4304c00fa9caf9d87976ba469bcbe06713b435f091ef2769fb160cdab33d3670680e",
		"6b4e03423667dbb73b6e15454f0eb1abd4597f9a1b078e3f5b5a6bc7",
		"a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a",
		"0c63a75b845e4f7d01107d852e4c2485c51a50aaaa94fc61995e71bbee983a2ac3713831264adb47fb6bd1e058d5f004",
		"a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26",
		/**/
		"310aee6b30c47350576ac2873fa89fd190cdc488442f3ef654cf23fe",
		"4d741b6f1eb29cb2a9b9911c82f56fa8d73b04959d3d9d222895df6c0b28aa15",
		"283990fa9d5fb731d786c5bbee94ea4db4910f18c62c03d173fc0a5e494422e8a0b3da7574dae7fa0baf005e504063b3",
		"d135bb84d0439dbac432247ee573a23ea7d3c9deb2a968eb31d47c4fb45f1ef4422d6c531b5b9bd6f449ebcc449ea94d0a8f05f62130fda612da53c79659f609",
		"d15dadceaa4d5d7bb3b48f446421d542e08ad8887305e28d58335795",
		"69070dda01975c8c120c3aada1b282394e7f032fa9cf32f4cb2259a0897dfc04",
		"7063465e08a93bce31cd89d2e3ca8f602498696e253592ed26f07bf7e703cf328581e1471a7ba7ab119b1a9ebdf8be41",
		"01dedd5de4ef14642445ba5f5b97c15e47b9ad931326e4b0727cd94cefc44fff23f07bf543139939b49128caf436dc1bdee54fcb24023a08d9403f9b4bf0d450"
	};
	/**/
#endif
	struct buffer str_output;
	SET_NULL_TO_BUFFER(str_output);

	for (uint8_t i = 0, count = sizeof(lengths) / sizeof(*lengths), o = 0; i < count; ++i)
	{
		for (uint8_t j = 0; j < FUNCTION_COUNT; ++j, ++o)
		{
#if CRC32
			uint32_t output = 0;

			if (!hash_algorithm_crc32(input[i], input[i] + lengths[i], &output, j))
			{
				buffer_release(&str_output);
				printf("%i %s\n", __LINE__, "FAILURE");
				return EXIT_FAILURE;
			}

			if (!buffer_resize(&str_output, 0) ||
				!hash_algorithm_bytes_to_string((const uint8_t*)(&output), (const uint8_t*)(&output + 1), &str_output))
#else
			if (!buffer_resize(&str_output, 0) ||
				!functions[j](input[i], input[i] + lengths[i], &str_output))
			{
				buffer_release(&str_output);
				printf("%i %s\n", __LINE__, "FAILURE");
				return EXIT_FAILURE;
			}

			const ptrdiff_t size = buffer_size(&str_output);

			if (!buffer_append(&str_output, NULL, 2 * size + 2) ||
				!buffer_resize(&str_output, size) ||
				!hash_algorithm_bytes_to_string(buffer_data(&str_output, 0), buffer_data(&str_output, 0), &str_output))
#endif
			{
				buffer_release(&str_output);
				printf("%i %s\n", __LINE__, "FAILURE");
				return EXIT_FAILURE;
			}
#if CRC32
			if (0 != memcmp(expected_output[o], buffer_char_data(&str_output, 0), 8))
#else
			if (0 != memcmp(expected_output[o], buffer_char_data(&str_output, 0), buffer_size(&str_output) - size))
#endif
			{
				buffer_release(&str_output);
				printf("%i %s\n", __LINE__, "FAILURE");
				return EXIT_FAILURE;
			}
		}
	}

	buffer_release(&str_output);
#if CRC32
#define HASH_ALGORITHM_NAME "CRC32"
#elif BLAKE2
#define HASH_ALGORITHM_NAME "BLAKE2b"
#elif SHA3
#define HASH_ALGORITHM_NAME "Keccak"
#endif
	printf("%s %s\n", HASH_ALGORITHM_NAME, "SUCCESS");
#endif
	/**/
	(void)argc;
	(void)argv;
	return EXIT_SUCCESS;
}
