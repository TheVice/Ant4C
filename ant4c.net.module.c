/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 https://github.com/TheVice/
 *
 */

#include "ant4c.net.module.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include "host_fxr.h"

#include <stdio.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

uint8_t net_host_get_hostfxr_path(
	const uint8_t* path_to_net_host_object,
	uint16_t path_to_net_host_object_length,
	const uint8_t* path_to_assembly,
	uint16_t path_to_assembly_length,
	const uint8_t* path_to_dot_net_root,
	uint16_t path_to_dot_net_root_length,
	struct buffer* path_to_host_fxr)
{
	if (!path_to_net_host_object ||
		!path_to_host_fxr)
	{
		return 0;
	}

	struct buffer platform_path;

	SET_NULL_TO_BUFFER(platform_path);

#if defined(_WIN32)
	if (!buffer_resize(&platform_path, sizeof(type_of_element) * 4 * UINT8_MAX) ||
		!buffer_resize(&platform_path, 0))
	{
		buffer_release(&platform_path);
		return 0;
	}

	if (!buffer_resize(path_to_host_fxr, 0) ||
		!buffer_append(path_to_host_fxr, path_to_net_host_object, path_to_net_host_object_length) ||
		!buffer_push_back(path_to_host_fxr, 0) ||
		NULL == (path_to_net_host_object = buffer_data(path_to_host_fxr, 0)) ||
		!file_system_path_to_pathW(path_to_net_host_object, &platform_path))
	{
		buffer_release(&platform_path);
		return 0;
	}

	const ptrdiff_t platform_path_to_assembly_index = path_to_assembly ? buffer_size(&platform_path) : 0;

	if (path_to_assembly &&
		(!buffer_resize(path_to_host_fxr, 0) ||
		 !buffer_append(path_to_host_fxr, path_to_assembly, path_to_assembly_length) ||
		 !buffer_push_back(path_to_host_fxr, 0) ||
		 NULL == (path_to_assembly = buffer_data(path_to_host_fxr, 0)) ||
		 !file_system_path_to_pathW(path_to_assembly, &platform_path)))
	{
		buffer_release(&platform_path);
		return 0;
	}

	const ptrdiff_t platform_path_to_dot_net_root_index = path_to_dot_net_root ? buffer_size(&platform_path) : 0;

	if (path_to_dot_net_root &&
		(!buffer_resize(path_to_host_fxr, 0) ||
		 !buffer_append(path_to_host_fxr, path_to_dot_net_root, path_to_dot_net_root_length) ||
		 !buffer_push_back(path_to_host_fxr, 0) ||
		 NULL == (path_to_dot_net_root = buffer_data(path_to_host_fxr, 0)) ||
		 !file_system_path_to_pathW(path_to_dot_net_root, &platform_path)))
	{
		buffer_release(&platform_path);
		return 0;
	}

	const type_of_element* platform_path_to_net_host_object =
		(const type_of_element*)buffer_data(&platform_path, 0);
	const type_of_element* platform_path_to_assembly =
		platform_path_to_assembly_index ? (const type_of_element*)buffer_data(
			&platform_path, platform_path_to_assembly_index) : NULL;
	const type_of_element* platform_path_to_dot_net_root =
		platform_path_to_dot_net_root_index ? (const type_of_element*)buffer_data(
			&platform_path, platform_path_to_dot_net_root_index) : NULL;

	if (!buffer_resize(path_to_host_fxr, sizeof(uint32_t)) ||
		!net_host_load(
			platform_path_to_net_host_object, platform_path_to_assembly,
			platform_path_to_dot_net_root, NULL, path_to_host_fxr))
	{
		buffer_release(&platform_path);
		return 0;
	}

	buffer_release(&platform_path);
	/**/
	platform_path_to_assembly = (const type_of_element*)buffer_data(path_to_host_fxr, sizeof(uint32_t));
	platform_path_to_dot_net_root = (const type_of_element*)(
										buffer_data(path_to_host_fxr, 0) + buffer_size(path_to_host_fxr));
	/**/
	return buffer_resize(path_to_host_fxr, 0) &&
		   text_encoding_UTF16LE_to_UTF8(platform_path_to_assembly, platform_path_to_dot_net_root, path_to_host_fxr);
#else

	if (!buffer_resize(&platform_path,
					   (ptrdiff_t)path_to_net_host_object_length + path_to_assembly_length + path_to_dot_net_root_length +
					   sizeof(uint32_t)) ||
		!buffer_resize(&platform_path, 0))
	{
		buffer_release(&platform_path);
		return 0;
	}

	if (!buffer_append(&platform_path, path_to_net_host_object, path_to_net_host_object_length) ||
		!buffer_push_back(&platform_path, 0))
	{
		buffer_release(&platform_path);
		return 0;
	}

	const ptrdiff_t platform_path_to_assembly_index = path_to_assembly ? buffer_size(&platform_path) : 0;

	if (platform_path_to_assembly_index &&
		(!buffer_append(&platform_path, path_to_assembly, path_to_assembly_length) ||
		 !buffer_push_back(&platform_path, 0)))
	{
		buffer_release(&platform_path);
		return 0;
	}

	const ptrdiff_t platform_path_to_dot_net_root_index = path_to_dot_net_root ? buffer_size(&platform_path) : 0;

	if (platform_path_to_dot_net_root_index &&
		(!buffer_append(&platform_path, path_to_dot_net_root, path_to_dot_net_root_length) ||
		 !buffer_push_back(&platform_path, 0)))
	{
		buffer_release(&platform_path);
		return 0;
	}

	path_to_net_host_object = buffer_data(&platform_path, 0);
	path_to_assembly = platform_path_to_assembly_index ? buffer_data(&platform_path,
					   platform_path_to_assembly_index) : NULL;
	path_to_dot_net_root = platform_path_to_dot_net_root_index ? buffer_data(&platform_path,
						   platform_path_to_dot_net_root_index) : NULL;

	if (!net_host_load(
			path_to_net_host_object, path_to_assembly, path_to_dot_net_root, NULL, path_to_host_fxr))
	{
		buffer_release(&platform_path);
		return 0;
	}

	buffer_release(&platform_path);
	return 1;
#endif
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

	if (values_count)
	{
		if (!buffer_resize(output, ((ptrdiff_t)1 + values_count) * sizeof(ptrdiff_t)))
		{
			return 0;
		}
	}

	for (uint16_t i = 0; i < values_count; ++i)
	{
		ptrdiff_t* positions = (ptrdiff_t*)buffer_data(output, 0);
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
		const ptrdiff_t size = buffer_size(output);

		if (!buffer_append(output, NULL, ((ptrdiff_t)1 + values_count) * sizeof(type_of_element*)) ||
			!buffer_resize(output, size))
		{
			return 0;
		}

		const ptrdiff_t* positions = (const ptrdiff_t*)buffer_data(output, 0);

		for (uint16_t i = 0; i < values_count; ++i)
		{
			const type_of_element* data_at_position = (const type_of_element*)buffer_data(output, positions[i]);

			if (!buffer_append(output, (const uint8_t*)&data_at_position, sizeof(type_of_element*)))
			{
				return 0;
			}
		}

		*argv = (const type_of_element**)buffer_data(output, size);
	}
	else
	{
		*argv = NULL;
	}

	return 1;
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
	void* ptr_to_host_fxr_object,
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
				function_name, function_name + common_count_bytes_until(function_name, '\0'),
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
	struct buffer* host_fxr_object,
	ptrdiff_t size)
{
	if (!path_to_host_fxr ||
		!path_to_host_fxr_length ||
		!host_fxr_object ||
		size < INT8_MAX)
	{
		return 0;
	}

	if (!buffer_resize(host_fxr_object, size) ||
		!buffer_resize(host_fxr_object, 0))
	{
		return 0;
	}

#if defined(_WIN32)

	if (!buffer_resize(host_fxr_object, (ptrdiff_t)4 * path_to_host_fxr_length + INT8_MAX) ||
		!buffer_resize(host_fxr_object, 0))
	{
		return 0;
	}

#endif

	if (!buffer_append(host_fxr_object, path_to_host_fxr, path_to_host_fxr_length) ||
		!buffer_push_back(host_fxr_object, 0))
	{
		return 0;
	}

#if defined(_WIN32)

	if (!file_system_path_to_pathW(buffer_data(host_fxr_object, 0), host_fxr_object))
	{
		return 0;
	}

	const type_of_element* path_to_host_fxr_ = (const type_of_element*)buffer_data(
				host_fxr_object, (ptrdiff_t)1 + path_to_host_fxr_length);
	void* ptr_to_host_fxr_object = buffer_data(host_fxr_object, 0);
#else
	const type_of_element* path_to_host_fxr_ = (const type_of_element*)buffer_data(host_fxr_object, 0);
	void* ptr_to_host_fxr_object = (void*)path_to_host_fxr_;
#endif

	if (!buffer_resize(host_fxr_object, size) ||
		!host_fx_resolver_load(path_to_host_fxr_, ptr_to_host_fxr_object, size))
	{
		return 0;
	}

	return 1;
}

uint8_t hostfxr_is_function_exists(
	void* ptr_to_host_fxr_object, const uint8_t* name_space,
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
#if __STDC_SEC_API__
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
	void* ptr_to_host_fxr_object,
	const uint8_t* executable_directory, uint16_t executable_directory_length,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!output)
	{
		return 0;
	}

#if defined(_WIN32)

	if (!buffer_resize(output, sizeof(uint64_t)))
	{
		return 0;
	}

#endif
	const type_of_element* exe_dir = NULL;
	struct buffer exe_dir_;
	SET_NULL_TO_BUFFER(exe_dir_);

	if (executable_directory)
	{
#if defined(_WIN32)

		if (!buffer_resize(&exe_dir_, (ptrdiff_t)4 * executable_directory_length + INT8_MAX) ||
			!buffer_resize(&exe_dir_, 0))
		{
			buffer_release(&exe_dir_);
			return 0;
		}

#endif

		if (!buffer_append(&exe_dir_, executable_directory, executable_directory_length) ||
			!buffer_push_back(&exe_dir_, 0))
		{
			buffer_release(&exe_dir_);
			return 0;
		}

#if defined(_WIN32)
		const uint8_t* path = buffer_data(&exe_dir_, 0);

		if (!file_system_path_to_pathW(path, &exe_dir_))
		{
			buffer_release(&exe_dir_);
			return 0;
		}

		exe_dir = (const type_of_element*)buffer_data(&exe_dir_, (ptrdiff_t)executable_directory_length + 1);
#else
		exe_dir = buffer_data(&exe_dir_, 0);
#endif
	}

	const int32_t result = host_fxr_get_available_sdks(ptr_to_host_fxr_object, exe_dir,
						   host_fx_resolver_available_sdks);
	buffer_release(&exe_dir_);

	if (HOST_FX_RESOLVER_NON_SUCCESS(result))
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

		ptr_to_host_fxr_object = buffer_data(output, sizeof(uint64_t));
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
	void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!output)
	{
		return 0;
	}

#if defined(_WIN32)

	if (!buffer_resize(output, sizeof(uint64_t) + FILENAME_MAX * sizeof(type_of_element)))
#else
	if (!buffer_resize(output, FILENAME_MAX * sizeof(type_of_element)))
#endif
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

	int32_t result = 0;
#if defined(_WIN32)
	type_of_element* out = (type_of_element*)buffer_data(output, sizeof(uint64_t));
#else
	type_of_element* out = (type_of_element*)buffer_data(output, 0);
#endif
	int32_t required_size = FILENAME_MAX;

	if (HOST_FX_RESOLVER_NON_SUCCESS(result = host_fxr_get_native_search_directories(
										 ptr_to_host_fxr_object, values_count, argv, out, FILENAME_MAX, &required_size)))
	{
		if ((int32_t)host_fxr_HostApiBufferTooSmall != result ||
			required_size < FILENAME_MAX ||
			!buffer_resize(output, required_size * sizeof(type_of_element)) ||
			HOST_FX_RESOLVER_NON_SUCCESS(/*result = */host_fxr_get_native_search_directories(
						ptr_to_host_fxr_object, values_count, argv,
						(type_of_element*)buffer_data(output, 0), required_size, &required_size)))
		{
			buffer_release(&arguments);
			return 0;
		}
	}

	buffer_release(&arguments);
#if defined(_WIN32)
	type_of_element* start = (type_of_element*)buffer_data(output, sizeof(uint64_t));
	out = start;

	while (L'\0' != *out)
	{
		++out;
	}

	required_size = (int32_t)(out - start);
	result = 4 * required_size * sizeof(type_of_element);

	if (!buffer_resize(output, result))
	{
		return 0;
	}

	start = (type_of_element*)buffer_data(output, sizeof(uint64_t));
	out = start + required_size;
	return buffer_resize(output, 0) &&
		   text_encoding_UTF16LE_to_UTF8(start, out, output) &&
		   buffer_push_back(output, 0);
#else
	result = (int32_t)common_count_bytes_until(buffer_data(output, 0), 0);
	return buffer_resize(output, (ptrdiff_t)result + 1);
#endif
}

uint8_t hostfxr_main_startupinfo(
	void* ptr_to_host_fxr_object,
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

	const ptrdiff_t array_size = sizeof(ptrdiff_t) * ((ptrdiff_t)1 + values_count);
	ptrdiff_t size = array_size;

	for (uint16_t i = 0; i < values_count; ++i)
	{
		size += values_lengths[i];
	}

#if defined(_WIN32)

	if (array_size < size)
	{
		size += 3 * INT8_MAX;
	}
	else
#endif
	{
		size += 3;
	}

	size *= sizeof(type_of_element);
	size += values_count * sizeof(type_of_element*);
	/**/
	struct buffer arguments;
	SET_NULL_TO_BUFFER(arguments);

	if (!buffer_resize(&arguments, size) ||
		!buffer_resize(&arguments, array_size))
	{
		buffer_release(&arguments);
		return 0;
	}

#if defined(_WIN32)

	for (uint16_t i = 0; i < values_count; ++i)
	{
		ptrdiff_t* positions = (ptrdiff_t*)buffer_data(&arguments, 0);
		positions[i] = buffer_size(&arguments);

		if (values_lengths[i])
		{
			if (i < 3)
			{
				if (!buffer_resize(output, 0) ||
					!buffer_append(output, values[i], values_lengths[i]) ||
					!buffer_push_back(output, 0))
				{
					buffer_release(&arguments);
					return 0;
				}

				const uint8_t* data = buffer_data(output, 0);

				if (!file_system_path_to_pathW(data, &arguments))
				{
					buffer_release(&arguments);
					return 0;
				}
			}
			else
			{
				if (!text_encoding_UTF8_to_UTF16LE(
						values[i], values[i] + values_lengths[i], &arguments))
				{
					buffer_release(&arguments);
					return 0;
				}
			}
		}

		if (!buffer_push_back_uint16(&arguments, 0))
		{
			buffer_release(&arguments);
			return 0;
		}
	}

#else

	for (uint16_t i = 0; i < values_count; ++i)
	{
		ptrdiff_t* positions = (ptrdiff_t*)buffer_data(&arguments, 0);
		positions[i] = buffer_size(&arguments);

		if (values_lengths[i])
		{
			if (!buffer_resize(&arguments, 0) ||
				!buffer_append(&arguments, values[i], values_lengths[i]))
			{
				buffer_release(&arguments);
				return 0;
			}
		}

		if (!buffer_push_back(&arguments, 0))
		{
			buffer_release(&arguments);
			return 0;
		}
	}

#endif
	size = buffer_size(&arguments);

	for (uint16_t i = 3; i < values_count; ++i)
	{
		const ptrdiff_t* positions = (const ptrdiff_t*)buffer_data(&arguments, 0);
		const type_of_element* data_at_position = (const type_of_element*)buffer_data(&arguments, positions[i]);

		if (!buffer_append(&arguments, (const uint8_t*)&data_at_position, sizeof(type_of_element*)))
		{
			buffer_release(&arguments);
			return 0;
		}
	}

	int32_t argc = values_count - 3;
	const type_of_element** argv = (const type_of_element**)buffer_data(&arguments, size);
	/**/
	const ptrdiff_t* p = (const ptrdiff_t*)buffer_data(&arguments, 0);
	const type_of_element* host_path = (const type_of_element*)buffer_data(&arguments, p[0]);
	const type_of_element* dotnet_root = (const type_of_element*)buffer_data(&arguments, p[1]);
	const type_of_element* app_path = (const type_of_element*)buffer_data(&arguments, p[2]);
	/**/
	argc = host_fxr_main_startupinfo(ptr_to_host_fxr_object, argc, argv, host_path, dotnet_root, app_path);
	buffer_release(&arguments);
	/**/
	return buffer_resize(output, 0) && int_to_string(argc, output);
}

uint8_t hostfxr_resolve_sdk(
	void* ptr_to_host_fxr_object,
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
	ptrdiff_t size = sizeof(uint64_t) + sizeof(type_of_element) * FILENAME_MAX;
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
	size = sizeof(uint64_t);
#else
	size = 0;
#endif
	type_of_element* out = (type_of_element*)buffer_data(output, size);
	int32_t result = host_fxr_resolve_sdk(ptr_to_host_fxr_object, argv[0]/*exe_dir*/, argv[1]/*working_dir*/, out,
										  FILENAME_MAX);

	if (FILENAME_MAX < result)
	{
#if defined(_WIN32)
		size = sizeof(uint64_t) + sizeof(type_of_element) * result;
#else
		size = sizeof(type_of_element) * result;
#endif

		if (!buffer_resize(output, size))
		{
			buffer_release(&arguments);
			return 0;
		}

#if defined(_WIN32)
		size = sizeof(uint64_t);
#else
		size = 0;
#endif
		out = (type_of_element*)buffer_data(output, size);
		result = host_fxr_resolve_sdk(ptr_to_host_fxr_object, argv[0]/*exe_dir*/, argv[1]/*working_dir*/, out,
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
	void* ptr_to_host_fxr_object,
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
				argv[0]/*exe_dir*/, argv[1]/*working_dir*/, flags,
				host_fx_resolver_sdk2);
	buffer_release(&arguments);

	if (!buffer_size(output))
	{
		return int_to_string(flags, output);
	}

	return 1;
}

/*uint8_t file_is_assembly(const uint8_t* path, uint16_t length, uint8_t* returned)
{
	if (!path ||
		!length ||
		!returned)
	{
		return 0;
	}

	if (!metahost_get_clr_version_from_file(path, length))
	{
		return 0;
	}

	*returned = 0 < buffer_size(&output_data);
	return buffer_resize(&output_data, 0);
}*/

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

struct buffer host_fxr_object_;
static uint8_t is_host_fxr_object_initialized = 0;

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

	if (!is_host_fxr_object_initialized)
	{
		SET_NULL_TO_BUFFER(host_fxr_object_);
		is_host_fxr_object_initialized = 1;
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

	void* ptr_to_host_fxr_object = buffer_data(&host_fxr_object_, 0);

	switch (function_number)
	{
		case net_host_get_hostfxr_path_:
			if (!values_count || 3 < values_count)
			{
				return 0;
			}

			if (1 == values_count &&
				!net_host_get_hostfxr_path(values[0], values_lengths[0], NULL, 0, NULL, 0, &output_data))
			{
				return 0;
			}
			else if (2 == values_count &&
					 !net_host_get_hostfxr_path(values[0], values_lengths[0], values[1], values_lengths[1], NULL, 0, &output_data))
			{
				return 0;
			}
			else if (3 == values_count &&
					 !net_host_get_hostfxr_path(
						 values[0], values_lengths[0], values[1], values_lengths[1], values[2], values_lengths[2], &output_data))
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

			if (!hostfxr_functions(
					ptr_to_host_fxr_object,
					name_spaces[1], all_functions[1],
					values[0], values_lengths[0],
					hostfxr_functions_, &output_data))
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

			if (buffer_size(&host_fxr_object_))
			{
				host_fx_resolver_unload(ptr_to_host_fxr_object);

				if (!buffer_resize(&host_fxr_object_, 0))
				{
					return 0;
				}
			}

			values_count = host_fx_resolver_init(values[0], values_lengths[0], &host_fxr_object_, UINT8_MAX);

			if (!values_count &&
				!buffer_resize(&host_fxr_object_, 0))
			{
				return 0;
			}

			if (!bool_to_string(values_count, &output_data))
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
#if 0

		case hostfxr_close_:
			break;
#endif

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
			if (!hostfxr_get_native_search_directories(
					ptr_to_host_fxr_object,
					values, values_lengths, values_count,
					&output_data))
			{
				return 0;
			}

			break;
#if 0
			hostfxr_get_runtime_delegate_:
				break;
			hostfxr_get_runtime_properties_:
				break;
			hostfxr_get_runtime_property_value_:
				break;
			hostfxr_initialize_for_dotnet_command_line_:
				break;
			hostfxr_initialize_for_runtime_config_:
				break;
#endif

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

			const int32_t result = host_fxr_main(ptr_to_host_fxr_object, values_count, argv);

			if (!buffer_resize(&output_data, 0) ||
				!int_to_string(result, &output_data))
			{
				return 0;
			}
		}
		break;
#if 0
		hostfxr_main_bundle_startupinfo_:
			break;
#endif

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
#if 0
		hostfxr_run_app_:
			break;
		hostfxr_set_error_writer_:
			break;
		hostfxr_set_runtime_property_value_:
			break;
		case file_is_assembly_:
			if (1 != values_count ||
				!file_is_assembly(values[0], values_lengths[0], &values_count) ||
				!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case framework_exists_:
			if (1 != values_count ||
				!text_encoding_UTF8_to_UTF16LE(values[0], values[0] + values_lengths[0], &output_data) ||
				!framework_get_function_return(buffer_wchar_t_data(&output_data, 0)))
			{
				return 0;
			}

			break;

		case framework_get_clr_version_:
			if (0 < values_count ||
				!FRAMEWORK_GET_CLR_VERSION())
			{
				return 0;
			}

			break;

		case framework_get_framework_directory_:
			if (1 < values_count ||
				!FRAMEWORK_GET_FRAMEWORK_DIRECTORY())
			{
				return 0;
			}

			break;

		case framework_get_frameworks_:
			if (1 < values_count ||
				!FRAMEWORK_GET_FRAMEWORKS())
			{
				return 0;
			}

			break;

		case framework_get_runtime_framework_:
			if (0 < values_count ||
				!framework_get_function_return(NULL))
			{
				return 0;
			}

			break;

		case metahost_runtime_:
			if (1 != values_count ||
				!metahost_runtime(values[0], values_lengths[0], &values_count) ||
				!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case metahost_get_clr_version_from_file_:
			if (1 != values_count ||
				!metahost_get_clr_version_from_file(values[0], values_lengths[0]))
			{
				return 0;
			}

			break;
#endif

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
		void* ptr_to_host_fxr_object = buffer_data(&host_fxr_object_, 0);
		host_fx_resolver_unload(ptr_to_host_fxr_object);
		buffer_release(&host_fxr_object_);
	}

	if (is_buffer_initialized)
	{
		buffer_release(&output_data);
	}
}
