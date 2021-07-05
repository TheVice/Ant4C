/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef _STDC_SECURE_API_
#define _STDC_SECURE_API_

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__))
#include <crtdefs.h>
#endif

#if defined(__STDC_SECURE_LIB__)

#if !defined(__STDC_LIB_EXT1__)
#define __STDC_LIB_EXT1__ 1
#endif

#endif

#if defined(__STDC_LIB_EXT1__)

#if !defined(__STDC_WANT_LIB_EXT1__)
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#if !defined(__STDC_WANT_SECURE_LIB__)
#define __STDC_WANT_SECURE_LIB__ 1
#endif

#endif

#endif
