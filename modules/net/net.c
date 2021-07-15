/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "net.h"

#include "arguments.h"
#include "core_host_initialize_request.h"
#include "error_writer.h"
#include "host_fxr.h"
#include "host_interface.h"
#include "host_policy.h"
#include "net.common.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "path.h"
#include "range.h"
#include "string_unit.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
static const type_of_element* zero = L"\0";
#else
static const type_of_element* zero = (const type_of_element*)"\0";
#endif
static const uint8_t* double_zero = (const uint8_t*)"\0\0";

const void* string_to_pointer(const uint8_t* input, uint8_t length, struct buffer* tmp)
{
	if (!input ||
		!length ||
		!tmp)
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
	const uint8_t* new_finish = find_any_symbol_like_or_not_like_that(finish, start, double_zero, 1, 0, -1);
	new_finish = find_any_symbol_like_or_not_like_that(new_finish, finish, double_zero, 1, 1, 1);
	/**/
	return buffer_resize(output, new_finish - start);
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

typedef uint8_t(*is_function_exists_type)(
	const void* ptr_to_object,
	const uint8_t* function_name,
	uint8_t function_name_length);

uint8_t get_exists_functions(
	const void* ptr_to_object,
	const uint8_t* name_space,
	const uint8_t* functions,
	const uint8_t* delimiter,
	uint16_t delimiter_length,
	const is_function_exists_type is_function_exists,
	struct buffer* output)
{
	if (!ptr_to_object ||
		!name_space ||
		!functions ||
		!is_function_exists ||
		!output)
	{
		return 0;
	}

	const uint8_t* finish = functions;

	while (0 != memcmp(finish, double_zero, 2))
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

typedef uint8_t(*loader_type)(
	const type_of_element* path_to_library, void* ptr_to_object, ptrdiff_t size);

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

uint8_t host_fx_resolver_init(
	const uint8_t* path_to_host_fxr,
	uint16_t path_to_host_fxr_length,
	struct buffer* tmp,
	void* ptr_to_host_fxr_object,
	ptrdiff_t size)
{
	if (size < 160)
	{
		return 0;
	}

	return load_library(
			   path_to_host_fxr, path_to_host_fxr_length, tmp,
			   ptr_to_host_fxr_object, size, host_fx_resolver_load);
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

void host_fx_resolver_available_sdks(int32_t count, const type_of_element** directories);

uint8_t hostfxr_get_available_sdks(
	const void* ptr_to_host_fxr_object,
	const uint8_t* executable_directory, uint16_t executable_directory_length,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!output)
	{
		return 0;
	}

#if defined(_WIN32)

	if (!buffer_resize(output, sizeof(uint32_t)))
	{
		return 0;
	}

#endif
	const type_of_element* exe_dir = NULL;
	struct buffer exe_dir_;
	SET_NULL_TO_BUFFER(exe_dir_);

	if (executable_directory)
	{
		if (!value_to_system_path(executable_directory, executable_directory_length, &exe_dir_))
		{
			buffer_release(&exe_dir_);
			return 0;
		}

#if defined(_WIN32)
		++executable_directory_length;
		exe_dir = (const type_of_element*)buffer_data(&exe_dir_, executable_directory_length);
#else
		exe_dir = (const type_of_element*)buffer_data(&exe_dir_, 0);
#endif
	}

	const int32_t result = host_fxr_get_available_sdks(
							   ptr_to_host_fxr_object, exe_dir, host_fx_resolver_available_sdks);
	buffer_release(&exe_dir_);

	if (IS_HOST_FAILED(result))
	{
		if (!buffer_resize(output, 0) ||
			!int_to_string(result, output))
		{
			return 0;
		}
	}

#if defined(_WIN32)
	else
	{
		const ptrdiff_t size = buffer_size(output);

		if (!size)
		{
			return 0;
		}

		if (!buffer_append(output, NULL, 3 * size + sizeof(uint32_t)))
		{
			return 0;
		}

		ptr_to_host_fxr_object = buffer_data(output, sizeof(uint32_t));
		exe_dir = (const type_of_element*)(buffer_data(output, 0) + size);

		if (!buffer_resize(output, 0) ||
			!text_encoding_UTF16LE_to_UTF8(ptr_to_host_fxr_object, exe_dir, output))
		{
			return 0;
		}
	}

#endif
	return 1;
}

uint8_t hostfxr_get_native_search_directories(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!output)
	{
		return 0;
	}

#if defined(_WIN32)
	int32_t result = sizeof(uint32_t) + FILENAME_MAX * sizeof(type_of_element);
#else
	int32_t result = FILENAME_MAX * sizeof(type_of_element);
#endif

	if (!buffer_resize(output, result))
	{
		return 0;
	}

	struct buffer arguments;

	SET_NULL_TO_BUFFER(arguments);

	const type_of_element** argv = NULL;

	if (!values_to_arguments(values, values_lengths, values_count, &arguments, &argv))
	{
		buffer_release(&arguments);
		return 0;
	}

#if defined(_WIN32)
	type_of_element* out = (type_of_element*)buffer_data(output, sizeof(uint32_t));
#else
	type_of_element* out = (type_of_element*)buffer_data(output, 0);
#endif
	int32_t required_size = FILENAME_MAX;
	result = host_fxr_get_native_search_directories(
				 ptr_to_host_fxr_object, values_count, argv, out, FILENAME_MAX, &required_size);

	if (IS_HOST_FAILED(result))
	{
		if ((int32_t)host_fxr_HostApiBufferTooSmall != result ||
			required_size < FILENAME_MAX ||
			!buffer_resize(output, required_size * sizeof(type_of_element)))
		{
			buffer_release(&arguments);
			return 0;
		}

		result = host_fxr_get_native_search_directories(
					 ptr_to_host_fxr_object, values_count, argv,
					 (type_of_element*)buffer_data(output, 0), required_size, &required_size);

		if (IS_HOST_FAILED(result))
		{
			buffer_release(&arguments);
			return 0;
		}
	}

	buffer_release(&arguments);
#if defined(_WIN32)
	wchar_t* start = (wchar_t*)buffer_data(output, sizeof(uint32_t));
	result = (int32_t)wcslen(start);
	required_size = (ptrdiff_t)3 * result;

	if (!buffer_append(output, NULL, required_size))
	{
		return 0;
	}

	start = (wchar_t*)buffer_data(output, sizeof(uint32_t));
	out = start + result;
	/**/
	return buffer_resize(output, 0) &&
		   text_encoding_UTF16LE_to_UTF8(start, out, output);
#else
	result = (int32_t)common_count_bytes_until(buffer_data(output, 0), 0);
	/**/
	return buffer_resize(output, result);
#endif
}

static const uint8_t* types_of_delegate[] =
{
	(const uint8_t*)"host_fxr_hdt_com_activation",
	(const uint8_t*)"host_fxr_hdt_load_in_memory_assembly",
	(const uint8_t*)"host_fxr_hdt_winrt_activation",
	(const uint8_t*)"host_fxr_hdt_com_register",
	(const uint8_t*)"host_fxr_hdt_com_unregister",
	(const uint8_t*)"host_fxr_hdt_load_assembly_and_get_function_pointer",
	(const uint8_t*)"host_fxr_hdt_get_function_pointer"
};

uint8_t hostfxr_get_runtime_delegate(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		(5 != values_count && 6 != values_count) ||
		!output)
	{
		return 0;
	}

	static const uint8_t host_fxr_hdt_max_value = host_fxr_hdt_get_function_pointer + 1;
	/**/
	const void* context = string_to_pointer(
							  values[0], (uint8_t)values_lengths[0], output);

	if (!context ||
		!buffer_resize(output, 0))
	{
		return 0;
	}

	int32_t type_of_delegate = common_string_to_enum(
								   values[1], values[1] + values_lengths[1],
								   types_of_delegate, host_fxr_hdt_max_value);

	if (host_fxr_hdt_max_value == type_of_delegate)
	{
		if (!buffer_append(output, values[1], values_lengths[1]) ||
			!buffer_push_back(output, 0))
		{
			return 0;
		}

		type_of_delegate = int_parse(buffer_data(output, 0));

		if (!buffer_resize(output, 0))
		{
			return 0;
		}
	}

	hostfxr_delegate_function_type the_delegate_function_type = NULL;
	int32_t result = host_fxr_get_runtime_delegate(
						 ptr_to_host_fxr_object, context, type_of_delegate, &the_delegate_function_type);

	if (IS_HOST_FAILED(result))
	{
		if (!buffer_push_back(output, 0) ||
			!int_to_string(result, output))
		{
			return 0;
		}

		return 1;
	}

	if (!value_to_system_path(values[2], values_lengths[2], output))
	{
		return 0;
	}

#if defined(_WIN32)
	const ptrdiff_t index = (ptrdiff_t)1 + values_lengths[2];
#else
	const ptrdiff_t index = 0;
#endif
	const type_of_element** argv = NULL;

	if (!values_to_arguments(values + 3, values_lengths + 3, values_count - 3, output, &argv))
	{
		return 0;
	}

	const type_of_element* assembly_path = (const type_of_element*)buffer_data(output, index);
	const type_of_element* type_name = argv[0];
	const type_of_element* method_name = argv[1];
	const type_of_element* delegate_type_name = 5 < values_count ? argv[2] : NULL;
	void* unused = NULL;
	void* the_delegate = NULL;
	/**/
	result = the_delegate_function_type(
				 assembly_path, type_name, method_name,
				 delegate_type_name, unused, &the_delegate);

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	if (IS_HOST_FAILED(result))
	{
		if (!buffer_push_back(output, 0) ||
			!int_to_string(result, output))
		{
			return 0;
		}

		return 1;
	}

	return pointer_to_string(the_delegate, output);
}

uint8_t hostfxr_get_runtime_properties(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		1 != values_count ||
		!output)
	{
		return 0;
	}

	const void* context = string_to_pointer(values[0], (uint8_t)values_lengths[0], output);

	if (!context ||
		!buffer_resize(output, 0))
	{
		return 0;
	}

	size_t count = 0;
	type_of_element** properties_keys = NULL;
	type_of_element** properties_values = NULL;
	/**/
	int32_t result = host_fxr_get_runtime_properties(
						 ptr_to_host_fxr_object, context,
						 &count, properties_keys, properties_values);

	if ((int32_t)host_fxr_HostApiBufferTooSmall == result)
	{
		if (!buffer_resize(output, 2 * count * sizeof(type_of_element*)))
		{
			return 0;
		}

		memset(buffer_data(output, 0), 0, buffer_size(output));
		/**/
		properties_keys = (type_of_element**)buffer_data(output, 0);
		properties_values = (type_of_element**)buffer_data(output, count * sizeof(type_of_element*));
		/**/
		result = host_fxr_get_runtime_properties(ptr_to_host_fxr_object, context, &count, properties_keys,
				 properties_values);
	}

	if (IS_HOST_FAILED(result))
	{
		return int_to_string(result, output);
	}

	const ptrdiff_t size = buffer_size(output);

	if (count && size < INT16_MAX)
	{
		if (!buffer_append(output, NULL, INT16_MAX) ||
			!buffer_resize(output, size))
		{
			return 0;
		}
	}

	for (size_t i = 0; i < count; ++i)
	{
		properties_keys = (type_of_element**)buffer_data(output, 0);
		properties_values = (type_of_element**)buffer_data(output, count * sizeof(type_of_element*));
		/**/
		const type_of_element* key_ = properties_keys[i];
		const type_of_element* value_ = properties_values[i];
#if defined(_WIN32)

		if (!buffer_push_back(output, '\'') ||
			!text_encoding_UTF16LE_to_UTF8(key_, key_ + wcslen(key_), output) ||
			!buffer_push_back(output, '\'') ||
			!common_append_string_to_buffer((const uint8_t*)" = ", output) ||
			!buffer_push_back(output, '\''))
		{
			return 0;
		}

		if (value_)
		{
			key_ = value_ + wcslen(value_);

			if (value_ < key_ &&
				!text_encoding_UTF16LE_to_UTF8(value_, key_, output))
			{
				return 0;
			}
		}

#else

		if (!buffer_push_back(output, '\'') ||
			!buffer_append(output, key_, common_count_bytes_until(key_, 0)) ||
			!buffer_push_back(output, '\'') ||
			!common_append_string_to_buffer((const uint8_t*)" = ", output) ||
			!buffer_push_back(output, '\''))
		{
			return 0;
		}

		if (value_)
		{
			if (!buffer_append(output, value_, common_count_bytes_until(value_, 0)))
			{
				return 0;
			}
		}

#endif

		if (!buffer_push_back(output, '\'') ||
			!buffer_push_back(output, '\n'))
		{
			return 0;
		}
	}

	if (0 < size && size < buffer_size(output))
	{
		const ptrdiff_t new_size = buffer_size(output) - size;
		const uint8_t* src = buffer_data(output, size);
		uint8_t* dst = buffer_data(output, 0);
		/**/
		MEM_CPY(dst, src, new_size);
		/**/
		dst = buffer_data(output, new_size);
		memset(dst, 0, size);
		/**/
		return buffer_resize(output, new_size);
	}

	return buffer_resize(output, 0) && int_to_string(result, output);
}

uint8_t hostfxr_get_runtime_property_value(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		2 != values_count ||
		!output)
	{
		return 0;
	}

	const void* context = string_to_pointer(values[0], (uint8_t)values_lengths[0], output);

	if (!context ||
		!buffer_resize(output, 0))
	{
		return 0;
	}

#if defined(_WIN32)

	if (!text_encoding_UTF8_to_UTF16LE(values[1], values[1] + values_lengths[1], output) ||
		!buffer_push_back_uint16(output, 0))
	{
		return 0;
	}

#else

	if (!buffer_append(output, values[1], values_lengths[1]) ||
		!buffer_push_back(output, 0))
	{
		return 0;
	}

#endif
	const type_of_element* name = (const type_of_element*)buffer_data(output, 0);
	const type_of_element* value = NULL;
	/**/
	const int32_t result = host_fxr_get_runtime_property_value(ptr_to_host_fxr_object, context, name, &value);

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	if (value)
	{
#if defined(_WIN32)
		name = value + wcslen(value);

		if ((value < name) && !text_encoding_UTF16LE_to_UTF8(value, name, output))
#else
		if (!buffer_append(output, value, common_count_bytes_until(value, 0)))
#endif
		{
			return 0;
		}
	}

	if (IS_HOST_FAILED(result))
	{
		if (!buffer_push_back(output, 0) ||
			!int_to_string(result, output))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t hostfxr_initialize_for_dotnet_command_line(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		values_count < 2 ||
		!output)
	{
		return 0;
	}

	ptrdiff_t positions[2];

	if (!buffer_resize(output, 0) ||
		!values_to_system_paths(values, values_lengths, positions, 2, output))
	{
		return 0;
	}

	const type_of_element** argv = NULL;

	if (2 < values_count)
	{
		if (!values_to_arguments(values + 2, values_lengths + 2, values_count - 2, output, &argv))
		{
			return 0;
		}
	}

	const type_of_element* paths[2];

	for (uint8_t i = 0; i < 2; ++i)
	{
		paths[i] = (const type_of_element*)buffer_data(output, positions[i]);
		paths[i] = paths[i] ? paths[i] : zero;
	}

	void* context = NULL;
	const int32_t result = host_fxr_initialize_for_dotnet_command_line_parameters_in_parts(
							   ptr_to_host_fxr_object, values_count - 2, argv,
							   paths[0], paths[1], &context);

	if (!buffer_resize(output, 0) ||
		!pointer_to_string(context, output))
	{
		if (context)
		{
			host_fxr_close(ptr_to_host_fxr_object, context);
		}

		return 0;
	}

	if (IS_HOST_FAILED(result) || !context)
	{
		if (!buffer_push_back(output, ' ') ||
			!int_to_string(result, output))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t hostfxr_initialize_for_runtime_config(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		3 != values_count ||
		!output)
	{
		return 0;
	}

	ptrdiff_t positions[3];

	if (!buffer_resize(output, 0) ||
		!values_to_system_paths(values, values_lengths, positions, values_count, output))
	{
		return 0;
	}

	const type_of_element* paths[3];

	for (uint8_t i = 0; i < values_count; ++i)
	{
		paths[i] = (const type_of_element*)buffer_data(output, positions[i]);
		paths[i] = paths[i] ? paths[i] : zero;
	}

	void* context = NULL;
	const int32_t result = host_fxr_initialize_for_runtime_config_parameters_in_parts(ptr_to_host_fxr_object,
						   paths[2], paths[0], paths[1], &context);

	if (!buffer_resize(output, 0) ||
		!pointer_to_string(context, output))
	{
		if (context)
		{
			host_fxr_close(ptr_to_host_fxr_object, context);
		}

		return 0;
	}

	if (IS_HOST_FAILED(result) || !context)
	{
		if (!buffer_push_back(output, ' ') ||
			!int_to_string(result, output))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t hostfxr_main_bundle_startupinfo(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		values_count < 4 ||
		!output)
	{
		return 0;
	}

	int64_t bundle_header_offset = 0;

	if (0 < values_lengths[3])
	{
		if (!buffer_append(output, values[3], values_lengths[3]))
		{
			return 0;
		}

		bundle_header_offset = int64_parse(buffer_data(output, 0));
	}

	ptrdiff_t positions[3];

	if (!buffer_resize(output, 0) ||
		!values_to_system_paths(values, values_lengths, positions, 3, output))
	{
		return 0;
	}

	int32_t argc = values_count - 4;
	const type_of_element** argv = NULL;

	if (4 < values_count)
	{
		if (!values_to_arguments(values + 4, values_lengths + 4, (uint8_t)argc, output, &argv))
		{
			return 0;
		}
	}

	const type_of_element* host_path = (const type_of_element*)buffer_data(output, positions[0]);
	const type_of_element* dotnet_root = (const type_of_element*)buffer_data(output, positions[1]);
	const type_of_element* app_path = (const type_of_element*)buffer_data(output, positions[2]);
	/**/
	host_path = host_path ? host_path : zero;
	dotnet_root = dotnet_root ? dotnet_root : zero;
	app_path = app_path ? app_path : zero;
	/**/
	argc = host_fxr_main_bundle_startupinfo(
			   ptr_to_host_fxr_object, argc, argv,
			   host_path, dotnet_root, app_path, bundle_header_offset);
	return buffer_resize(output, 0) && int_to_string(argc, output);
}

uint8_t hostfxr_main_startupinfo(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		values_count < 3 ||
		!output)
	{
		return 0;
	}

	ptrdiff_t positions[3];

	if (!buffer_resize(output, 0) ||
		!values_to_system_paths(values, values_lengths, positions, 3, output))
	{
		return 0;
	}

	int32_t argc = values_count - 3;
	const type_of_element** argv = NULL;

	if (3 < values_count)
	{
		if (!values_to_arguments(values + 3, values_lengths + 3, (uint8_t)argc, output, &argv))
		{
			return 0;
		}
	}

	const type_of_element* host_path = (const type_of_element*)buffer_data(output, positions[0]);
	const type_of_element* dotnet_root = (const type_of_element*)buffer_data(output, positions[1]);
	const type_of_element* app_path = (const type_of_element*)buffer_data(output, positions[2]);
	/**/
	host_path = host_path ? host_path : zero;
	dotnet_root = dotnet_root ? dotnet_root : zero;
	app_path = app_path ? app_path : zero;
	/**/
	argc = host_fxr_main_startupinfo(ptr_to_host_fxr_object, argc, argv, host_path, dotnet_root, app_path);
	return buffer_resize(output, 0) && int_to_string(argc, output);
}

uint8_t hostfxr_resolve_sdk(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		2 != values_count ||
		!output)
	{
		return 0;
	}

#if defined(_WIN32)
	ptrdiff_t size = sizeof(uint32_t) + sizeof(type_of_element) * FILENAME_MAX;
#else
	ptrdiff_t size = sizeof(type_of_element) * FILENAME_MAX;
#endif

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	struct buffer arguments;

	SET_NULL_TO_BUFFER(arguments);

	const type_of_element** argv = NULL;

	if (!values_to_arguments(values, values_lengths, values_count, &arguments, &argv))
	{
		buffer_release(&arguments);
		return 0;
	}

#if defined(_WIN32)
	size = sizeof(uint32_t);
#else
	size = 0;
#endif
	type_of_element* out = (type_of_element*)buffer_data(output, size);
	int32_t result = host_fxr_resolve_sdk(
						 ptr_to_host_fxr_object,
						 argv[0], argv[1], out,
						 FILENAME_MAX);

	if (FILENAME_MAX < result)
	{
#if defined(_WIN32)
		size = sizeof(uint32_t) + sizeof(type_of_element) * result;
#else
		size = sizeof(type_of_element) * result;
#endif

		if (!buffer_resize(output, size))
		{
			buffer_release(&arguments);
			return 0;
		}

#if defined(_WIN32)
		size = sizeof(uint32_t);
#else
		size = 0;
#endif
		out = (type_of_element*)buffer_data(output, size);
		result = host_fxr_resolve_sdk(
					 ptr_to_host_fxr_object,
					 argv[0], argv[1], out,
					 result);
	}

	buffer_release(&arguments);

	if (0 < result)
	{
#if defined(_WIN32)

		if (!buffer_resize(output, (ptrdiff_t)4 * result + sizeof(uint32_t)))
		{
			return 0;
		}

		out = (type_of_element*)buffer_data(output, size);

		if (!buffer_resize(output, 0) ||
			!text_encoding_UTF16LE_to_UTF8(out, out + result, output))
		{
			return 0;
		}

#else

		if (!buffer_resize(output, result))
		{
			return 0;
		}

#endif
	}
	else
	{
		if (!buffer_resize(output, 0) ||
			!int_to_string(result, output))
		{
			return 0;
		}
	}

	return 1;
}

void host_fx_resolver_sdk2(int32_t key, const type_of_element* value);

uint8_t hostfxr_resolve_sdk2(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		3 != values_count ||
		!output)
	{
		return 0;
	}

	if (!buffer_append(output, values[2], values_lengths[2]) ||
		!buffer_push_back(output, 0))
	{
		return 0;
	}

	int32_t flags = int_parse(buffer_data(output, 0));

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	struct buffer arguments;

	SET_NULL_TO_BUFFER(arguments);

	const type_of_element** argv = NULL;

	if (!values_to_arguments(values, values_lengths, values_count - 1, &arguments, &argv))
	{
		buffer_release(&arguments);
	}

	flags = host_fxr_resolve_sdk2(
				ptr_to_host_fxr_object,
				argv[0], argv[1],
				flags, host_fx_resolver_sdk2);
	buffer_release(&arguments);

	if (!buffer_size(output))
	{
		return int_to_string(flags, output);
	}

	return 1;
}

uint8_t hostfxr_set_runtime_property_value(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		values_count < 2 || 3 < values_count ||
		!output)
	{
		return 0;
	}

	const void* context = string_to_pointer(values[0], (uint8_t)values_lengths[0], output);

	if (!context ||
		!buffer_resize(output, 0))
	{
		return 0;
	}

#if defined(_WIN32)

	if (!text_encoding_UTF8_to_UTF16LE(values[1], values[1] + values_lengths[1], output) ||
		!buffer_push_back_uint16(output, 0))
#else
	if (!buffer_append(output, values[1], values_lengths[1]) ||
		!buffer_push_back(output, 0))
#endif
	{
		return 0;
	}

	const ptrdiff_t index = buffer_size(output);

	if (3 == values_count && values_lengths[2])
	{
#if defined(_WIN32)

		if (!text_encoding_UTF8_to_UTF16LE(values[2], values[2] + values_lengths[2], output))
#else
		if (!buffer_append(output, values[2], values_lengths[2]))
#endif
		{
			return 0;
		}
	}

#if defined(_WIN32)

	if (!buffer_push_back_uint16(output, 0))
#else
	if (!buffer_push_back(output, 0))
#endif
	{
		return 0;
	}

	const type_of_element* name = (const type_of_element*)buffer_data(output, 0);
	const type_of_element* value = (const type_of_element*)buffer_data(output, index);
	/**/
	const int32_t result = host_fxr_set_runtime_property_value(ptr_to_host_fxr_object, context, name, value);

	if (!buffer_resize(output, 0) ||
		!int_to_string(result, output))
	{
		return 0;
	}

	return 1;
}

static const uint8_t* name_spaces[] =
{
	(const uint8_t*)"net",
	(const uint8_t*)"nethost",
	(const uint8_t*)"hostfxr",
	(const uint8_t*)"hostpolicy",
	(const uint8_t*)"hostinterface",
	(const uint8_t*)"corehost",
	(const uint8_t*)"corehost-initialize-request",
	(const uint8_t*)"file"
};

static const uint8_t* all_functions =
	(const uint8_t*)"result-to-string\0" \
	"\0" \
	"get-hostfxr-path\0" \
	"\0" \
	"functions\0" \
	"initialize\0" \
	"is-function-exists\0" \
	"close\0" \
	"get-available-sdks\0" \
	"get-native-search-directories\0" \
	"get-runtime-delegate\0" \
	"get-runtime-properties\0" \
	"get-runtime-property-value\0" \
	"initialize-for-dotnet-command-line\0" \
	"initialize-for-runtime-config\0" \
	"main\0" \
	"main-bundle-startupinfo\0" \
	"main-startupinfo\0" \
	"resolve-sdk\0" \
	"resolve-sdk2\0" \
	"run-app\0" \
	"set-error-writer\0" \
	"set-runtime-property-value\0" \
	"\0" \
	"initialize\0" \
	"\0" \
	"initialize\0" \
	"set-additional-dependency-serialized\0" \
	"set-application-path\0" \
	"set-config-keys\0" \
	"set-config-values\0" \
	"set-dependency-file\0" \
	"set-dotnet-root\0" \
	"set-file-bundle-header-offset\0" \
	"set-framework-dependent\0" \
	"set-framework-directories\0" \
	"set-framework-directory\0" \
	"set-framework-found-versions\0" \
	"set-framework-name\0" \
	"set-framework-names\0" \
	"set-framework-requested-versions\0" \
	"set-framework-version\0" \
	"set-host-command\0" \
	"set-host-mode\0" \
	"set-host-path\0" \
	"set-patch-roll-forward\0" \
	"set-paths-for-probing\0" \
	"set-prerelease-roll-forward\0" \
	"set-target-framework-moniker\0" \
	"\0" \
	"functions\0" \
	"is-function-exists\0" \
	"initialize\0" \
	"load\0" \
	"main\0" \
	"main-with-output-buffer\0" \
	"resolve-component-dependencies\0" \
	"set-error-writer\0" \
	"unload\0" \
	"\0" \
	"initialize\0" \
	"set-config-keys\0" \
	"set-config-values\0" \
	"\0" \
	"is-assembly\0" \
	"\0";

#define COUNT_OF_ALL_FUNCTIONS 1106

const uint8_t* all_functions_get_string_at(uint8_t x, uint8_t y)
{
	const uint8_t* start = all_functions;
	const uint8_t* finish = start + COUNT_OF_ALL_FUNCTIONS;
	ptrdiff_t i;

	for (i = 0; i < x; ++i)
	{
		const ptrdiff_t index = string_index_of(start, finish, double_zero, double_zero + 2);

		if (-1 == index)
		{
			break;
		}

		start += index + 2;
	}

	if (i != x)
	{
		return NULL;
	}

	i = string_index_of(start, finish, double_zero, double_zero + 2);

	if (-1 == i)
	{
		return NULL;
	}

	finish = start + i;

	for (i = 0; i < y; ++i)
	{
		const ptrdiff_t index = string_index_of(start, finish, double_zero, double_zero + 1);

		if (-1 == index)
		{
			break;
		}

		start += index + 1;
	}

	if (i != y)
	{
		return NULL;
	}

	return start;
}

enum ant4c_net_module_
{
	net_result_to_string_,
	/**/
	net_host_get_hostfxr_path_,
	/**/
	host_fxr_functions_,
	host_fxr_initialize_,
	host_fxr_is_function_exists_,
	/**/
	host_fxr_close_,
	host_fxr_get_available_sdks_,
	host_fxr_get_native_search_directories_,
	host_fxr_get_runtime_delegate_,
	host_fxr_get_runtime_properties_,
	host_fxr_get_runtime_property_value_,
	host_fxr_initialize_for_dotnet_command_line_,
	host_fxr_initialize_for_runtime_config_,
	host_fxr_main_,
	host_fxr_main_bundle_startupinfo_,
	host_fxr_main_startupinfo_,
	host_fxr_resolve_sdk_,
	host_fxr_resolve_sdk2_,
	host_fxr_run_app_,
	host_fxr_set_error_writer_,
	host_fxr_set_runtime_property_value_,
	/**/
	host_policy_initialize_,
	/**/
	host_interface_initialize_,
	host_interface_set_additional_dependency_serialized_,
	host_interface_set_application_path_,
	host_interface_set_config_keys_,
	host_interface_set_config_values_,
	host_interface_set_dependency_file_,
	host_interface_set_dotnet_root_,
	host_interface_set_file_bundle_header_offset_,
	host_interface_set_framework_dependent_,
	host_interface_set_framework_directories_,
	host_interface_set_framework_directory_,
	host_interface_set_framework_found_versions_,
	host_interface_set_framework_name_,
	host_interface_set_framework_names_,
	host_interface_set_framework_requested_versions_,
	host_interface_set_framework_version_,
	host_interface_set_host_command_,
	host_interface_set_host_mode_,
	host_interface_set_host_path_,
	host_interface_set_patch_roll_forward_,
	host_interface_set_paths_for_probing_,
	host_interface_set_prerelease_roll_forward_,
	host_interface_set_target_framework_moniker_,
	/**/
	core_host_functions_,
	core_host_is_function_exists_,
	/**/
	core_host_initialize_,
	core_host_load_,
	core_host_main_,
	core_host_main_with_output_buffer_,
	core_host_resolve_component_dependencies_,
	core_host_set_error_writer_,
	core_host_unload_,
	/**/
	corehost_initialize_request_initialize_,
	corehost_initialize_request_set_config_keys_,
	corehost_initialize_request_set_config_values_,
	/**/
	file_is_assembly_
};

uint8_t file_is_assembly(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	struct assembly_argument
	{
		const type_of_element* path;
	};
	/**/
	typedef uint8_t(delegate_calling_convention * custom_entry_point_fn)(struct assembly_argument args);

	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		(1 != values_count && 2 != values_count) ||
		!output)
	{
		return 0;
	}

	static const uint8_t* ant4c_net_clr = (const uint8_t*)"ant4c.net.module.clr.dll";
	/**/
	const uint8_t* get_runtime_delegate = all_functions_get_string_at(2, 6);
	const uint8_t get_runtime_delegate_length = (uint8_t)common_count_bytes_until(get_runtime_delegate, 0);

	if (!is_function_exists(
			ptr_to_host_fxr_object, name_spaces[2],
			get_runtime_delegate, get_runtime_delegate_length,
			host_fx_resolver_is_function_exists, output))
	{
		return 0;
	}

	uint8_t is_function_exist = 0;

	if (!bool_parse(buffer_data(output, 0), buffer_size(output), &is_function_exist))
	{
		return 0;
	}

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	if (is_function_exist)
	{
		const void* the_delegate = NULL;

		if (1 == values_count)
		{
			static const uint8_t* json_file_content =
				(const uint8_t*)""								\
				"{\n"											\
				"  \"runtimeOptions\": {\n"						\
				"    \"tfm\": \"netcoreapp3.1\",\n"				\
				"    \"rollForward\": \"LatestMinor\",\n"		\
				"    \"framework\": {\n"						\
				"      \"name\": \"Microsoft.NETCore.App\",\n"	\
				"      \"version\": \"3.1.0\" \n"				\
				"    }\n"										\
				"  }\n"											\
				"}\n";

			if (!common_append_string_to_buffer(json_file_content, output))
			{
				return 0;
			}

			values_count = (uint8_t)buffer_size(output);

			if (!path_get_temp_file_name(output))
			{
				return 0;
			}

			if (!buffer_resize(output, buffer_size(output) - 2) ||
				!common_append_string_to_buffer((const uint8_t*)".json", output) ||
				!buffer_push_back(output, 0))
			{
				return 0;
			}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
			const uint8_t* sub_values[6];
			sub_values[0] = 0;
			sub_values[1] = 0;
			sub_values[2] = buffer_data(output, values_count);
			sub_values[3] = 0;
			sub_values[4] = 0;
			sub_values[5] = 0;
			/**/
			uint16_t sub_values_lengths[6];
			memset(sub_values_lengths, 0, sizeof(sub_values_lengths));
			sub_values_lengths[2] = (uint16_t)(buffer_size(output) - values_count);
#else
			const uint8_t* sub_values[] = { NULL, NULL, buffer_data(output, values_count), NULL, NULL, NULL };
			uint16_t sub_values_lengths[] = { 0, 0, (uint16_t)(buffer_size(output) - values_count), 0, 0, 0 };
#endif

			if (!buffer_resize(output, values_count))
			{
				return 0;
			}

			if (!file_write_all(sub_values[2], output))
			{
				return 0;
			}

			struct buffer sub_output;

			SET_NULL_TO_BUFFER(sub_output);

			if (!hostfxr_initialize_for_runtime_config(
					ptr_to_host_fxr_object,
					sub_values, sub_values_lengths, 3, &sub_output))
			{
				buffer_release(&sub_output);
				return 0;
			}

			sub_values[0] = buffer_data(&sub_output, 0);

			if (!sub_values[0] ||
				0 == *(sub_values[0]))
			{
				buffer_release(&sub_output);
				return 0;
			}

			const void* context = pointer_parse(sub_values[0]);

			if (!buffer_resize(output, 0) ||
				!buffer_append_data_from_buffer(output, &sub_output))
			{
				host_fxr_close(ptr_to_host_fxr_object, context);
				buffer_release(&sub_output);
				return 0;
			}

			sub_values[0] = buffer_data(output, 0);
			sub_values[1] = types_of_delegate[host_fxr_hdt_load_assembly_and_get_function_pointer];
			sub_values[2] = ant4c_net_clr;
			sub_values[3] = (const uint8_t*)"Ant4C.Net.Module.Delegates, ant4c.net.module.clr";
			sub_values[4] = (const uint8_t*)"FileUnit_IsAssembly";
			sub_values[5] = (const uint8_t*)
							"Ant4C.Net.Module.Delegates+FileUnit_IsAssemblyDelegate, ant4c.net.module.clr";
			/**/
			sub_values_lengths[0] = (uint16_t)buffer_size(output);

			for (uint8_t i = 1; i < 6; ++i)
			{
				sub_values_lengths[i] = (uint16_t)common_count_bytes_until(sub_values[i], 0);
			}

			if (!buffer_resize(&sub_output, 0) ||
				!hostfxr_get_runtime_delegate(ptr_to_host_fxr_object, sub_values, sub_values_lengths, 6, &sub_output))
			{
				host_fxr_close(ptr_to_host_fxr_object, context);
				buffer_release(&sub_output);
				return 0;
			}

			sub_values[0] = buffer_data(&sub_output, 0);

			if (!sub_values[0] ||
				0 == *(sub_values[0]))
			{
				host_fxr_close(ptr_to_host_fxr_object, context);
				buffer_release(&sub_output);
				return 0;
			}

			the_delegate = pointer_parse(sub_values[0]);
			buffer_release(&sub_output);
			const int32_t result = host_fxr_close(ptr_to_host_fxr_object, context);

			if (IS_HOST_FAILED(result))
			{
				return 0;
			}
		}
		else
		{
			the_delegate = string_to_pointer(values[1], (uint8_t)values_lengths[1], output);
		}

		if (!the_delegate)
		{
			return 0;
		}

		if (!buffer_resize(output, 0) ||
			!value_to_system_path(values[0], values_lengths[0], output))
		{
			return 0;
		}

		struct assembly_argument custom_argument;

#if defined(_WIN32)
		custom_argument.path = (const type_of_element*)buffer_data(output, (ptrdiff_t)1 + values_lengths[0]);

#else
		custom_argument.path = (const type_of_element*)buffer_data(output, 0);

#endif
		if (!custom_argument.path)
		{
			return 0;
		}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(suppress: 4055)
		const uint8_t is_assembly = ((custom_entry_point_fn)the_delegate)(custom_argument);
#else
		const uint8_t is_assembly = ((custom_entry_point_fn)the_delegate)(custom_argument);
#endif

		if (!buffer_resize(output, 0) ||
			!bool_to_string(is_assembly, output))
		{
			return 0;
		}
	}
	else
	{
		const type_of_element* argv[5];
#if defined(_WIN32)
		argv[0] = L"";
		argv[2] = L"file";
		argv[3] = L"is-assembly";
#else
		argv[0] = (const uint8_t*)"";
		argv[2] = (const uint8_t*)"file";
		argv[3] = (const uint8_t*)"is-assembly";
#endif

		if (1 == values_count)
		{
			if (!value_to_system_path(values[0], values_lengths[0], output))
			{
				return 0;
			}

#if defined(_WIN32)
			argv[1] = L"ant4c.net.module.clr.dll";
			argv[4] = (const type_of_element*)buffer_data(output, (ptrdiff_t)1 + values_lengths[0]);
#else
			argv[1] = ant4c_net_clr;
			argv[4] = buffer_data(output, 0);
#endif
		}
		else
		{
			ptrdiff_t positions[2];

			if (!values_to_system_paths(values, values_lengths, positions, values_count, output))
			{
				return 0;
			}

			argv[1] = (const type_of_element*)buffer_data(output, positions[1]);
			argv[4] = (const type_of_element*)buffer_data(output, positions[0]);
		}

		values_count = COUNT_OF(argv);
		const int32_t result = host_fxr_main(ptr_to_host_fxr_object, values_count, argv);

		if (!buffer_resize(output, 0) ||
			!bool_to_string((uint8_t)result, output))
		{
			return 0;
		}
	}

	return 1;
}

static struct buffer output_data;
static uint8_t is_buffer_initialized = 0;

void host_fx_resolver_available_sdks(int32_t count, const type_of_element** directories)
{
	for (int32_t i = 0; i < count; ++i)
	{
#if defined(_WIN32)

		if (!buffer_append_wchar_t(&output_data, directories[i], wcslen(directories[i])) ||
			!buffer_push_back_uint16(&output_data, 0))
#else
		if (!buffer_append(&output_data, directories[i], common_count_bytes_until(directories[i], 0)) ||
			!buffer_push_back(&output_data, 0))
#endif
		{
			buffer_resize(&output_data, 0);
			break;
		}
	}
}

void host_fx_resolver_sdk2(int32_t key, const type_of_element* value)
{
	if (!int_to_string(key, &output_data) ||
		!buffer_push_back(&output_data, ' '))
	{
		buffer_resize(&output_data, 0);
	}

#if defined(_WIN32)

	if (!text_encoding_UTF16LE_to_UTF8(value, value + wcslen(value), &output_data) ||
		!buffer_push_back_uint16(&output_data, 0))
#else
	if (!buffer_append(&output_data, value, common_count_bytes_until(value, 0)) ||
		!buffer_push_back(&output_data, 0))
#endif
	{
		buffer_resize(&output_data, 0);
	}
}

uint8_t host_policy_init(
	const uint8_t* path_to_core_host,
	uint16_t path_to_core_host_length,
	struct buffer* tmp,
	void* ptr_to_core_host_object,
	ptrdiff_t size)
{
	if (size < 96)
	{
		return 0;
	}

	return load_library(
			   path_to_core_host, path_to_core_host_length, tmp,
			   ptr_to_core_host_object, size, host_policy_load);
}

static struct buffer* dependencies = NULL;

void core_host_resolve_component_dependencies_callback(
	const type_of_element* assembly_paths,
	const type_of_element* native_search_paths,
	const type_of_element* resource_search_paths)
{
	if (!dependencies)
	{
		return;
	}

#if defined(_WIN32)

	if (!text_encoding_UTF16LE_to_UTF8(assembly_paths, assembly_paths + wcslen(assembly_paths), dependencies) ||
		!buffer_push_back(dependencies, '\n') ||
		!text_encoding_UTF16LE_to_UTF8(native_search_paths, native_search_paths + wcslen(native_search_paths),
									   dependencies) ||
		!buffer_push_back(dependencies, '\n') ||
		!text_encoding_UTF16LE_to_UTF8(resource_search_paths, resource_search_paths + wcslen(resource_search_paths),
									   dependencies) ||
		!buffer_push_back(dependencies, 0))
	{
		return;
	}

#else

	if (!buffer_append(dependencies, assembly_paths, common_count_bytes_until(assembly_paths, 0)) ||
		!buffer_push_back(dependencies, '\n') ||
		!buffer_append(dependencies, native_search_paths, common_count_bytes_until(native_search_paths, 0)) ||
		!buffer_push_back(dependencies, '\n') ||
		!buffer_append(dependencies, resource_search_paths, common_count_bytes_until(resource_search_paths, 0)) ||
		!buffer_push_back(dependencies, 0))
	{
		return;
	}

#endif
}

const uint8_t* enumerate_name_spaces(ptrdiff_t index)
{
	static const ptrdiff_t count = COUNT_OF(name_spaces);

	if (index < 0 || count <= index)
	{
		return NULL;
	}

	return name_spaces[index];
}

const uint8_t* enumerate_functions(const uint8_t* name_space, ptrdiff_t index)
{
	if (NULL == name_space)
	{
		return NULL;
	}

	ptrdiff_t i = 0;
	const uint8_t* ptr = NULL;

	while (NULL != (ptr = enumerate_name_spaces(i++)))
	{
		if (ptr == name_space)
		{
			break;
		}
	}

	if (NULL == ptr)
	{
		return NULL;
	}

	const ptrdiff_t name_space_number = i - 1;
	i = 0;

	while (NULL != (ptr = all_functions_get_string_at((uint8_t)name_space_number, (uint8_t)(i++))))
	{
		if (index == (i - 1))
		{
			return ptr;
		}
	}

	return NULL;
}

#if defined(_WIN32)
extern uint8_t file_open_wchar_t(const wchar_t* path, const wchar_t* mode, void** output);
#endif

static uint8_t host_fxr_object[160];
static uint8_t is_host_fxr_object_initialized = 0;

static uint8_t host_policy_object[96];
static uint8_t is_host_policy_object_initialized = 0;
#if 0
uint8_t get_function_number(const uint8_t* function)
{
	const uint8_t* ptr = NULL;
	ptrdiff_t function_number = 0;

	for (uint8_t x = 0, count = COUNT_OF(name_spaces); x < count; ++x)
	{
		uint8_t y = 0;

		while (NULL != (ptr = all_functions_get_string_at(x, y++)))
		{
			if (function == ptr)
			{
				x = count;
				break;
			}

			++function_number;
		}
	}

	if (NULL == ptr)
	{
		return 0;
	}

	return 1;
}
#endif
uint8_t evaluate_function(
	const uint8_t* function,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	const uint8_t** output, uint16_t* output_length)
{
	static const uint8_t space = ' ';
	static const uint16_t one = 1;

	if (NULL == function ||
		NULL == values ||
		NULL == values_lengths ||
		NULL == output ||
		NULL == output_length)
	{
		return 0;
	}

	if (is_buffer_initialized)
	{
		if (!buffer_resize(&output_data, 0))
		{
			return 0;
		}
	}
	else
	{
		SET_NULL_TO_BUFFER(output_data);
		is_buffer_initialized = 1;
	}

	const uint8_t* ptr = NULL;
	ptrdiff_t function_number = 0;

	for (uint8_t x = 0, count = COUNT_OF(name_spaces); x < count; ++x)
	{
		uint8_t y = 0;

		while (NULL != (ptr = all_functions_get_string_at(x, y++)))
		{
			if (function == ptr)
			{
				x = count;
				break;
			}

			++function_number;
		}
	}

	if (NULL == ptr)
	{
		return 0;
	}

	const uint8_t* space_in_array[1];
	const void* ptr_to_host_fxr_object = is_host_fxr_object_initialized ? host_fxr_object : NULL;
	const void* ptr_to_host_policy_object = is_host_policy_object_initialized ? host_policy_object : NULL;

	switch (function_number)
	{
		case net_result_to_string_:
		{
			if (1 != values_count)
			{
				return 0;
			}

			if (!buffer_append(&output_data, values[0], values_lengths[0]) ||
				!buffer_push_back(&output_data, 0))
			{
				return 0;
			}

			const int32_t result = int_parse(buffer_data(&output_data, 0));

			if (!buffer_resize(&output_data, 0) ||
				!result_code_to_string(result, &output_data))
			{
				return 0;
			}
		}
		break;

		case net_host_get_hostfxr_path_:
			if (!net_host_get_hostfxr_path(values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_functions_:
			if (1 < values_count)
			{
				return 0;
			}

			if (!values_count)
			{
				space_in_array[0] = &space;
				values = space_in_array;
				values_lengths = &one;
			}

			if (!get_exists_functions(
					ptr_to_host_fxr_object,
					name_spaces[2],
					all_functions_get_string_at(2, 3),
					values[0],
					values_lengths[0],
					host_fx_resolver_is_function_exists,
					&output_data))
			{
				return 0;
			}

			break;

		case host_fxr_initialize_:
			if (1 != values_count)
			{
				return 0;
			}

			if (is_host_fxr_object_initialized)
			{
				host_fx_resolver_unload(host_fxr_object);
				is_host_fxr_object_initialized = 0;
			}

			is_host_fxr_object_initialized = host_fx_resolver_init(
												 values[0], values_lengths[0], &output_data, host_fxr_object, sizeof(host_fxr_object));

			if (!buffer_resize(&output_data, 0) ||
				!bool_to_string(is_host_fxr_object_initialized, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_is_function_exists_:
			if (1 != values_count)
			{
				return 0;
			}

			if (!is_function_exists(
					ptr_to_host_fxr_object, name_spaces[2],
					values[0], values_lengths[0],
					host_fx_resolver_is_function_exists, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_close_:
		{
			if (1 != values_count)
			{
				return 0;
			}

			ptr = string_to_pointer(values[0], (uint8_t)values_lengths[0], &output_data);

			if (!ptr)
			{
				return 0;
			}

			const int32_t result = host_fxr_close(ptr_to_host_fxr_object, ptr);

			if (!buffer_resize(&output_data, 0) ||
				!int_to_string(result, &output_data))
			{
				return 0;
			}
		}
		break;

		case host_fxr_get_available_sdks_:
			if (1 < values_count)
			{
				return 0;
			}

			if (values_count)
			{
				values_count = hostfxr_get_available_sdks(ptr_to_host_fxr_object, values[0], values_lengths[0], &output_data);
			}
			else
			{
				values_count = hostfxr_get_available_sdks(ptr_to_host_fxr_object, NULL, 0, &output_data);
			}

			if (!values_count)
			{
				return 0;
			}

			break;

		case host_fxr_get_native_search_directories_:
			if (!hostfxr_get_native_search_directories(ptr_to_host_fxr_object,
					values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_get_runtime_delegate_:
			if (!hostfxr_get_runtime_delegate(ptr_to_host_fxr_object,
											  values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_get_runtime_properties_:
			if (!hostfxr_get_runtime_properties(ptr_to_host_fxr_object,
												values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_get_runtime_property_value_:
			if (!hostfxr_get_runtime_property_value(ptr_to_host_fxr_object,
													values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_initialize_for_dotnet_command_line_:
			if (!hostfxr_initialize_for_dotnet_command_line(ptr_to_host_fxr_object,
					values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_initialize_for_runtime_config_:
			if (!hostfxr_initialize_for_runtime_config(ptr_to_host_fxr_object,
					values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_main_:
		{
			const type_of_element** argv = NULL;

			if (!values_to_arguments(values, values_lengths, values_count, &output_data, &argv))
			{
				return 0;
			}

			const int32_t result = host_fxr_main(ptr_to_host_fxr_object, values_count, argv);

			if (!buffer_resize(&output_data, 0) ||
				!int_to_string(result, &output_data))
			{
				return 0;
			}
		}
		break;

		case host_fxr_main_bundle_startupinfo_:
			if (!hostfxr_main_bundle_startupinfo(
					ptr_to_host_fxr_object, values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_main_startupinfo_:
			if (!hostfxr_main_startupinfo(ptr_to_host_fxr_object, values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_resolve_sdk_:
			if (!hostfxr_resolve_sdk(ptr_to_host_fxr_object, values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_resolve_sdk2_:
			if (!hostfxr_resolve_sdk2(ptr_to_host_fxr_object, values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_fxr_run_app_:
		{
			if (1 != values_count)
			{
				return 0;
			}

			ptr = string_to_pointer(values[0], (uint8_t)values_lengths[0], &output_data);

			if (!ptr)
			{
				return 0;
			}

			const int32_t result = host_fxr_run_app(ptr_to_host_fxr_object, ptr);

			if (!buffer_resize(&output_data, 0) ||
				!int_to_string(result, &output_data))
			{
				return 0;
			}
		}
		break;

		case host_fxr_set_error_writer_:
		{
			if (1 < values_count)
			{
				return 0;
			}

			const type_of_element* path = NULL;

			if (values_count && values_lengths[0])
			{
				if (!value_to_system_path(values[0], values_lengths[0], &output_data))
				{
					return 0;
				}

#if defined(_WIN32)
				path = (const type_of_element*)buffer_data(&output_data, (ptrdiff_t)1 + values_lengths[0]);
#else
				path = buffer_data(&output_data, 0);
#endif
			}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4054)
			ptr = (const uint8_t*)set_error_writer(
					  ptr_to_host_fxr_object,
					  host_fx_resolver_error_writer, host_fxr_set_error_writer,
					  path, 0);
#pragma warning(default: 4054)
#else
				ptr = (const uint8_t*)set_error_writer(
						  ptr_to_host_fxr_object,
						  host_fx_resolver_error_writer, host_fxr_set_error_writer,
						  path, 0);
#endif

				if (!buffer_resize(&output_data, 0) ||
					!pointer_to_string(ptr, &output_data))
				{
					return 0;
				}
			}
			break;

		case host_fxr_set_runtime_property_value_:
			if (!hostfxr_set_runtime_property_value(ptr_to_host_fxr_object,
													values, values_lengths,
													values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_policy_initialize_:
			if (1 != values_count)
			{
				return 0;
			}

			if (is_host_policy_object_initialized)
			{
				host_fx_resolver_unload(host_policy_object);
				is_host_policy_object_initialized = 0;
			}

			is_host_policy_object_initialized = host_policy_init(
													values[0], values_lengths[0],
													&output_data, host_policy_object,
													sizeof(host_policy_object));

			if (!buffer_resize(&output_data, 0) ||
				!bool_to_string(is_host_policy_object_initialized, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_initialize_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_init(
							   host_interface_get(), HOST_INTERFACE_SIZE,
							   (size_t)uint64_parse(values[0], values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_additional_dependency_serialized_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_additional_dependency_serialized(
							   host_interface_get(), values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_application_path_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_application_path(
							   host_interface_get(), values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_config_keys_:
			values_count = host_interface_set_config_keys(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_config_values_:
			values_count = host_interface_set_config_values(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_dependency_file_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_dependency_file(
							   host_interface_get(), values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_dotnet_root_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_dotnet_root(
							   host_interface_get(), values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_file_bundle_header_offset_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_file_bundle_header_offset(
							   host_interface_get(),
							   (size_t)uint64_parse(values[0], values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_dependent_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_framework_dependent(
							   host_interface_get(),
							   (size_t)uint64_parse(values[0], values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_directories_:
			values_count = host_interface_set_framework_directories(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_directory_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_framework_directory(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_found_versions_:
			values_count = host_interface_set_framework_found_versions(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_name_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_framework_name(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_names_:
			values_count = host_interface_set_framework_names(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_requested_versions_:
			values_count = host_interface_set_framework_requested_versions(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_version_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_framework_version(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_host_command_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_host_command(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_host_mode_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_host_mode(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_host_path_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_host_path(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_patch_roll_forward_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_patch_roll_forward(
							   host_interface_get(),
							   (size_t)uint64_parse(values[0], values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_paths_for_probing_:
			values_count = host_interface_set_paths_for_probing(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_prerelease_roll_forward_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_prerelease_roll_forward(
							   host_interface_get(),
							   (size_t)uint64_parse(values[0], values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_target_framework_moniker_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_target_framework_moniker(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case core_host_functions_:
			if (1 < values_count)
			{
				return 0;
			}

			if (!values_count)
			{
				space_in_array[0] = &space;
				values = space_in_array;
				values_lengths = &one;
			}

			if (!get_exists_functions(
					ptr_to_host_policy_object,
					name_spaces[5],
					all_functions_get_string_at(5, 2),
					values[0],
					values_lengths[0],
					core_host_is_function_exists,
					&output_data))
			{
				return 0;
			}

			break;

		case core_host_is_function_exists_:
			if (1 != values_count)
			{
				return 0;
			}

			if (!is_function_exists(
					ptr_to_host_policy_object,
					name_spaces[5],
					values[0], values_lengths[0], core_host_is_function_exists, &output_data))
			{
				return 0;
			}

			break;

		/*case core_host_initialize_:
			break;*/

		case core_host_load_:
			if (values_count ||
				!int_to_string(core_host_load(ptr_to_host_policy_object, host_interface_get()), &output_data))
			{
				return 0;
			}

			break;

		case core_host_main_:
		{
			const type_of_element** argv = NULL;

			if (!values_to_arguments(values, values_lengths, values_count, &output_data, &argv))
			{
				return 0;
			}

			const int32_t result = core_host_main(ptr_to_host_policy_object, values_count, argv);

			if (!buffer_resize(&output_data, 0) ||
				!int_to_string(result, &output_data))
			{
				return 0;
			}
		}
		break;

		case core_host_main_with_output_buffer_:
		{
			struct buffer arguments;
			SET_NULL_TO_BUFFER(arguments);
			const type_of_element** argv = NULL;

			if (!values_to_arguments(values, values_lengths, values_count, &arguments, &argv))
			{
				buffer_release(&arguments);
				return 0;
			}

			if (!core_host_main_with_output_buffer(ptr_to_host_policy_object, values_count, argv, &output_data))
			{
				buffer_release(&arguments);
				return 0;
			}

			buffer_release(&arguments);
		}
		break;

		case core_host_resolve_component_dependencies_:
		{
			if (1 != values_count ||
				!value_to_system_path(values[0], values_lengths[0], &output_data))
			{
				return 0;
			}

			const type_of_element* path = (const type_of_element*)buffer_data(&output_data, values_lengths[0] + 1);
			struct buffer sub_output;
			SET_NULL_TO_BUFFER(sub_output);
			dependencies = &sub_output;
			const int32_t result = core_host_resolve_component_dependencies(
									   ptr_to_host_policy_object, path,
									   core_host_resolve_component_dependencies_callback);
			dependencies = NULL;

			if (!buffer_resize(&output_data, 0) ||
				!buffer_append_data_from_buffer(&output_data, &sub_output))
			{
				buffer_release(&sub_output);
				return 0;
			}

			buffer_release(&sub_output);

			if (IS_HOST_FAILED(result))
			{
				if (!buffer_push_back(&output_data, ' ') ||
					!int_to_string(result, &output_data))
				{
					return 0;
				}
			}
		}
		break;

		case core_host_set_error_writer_:
		{
			if (1 < values_count)
			{
				return 0;
			}

			const type_of_element* path = NULL;

			if (values_count && values_lengths[0])
			{
				if (!value_to_system_path(values[0], values_lengths[0], &output_data))
				{
					return 0;
				}

#if defined(_WIN32)
				path = (const type_of_element*)buffer_data(&output_data, (ptrdiff_t)1 + values_lengths[0]);
#else
				path = buffer_data(&output_data, 0);
#endif
			}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4054)
			ptr = (const uint8_t*)set_error_writer(
					  ptr_to_host_policy_object,
					  host_policy_error_writer, core_host_set_error_writer,
					  path, 1);
#pragma warning(default: 4054)
#else
				ptr = (const uint8_t*)set_error_writer(
						  ptr_to_host_policy_object,
						  host_policy_error_writer, core_host_set_error_writer,
						  path, 1);
#endif

				if (!buffer_resize(&output_data, 0) ||
					!pointer_to_string(ptr, &output_data))
				{
					return 0;
				}
			}
			break;

		case core_host_unload_:
			if (values_count ||
				!int_to_string(core_host_unload(ptr_to_host_policy_object), &output_data))
			{
				return 0;
			}

			break;

		case corehost_initialize_request_initialize_:
			if (values_count)
			{
				return 0;
			}

			values_count = core_host_initialize_request_init(
							   core_host_initialize_request_get(), CORE_HOST_INITIALIZE_REQUEST_SIZE);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case corehost_initialize_request_set_config_keys_:
			values_count = core_host_initialize_request_set_config_keys(
							   core_host_initialize_request_get(), values,
							   values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case corehost_initialize_request_set_config_values_:
			values_count = core_host_initialize_request_set_config_values(
							   core_host_initialize_request_get(), values,
							   values_lengths, values_count);

			if (!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case file_is_assembly_:
			if (!file_is_assembly(ptr_to_host_fxr_object,
								  values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		default:
			return 0;
	}

	*output = buffer_data(&output_data, 0);
	*output_length = (uint16_t)buffer_size(&output_data);
	/**/
	return 1;
}

void module_release()
{
	host_interface_release_buffers();
	core_host_initialize_request_release_buffers();
	error_writer_release_buffers(
		is_host_fxr_object_initialized ? host_fxr_object : NULL,
		is_host_policy_object_initialized ? host_policy_object : NULL);

	if (is_host_fxr_object_initialized)
	{
		host_fx_resolver_unload(host_fxr_object);
		is_host_fxr_object_initialized = 0;
	}

	if (is_host_policy_object_initialized)
	{
		host_policy_unload(host_policy_object);
		is_host_policy_object_initialized = 0;
	}

	if (is_buffer_initialized)
	{
		buffer_release(&output_data);
	}
}
