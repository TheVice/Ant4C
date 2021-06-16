/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_HOST_FXR_H__
#define __MODULES_NET_HOST_FXR_H__

#include "net.common.h"

#include <stddef.h>
#include <stdint.h>

struct buffer;

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

uint8_t result_code_to_string(int32_t code, struct buffer* output);

enum hostfxr_resolve_sdk2_result_keys
{
	host_fxr_resolved_sdk_dir,
	host_fxr_global_json_path
};

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

#endif
