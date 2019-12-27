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
#elif CRC32
	const uint8_t* input[] = { (const uint8_t*)"", (const uint8_t*)"The quick brown fox jumps over the lazy dog" };
	const uint8_t lengths[] = { 0, 43 };
	const char* expected_output[] = { "00000000", "00000000", "414fa339", "39a34f41" };
	struct buffer str_output;
	SET_NULL_TO_BUFFER(str_output);

	for (uint8_t i = 0, count = sizeof(lengths) / sizeof(*lengths), o = 0; i < count; ++i)
	{
		for (uint8_t j = 0; j < 2; ++j, ++o)
		{
			uint32_t output = 0;

			if (!hash_algorithm_crc32(input[i], input[i] + lengths[i], &output, j))
			{
				buffer_release(&str_output);
				printf("%s\n", "FAILURE");
				return EXIT_FAILURE;
			}

			if (!buffer_resize(&str_output, 0) ||
				!hash_algorithm_bytes_to_string((const uint8_t*)(&output), (const uint8_t*)(&output + 1), &str_output))
			{
				buffer_release(&str_output);
				printf("%s\n", "FAILURE");
				return EXIT_FAILURE;
			}

			if (0 != memcmp(expected_output[o], buffer_char_data(&str_output, 0), 8))
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
