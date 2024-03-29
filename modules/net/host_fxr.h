/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#ifndef __MODULES_NET_HOST_FX_RESOLVER_H__
#define __MODULES_NET_HOST_FX_RESOLVER_H__

#include "net.common.h"

#include <stddef.h>
#include <stdint.h>

uint8_t net_host_load(
	const type_of_element* path_to_net_host_library,
	const type_of_element* path_to_assembly,
	const type_of_element* path_to_dot_net_root,
	void** net_host_object,
	void* path_to_host_fxr);

uint8_t host_fx_resolver_load(
	const type_of_element* path_to_host_fxr,
	void* ptr_to_host_fxr_object,
	ptrdiff_t size);
void host_fx_resolver_unload(void* ptr_to_host_fxr_object);

uint8_t host_fx_resolver_is_function_exists(
	const void* ptr_to_host_fxr_object,
	const uint8_t* function_name,
	uint8_t function_name_length);

uint8_t result_code_to_string(int32_t code, void* output);

enum hostfxr_resolve_sdk2_result_keys
{
	host_fxr_resolved_sdk_dir,
	host_fxr_global_json_path
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

struct hostfxr_dotnet_environment_sdk_information
{
	size_t size;
	const type_of_element* version;
	const type_of_element* path;
};

struct hostfxr_dotnet_environment_framework_information
{
	size_t size;
	const type_of_element* name;
	const type_of_element* version;
	const type_of_element* path;
};

struct hostfxr_dotnet_environment_information
{
	size_t size;
	const type_of_element* hostfxr_version;
	const type_of_element* hostfxr_commit_hash;
	size_t sdk_count;
	const struct hostfxr_dotnet_environment_sdk_information* sdks;
	size_t framework_count;
	const struct hostfxr_dotnet_environment_framework_information* frameworks;
};

typedef void(calling_convention* hostfxr_get_dotnet_environment_information_result_type)(
	const struct hostfxr_dotnet_environment_information* information,
	void* result_context);

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
error_writer_type host_fxr_set_error_writer(
	const void* ptr_to_host_fxr_object,
	error_writer_type writer);
int32_t host_fxr_set_runtime_property_value(
	const void* ptr_to_host_fxr_object,
	const void* context,
	const type_of_element* name,
	const type_of_element* value);
int32_t host_fxr_get_dotnet_environment_information(
	const void* ptr_to_host_fxr_object,
	const type_of_element* dotnet_root,
	hostfxr_get_dotnet_environment_information_result_type result,
	void* result_context);

#endif
