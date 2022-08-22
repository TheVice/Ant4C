/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "host_fxr.h"

#include "buffer.h"
#include "common.h"
#include "shared_object.h"

#include <stdio.h>
#if defined(_WIN32)
#include <wchar.h>
#endif
#include <string.h>

#if defined(_WIN32)
extern void* shared_object_load_wchar_t(const wchar_t* path);
#define net_host_calling_convention __stdcall
#else
#define net_host_calling_convention
#endif

struct _paths
{
	size_t size;
	const type_of_element* path_to_assembly;
	const type_of_element* path_to_dot_net_root;
};

static struct _paths paths;

#define SET_PARAMETERS(PARAMETERS, PATH_TO_ASSEMBLY, PATH_TO_DOT_NET_ROOT, PARAMETERS_OBJECT)	\
	\
	if ((PATH_TO_ASSEMBLY) || (PATH_TO_DOT_NET_ROOT))											\
	{																							\
		(PARAMETERS_OBJECT).size = sizeof(struct _paths);										\
		(PARAMETERS_OBJECT).path_to_assembly = (PATH_TO_ASSEMBLY);								\
		(PARAMETERS_OBJECT).path_to_dot_net_root = (PATH_TO_DOT_NET_ROOT);						\
		(PARAMETERS) = (const void*)&(PARAMETERS_OBJECT);										\
	}

typedef int32_t(net_host_calling_convention* get_hostfxr_path_type)(
	type_of_element* output, size_t* output_size, const void* parameters);

uint8_t net_host_load(const type_of_element* path_to_net_host_library,
					  const type_of_element* path_to_assembly, const type_of_element* path_to_dot_net_root,
					  void** net_host_object, void* path_to_host_fxr)
{
	if (!path_to_net_host_library || !path_to_host_fxr)
	{
		return 0;
	}

	void* net_host_object_ = NULL;
#if defined(_WIN32)
	net_host_object_ = shared_object_load_wchar_t(path_to_net_host_library);
#else
	net_host_object_ = shared_object_load(path_to_net_host_library);
#endif

	if (!net_host_object_)
	{
		return 0;
	}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4055)
#endif
	get_hostfxr_path_type get_hostfxr_path =
		(get_hostfxr_path_type)shared_object_get_procedure_address(
			net_host_object_, (const uint8_t*)"get_hostfxr_path");
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(default: 4055)
#endif

	if (!get_hostfxr_path)
	{
		shared_object_unload(net_host_object_);
		return 0;
	}

	const void* parameters = NULL;
	SET_PARAMETERS(parameters, path_to_assembly, path_to_dot_net_root, paths);
	const ptrdiff_t size = buffer_size(path_to_host_fxr);

	if (!buffer_append(path_to_host_fxr, NULL, FILENAME_MAX * sizeof(type_of_element)))
	{
		shared_object_unload(net_host_object_);
		return 0;
	}

	int32_t result = 0;
	size_t required_size = FILENAME_MAX;
	type_of_element* out = (type_of_element*)buffer_data(path_to_host_fxr, size);
	result = get_hostfxr_path(out, &required_size, parameters);

	if (IS_HOST_FAILED(result))
	{
		if ((int32_t)net_HostApiBufferTooSmall != result ||
			required_size < FILENAME_MAX ||
			!buffer_resize(path_to_host_fxr, size) ||
			!buffer_append(path_to_host_fxr, NULL, required_size * sizeof(type_of_element)))
		{
			shared_object_unload(net_host_object_);
			return 0;
		}

		out = (type_of_element*)buffer_data(path_to_host_fxr, size);
		result = get_hostfxr_path(out, &required_size, parameters);

		if (IS_HOST_FAILED(result))
		{
			shared_object_unload(net_host_object_);
			return 0;
		}
	}

	if (net_host_object)
	{
		*net_host_object = net_host_object_;
	}
	else
	{
		shared_object_unload(net_host_object_);
	}

	required_size *= sizeof(type_of_element);
	return buffer_resize(path_to_host_fxr, size + required_size);
}

typedef int32_t(calling_convention* hostfxr_close_type)(const void* context);
typedef int32_t(calling_convention* hostfxr_get_available_sdks_type)(
	const type_of_element* exe_dir, hostfxr_get_available_sdks_result_type result);
typedef int32_t(calling_convention* hostfxr_get_native_search_directories_type)(
	const int32_t argc, const type_of_element** argv, type_of_element* output,
	int32_t output_size, int32_t* required_output_size);
typedef int32_t(calling_convention* hostfxr_get_runtime_delegate_type)(
	const void* context, int32_t type_of_delegate, hostfxr_delegate_function_type* the_delegate);
typedef int32_t(calling_convention* hostfxr_get_runtime_properties_type)(
	const void* context, size_t* count,
	type_of_element** keys, type_of_element** values);
typedef int32_t(calling_convention* hostfxr_get_runtime_property_value_type)(
	const void* context, const type_of_element* name, const type_of_element** value);
typedef int32_t(calling_convention* hostfxr_initialize_for_dotnet_command_line_type)(
	int32_t argc, const type_of_element** argv,
	const void* parameters, void** context);
typedef int32_t(calling_convention* hostfxr_initialize_for_runtime_config_type)(
	const type_of_element* runtime_config_path,
	const void* parameters, void** context);
typedef int32_t(calling_convention* hostfxr_main_type)(const int32_t argc, const type_of_element** argv);
typedef int32_t(calling_convention* hostfxr_main_bundle_startupinfo_type)(
	const int32_t argc, const type_of_element** argv, const type_of_element* host_path,
	const type_of_element* dotnet_root, const type_of_element* app_path, int64_t bundle_header_offset);
typedef int32_t(calling_convention* hostfxr_main_startupinfo_type)(
	const int32_t argc, const type_of_element** argv, const type_of_element* host_path,
	const type_of_element* dotnet_root, const type_of_element* app_path);
typedef int32_t(calling_convention* hostfxr_resolve_sdk_type)(
	const type_of_element* exe_dir, const type_of_element* working_dir,
	type_of_element* output, int32_t output_size);
typedef int32_t(calling_convention* hostfxr_resolve_sdk2_type)(
	const type_of_element* exe_dir, const type_of_element* working_dir,
	int32_t flags, hostfxr_resolve_sdk2_result_type result);
typedef int32_t(calling_convention* hostfxr_run_app_type)(const void* context);
typedef error_writer_type(calling_convention* hostfxr_set_error_writer_type)(
	error_writer_type writer);
typedef int32_t(calling_convention* hostfxr_set_runtime_property_value_type)(
	const void* context, const type_of_element* name, const type_of_element* value);
typedef int32_t(calling_convention* hostfxr_get_dotnet_environment_information_type)(
	const type_of_element* dotnet_root, void* reserved,
	hostfxr_get_dotnet_environment_information_result_type result,
	void* result_context);

struct host_fxr
{
	void* shared_object;
	/**/
	hostfxr_close_type hostfxr_close;
	hostfxr_get_available_sdks_type hostfxr_get_available_sdks;
	hostfxr_get_native_search_directories_type hostfxr_get_native_search_directories;
	hostfxr_get_runtime_delegate_type hostfxr_get_runtime_delegate;
	hostfxr_get_runtime_properties_type hostfxr_get_runtime_properties;
	hostfxr_get_runtime_property_value_type hostfxr_get_runtime_property_value;
	hostfxr_initialize_for_dotnet_command_line_type hostfxr_initialize_for_dotnet_command_line;
	hostfxr_initialize_for_runtime_config_type hostfxr_initialize_for_runtime_config;
	hostfxr_main_type hostfxr_main;
	hostfxr_main_bundle_startupinfo_type hostfxr_main_bundle_startupinfo;
	hostfxr_main_startupinfo_type hostfxr_main_startupinfo;
	hostfxr_resolve_sdk_type hostfxr_resolve_sdk;
	hostfxr_resolve_sdk2_type hostfxr_resolve_sdk2;
	hostfxr_run_app_type hostfxr_run_app;
	hostfxr_set_error_writer_type hostfxr_set_error_writer;
	hostfxr_set_runtime_property_value_type hostfxr_set_runtime_property_value;
	hostfxr_get_dotnet_environment_information_type hostfxr_get_dotnet_environment_information;
};

enum host_fxr_functions
{
	hostfxr_close_, hostfxr_get_available_sdks_, hostfxr_get_native_search_directories_,
	hostfxr_get_runtime_delegate_, hostfxr_get_runtime_properties_,
	hostfxr_get_runtime_property_value_, hostfxr_initialize_for_dotnet_command_line_,
	hostfxr_initialize_for_runtime_config_, hostfxr_main_, hostfxr_main_bundle_startupinfo_,
	hostfxr_main_startupinfo_, hostfxr_resolve_sdk_, hostfxr_resolve_sdk2_, hostfxr_run_app_,
	hostfxr_set_error_writer_, hostfxr_set_runtime_property_value_,
	hostfxr_get_dotnet_environment_information_
};

static const uint8_t* host_fxr_functions_string[] =
{
	(const uint8_t*)"hostfxr_close",
	(const uint8_t*)"hostfxr_get_available_sdks",
	(const uint8_t*)"hostfxr_get_native_search_directories",
	(const uint8_t*)"hostfxr_get_runtime_delegate",
	(const uint8_t*)"hostfxr_get_runtime_properties",
	(const uint8_t*)"hostfxr_get_runtime_property_value",
	(const uint8_t*)"hostfxr_initialize_for_dotnet_command_line",
	(const uint8_t*)"hostfxr_initialize_for_runtime_config",
	(const uint8_t*)"hostfxr_main",
	(const uint8_t*)"hostfxr_main_bundle_startupinfo",
	(const uint8_t*)"hostfxr_main_startupinfo",
	(const uint8_t*)"hostfxr_resolve_sdk",
	(const uint8_t*)"hostfxr_resolve_sdk2",
	(const uint8_t*)"hostfxr_run_app",
	(const uint8_t*)"hostfxr_set_error_writer",
	(const uint8_t*)"hostfxr_set_runtime_property_value",
	(const uint8_t*)"hostfxr_get_dotnet_environment_info"
};

uint8_t host_fx_resolver_load(
	const type_of_element* path_to_host_fxr, void* ptr_to_host_fxr_object, ptrdiff_t size)
{
	if (!path_to_host_fxr ||
		!ptr_to_host_fxr_object ||
		size < (ptrdiff_t)sizeof(struct host_fxr))
	{
		return 0;
	}

	void* host_fx_object = NULL;
#if defined(_WIN32)
	host_fx_object = shared_object_load_wchar_t(path_to_host_fxr);
#else
	host_fx_object = shared_object_load(path_to_host_fxr);
#endif

	if (!host_fx_object)
	{
		return 0;
	}

	memset(ptr_to_host_fxr_object, 0, sizeof(struct host_fxr));
	struct host_fxr* ptr_to_host_fxr_object_ = (struct host_fxr*)ptr_to_host_fxr_object;

	for (uint8_t i = 0, count = COUNT_OF(host_fxr_functions_string); i < count; ++i)
	{
		void* address = shared_object_get_procedure_address(host_fx_object, host_fxr_functions_string[i]);

		if (!address)
		{
			continue;
		}

		switch (i)
		{
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4055)
#endif

			case hostfxr_close_:
				ptr_to_host_fxr_object_->hostfxr_close =
					(hostfxr_close_type)address;
				break;

			case hostfxr_get_available_sdks_:
				ptr_to_host_fxr_object_->hostfxr_get_available_sdks =
					(hostfxr_get_available_sdks_type)address;
				break;

			case hostfxr_get_native_search_directories_:
				ptr_to_host_fxr_object_->hostfxr_get_native_search_directories =
					(hostfxr_get_native_search_directories_type)address;
				break;

			case hostfxr_get_runtime_delegate_:
				ptr_to_host_fxr_object_->hostfxr_get_runtime_delegate =
					(hostfxr_get_runtime_delegate_type)address;
				break;

			case hostfxr_get_runtime_properties_:
				ptr_to_host_fxr_object_->hostfxr_get_runtime_properties =
					(hostfxr_get_runtime_properties_type)address;
				break;

			case hostfxr_get_runtime_property_value_:
				ptr_to_host_fxr_object_->hostfxr_get_runtime_property_value =
					(hostfxr_get_runtime_property_value_type)address;
				break;

			case hostfxr_initialize_for_dotnet_command_line_:
				ptr_to_host_fxr_object_->hostfxr_initialize_for_dotnet_command_line =
					(hostfxr_initialize_for_dotnet_command_line_type)address;
				break;

			case hostfxr_initialize_for_runtime_config_:
				ptr_to_host_fxr_object_->hostfxr_initialize_for_runtime_config =
					(hostfxr_initialize_for_runtime_config_type)address;
				break;

			case hostfxr_main_:
				ptr_to_host_fxr_object_->hostfxr_main =
					(hostfxr_main_type)address;
				break;

			case hostfxr_main_bundle_startupinfo_:
				ptr_to_host_fxr_object_->hostfxr_main_bundle_startupinfo =
					(hostfxr_main_bundle_startupinfo_type)address;
				break;

			case hostfxr_main_startupinfo_:
				ptr_to_host_fxr_object_->hostfxr_main_startupinfo =
					(hostfxr_main_startupinfo_type)address;
				break;

			case hostfxr_resolve_sdk_:
				ptr_to_host_fxr_object_->hostfxr_resolve_sdk =
					(hostfxr_resolve_sdk_type)address;
				break;

			case hostfxr_resolve_sdk2_:
				ptr_to_host_fxr_object_->hostfxr_resolve_sdk2 =
					(hostfxr_resolve_sdk2_type)address;
				break;

			case hostfxr_run_app_:
				ptr_to_host_fxr_object_->hostfxr_run_app =
					(hostfxr_run_app_type)address;
				break;

			case hostfxr_set_error_writer_:
				ptr_to_host_fxr_object_->hostfxr_set_error_writer =
					(hostfxr_set_error_writer_type)address;
				break;

			case hostfxr_set_runtime_property_value_:
				ptr_to_host_fxr_object_->hostfxr_set_runtime_property_value =
					(hostfxr_set_runtime_property_value_type)address;
				break;

			case hostfxr_get_dotnet_environment_information_:
				ptr_to_host_fxr_object_->hostfxr_get_dotnet_environment_information =
					(hostfxr_get_dotnet_environment_information_type)address;
				break;
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(default: 4055)
#endif

			default:
				break;
		}
	}

	ptr_to_host_fxr_object_->shared_object = host_fx_object;
	return 1;
}

void host_fx_resolver_unload(void* ptr_to_host_fxr_object)
{
	if (!ptr_to_host_fxr_object)
	{
		return;
	}

	struct host_fxr* ptr_to_host_fxr_object_ = (struct host_fxr*)(ptr_to_host_fxr_object);

	shared_object_unload(ptr_to_host_fxr_object_->shared_object);

	memset(ptr_to_host_fxr_object, 0, sizeof(struct host_fxr));
}

uint8_t host_fx_resolver_is_function_exists(
	const void* ptr_to_host_fxr_object, const uint8_t* function_name, uint8_t function_name_length)
{
	static const uint8_t host_fxr_functions_lengths[] =
	{
		13,
		26,
		37,
		28,
		30,
		34,
		42,
		37,
		12,
		31,
		24,
		19,
		20,
		15,
		24,
		34,
		35
	};

	if (!ptr_to_host_fxr_object ||
		!function_name ||
		!function_name_length)
	{
		return 0;
	}

	const struct host_fxr* ptr_to_host_fxr_object_ = (const struct host_fxr*)ptr_to_host_fxr_object;

	for (uint8_t i = 0, count = COUNT_OF(host_fxr_functions_string); i < count; ++i)
	{
		if (host_fxr_functions_lengths[i] != function_name_length ||
			0 != memcmp(host_fxr_functions_string[i], function_name, function_name_length))
		{
			continue;
		}

		switch (i)
		{
			case hostfxr_close_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_close;

			case hostfxr_get_available_sdks_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_available_sdks;

			case hostfxr_get_native_search_directories_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_native_search_directories;

			case hostfxr_get_runtime_delegate_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_runtime_delegate;

			case hostfxr_get_runtime_properties_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_runtime_properties;

			case hostfxr_get_runtime_property_value_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_runtime_property_value;

			case hostfxr_initialize_for_dotnet_command_line_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_initialize_for_dotnet_command_line;

			case hostfxr_initialize_for_runtime_config_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_initialize_for_runtime_config;

			case hostfxr_main_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_main;

			case hostfxr_main_bundle_startupinfo_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_main_bundle_startupinfo;

			case hostfxr_main_startupinfo_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_main_startupinfo;

			case hostfxr_resolve_sdk_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_resolve_sdk;

			case hostfxr_resolve_sdk2_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_resolve_sdk2;

			case hostfxr_run_app_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_run_app;

			case hostfxr_set_error_writer_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_set_error_writer;

			case hostfxr_set_runtime_property_value_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_set_runtime_property_value;

			case hostfxr_get_dotnet_environment_information_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_dotnet_environment_information;

			default:
				break;
		}
	}

	return 0;
}

uint8_t int_to_hex_and_byte_representation(int32_t code, void* output)
{
#define CODE_IN_STR_LENGTH 32

	if (!output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, CODE_IN_STR_LENGTH))
	{
		return 0;
	}

	char* out = (char*)buffer_data(output, size);
#if __STDC_LIB_EXT1__
	code = sprintf_s(out, CODE_IN_STR_LENGTH, "0x%x %i %i", code, code, code & 0xFF);
#else
	code = sprintf(out, "0x%x %i %i", code, code, code & 0xFF);
#endif
	return buffer_resize(output, size + (ptrdiff_t)code);
}

uint8_t result_code_to_string(int32_t code, void* output)
{
	if (!output)
	{
		return 0;
	}

	static const int32_t codes[] =
	{
		net_Success,
		net_Success_HostAlreadyInitialized,
		net_Success_DifferentRuntimeProperties,
		win_error_E_INVALIDARG,
		net_InvalidArgFailure,
		net_CoreHostLibLoadFailure,
		net_CoreHostLibMissingFailure,
		net_CoreHostEntryPointFailure,
		net_CoreHostCurHostFindFailure,
		net_CoreClrResolveFailure,
		net_CoreClrBindFailure,
		net_CoreClrInitFailure,
		net_CoreClrExeFailure,
		net_ResolverInitFailure,
		net_ResolverResolveFailure,
		net_LibHostCurExeFindFailure,
		net_LibHostInitFailure,
		net_LibHostSdkFindFailure,
		net_LibHostInvalidArgs,
		net_InvalidConfigFile,
		net_AppArgNotRunnable,
		net_AppHostExeNotBoundFailure,
		net_FrameworkMissingFailure,
		net_HostApiFailed,
		net_HostApiBufferTooSmall,
		net_LibHostUnknownCommand,
		net_LibHostAppRootFindFailure,
		net_SdkResolverResolveFailure,
		net_FrameworkCompatFailure,
		net_FrameworkCompatRetry,
		net_AppHostExeNotBundle,
		net_BundleExtractionFailure,
		net_BundleExtractionIOError,
		net_LibHostDuplicateProperty,
		net_HostApiUnsupportedVersion,
		net_HostInvalidState,
		net_HostPropertyNotFound,
		net_CoreHostIncompatibleConfig,
		net_HostApiUnsupportedScenario
	};
	/**/
	static const uint8_t* strings[] =
	{
		(const uint8_t*)"[net]::Success",
		(const uint8_t*)"[net]::Success_HostAlreadyInitialized",
		(const uint8_t*)"[net]::Success_DifferentRuntimeProperties",
		(const uint8_t*)"[win]::E_INVALIDARG",
		(const uint8_t*)"[net]::InvalidArgFailure",
		(const uint8_t*)"[net]::CoreHostLibLoadFailure",
		(const uint8_t*)"[net]::CoreHostLibMissingFailure",
		(const uint8_t*)"[net]::CoreHostEntryPointFailure",
		(const uint8_t*)"[net]::CoreHostCurHostFindFailure",
		(const uint8_t*)"[net]::CoreClrResolveFailure",
		(const uint8_t*)"[net]::CoreClrBindFailure",
		(const uint8_t*)"[net]::CoreClrInitFailure",
		(const uint8_t*)"[net]::CoreClrExeFailure",
		(const uint8_t*)"[net]::ResolverInitFailure",
		(const uint8_t*)"[net]::ResolverResolveFailure",
		(const uint8_t*)"[net]::LibHostCurExeFindFailure",
		(const uint8_t*)"[net]::LibHostInitFailure",
		(const uint8_t*)"[net]::LibHostSdkFindFailure",
		(const uint8_t*)"[net]::LibHostInvalidArgs",
		(const uint8_t*)"[net]::InvalidConfigFile",
		(const uint8_t*)"[net]::AppArgNotRunnable",
		(const uint8_t*)"[net]::AppHostExeNotBoundFailure",
		(const uint8_t*)"[net]::FrameworkMissingFailure",
		(const uint8_t*)"[net]::HostApiFailed",
		(const uint8_t*)"[net]::HostApiBufferTooSmall",
		(const uint8_t*)"[net]::LibHostUnknownCommand",
		(const uint8_t*)"[net]::LibHostAppRootFindFailure",
		(const uint8_t*)"[net]::SdkResolverResolveFailure",
		(const uint8_t*)"[net]::FrameworkCompatFailure",
		(const uint8_t*)"[net]::FrameworkCompatRetry",
		(const uint8_t*)"[net]::AppHostExeNotBundle",
		(const uint8_t*)"[net]::BundleExtractionFailure",
		(const uint8_t*)"[net]::BundleExtractionIOError",
		(const uint8_t*)"[net]::LibHostDuplicateProperty",
		(const uint8_t*)"[net]::HostApiUnsupportedVersion",
		(const uint8_t*)"[net]::HostInvalidState",
		(const uint8_t*)"[net]::HostPropertyNotFound",
		(const uint8_t*)"[net]::CoreHostIncompatibleConfig",
		(const uint8_t*)"[net]::HostApiUnsupportedScenario"
	};

	for (uint8_t i = 0, count = COUNT_OF(codes); i < count; ++i)
	{
		if (codes[i] != code)
		{
			continue;
		}

		if (!common_append_string_to_buffer(strings[i], output) ||
			!buffer_append_char(output, " (", 2) ||
			!int_to_hex_and_byte_representation(code, output) ||
			!buffer_push_back(output, ')'))
		{
			return 0;
		}

		return 1;
	}

	return int_to_hex_and_byte_representation(code, output);
}

int32_t host_fxr_close(const void* ptr_to_host_fxr_object, const void* context)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_close)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_close(context);
	}

	return -1;
}

int32_t host_fxr_get_available_sdks(
	const void* ptr_to_host_fxr_object, const type_of_element* exe_dir,
	hostfxr_get_available_sdks_result_type result)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_available_sdks)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_available_sdks(exe_dir, result);
	}

	return -1;
}

int32_t host_fxr_get_native_search_directories(
	const void* ptr_to_host_fxr_object, const int32_t argc, const type_of_element** argv,
	type_of_element* output, int32_t output_size, int32_t* required_output_size)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_native_search_directories)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_native_search_directories(
				   argc, argv, output, output_size, required_output_size);
	}

	return -1;
}

int32_t host_fxr_get_runtime_delegate(
	const void* ptr_to_host_fxr_object, const void* context,
	int32_t type_of_delegate, hostfxr_delegate_function_type* the_delegate)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_delegate)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_delegate(
				   context, type_of_delegate, the_delegate);
	}

	return -1;
}

int32_t host_fxr_get_runtime_properties(
	const void* ptr_to_host_fxr_object, const void* context,
	size_t* count, type_of_element** keys, type_of_element** values)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_properties)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_properties(
				   context, count, keys, values);
	}

	return -1;
}

int32_t host_fxr_get_runtime_property_value(
	const void* ptr_to_host_fxr_object, const void* context, const type_of_element* name,
	const type_of_element** value)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_property_value)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_property_value(
				   context, name, value);
	}

	return -1;
}

int32_t host_fxr_initialize_for_dotnet_command_line(
	const void* ptr_to_host_fxr_object, int32_t argc, const type_of_element** argv,
	const void* parameters, void** context)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_initialize_for_dotnet_command_line)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_initialize_for_dotnet_command_line(
				   argc, argv, parameters, context);
	}

	return -1;
}

int32_t host_fxr_initialize_for_dotnet_command_line_parameters_in_parts(
	const void* ptr_to_host_fxr_object, int32_t argc, const type_of_element** argv,
	const type_of_element* path_to_assembly, const type_of_element* path_to_dot_net_root, void** context)
{
	const void* parameters = NULL;
	SET_PARAMETERS(parameters, path_to_assembly, path_to_dot_net_root, paths);
	return host_fxr_initialize_for_dotnet_command_line(ptr_to_host_fxr_object, argc, argv, parameters, context);
}

int32_t host_fxr_initialize_for_runtime_config(
	const void* ptr_to_host_fxr_object, const type_of_element* runtime_config_path,
	const void* parameters, void** context)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_initialize_for_runtime_config)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_initialize_for_runtime_config(
				   runtime_config_path, parameters, context);
	}

	return -1;
}

int32_t host_fxr_initialize_for_runtime_config_parameters_in_parts(
	const void* ptr_to_host_fxr_object, const type_of_element* runtime_config_path,
	const type_of_element* path_to_assembly, const type_of_element* path_to_dot_net_root, void** context)
{
	const void* parameters = NULL;
	SET_PARAMETERS(parameters, path_to_assembly, path_to_dot_net_root, paths);
	return host_fxr_initialize_for_runtime_config(
			   ptr_to_host_fxr_object, runtime_config_path, parameters, context);
}

int32_t host_fxr_main(const void* ptr_to_host_fxr_object, const int32_t argc, const type_of_element** argv)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main(argc, argv);
	}

	return -1;
}

int32_t host_fxr_main_bundle_startupinfo(
	const void* ptr_to_host_fxr_object, const int32_t argc, const type_of_element** argv,
	const type_of_element* host_path, const type_of_element* dotnet_root,
	const type_of_element* app_path, int64_t bundle_header_offset)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main_bundle_startupinfo)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main_bundle_startupinfo(
				   argc, argv, host_path, dotnet_root, app_path, bundle_header_offset);
	}

	return -1;
}

int32_t host_fxr_main_startupinfo(
	const void* ptr_to_host_fxr_object, const int32_t argc, const type_of_element** argv,
	const type_of_element* host_path, const type_of_element* dotnet_root,
	const type_of_element* app_path)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main_startupinfo)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main_startupinfo(
				   argc, argv, host_path, dotnet_root, app_path);
	}

	return -1;
}

int32_t host_fxr_resolve_sdk(
	const void* ptr_to_host_fxr_object, const type_of_element* exe_dir,
	const type_of_element* working_dir, type_of_element* output, int32_t output_size)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_resolve_sdk)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_resolve_sdk(
				   exe_dir, working_dir, output, output_size);
	}

	return -1;
}

int32_t host_fxr_resolve_sdk2(
	const void* ptr_to_host_fxr_object, const type_of_element* exe_dir,
	const type_of_element* working_dir, int32_t flags,
	hostfxr_resolve_sdk2_result_type result)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_resolve_sdk2)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_resolve_sdk2(
				   exe_dir, working_dir, flags, result);
	}

	return -1;
}

int32_t host_fxr_run_app(const void* ptr_to_host_fxr_object, const void* context)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_run_app)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_run_app(context);
	}

	return -1;
}

error_writer_type host_fxr_set_error_writer(
	const void* ptr_to_host_fxr_object, error_writer_type writer)
{
	error_writer_type result = NULL;

	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_set_error_writer)
	{
		result = ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_set_error_writer(writer);
	}

	return result;
}

int32_t host_fxr_set_runtime_property_value(
	const void* ptr_to_host_fxr_object, const void* context, const type_of_element* name,
	const type_of_element* value)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_set_runtime_property_value)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_set_runtime_property_value(
				   context, name, value);
	}

	return -1;
}

int32_t host_fxr_get_dotnet_environment_information(
	const void* ptr_to_host_fxr_object,
	const type_of_element* dotnet_root,
	hostfxr_get_dotnet_environment_information_result_type result,
	void* result_context)
{
	if (ptr_to_host_fxr_object &&
		((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_dotnet_environment_information)
	{
		return ((const struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_dotnet_environment_information(
				   dotnet_root, NULL, result, result_context);
	}

	return -1;
}
