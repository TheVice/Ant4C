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
	uint8_t output[UINT8_MAX];

	if (!BLAKE3((const uint8_t*)&argc, (const uint8_t*)&argc, 32/*131*/, output))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	for (argc = 0; argc < 32; ++argc)
	{
		printf(output[argc] < 16 ? "0%x" : "%x", output[argc]);
	}
	
	printf("\nSUCCESS\n");
	(void)argv;
	return EXIT_SUCCESS;
}
