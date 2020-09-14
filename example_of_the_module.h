/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef _EXAMPLE_OF_THE_MODULE_H_
#define _EXAMPLE_OF_THE_MODULE_H_

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)

#  ifndef ANT4C_MODULE_EXPORT
#    ifdef example_of_the_module_EXPORTS
#      define ANT4C_MODULE_EXPORT __declspec(dllexport)
#    else
#      define ANT4C_MODULE_EXPORT __declspec(dllimport)
#    endif
#  endif

#else

#  ifndef ANT4C_MODULE_EXPORT
#    ifdef example_of_the_module_EXPORTS
#      define ANT4C_MODULE_EXPORT __attribute__((visibility("default")))
#    else
#      define ANT4C_MODULE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#endif

ANT4C_MODULE_EXPORT const uint8_t* enumerate_tasks(ptrdiff_t index);
ANT4C_MODULE_EXPORT const uint8_t* enumerate_name_spaces(ptrdiff_t index);
ANT4C_MODULE_EXPORT const uint8_t* enumerate_functions(const uint8_t* name_space, ptrdiff_t index);

ANT4C_MODULE_EXPORT uint8_t get_attributes_and_arguments_for_task(
	const uint8_t* task,
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count);

ANT4C_MODULE_EXPORT uint8_t evaluate_task(
	const uint8_t* task,
	const uint8_t** arguments, const uint16_t* arguments_lengths, uint8_t arguments_count,
	const uint8_t** output, uint16_t* output_length,
	uint8_t verbose);

ANT4C_MODULE_EXPORT uint8_t evaluate_function(
	const uint8_t* function,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	const uint8_t** output, uint16_t* output_length);

ANT4C_MODULE_EXPORT void module_release();

#endif
