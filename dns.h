/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef _DNS_MODULE_H_
#define _DNS_MODULE_H_

#include <stdint.h>

#ifdef ANT4C_DNS_STATIC_DEFINE
#  define ANT4C_DNS_EXPORT
#  define ANT4C_DNS_NO_EXPORT
#else
#if defined(_WIN32)
#  ifndef ANT4C_DNS_EXPORT
#    ifdef ant4c_dns_EXPORTS
#      define ANT4C_DNS_EXPORT __declspec(dllexport)
#    else
#      define ANT4C_DNS_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef ANT4C_DNS_NO_EXPORT
#    define ANT4C_DNS_NO_EXPORT
#  endif
#else
#  ifndef ANT4C_DNS_EXPORT
#    ifdef ant4c_dns_EXPORTS
#      define ANT4C_DNS_EXPORT __attribute__((visibility("default")))
#    else
#      define ANT4C_DNS_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef ANT4C_DNS_NO_EXPORT
#    define ANT4C_DNS_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#endif

#if defined(_WIN32)
#ifndef ANT4C_DNS_DEPRECATED
#  define ANT4C_DNS_DEPRECATED __declspec(deprecated)
#endif
#else
#ifndef ANT4C_DNS_DEPRECATED
#  define ANT4C_DNS_DEPRECATED __attribute__ ((__deprecated__))
#endif
#endif

#ifndef ANT4C_DNS_DEPRECATED_EXPORT
#  define ANT4C_DNS_DEPRECATED_EXPORT ANT4C_DNS_EXPORT ANT4C_DNS_DEPRECATED
#endif

#ifndef ANT4C_DNS_DEPRECATED_NO_EXPORT
#  define ANT4C_DNS_DEPRECATED_NO_EXPORT ANT4C_DNS_NO_EXPORT ANT4C_DNS_DEPRECATED
#endif

#if 0
#  ifndef ANT4C_DNS_NO_DEPRECATED
#    define ANT4C_DNS_NO_DEPRECATED
#  endif
#endif

extern "C" ANT4C_DNS_EXPORT const uint8_t* enumerate_name_spaces(uint8_t index);
extern "C" ANT4C_DNS_EXPORT const uint8_t* enumerate_functions(const uint8_t* name_space, uint8_t index);

extern "C" ANT4C_DNS_EXPORT uint8_t evaluate_function(const uint8_t* function,
		const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
		const uint8_t** output, uint16_t* output_length);

/*extern "C" ANT4C_DNS_EXPORT void module_release();*/

#endif
