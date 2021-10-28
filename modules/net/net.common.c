/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "net.common.h"
#include "arguments.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"
#include "string_unit.h"

#include <string.h>

const void* string_to_pointer(const uint8_t* input, uint8_t length, struct buffer* tmp)
{
	if (!input || !length || !tmp)
	{
		return NULL;
	}

	if (!buffer_resize(tmp, 0) ||
		!buffer_append(tmp, input, length) ||
		!buffer_push_back(tmp, 0))
	{
		return NULL;
	}

	return pointer_parse(buffer_data(tmp, 0));
}

uint8_t convert_function_name(
	const uint8_t* name_space,
	const uint8_t* function_name_start, const uint8_t* function_name_finish,
	struct buffer* output)
{
	if (!name_space ||
		range_in_parts_is_null_or_empty(function_name_start, function_name_finish) ||
		!output)
	{
		return 0;
	}

	static const uint8_t hyphen = '-';
	static const uint8_t underscore = '_';

	if (!common_append_string_to_buffer(name_space, output) ||
		!buffer_push_back(output, underscore))
	{
		return 0;
	}

	return string_replace(
			   function_name_start, function_name_finish,
			   &hyphen, &hyphen + 1,
			   &underscore, &underscore + 1,
			   output);
}

uint8_t get_exists_functions(
	const void* ptr_to_object,
	const uint8_t* name_space,
	const uint8_t* functions,
	const uint8_t* delimiter,
	uint16_t delimiter_length,
	const is_function_exists_type is_function_exists,
	struct buffer* output)
{
	static const uint8_t* double_zero = (const uint8_t*)"\0\0";

	if (!ptr_to_object ||
		!name_space ||
		!functions ||
		!is_function_exists ||
		!output)
	{
		return 0;
	}

	const uint8_t* finish = functions;

	while (memcmp(finish, double_zero, 2))
	{
		++finish;
	}

	uint8_t i = 0;
	uint8_t function_exists[16];
	const uint8_t* functions_names[16];
	memset(function_exists, 0, sizeof(function_exists));

	do
	{
		functions = find_any_symbol_like_or_not_like_that(functions, finish, double_zero, 1, 0, 1);

		if (!buffer_resize(output, 0) ||
			!convert_function_name(
				name_space,
				functions, functions + common_count_bytes_until(functions, 0),
				output))
		{
			return 0;
		}

		const uint8_t* function_name = buffer_data(output, 0);
		/**/
		function_exists[i] =
			is_function_exists(
				ptr_to_object, function_name, (uint8_t)buffer_size(output));
		functions_names[i] = functions;
		/**/
		++i;
	}
	while (finish !=
		   (functions = find_any_symbol_like_or_not_like_that(functions, finish, double_zero, 1, 1, 1)) && i < 16);

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	const uint8_t count = i;

	for (i = 0; i < count; ++i)
	{
		if (!function_exists[i])
		{
			continue;
		}

		if (!common_append_string_to_buffer(functions_names[i], output) ||
			!buffer_append(output, delimiter, delimiter_length))
		{
			return 0;
		}
	}

	const ptrdiff_t size = buffer_size(output);

	if (0 < size && !buffer_resize(output, size - delimiter_length))
	{
		return 0;
	}

	return 1;
}

uint8_t load_library(const uint8_t* path_to_library,
					 uint16_t path_to_library_length,
					 struct buffer* tmp,
					 void* ptr_to_object,
					 ptrdiff_t size,
					 const loader_type loader)
{
	if (!path_to_library ||
		!path_to_library_length ||
		!tmp ||
		!ptr_to_object ||
		size < 1 ||
		!loader)
	{
		return 0;
	}

	if (!buffer_resize(tmp, 0) ||
		!value_to_system_path(path_to_library, path_to_library_length, tmp))
	{
		return 0;
	}

#if defined(_WIN32)
	++path_to_library_length;
	const type_of_element* path_to_library_ =
		(const type_of_element*)buffer_data(tmp, path_to_library_length);
#else
	const type_of_element* path_to_library_ = (const type_of_element*)buffer_data(tmp, 0);
#endif
	return loader(path_to_library_, ptr_to_object, size);
}

uint8_t is_function_exists(
	const void* ptr_to_object, const uint8_t* name_space,
	const uint8_t* function_name, uint16_t function_name_length,
	const is_function_exists_type is_exists,
	struct buffer* output)
{
	if (!ptr_to_object ||
		!name_space ||
		!function_name ||
		!function_name_length ||
		!is_exists ||
		!output)
	{
		return 0;
	}

	if (!buffer_resize(output, 0) ||
		!convert_function_name(
			name_space, function_name, function_name + function_name_length, output))
	{
		return 0;
	}

	function_name = buffer_data(output, 0);
	function_name_length = (uint16_t)buffer_size(output);
	function_name_length = is_exists(
							   ptr_to_object, function_name, (uint8_t)function_name_length);
	/**/
	return buffer_resize(output, 0) && bool_to_string(0 < function_name_length, output);
}
