/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "net.host.h"
#include "arguments.h"
#include "net.common.h"
#include "host_fxr.h"

#include "buffer.h"
#include "conversion.h"
#include "string_unit.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

uint8_t net_result_to_string(
	const uint8_t* result, uint16_t result_length, void* output)
{
	const int32_t code = int_parse(result, result + result_length);
	return result_code_to_string(code, output);
}

uint8_t net_host_get_hostfxr_path(
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output)
{
	if (!values ||
		!values_lengths ||
		(!values_count || 3 < values_count) ||
		!output)
	{
		return 0;
	}

	uint8_t arguments_buffer[BUFFER_SIZE_OF];
	void* arguments = (void*)arguments_buffer;

	if (!buffer_init(arguments, BUFFER_SIZE_OF))
	{
		return 0;
	}

	ptrdiff_t positions[3];

	if (!values_to_system_paths(values, values_lengths, positions, values_count, arguments))
	{
		buffer_release(arguments);
		return 0;
	}

	const type_of_element* path_to_net_host_library =
		(const type_of_element*)buffer_data(arguments, positions[0]);
	const type_of_element* path_to_assembly =
		1 < values_count ? (const type_of_element*)buffer_data(
			arguments, positions[1]) : NULL;
	const type_of_element* path_to_dot_net_root =
		2 < values_count ? (const type_of_element*)buffer_data(
			arguments, positions[2]) : NULL;
#if defined(_WIN32)

	if (!buffer_resize(output, sizeof(uint32_t)))
	{
		buffer_release(arguments);
		return 0;
	}

#endif

	if (!net_host_load(
			path_to_net_host_library, path_to_assembly,
			path_to_dot_net_root, NULL, output))
	{
		buffer_release(arguments);
		return 0;
	}

	buffer_release(arguments);
#if defined(_WIN32)
	path_to_assembly = (const type_of_element*)buffer_data(output, sizeof(uint32_t));
	path_to_dot_net_root = (const type_of_element*)(
							   buffer_uint8_t_data(output, 0) + buffer_size(output));

	if (!buffer_resize(output, 0) ||
		!text_encoding_UTF16LE_to_UTF8(path_to_assembly, path_to_dot_net_root, output))
	{
		return 0;
	}

#endif
	const uint8_t* start = buffer_uint8_t_data(output, 0);
	const uint8_t* finish = start + buffer_size(output);
	/**/
	static const uint8_t zero = 0;
	/**/
	const uint8_t* new_finish =
		string_find_any_symbol_like_or_not_like_that(finish, start, &zero, &zero + 1, 0, -1);
	new_finish =
		string_find_any_symbol_like_or_not_like_that(new_finish, finish, &zero, &zero + 1, 1, 1);
	/**/
	return buffer_resize(output, new_finish - start);
}
