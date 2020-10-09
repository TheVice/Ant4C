/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "common.h"
#include "conversion.h"
#include "buffer.h"
#include "range.h"
#include "string_unit.h"

#include <stdio.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#define FIND_ANY_SYMBOL_LIKE_OR_NOT_LIKE_THAT(start, finish, that, that_length, like, step) \
	if ((NULL == start) || (NULL == finish) ||												\
		(NULL == that) || (that_length < 1) ||												\
		(0 != like && 1 != like) ||															\
		(-1 != step && 1 != step) ||														\
		((0 < step) ? (finish < start) : (finish > start)))									\
	{																						\
		return finish;																		\
	}																						\
	\
	while (start != finish)																	\
	{																						\
		uint8_t is_like = 0;																\
		\
		for (ptrdiff_t i = 0; i < that_length; ++i)											\
		{																					\
			is_like = (that[i] == (*start));												\
			\
			if (is_like)																	\
			{																				\
				break;																		\
			}																				\
		}																					\
		\
		if (like == is_like)																\
		{																					\
			return start;																	\
		}																					\
		\
		start += step;																		\
	}																						\
	\
	return start;

const uint8_t* find_any_symbol_like_or_not_like_that(
	const uint8_t* start, const uint8_t* finish, const uint8_t* that,
	ptrdiff_t that_length, uint8_t like, int8_t step)
{
	FIND_ANY_SYMBOL_LIKE_OR_NOT_LIKE_THAT(start, finish, that, that_length, like, step);
}

const uint32_t* find_any_symbol_like_or_not_like_that_UTF32LE(
	const uint32_t* start, const uint32_t* finish,
	const uint16_t* that, ptrdiff_t that_length, uint8_t like, int8_t step)
{
	FIND_ANY_SYMBOL_LIKE_OR_NOT_LIKE_THAT(start, finish, that, that_length, like, step);
}

#if defined(_WIN32)
const wchar_t* find_any_symbol_like_or_not_like_that_wchar_t(
	const wchar_t* start, const wchar_t* finish, const wchar_t* that,
	ptrdiff_t that_length, uint8_t like, int8_t step)
{
	FIND_ANY_SYMBOL_LIKE_OR_NOT_LIKE_THAT(start, finish, that, that_length, like, step);
}
#endif
uint8_t common_replace_double_byte_by_single(uint8_t* input, ptrdiff_t* size, uint8_t to_be_replaced)
{
	ptrdiff_t match = 0;

	if (NULL == input || NULL == size || 0 == (match = (*size)))
	{
		return 0;
	}

	for (ptrdiff_t i = 0, current = -1, count = match; i < count; ++i)
	{
		if (i + 1 < count && to_be_replaced == input[i + 1] && to_be_replaced == input[i])
		{
			--match;

			if (-1 == current)
			{
				current = i;
			}

			continue;
		}

		if (-1 != current && current != i)
		{
			input[current++] = input[i];
		}
	}

	if (match != (*size))
	{
		(*size) = match;
	}

	return 1;
}

ptrdiff_t common_count_bytes_until(const uint8_t* bytes, uint8_t until)
{
	if (NULL == bytes)
	{
		return 0;
	}

	ptrdiff_t count = 0;

	while (until != *bytes)
	{
		++count;
		++bytes;
	}

	return count;
}

uint8_t common_string_to_enum(const uint8_t* string_start, const uint8_t* string_finish,
							  const uint8_t** reference_strings, uint8_t max_enum_value)
{
	if (range_in_parts_is_null_or_empty(string_start, string_finish) ||
		NULL == reference_strings ||
		0 == max_enum_value)
	{
		return max_enum_value;
	}

	for (uint8_t i = 0; i < max_enum_value; ++i)
	{
		const size_t length = common_count_bytes_until(reference_strings[i], 0);

		if (string_equal(string_start, string_finish,
						 reference_strings[i], reference_strings[i] + length))
		{
			return i;
		}
	}

	return max_enum_value;
}

uint8_t common_append_string_to_buffer(const uint8_t* input, struct buffer* output)
{
	if (NULL == input || NULL == output)
	{
		return 0;
	}

	return buffer_append(output, input, common_count_bytes_until(input, 0));
}

uint8_t common_get_arguments(const struct buffer* boxed_arguments, uint8_t arguments_count,
							 struct range* arguments, uint8_t terminate)
{
	if (NULL == arguments)
	{
		return 0;
	}

	for (uint8_t i = 0; i < arguments_count; ++i)
	{
		struct buffer* argument = buffer_buffer_data(boxed_arguments, i);

		if (argument)
		{
			const ptrdiff_t size = buffer_size(argument);

			if (size)
			{
				if (terminate)
				{
					if (!buffer_push_back(argument, '\0'))
					{
						return 0;
					}
				}

				arguments[i].start = buffer_data(argument, 0);
				arguments[i].finish = arguments[i].start + size;
			}
			else
			{
				arguments[i].start = arguments[i].finish = NULL;
			}
		}
		else
		{
			arguments[i].start = arguments[i].finish = (const uint8_t*)(&(arguments[i]));
		}
	}

	return 1;
}

uint8_t common_get_attributes_and_arguments_for_task(
	const uint8_t** input_task_attributes,
	const uint8_t* input_task_attributes_lengths,
	uint8_t input_task_attributes_count,
	const uint8_t*** task_attributes,
	const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count,
	struct buffer* task_arguments)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	if (NULL != task_attributes)
	{
		*task_attributes = input_task_attributes;
	}

	if (NULL != task_attributes_lengths)
	{
		*task_attributes_lengths = input_task_attributes_lengths;
	}

	if (NULL != task_attributes_count)
	{
		*task_attributes_count = input_task_attributes_count;
	}

	buffer_release_inner_buffers(task_arguments);

	if (!buffer_resize(task_arguments, 0) ||
		!buffer_append_buffer(task_arguments, NULL, input_task_attributes_count))
	{
		return 0;
	}

	for (uint8_t i = 0; i < input_task_attributes_count; ++i)
	{
		struct buffer* attribute = buffer_buffer_data(task_arguments, i);
		SET_NULL_TO_BUFFER(*attribute);
	}

	return 1;
}

static void* output_stream = NULL;
static void* error_output_stream = NULL;

void common_set_output_stream(void* stream)
{
	output_stream = stream;
}

void common_set_error_output_stream(void* stream)
{
	error_output_stream = stream;
}

void* common_get_output_stream()
{
	return NULL == output_stream ? stdout : output_stream;
}

void* common_get_error_output_stream()
{
	return NULL == error_output_stream ? stderr : error_output_stream;
}

uint8_t common_is_output_stream_standard()
{
	return NULL == output_stream;
}

uint8_t common_is_error_output_stream_standard()
{
	return NULL == error_output_stream;
}

static uint8_t module_priority = 0;

void common_set_module_priority(uint8_t priority)
{
	module_priority = 0 < priority;
}

uint8_t common_get_module_priority()
{
	return module_priority;
}
