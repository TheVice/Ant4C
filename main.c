/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

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

void bytes_to_string(const uint8_t* input, uint8_t length, char* output)
{
	for (uint8_t j = 0; j < length; ++j)
	{
		output += sprintf(output, input[j] < 16 ? "0%x" : "%x", input[j]);
	}
}

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
	uint8_t input[31745];

	for (uint16_t i = 0, j = 0; i < 31745; ++i, j = j < 250 ? j + 1 : 0)
	{
		input[i] = (uint8_t)j;
	}

	const uint16_t lengths[] =
	{
		0,
		1,
		1023
	};
	/**/
	const uint8_t* expected_outputs[] =
	{
		"af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262e00f03e7b69af26b7faaf09fcd333050338ddfe085b8cc869ca98b206c08243a26f5487789e8f660afe6c99ef9e0c52b92e7393024a80459cf91f476f9ffdbda7001c22e159b402631f277ca96f2defdf1078282314e763699a31c5363165421cce14d",
		"2d3adedff11b61f14c886e35afa036736dcd87a74d27b5c1510225d0f592e213c3a6cb8bf623e20cdb535f8d1a5ffb86342d9c0b64aca3bce1d31f60adfa137b358ad4d79f97b47c3d5e79f179df87a3b9776ef8325f8329886ba42f07fb138bb502f4081cbcec3195c5871e6c23e2cc97d3c69a613eba131e5f1351f3f1da786545e5",
		"10108970eeda3eb932baac1428c7a2163b0e924c9a9e25b35bba72b28f70bd11a182d27a591b05592b15607500e1e8dd56bc6c7fc063715b7a1d737df5bad3339c56778957d870eb9717b57ea3d9fb68d1b55127bba6a906a4a24bbd5acb2d123a37b28f9e9a81bbaae360d58f85e5fc9d75f7c370a0cc09b6522d9c8d822f2f28f485"
	};

	const uint8_t count = (uint8_t)(sizeof(lengths) / sizeof(*lengths));

	if (count != sizeof(expected_outputs) / sizeof(*expected_outputs))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	uint8_t output[2 * UINT8_MAX];

	for (uint8_t i = 0; i < count; ++i)
	{
		if (31744 < lengths[i])
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return EXIT_FAILURE;
		}

		if (!BLAKE3(input, input + lengths[i], 131, output))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return EXIT_FAILURE;
		}

		char* ptr = (char*)(output + 131);
		bytes_to_string(output, 131, ptr);

		if (strlen(expected_outputs[i]) != strlen(ptr) ||
			0 != memcmp(expected_outputs[i], ptr, strlen(expected_outputs[i])))
		{
			printf("%i\nexpected: '%s'\nreturned: '%s'\n%s\n", i, expected_outputs[i], ptr, "FAILURE");
			return EXIT_FAILURE;
		}

		if (!BLAKE3(input + 1, input + lengths[i] + 1, 131, output))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return EXIT_FAILURE;
		}

		bytes_to_string(output, 131, ptr);

		if (strlen(expected_outputs[i]) != strlen(ptr))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return EXIT_FAILURE;
		}

		if (0 < lengths[i] && 0 == memcmp(expected_outputs[i], ptr, strlen(expected_outputs[i])))
		{
			printf("%i\nhash '%s' should not be equal to expected at this point.\n%s\n", i, ptr, "FAILURE");
			return EXIT_FAILURE;
		}
	}

	printf("\nSUCCESS\n");
	(void)argc;
	(void)argv;
	return EXIT_SUCCESS;
}
