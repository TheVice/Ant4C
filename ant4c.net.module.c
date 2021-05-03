/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "ant4c.net.module.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "path.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include "host_fxr.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
static const type_of_element* zero = L"\0";
#else
static const type_of_element* zero = (const type_of_element*)"\0";
#endif

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

uint8_t values_to_arguments(
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count, struct buffer* output, const type_of_element*** argv)
{
	if (NULL == values ||
		NULL == values_lengths ||
		!output ||
		!argv)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (values_count)
	{
		if (!buffer_append(output, NULL, ((ptrdiff_t)1 + values_count) * sizeof(ptrdiff_t)))
		{
			return 0;
		}
	}

	for (uint16_t i = 0; i < values_count; ++i)
	{
		ptrdiff_t* positions = (ptrdiff_t*)buffer_data(output, size);
		positions[i] = buffer_size(output);
#if defined(_WIN32)

		if (values_lengths[i] &&
			!text_encoding_UTF8_to_UTF16LE(values[i], values[i] + values_lengths[i], output))
		{
			return 0;
		}

		if (!buffer_push_back_uint16(output, 0))
		{
			return 0;
		}

#else

		if (values_lengths[i] &&
			!buffer_append(output, values[i], values_lengths[i]))
		{
			return 0;
		}

		if (!buffer_push_back(output, 0))
		{
			return 0;
		}

#endif
	}

	if (values_count)
	{
		const ptrdiff_t sz = buffer_size(output);

		if (!buffer_append(output, NULL, ((ptrdiff_t)1 + values_count) * sizeof(type_of_element*)) ||
			!buffer_resize(output, sz))
		{
			return 0;
		}

		const ptrdiff_t* positions = (const ptrdiff_t*)buffer_data(output, size);

		for (uint16_t i = 0; i < values_count; ++i)
		{
			const type_of_element* data_at_position = (const type_of_element*)buffer_data(output, positions[i]);

			if (!buffer_append(output, (const uint8_t*)&data_at_position, sizeof(type_of_element*)))
			{
				return 0;
			}
		}

		*argv = (const type_of_element**)buffer_data(output, sz);
	}
	else
	{
		*argv = NULL;
	}

	return 1;
}

uint8_t value_to_system_path(const uint8_t* input, ptrdiff_t length, struct buffer* output)
{
#if defined(_WIN32)
	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (ptrdiff_t)4 * length + INT8_MAX) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

#endif

	if (!buffer_append(output, input, length) ||
		!buffer_push_back(output, 0))
	{
		return 0;
	}

#if defined(_WIN32)

	if (!file_system_path_to_pathW(buffer_data(output, size), output))
	{
		return 0;
	}

#endif
	return 1;
}

uint8_t values_to_system_paths(
	const uint8_t** values, const uint16_t* values_lengths,
	ptrdiff_t* positions, uint8_t values_count, struct buffer* output)
{
	if (!values ||
		!values_lengths ||
		!positions ||
		!output)
	{
		return 0;
	}

	for (uint8_t i = 0; i < values_count; ++i)
	{
		positions[i] = PTRDIFF_MAX;

		if (!values_lengths[i])
		{
			continue;
		}

		positions[i] = buffer_size(output);

		if (!value_to_system_path(values[i], values_lengths[i], output))
		{
			return 0;
		}

#if defined(_WIN32)
		positions[i] += values_lengths[i];
		++positions[i];
#endif
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
	static const uint8_t zero_symbol = '\0';
	/**/
	const uint8_t* new_finish = find_any_symbol_like_or_not_like_that(finish, start, &zero_symbol, 1, 0, -1);
	new_finish = find_any_symbol_like_or_not_like_that(new_finish, finish, &zero_symbol, 1, 1, 1);
	/**/
	return buffer_resize(output, new_finish - start);
}

uint8_t hostfxr_convert_function_name(
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

uint8_t hostfxr_functions(
	const void* ptr_to_host_fxr_object,
	const uint8_t* name_space, const uint8_t** functions,
	const uint8_t* delimiter, uint16_t delimiter_length,
	uint8_t offset, struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!name_space ||
		!functions ||
		!output)
	{
		return 0;
	}

	uint8_t function_exists[16];
	memset(function_exists, 0, sizeof(function_exists));
	const uint8_t* function_name = NULL;

	for (uint8_t i = offset, count = COUNT_OF(function_exists);
		 NULL != (function_name = functions[i]); ++i)
	{
		if (!buffer_resize(output, 0) ||
			!hostfxr_convert_function_name(
				name_space,
				function_name, function_name + common_count_bytes_until(function_name, 0),
				output))
		{
			return 0;
		}

		if (i - offset < count)
		{
			function_name = buffer_data(output, 0);
			function_exists[i - offset] =
				host_fx_resolver_is_function_exists(
					ptr_to_host_fxr_object, function_name, (uint8_t)buffer_size(output));
		}
		else
		{
			break;
		}
	}

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	for (uint8_t i = 0, count = COUNT_OF(function_exists); i < count; ++i)
	{
		if (!function_exists[i])
		{
			continue;
		}

		if (!common_append_string_to_buffer(functions[offset + i], output) ||
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

uint8_t host_fx_resolver_init(
	const uint8_t* path_to_host_fxr,
	uint16_t path_to_host_fxr_length,
	struct buffer* tmp,
	void* ptr_to_host_fxr_object,
	ptrdiff_t size)
{
	if (!path_to_host_fxr ||
		!path_to_host_fxr_length ||
		!tmp ||
		!ptr_to_host_fxr_object ||
		size < INT8_MAX)
	{
		return 0;
	}

	if (!buffer_resize(tmp, 0) ||
		!value_to_system_path(path_to_host_fxr, path_to_host_fxr_length, tmp))
	{
		return 0;
	}

#if defined(_WIN32)
	++path_to_host_fxr_length;
	const type_of_element* path_to_host_fxr_ =
		(const type_of_element*)buffer_data(tmp, path_to_host_fxr_length);
#else
	const type_of_element* path_to_host_fxr_ = (const type_of_element*)buffer_data(tmp, 0);
#endif

	if (!host_fx_resolver_load(path_to_host_fxr_, ptr_to_host_fxr_object, size))
	{
		return 0;
	}

	return 1;
}

uint8_t hostfxr_is_function_exists(
	const void* ptr_to_host_fxr_object, const uint8_t* name_space,
	const uint8_t* function_name, uint16_t function_name_length,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!name_space ||
		!function_name ||
		!function_name_length ||
		!output)
	{
		return 0;
	}

	if (!buffer_resize(output, 0) ||
		!hostfxr_convert_function_name(
			name_space, function_name, function_name + function_name_length, output))
	{
		return 0;
	}

	function_name = buffer_data(output, 0);
	function_name_length = (uint16_t)buffer_size(output);
	function_name_length = host_fx_resolver_is_function_exists(
							   ptr_to_host_fxr_object, function_name, (uint8_t)function_name_length);
	/**/
	return buffer_resize(output, 0) && bool_to_string(0 < function_name_length, output);
}

uint8_t hostfxr_result_code_to_string(int32_t code, struct buffer* output)
{
	if (!output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 32))
	{
		return 0;
	}

	char* ptr = (char*)buffer_data(output, size);
#if __STDC_LIB_EXT1__
	code = sprintf_s(ptr, 32, "0x%x %i %i", (int)code, (int)code, (int)(code & 0xFF));
#else
	code = sprintf(ptr, "0x%x %i %i", (int)code, (int)code, (int)(code & 0xFF));
#endif
	return buffer_resize(output, size + code);
}

uint8_t hostfxr_result_to_string(int32_t code, struct buffer* output)
{
	if (!output)
	{
		return 0;
	}

	switch (code)
	{
		case host_fxr_Success:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::Success", output))
			{
				return 0;
			}

			break;

		case host_fxr_Success_HostAlreadyInitialized:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::Success_HostAlreadyInitialized",
												output))
			{
				return 0;
			}

			break;

		case host_fxr_Success_DifferentRuntimeProperties:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::Success_DifferentRuntimeProperties",
												output))
			{
				return 0;
			}

			break;

		case win_error_E_INVALIDARG:
			if (!common_append_string_to_buffer((const uint8_t*)"[win error]::E_INVALIDARG", output))
			{
				return 0;
			}

			break;

		case host_fxr_InvalidArgFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::InvalidArgFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreHostLibLoadFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreHostLibLoadFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreHostLibMissingFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreHostLibMissingFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreHostEntryPointFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreHostEntryPointFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreHostCurHostFindFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreHostCurHostFindFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreClrResolveFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreClrResolveFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreClrBindFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreClrBindFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreClrInitFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreClrInitFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreClrExeFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreClrExeFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_ResolverInitFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::ResolverInitFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_ResolverResolveFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::ResolverResolveFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_LibHostCurExeFindFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::LibHostCurExeFindFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_LibHostInitFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::LibHostInitFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_LibHostSdkFindFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::LibHostSdkFindFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_LibHostInvalidArgs:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::LibHostInvalidArgs", output))
			{
				return 0;
			}

			break;

		case host_fxr_InvalidConfigFile:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::InvalidConfigFile", output))
			{
				return 0;
			}

			break;

		case host_fxr_AppArgNotRunnable:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::AppArgNotRunnable", output))
			{
				return 0;
			}

			break;

		case host_fxr_AppHostExeNotBoundFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::AppHostExeNotBoundFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_FrameworkMissingFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::FrameworkMissingFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_HostApiFailed:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::HostApiFailed", output))
			{
				return 0;
			}

			break;

		case host_fxr_HostApiBufferTooSmall:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::HostApiBufferTooSmall", output))
			{
				return 0;
			}

			break;

		case host_fxr_LibHostUnknownCommand:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::LibHostUnknownCommand", output))
			{
				return 0;
			}

			break;

		case host_fxr_LibHostAppRootFindFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::LibHostAppRootFindFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_SdkResolverResolveFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::SdkResolverResolveFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_FrameworkCompatFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::FrameworkCompatFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_FrameworkCompatRetry:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::FrameworkCompatRetry", output))
			{
				return 0;
			}

			break;

		case host_fxr_AppHostExeNotBundle:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::AppHostExeNotBundle", output))
			{
				return 0;
			}

			break;

		case host_fxr_BundleExtractionFailure:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::BundleExtractionFailure", output))
			{
				return 0;
			}

			break;

		case host_fxr_BundleExtractionIOError:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::BundleExtractionIOError", output))
			{
				return 0;
			}

			break;

		case host_fxr_LibHostDuplicateProperty:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::LibHostDuplicateProperty", output))
			{
				return 0;
			}

			break;

		case host_fxr_HostApiUnsupportedVersion:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::HostApiUnsupportedVersion", output))
			{
				return 0;
			}

			break;

		case host_fxr_HostInvalidState:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::HostInvalidState", output))
			{
				return 0;
			}

			break;

		case host_fxr_HostPropertyNotFound:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::HostPropertyNotFound", output))
			{
				return 0;
			}

			break;

		case host_fxr_CoreHostIncompatibleConfig:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::CoreHostIncompatibleConfig", output))
			{
				return 0;
			}

			break;

		case host_fxr_HostApiUnsupportedScenario:
			if (!common_append_string_to_buffer((const uint8_t*)"[host fx resolver]::HostApiUnsupportedScenario", output))
			{
				return 0;
			}

			break;

		default:
			return hostfxr_result_code_to_string(code, output);
	}

	if (!buffer_append_char(output, " (", 2) ||
		!hostfxr_result_code_to_string(code, output) ||
		!buffer_push_back(output, ')'))
	{
		return 0;
	}

	return 1;
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

	const int32_t code = host_fxr_get_available_sdks(ptr_to_host_fxr_object, exe_dir,
						 host_fx_resolver_available_sdks);
	buffer_release(&exe_dir_);

	if (HOST_FX_RESOLVER_NON_SUCCESS(code))
	{
		if (!buffer_resize(output, 0) ||
			!int_to_string(code, output))
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
		exe_dir = (const wchar_t*)(buffer_data(output, 0) + size);

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
	int32_t code = sizeof(uint32_t) + FILENAME_MAX * sizeof(type_of_element);
#else
	int32_t code = FILENAME_MAX * sizeof(type_of_element);
#endif

	if (!buffer_resize(output, code))
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

	if (HOST_FX_RESOLVER_NON_SUCCESS(code = host_fxr_get_native_search_directories(
			ptr_to_host_fxr_object, values_count, argv, out, FILENAME_MAX, &required_size)))
	{
		if ((int32_t)host_fxr_HostApiBufferTooSmall != code ||
			required_size < FILENAME_MAX ||
			!buffer_resize(output, required_size * sizeof(type_of_element)) ||
			HOST_FX_RESOLVER_NON_SUCCESS(
				host_fxr_get_native_search_directories(
					ptr_to_host_fxr_object, values_count, argv,
					(type_of_element*)buffer_data(output, 0), required_size, &required_size)))
		{
			buffer_release(&arguments);
			return 0;
		}
	}

	buffer_release(&arguments);
#if defined(_WIN32)
	wchar_t* start = (wchar_t*)buffer_data(output, sizeof(uint32_t));
	code = (int32_t)wcslen(start);
	required_size = (ptrdiff_t)3 * code;

	if (!buffer_append(output, NULL, required_size))
	{
		return 0;
	}

	start = (wchar_t*)buffer_data(output, sizeof(uint32_t));
	out = start + code;
	/**/
	return buffer_resize(output, 0) &&
		   text_encoding_UTF16LE_to_UTF8(start, out, output);
#else
	code = (int32_t)common_count_bytes_until(buffer_data(output, 0), 0);
	/**/
	return buffer_resize(output, code);
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
	int32_t code = host_fxr_get_runtime_delegate(
					   ptr_to_host_fxr_object, context, type_of_delegate, &the_delegate_function_type);

	if (HOST_FX_RESOLVER_NON_SUCCESS(code))
	{
		if (!buffer_push_back(output, 0) ||
			!int_to_string(code, output))
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
	code = the_delegate_function_type(
			   assembly_path, type_name, method_name,
			   delegate_type_name, unused, &the_delegate);

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	if (HOST_FX_RESOLVER_NON_SUCCESS(code))
	{
		if (!buffer_push_back(output, 0) ||
			!int_to_string(code, output))
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
	int32_t code = host_fxr_get_runtime_properties(
					   ptr_to_host_fxr_object, context,
					   &count, properties_keys, properties_values);

	if ((int32_t)host_fxr_HostApiBufferTooSmall == code)
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
		code = host_fxr_get_runtime_properties(ptr_to_host_fxr_object, context, &count, properties_keys,
											   properties_values);
	}

	if (HOST_FX_RESOLVER_NON_SUCCESS(code))
	{
		return int_to_string(code, output);
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

	return buffer_resize(output, 0) && int_to_string(code, output);
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
	const int32_t code = host_fxr_get_runtime_property_value(ptr_to_host_fxr_object, context, name, &value);

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

	if (HOST_FX_RESOLVER_NON_SUCCESS(code))
	{
		if (!buffer_push_back(output, 0) ||
			!int_to_string(code, output))
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
	const int32_t code = host_fxr_initialize_for_dotnet_command_line_parameters_in_parts(
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

	if (HOST_FX_RESOLVER_NON_SUCCESS(code) || !context)
	{
		if (!buffer_push_back(output, ' ') ||
			!int_to_string(code, output))
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
	const int32_t code = host_fxr_initialize_for_runtime_config_parameters_in_parts(ptr_to_host_fxr_object,
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

	if (HOST_FX_RESOLVER_NON_SUCCESS(code) || !context)
	{
		if (!buffer_push_back(output, ' ') ||
			!int_to_string(code, output))
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
	int32_t code = host_fxr_resolve_sdk(
					   ptr_to_host_fxr_object,
					   argv[0], argv[1], out,
					   FILENAME_MAX);

	if (FILENAME_MAX < code)
	{
#if defined(_WIN32)
		size = sizeof(uint32_t) + sizeof(type_of_element) * code;
#else
		size = sizeof(type_of_element) * code;
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
		code = host_fxr_resolve_sdk(
				   ptr_to_host_fxr_object,
				   argv[0], argv[1], out,
				   code);
	}

	buffer_release(&arguments);

	if (0 < code)
	{
#if defined(_WIN32)

		if (!buffer_resize(output, (ptrdiff_t)4 * code + sizeof(uint32_t)))
		{
			return 0;
		}

		out = (type_of_element*)buffer_data(output, size);

		if (!buffer_resize(output, 0) ||
			!text_encoding_UTF16LE_to_UTF8(out, out + code, output))
		{
			return 0;
		}

#else

		if (!buffer_resize(output, code))
		{
			return 0;
		}

#endif
	}
	else
	{
		if (!buffer_resize(output, 0) ||
			!int_to_string(code, output))
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
	const int32_t code = host_fxr_set_runtime_property_value(ptr_to_host_fxr_object, context, name, value);

	if (!buffer_resize(output, 0) ||
		!int_to_string(code, output))
	{
		return 0;
	}

	return 1;
}

static const uint8_t* name_spaces[] =
{
	(const uint8_t*)"nethost",
	(const uint8_t*)"hostfxr",
	(const uint8_t*)"file"
};

static const uint8_t* all_functions[][21] =
{
	{ (const uint8_t*)"get-hostfxr-path", NULL },
	{
		(const uint8_t*)"functions",
		(const uint8_t*)"initialize",
		(const uint8_t*)"is-function-exists",
		(const uint8_t*)"result-to-string",
		/**/
		(const uint8_t*)"close",
		(const uint8_t*)"get-available-sdks",
		(const uint8_t*)"get-native-search-directories",
		(const uint8_t*)"get-runtime-delegate",
		(const uint8_t*)"get-runtime-properties",
		(const uint8_t*)"get-runtime-property-value",
		(const uint8_t*)"initialize-for-dotnet-command-line",
		(const uint8_t*)"initialize-for-runtime-config",
		(const uint8_t*)"main",
		(const uint8_t*)"main-bundle-startupinfo",
		(const uint8_t*)"main-startupinfo",
		(const uint8_t*)"resolve-sdk",
		(const uint8_t*)"resolve-sdk2",
		(const uint8_t*)"run-app",
		(const uint8_t*)"set-error-writer",
		(const uint8_t*)"set-runtime-property-value",
		NULL
	},
	{ (const uint8_t*)"is-assembly", NULL },
};

enum ant4c_net_module_
{
	net_host_get_hostfxr_path_,
	/**/
	hostfxr_functions_,
	hostfxr_initialize_,
	hostfxr_is_function_exists_,
	hostfxr_result_to_string_,
	/**/
	hostfxr_close_,
	hostfxr_get_available_sdks_,
	hostfxr_get_native_search_directories_,
	hostfxr_get_runtime_delegate_,
	hostfxr_get_runtime_properties_,
	hostfxr_get_runtime_property_value_,
	hostfxr_initialize_for_dotnet_command_line_,
	hostfxr_initialize_for_runtime_config_,
	hostfxr_main_,
	hostfxr_main_bundle_startupinfo_,
	hostfxr_main_startupinfo_,
	hostfxr_resolve_sdk_,
	hostfxr_resolve_sdk2_,
	hostfxr_run_app_,
	hostfxr_set_error_writer_,
	hostfxr_set_runtime_property_value_,
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
	const uint8_t* get_runtime_delegate = all_functions[1][7];
	const uint8_t get_runtime_delegate_length = (uint8_t)common_count_bytes_until(get_runtime_delegate, 0);

	if (!hostfxr_is_function_exists(
			ptr_to_host_fxr_object, name_spaces[1],
			get_runtime_delegate, get_runtime_delegate_length, output))
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

			const uint8_t* sub_values[] = { NULL, NULL, buffer_data(output, values_count), NULL, NULL, NULL };
			uint16_t sub_values_lengths[] = { 0, 0, (uint16_t)(buffer_size(output) - values_count), 0, 0, 0 };

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

			if (HOST_FX_RESOLVER_NON_SUCCESS(host_fxr_close(ptr_to_host_fxr_object, context)))
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

		const uint8_t is_assembly = ((custom_entry_point_fn)the_delegate)(custom_argument);

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
		argv[2] = name_spaces[2];
		argv[3] = all_functions[2][0];
#endif

		if (1 == values_count)
		{
			if (!value_to_system_path(values[0], values_lengths[0], output))
			{
				return 0;
			}

#if defined(_WIN32)
			argv[1] = L"ant4c.net.module.clr.dll";
			argv[4] = (const wchar_t*)buffer_data(output, (ptrdiff_t)1 + values_lengths[0]);
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
		const int32_t code = host_fxr_main(ptr_to_host_fxr_object, values_count, argv);

		if (!buffer_resize(output, 0) ||
			!bool_to_string((uint8_t)code, output))
		{
			return 0;
		}
	}

	return 1;
}

void* error_writer = NULL;

#if defined(_WIN32)
static struct buffer content;
static uint8_t is_content_initialized = 0;

void host_fx_resolver_error_writer(const type_of_element* message)
{
	if (is_content_initialized)
	{
		if (!buffer_resize(&content, 0))
		{
			return;
		}
	}
	else
	{
		SET_NULL_TO_BUFFER(content);
		is_content_initialized = 1;
	}

	if (error_writer)
	{
		if (!text_encoding_UTF16LE_to_UTF8(message, message + wcslen(message), &content) ||
			!buffer_push_back(&content, '\n'))
		{
			return;
		}

		if (!file_write_with_several_steps(&content, error_writer))
		{
			return;
		}

		file_flush(error_writer);
	}
}
#else
void host_fx_resolver_error_writer(const type_of_element* message)
{
	if (error_writer)
	{
		static const uint8_t n = '\n';

		if (!file_write(message, sizeof(type_of_element), common_count_bytes_until(message, 0), error_writer) ||
			!file_write(&n, sizeof(type_of_element), 1, error_writer))
		{
			return;
		}

		file_flush(error_writer);
	}
}
#endif

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

const uint8_t* enumerate_name_spaces(ptrdiff_t index)
{
	static const ptrdiff_t count = sizeof(name_spaces) / sizeof(*name_spaces);

	if (index < 0 || count <= index)
	{
		return NULL;
	}

	return name_spaces[index];
}

const uint8_t* enumerate_functions(const uint8_t* name_space, ptrdiff_t index)
{
	static const ptrdiff_t count = sizeof(all_functions) / sizeof(*all_functions);

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

	if (NULL == ptr || (count <= (i - 1)))
	{
		return NULL;
	}

	const uint8_t** function_from_name_space = all_functions[i - 1];
	i = 0;

	while (NULL != (ptr = function_from_name_space[i++]))
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

static uint8_t host_fxr_object[UINT8_MAX];
static uint8_t is_host_fxr_object_initialized = 0;

uint8_t evaluate_function(
	const uint8_t* function,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	const uint8_t** output, uint16_t* output_length)
{
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

	for (ptrdiff_t i = 0, count = sizeof(all_functions) / sizeof(*all_functions); i < count; ++i)
	{
		ptrdiff_t j = 0;
		const uint8_t** functions_from_name_space = all_functions[i];

		while (NULL != (ptr = functions_from_name_space[j++]))
		{
			if (function == ptr)
			{
				i = count;
				break;
			}

			++function_number;
		}
	}

	if (NULL == ptr)
	{
		return 0;
	}

	const void* ptr_to_host_fxr_object = is_host_fxr_object_initialized ? host_fxr_object : NULL;

	switch (function_number)
	{
		case net_host_get_hostfxr_path_:
			if (!net_host_get_hostfxr_path(values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_functions_:
		{
			if (1 < values_count)
			{
				return 0;
			}

			static const uint8_t space = ' ';
			static const uint16_t one = 1;
			const uint8_t* space_in_array[1];

			if (!values_count)
			{
				space_in_array[0] = &space;
				values = space_in_array;
				values_lengths = &one;
			}

			if (!hostfxr_functions(ptr_to_host_fxr_object,
								   name_spaces[1], all_functions[1],
								   values[0], values_lengths[0],
								   hostfxr_result_to_string_, &output_data))
			{
				return 0;
			}
		}
		break;

		case hostfxr_initialize_:
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
												 values[0], values_lengths[0], &output_data, host_fxr_object, UINT8_MAX);

			if (!buffer_resize(&output_data, 0) ||
				!bool_to_string(is_host_fxr_object_initialized, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_is_function_exists_:
			if (1 != values_count)
			{
				return 0;
			}

			if (!hostfxr_is_function_exists(
					ptr_to_host_fxr_object, name_spaces[1],
					values[0], values_lengths[0], &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_result_to_string_:
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

			const int32_t code = int_parse(buffer_data(&output_data, 0));

			if (!buffer_resize(&output_data, 0) ||
				!hostfxr_result_to_string(code, &output_data))
			{
				return 0;
			}
		}
		break;

		case hostfxr_close_:
		{
			if (1 != values_count ||
				!ptr_to_host_fxr_object)
			{
				return 0;
			}

			ptr = string_to_pointer(values[0], (uint8_t)values_lengths[0], &output_data);

			if (!ptr)
			{
				return 0;
			}

			const int32_t code = host_fxr_close(ptr_to_host_fxr_object, ptr);

			if (!buffer_resize(&output_data, 0) ||
				!int_to_string(code, &output_data))
			{
				return 0;
			}
		}
		break;

		case hostfxr_get_available_sdks_:
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

		case hostfxr_get_native_search_directories_:
			if (!hostfxr_get_native_search_directories(ptr_to_host_fxr_object,
					values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_get_runtime_delegate_:
			if (!hostfxr_get_runtime_delegate(ptr_to_host_fxr_object,
											  values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_get_runtime_properties_:
			if (!hostfxr_get_runtime_properties(ptr_to_host_fxr_object,
												values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_get_runtime_property_value_:
			if (!hostfxr_get_runtime_property_value(ptr_to_host_fxr_object,
													values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_initialize_for_dotnet_command_line_:
			if (!hostfxr_initialize_for_dotnet_command_line(ptr_to_host_fxr_object,
					values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_initialize_for_runtime_config_:
			if (!hostfxr_initialize_for_runtime_config(ptr_to_host_fxr_object,
					values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_main_:
		{
			if (!ptr_to_host_fxr_object)
			{
				return 0;
			}

			const type_of_element** argv = NULL;

			if (!values_to_arguments(values, values_lengths, values_count, &output_data, &argv))
			{
				return 0;
			}

			const int32_t code = host_fxr_main(ptr_to_host_fxr_object, values_count, argv);

			if (!buffer_resize(&output_data, 0) ||
				!int_to_string(code, &output_data))
			{
				return 0;
			}
		}
		break;

		case hostfxr_main_bundle_startupinfo_:
			if (!hostfxr_main_bundle_startupinfo(
					ptr_to_host_fxr_object, values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_main_startupinfo_:
			if (!hostfxr_main_startupinfo(ptr_to_host_fxr_object, values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_resolve_sdk_:
			if (!hostfxr_resolve_sdk(ptr_to_host_fxr_object, values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_resolve_sdk2_:
			if (!hostfxr_resolve_sdk2(ptr_to_host_fxr_object, values, values_lengths, values_count, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_run_app_:
		{
			if (1 != values_count ||
				!ptr_to_host_fxr_object)
			{
				return 0;
			}

			ptr = string_to_pointer(values[0], (uint8_t)values_lengths[0], &output_data);

			if (!ptr)
			{
				return 0;
			}

			const int32_t code = host_fxr_run_app(ptr_to_host_fxr_object, ptr);

			if (!buffer_resize(&output_data, 0) ||
				!int_to_string(code, &output_data))
			{
				return 0;
			}
		}
		break;

		case hostfxr_set_error_writer_:
			if (1 < values_count)
			{
				return 0;
			}

			ptr = NULL;

			if (!values_count || !values_lengths[0])
			{
				ptr = (const void*)host_fxr_set_error_writer(ptr_to_host_fxr_object, NULL);
			}

			if (error_writer && !file_close(error_writer))
			{
				return 0;
			}

			error_writer = NULL;

			if (values_count && values_lengths[0])
			{
				if (!value_to_system_path(values[0], values_lengths[0], &output_data))
				{
					return 0;
				}

#if defined(_WIN32)

				if (!file_open_wchar_t(
						(const wchar_t*)buffer_data(&output_data, (ptrdiff_t)1 + values_lengths[0]),
						L"ab", &error_writer))
				{
					return 0;
				}

#else

				if (!file_open(buffer_data(&output_data, 0), (const uint8_t*)"ab", &error_writer))
				{
					return 0;
				}

#endif
				ptr = (const void*)host_fxr_set_error_writer(ptr_to_host_fxr_object, host_fx_resolver_error_writer);
			}

			if (!buffer_resize(&output_data, 0) ||
				!pointer_to_string(ptr, &output_data))
			{
				return 0;
			}

			break;

		case hostfxr_set_runtime_property_value_:
			if (!hostfxr_set_runtime_property_value(ptr_to_host_fxr_object,
													values, values_lengths, values_count, &output_data))
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
	if (is_host_fxr_object_initialized)
	{
		if (error_writer)
		{
			host_fxr_set_error_writer(host_fxr_object, NULL);
			file_close(error_writer);
			error_writer = NULL;
		}

		host_fx_resolver_unload(host_fxr_object);
		is_host_fxr_object_initialized = 0;
	}

#if defined(_WIN32)

	if (is_content_initialized)
	{
		buffer_release(&content);
	}

#endif

	if (is_buffer_initialized)
	{
		buffer_release(&output_data);
	}
}
