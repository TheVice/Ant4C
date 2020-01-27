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
		1023,
		1024,
		1025,
		2048,
		2049,
	};
	/**/
	const uint8_t* expected_outputs[] =
	{
		"af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262e00f03e7b69af26b7faaf09fcd333050338ddfe085b8cc869ca98b206c08243a26f5487789e8f660afe6c99ef9e0c52b92e7393024a80459cf91f476f9ffdbda7001c22e159b402631f277ca96f2defdf1078282314e763699a31c5363165421cce14d",
		"2d3adedff11b61f14c886e35afa036736dcd87a74d27b5c1510225d0f592e213c3a6cb8bf623e20cdb535f8d1a5ffb86342d9c0b64aca3bce1d31f60adfa137b358ad4d79f97b47c3d5e79f179df87a3b9776ef8325f8329886ba42f07fb138bb502f4081cbcec3195c5871e6c23e2cc97d3c69a613eba131e5f1351f3f1da786545e5",
		"10108970eeda3eb932baac1428c7a2163b0e924c9a9e25b35bba72b28f70bd11a182d27a591b05592b15607500e1e8dd56bc6c7fc063715b7a1d737df5bad3339c56778957d870eb9717b57ea3d9fb68d1b55127bba6a906a4a24bbd5acb2d123a37b28f9e9a81bbaae360d58f85e5fc9d75f7c370a0cc09b6522d9c8d822f2f28f485",
		"42214739f095a406f3fc83deb889744ac00df831c10daa55189b5d121c855af71cf8107265ecdaf8505b95d8fcec83a98a6a96ea5109d2c179c47a387ffbb404756f6eeae7883b446b70ebb144527c2075ab8ab204c0086bb22b7c93d465efc57f8d917f0b385c6df265e77003b85102967486ed57db5c5ca170ba441427ed9afa684e",
		"d00278ae47eb27b34faecf67b4fe263f82d5412916c1ffd97c8cb7fb814b8444f4c4a22b4b399155358a994e52bf255de60035742ec71bd08ac275a1b51cc6bfe332b0ef84b409108cda080e6269ed4b3e2c3f7d722aa4cdc98d16deb554e5627be8f955c98e1d5f9565a9194cad0c4285f93700062d9595adb992ae68ff12800ab67a",
		"e776b6028c7cd22a4d0ba182a8bf62205d2ef576467e838ed6f2529b85fba24a9a60bf80001410ec9eea6698cd537939fad4749edd484cb541aced55cd9bf54764d063f23f6f1e32e12958ba5cfeb1bf618ad094266d4fc3c968c2088f677454c288c67ba0dba337b9d91c7e1ba586dc9a5bc2d5e90c14f53a8863ac75655461cea8f9",
		"5f4d72f40d7a5f82b15ca2b2e44b1de3c2ef86c426c95c1af0b687952256303096de31d71d74103403822a2e0bc1eb193e7aecc9643a76b7bbc0c9f9c52e8783aae98764ca468962b5c2ec92f0c74eb5448d519713e09413719431c802f948dd5d90425a4ecdadece9eb178d80f26efccae630734dff63340285adec2aed3b51073ad3",
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
