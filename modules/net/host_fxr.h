/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __HOST_FXR_H__
#define __HOST_FXR_H__

#if defined(_WIN32)
#include <wchar.h>
#endif

#include <stddef.h>
#include <stdint.h>

struct buffer;

#if defined(_WIN32)
#define type_of_element wchar_t
#define calling_convention __cdecl
#define delegate_calling_convention __stdcall
#define host_fxr_PATH_DELIMITER ';'
#define host_fxr_PATH_DELIMITER_wchar_t L';'
#else
#define type_of_element uint8_t
#define calling_convention
#define delegate_calling_convention
#define host_fxr_PATH_DELIMITER ':'
#endif

uint8_t net_host_load(
	const type_of_element* path_to_net_host_library,
	const type_of_element* path_to_assembly,
	const type_of_element* path_to_dot_net_root,
	void** net_host_object,
	struct buffer* path_to_host_fxr);

uint8_t host_fx_resolver_load(
	const type_of_element* path_to_host_fxr,
	void* ptr_to_host_fxr_object,
	ptrdiff_t size);
void host_fx_resolver_unload(void* ptr_to_host_fxr_object);

uint8_t host_fx_resolver_is_function_exists(
	const void* ptr_to_host_fxr_object,
	const uint8_t* function_name,
	uint8_t function_name_length);

enum hostfxr_resolve_sdk2_result_keys
{
	host_fxr_resolved_sdk_dir,
	host_fxr_global_json_path
};

enum hostfxr_status_code
{
	host_fxr_Success,
	host_fxr_Success_HostAlreadyInitialized,
	host_fxr_Success_DifferentRuntimeProperties,
	/**/
	win_error_E_INVALIDARG = 0x80070057,
	/**/
	host_fxr_InvalidArgFailure = 0x80008081,
	host_fxr_CoreHostLibLoadFailure,
	host_fxr_CoreHostLibMissingFailure,
	host_fxr_CoreHostEntryPointFailure,
	host_fxr_CoreHostCurHostFindFailure,
	host_fxr_CoreClrResolveFailure = 0x80008087,
	host_fxr_CoreClrBindFailure,
	host_fxr_CoreClrInitFailure,
	host_fxr_CoreClrExeFailure,
	host_fxr_ResolverInitFailure,
	host_fxr_ResolverResolveFailure,
	host_fxr_LibHostCurExeFindFailure,
	host_fxr_LibHostInitFailure,
	host_fxr_LibHostSdkFindFailure = 0x80008091,
	host_fxr_LibHostInvalidArgs,
	host_fxr_InvalidConfigFile,
	host_fxr_AppArgNotRunnable,
	host_fxr_AppHostExeNotBoundFailure,
	host_fxr_FrameworkMissingFailure,
	host_fxr_HostApiFailed,
	host_fxr_HostApiBufferTooSmall,
	host_fxr_LibHostUnknownCommand,
	host_fxr_LibHostAppRootFindFailure,
	host_fxr_SdkResolverResolveFailure,
	host_fxr_FrameworkCompatFailure,
	host_fxr_FrameworkCompatRetry,
	host_fxr_AppHostExeNotBundle,
	host_fxr_BundleExtractionFailure,
	host_fxr_BundleExtractionIOError,
	host_fxr_LibHostDuplicateProperty,
	host_fxr_HostApiUnsupportedVersion,
	host_fxr_HostInvalidState,
	host_fxr_HostPropertyNotFound,
	host_fxr_CoreHostIncompatibleConfig,
	host_fxr_HostApiUnsupportedScenario
};

#define HOST_FX_RESOLVER_NON_SUCCESS(RESULT) ((RESULT) < host_fxr_Success || host_fxr_Success_DifferentRuntimeProperties < (RESULT))

enum hostfxr_delegate_types
{
	host_fxr_hdt_com_activation,
	host_fxr_hdt_load_in_memory_assembly,
	host_fxr_hdt_winrt_activation,
	host_fxr_hdt_com_register,
	host_fxr_hdt_com_unregister,
	host_fxr_hdt_load_assembly_and_get_function_pointer,
	host_fxr_hdt_get_function_pointer
};

typedef void(calling_convention* hostfxr_get_available_sdks_result_type)(
	int32_t count, const type_of_element** directories);
typedef int32_t(delegate_calling_convention* hostfxr_delegate_function_type)(
	const type_of_element* assembly_path,
	const type_of_element* type_name,
	const type_of_element* method_name,
	const type_of_element* delegate_type_name,
	void* unused,
	void** the_delegate);
typedef void(calling_convention* hostfxr_resolve_sdk2_result_type)(
	int32_t key, const type_of_element* value);
typedef void(calling_convention* hostfxr_error_writer_type)(
	const type_of_element* message);

int32_t host_fxr_close(
	const void* ptr_to_host_fxr_object,
	const void* context);
int32_t host_fxr_get_available_sdks(
	const void* ptr_to_host_fxr_object,
	const type_of_element* exe_dir,
	hostfxr_get_available_sdks_result_type result);
int32_t host_fxr_get_native_search_directories(
	const void* ptr_to_host_fxr_object,
	const int32_t argc,
	const type_of_element** argv,
	type_of_element* output,
	int32_t output_size,
	int32_t* required_output_size);
int32_t host_fxr_get_runtime_delegate(
	const void* ptr_to_host_fxr_object,
	const void* context,
	int32_t type_of_delegate,
	hostfxr_delegate_function_type* the_delegate);
int32_t host_fxr_get_runtime_properties(
	const void* ptr_to_host_fxr_object,
	const void* context,
	size_t* count,
	type_of_element** keys,
	type_of_element** values);
int32_t host_fxr_get_runtime_property_value(
	const void* ptr_to_host_fxr_object,
	const void* context,
	const type_of_element* name,
	const type_of_element** value);
int32_t host_fxr_initialize_for_dotnet_command_line(
	const void* ptr_to_host_fxr_object,
	int32_t argc,
	const type_of_element** argv,
	const void* parameters,
	void** context);
int32_t host_fxr_initialize_for_dotnet_command_line_parameters_in_parts(
	const void* ptr_to_host_fxr_object,
	int32_t argc,
	const type_of_element** argv,
	const type_of_element* path_to_assembly,
	const type_of_element* path_to_dot_net_root,
	void** context);
int32_t host_fxr_initialize_for_runtime_config(
	const void* ptr_to_host_fxr_object,
	const type_of_element* runtime_config_path,
	const void* parameters,
	void** context);
int32_t host_fxr_initialize_for_runtime_config_parameters_in_parts(
	const void* ptr_to_host_fxr_object,
	const type_of_element* runtime_config_path,
	const type_of_element* path_to_assembly,
	const type_of_element* path_to_dot_net_root,
	void** context);
int32_t host_fxr_main(
	const void* ptr_to_host_fxr_object,
	const int32_t argc,
	const type_of_element** argv);
int32_t host_fxr_main_bundle_startupinfo(
	const void* ptr_to_host_fxr_object,
	const int32_t argc,
	const type_of_element** argv,
	const type_of_element* host_path,
	const type_of_element* dotnet_root,
	const type_of_element* app_path,
	int64_t bundle_header_offset);
int32_t host_fxr_main_startupinfo(
	const void* ptr_to_host_fxr_object,
	const int32_t argc,
	const type_of_element** argv,
	const type_of_element* host_path,
	const type_of_element* dotnet_root,
	const type_of_element* app_path);
int32_t host_fxr_resolve_sdk(
	const void* ptr_to_host_fxr_object,
	const type_of_element* exe_dir,
	const type_of_element* working_dir,
	type_of_element* output,
	int32_t output_size);
int32_t host_fxr_resolve_sdk2(
	const void* ptr_to_host_fxr_object,
	const type_of_element* exe_dir,
	const type_of_element* working_dir,
	int32_t flags,
	hostfxr_resolve_sdk2_result_type result);
int32_t host_fxr_run_app(
	const void* ptr_to_host_fxr_object,
	const void* context);
hostfxr_error_writer_type host_fxr_set_error_writer(
	const void* ptr_to_host_fxr_object,
	hostfxr_error_writer_type writer);
int32_t host_fxr_set_runtime_property_value(
	const void* ptr_to_host_fxr_object,
	const void* context,
	const type_of_element* name,
	const type_of_element* value);

#endif
