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
