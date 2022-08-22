/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#ifndef __MODULES_NET_NET_HOST_FX_RESOLVER_H__
#define __MODULES_NET_NET_HOST_FX_RESOLVER_H__

#include <stddef.h>
#include <stdint.h>

uint8_t host_fx_resolver_init(
	const uint8_t* path_to_host_fxr,
	uint16_t path_to_host_fxr_length,
	void* tmp,
	void* ptr_to_host_fxr_object,
	ptrdiff_t size);

uint8_t hostfxr_close(
	const void* ptr_to_host_fxr_object,
	const uint8_t* context, uint16_t context_length, void* output);
uint8_t hostfxr_get_available_sdks(
	const void* ptr_to_host_fxr_object,
	const uint8_t* executable_directory, uint16_t executable_directory_length,
	void* output);
uint8_t hostfxr_get_native_search_directories(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_get_runtime_delegate(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_get_runtime_properties(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_get_runtime_property_value(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_initialize_for_dotnet_command_line(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_initialize_for_runtime_config(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_main_bundle_startupinfo(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_main_startupinfo(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_resolve_sdk(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_resolve_sdk2(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_main(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_run_app(
	const void* ptr_to_host_fxr_object,
	const uint8_t* context, uint16_t context_length, void* output);
uint8_t hostfxr_set_error_writer(
	const void* ptr_to_host_fxr_object,
	const uint8_t* error_writer_file_name, uint16_t error_writer_file_name_length,
	void* output);
uint8_t hostfxr_set_runtime_property_value(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);
uint8_t hostfxr_get_dotnet_environment_information(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);

#endif
