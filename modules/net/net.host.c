/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "net.host.h"
#include "arguments.h"
#include "net.common.h"
#include "host_fxr.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

uint8_t net_result_to_string(
	const uint8_t* result, uint16_t result_length, struct buffer* output)
{
	if (!buffer_append(output, result, result_length) ||
		!buffer_push_back(output, 0))
	{
		return 0;
	}

	const int32_t result_ = int_parse(buffer_data(output, 0));

	if (!buffer_resize(output, 0) ||
		!result_code_to_string(result_, output))
	{
		return 0;
	}

	return 1;
}

uint8_t net_host_get_hostfxr_path(
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!values ||
		!values_lengths ||
		(!values_count || 3 < values_count) ||
		!output)
	{
		return 0;
	}

	ptrdiff_t positions[3];
	struct buffer arguments;
	SET_NULL_TO_BUFFER(arguments);

	if (!values_to_system_paths(values, values_lengths, positions, values_count, &arguments))
	{
		buffer_release(&arguments);
		return 0;
	}

	const type_of_element* path_to_net_host_library =
		(const type_of_element*)buffer_data(&arguments, positions[0]);
	const type_of_element* path_to_assembly =
		1 < values_count ? (const type_of_element*)buffer_data(
			&arguments, positions[1]) : NULL;
	const type_of_element* path_to_dot_net_root =
		2 < values_count ? (const type_of_element*)buffer_data(
			&arguments, positions[2]) : NULL;
#if defined(_WIN32)

	if (!buffer_resize(output, sizeof(uint32_t)))
	{
		buffer_release(&arguments);
		return 0;
	}

#endif

	if (!net_host_load(
			path_to_net_host_library, path_to_assembly,
			path_to_dot_net_root, NULL, output))
	{
		buffer_release(&arguments);
		return 0;
	}

	buffer_release(&arguments);
#if defined(_WIN32)
	path_to_assembly = (const type_of_element*)buffer_data(output, sizeof(uint32_t));
	path_to_dot_net_root = (const type_of_element*)(
							   buffer_data(output, 0) + buffer_size(output));

	if (!buffer_resize(output, 0) ||
		!text_encoding_UTF16LE_to_UTF8(path_to_assembly, path_to_dot_net_root, output))
	{
		return 0;
	}

#endif
	const uint8_t* start = buffer_data(output, 0);
	const uint8_t* finish = start + buffer_size(output);
	/**/
	static const uint8_t zero = 0;
	/**/
	const uint8_t* new_finish = find_any_symbol_like_or_not_like_that(finish, start, &zero, 1, 0, -1);
	new_finish = find_any_symbol_like_or_not_like_that(new_finish, finish, &zero, 1, 1, 1);
	/**/
	return buffer_resize(output, new_finish - start);
}