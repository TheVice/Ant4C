/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "net.host_fxr.h"
#include "arguments.h"
#include "error_writer.h"
#include "host_fxr.h"
#include "net.common.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
static const type_of_element* zero = L"\0";
#else
static const type_of_element* zero = (const type_of_element*)"\0";
#endif

static const uint8_t space = ' ';
static struct buffer* g_buffer = NULL;

void host_fx_resolver_available_sdks(int32_t count, const type_of_element** directories)
{
	for (int32_t i = 0; i < count; ++i)
	{
#if defined(_WIN32)

		if (!buffer_append_wchar_t(g_buffer, directories[i], wcslen(directories[i])) ||
			!buffer_push_back_uint16(g_buffer, 0))
#else
		if (!buffer_append(g_buffer, directories[i], common_count_bytes_until(directories[i], 0)) ||
			!buffer_push_back(g_buffer, 0))
#endif
		{
			buffer_resize(g_buffer, 0);
			break;
		}
	}
}

void host_fx_resolver_sdk2(int32_t key, const type_of_element* value)
{
	if (!int_to_string(key, g_buffer) ||
		!buffer_push_back(g_buffer, space))
	{
		buffer_resize(g_buffer, 0);
	}

#if defined(_WIN32)

	if (!text_encoding_UTF16LE_to_UTF8(value, value + wcslen(value), g_buffer) ||
		!buffer_push_back_uint16(g_buffer, 0))
#else
	if (!buffer_append(g_buffer, value, common_count_bytes_until(value, 0)) ||
		!buffer_push_back(g_buffer, 0))
#endif
	{
		buffer_resize(g_buffer, 0);
	}
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

uint8_t hostfxr_close(
	const void* ptr_to_host_fxr_object,
	const uint8_t* context, uint16_t context_length, struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!context ||
		!context_length ||
		!output)
	{
		return 0;
	}

	const uint8_t* context_ = string_to_pointer(context, (uint8_t)context_length, output);
	const int32_t result = host_fxr_close(ptr_to_host_fxr_object, context_);

	if (!buffer_resize(output, 0) ||
		!int_to_string(result, output))
	{
		return 0;
	}

	return 1;
}

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

	g_buffer = output;
	const int32_t result = host_fxr_get_available_sdks(
							   ptr_to_host_fxr_object, exe_dir, host_fx_resolver_available_sdks);
	g_buffer = NULL;
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
		if ((int32_t)net_HostApiBufferTooSmall != result ||
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
	return buffer_resize(output, result);
#endif
}

uint8_t hostfxr_get_runtime_delegate(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
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

	if ((int32_t)net_HostApiBufferTooSmall == result)
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
		if (!buffer_push_back(output, space) ||
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
		if (!buffer_push_back(output, space) ||
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

	g_buffer = output;
	flags = host_fxr_resolve_sdk2(
				ptr_to_host_fxr_object,
				argv[0], argv[1],
				flags, host_fx_resolver_sdk2);
	g_buffer = NULL;
	buffer_release(&arguments);

	if (!buffer_size(output))
	{
		return int_to_string(flags, output);
	}

	return 1;
}

uint8_t hostfxr_main(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	const type_of_element** argv = NULL;

	if (!values_to_arguments(values, values_lengths, values_count, output, &argv))
	{
		return 0;
	}

	const int32_t result = host_fxr_main(ptr_to_host_fxr_object, values_count, argv);

	if (!buffer_resize(output, 0) ||
		!int_to_string(result, output))
	{
		return 0;
	}

	return 1;
}

uint8_t hostfxr_run_app(
	const void* ptr_to_host_fxr_object,
	const uint8_t* context, uint16_t context_length, struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!context ||
		!context_length ||
		!output)
	{
		return 0;
	}

	const uint8_t* context_ = string_to_pointer(context, (uint8_t)context_length, output);
	const int32_t result = host_fxr_run_app(ptr_to_host_fxr_object, context_);

	if (!buffer_resize(output, 0) ||
		!int_to_string(result, output))
	{
		return 0;
	}

	return 1;
}

uint8_t hostfxr_set_error_writer(
	const void* ptr_to_host_fxr_object,
	const uint8_t* error_writer_file_name, uint16_t error_writer_file_name_length,
	struct buffer* output)
{
	const type_of_element* path = NULL;

	if (error_writer_file_name && error_writer_file_name_length)
	{
		if (!value_to_system_path(error_writer_file_name, error_writer_file_name_length, output))
		{
			return 0;
		}

#if defined(_WIN32)
		path = (const type_of_element*)buffer_data(output, (ptrdiff_t)1 + error_writer_file_name_length);
#else
		path = buffer_data(output, 0);
#endif
	}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4054)
	error_writer_file_name = (const uint8_t*)set_error_writer(
								 ptr_to_host_fxr_object,
								 host_fx_resolver_error_writer, host_fxr_set_error_writer,
								 path, 0);
#pragma warning(default: 4054)
#else
	error_writer_file_name = (const uint8_t*)set_error_writer(
								 ptr_to_host_fxr_object,
								 host_fx_resolver_error_writer, host_fxr_set_error_writer,
								 path, 0);
#endif

	if (!buffer_resize(output, 0) ||
		!pointer_to_string(error_writer_file_name, output))
	{
		return 0;
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
	const type_of_element* value = 3 == values_count ? (const type_of_element*)buffer_data(output, index) : NULL;
	/**/
	const int32_t result = host_fxr_set_runtime_property_value(ptr_to_host_fxr_object, context, name, value);

	if (!buffer_resize(output, 0) ||
		!int_to_string(result, output))
	{
		return 0;
	}

	return 1;
}
