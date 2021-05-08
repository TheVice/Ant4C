/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#ifndef _REGEX_MODULE_H_
#define _REGEX_MODULE_H_

#include <cstddef>
#include <cstdint>

#ifdef ANT4C_REGEX_STATIC_DEFINE
#  define ANT4C_REGEX_EXPORT
#  define ANT4C_REGEX_NO_EXPORT
#else
#if defined(_WIN32)
#  ifndef ANT4C_REGEX_EXPORT
#    ifdef ant4c_regex_EXPORTS
#      define ANT4C_REGEX_EXPORT __declspec(dllexport)
#    else
#      define ANT4C_REGEX_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef ANT4C_REGEX_NO_EXPORT
#    define ANT4C_REGEX_NO_EXPORT
#  endif
#else
#  ifndef ANT4C_REGEX_EXPORT
#    ifdef ant4c_regex_EXPORTS
#      define ANT4C_REGEX_EXPORT __attribute__((visibility("default")))
#    else
#      define ANT4C_REGEX_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef ANT4C_REGEX_NO_EXPORT
#    define ANT4C_REGEX_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#endif

#if defined(_WIN32)
#ifndef ANT4C_REGEX_DEPRECATED
#  define ANT4C_REGEX_DEPRECATED __declspec(deprecated)
#endif
#else
#ifndef ANT4C_REGEX_DEPRECATED
#  define ANT4C_REGEX_DEPRECATED __attribute__ ((__deprecated__))
#endif
#endif

#ifndef ANT4C_REGEX_DEPRECATED_EXPORT
#  define ANT4C_REGEX_DEPRECATED_EXPORT ANT4C_REGEX_EXPORT ANT4C_REGEX_DEPRECATED
#endif

#ifndef ANT4C_REGEX_DEPRECATED_NO_EXPORT
#  define ANT4C_REGEX_DEPRECATED_NO_EXPORT ANT4C_REGEX_NO_EXPORT ANT4C_REGEX_DEPRECATED
#endif

#if 0
#  ifndef ANT4C_REGEX_NO_DEPRECATED
#    define ANT4C_REGEX_NO_DEPRECATED
#  endif
#endif

extern "C" ANT4C_REGEX_EXPORT const uint8_t* enumerate_tasks(ptrdiff_t index);

extern "C" ANT4C_REGEX_EXPORT uint8_t get_attributes_and_arguments_for_task(const uint8_t* task,
		const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
		uint8_t* task_attributes_count);

extern "C" ANT4C_REGEX_EXPORT uint8_t evaluate_task(const uint8_t* task,
		const uint8_t** arguments, const uint16_t* arguments_lengths, uint8_t arguments_count,
		const uint8_t** output, uint16_t* output_length,
		uint8_t verbose);

#endif
