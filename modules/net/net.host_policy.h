/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#ifndef __MODULES_NET_NET_HOST_POLICY_H__
#define __MODULES_NET_NET_HOST_POLICY_H__

#include <stddef.h>
#include <stdint.h>

uint8_t host_policy_init(
	const uint8_t* path_to_core_host,
	uint16_t path_to_core_host_length,
	void* tmp,
	void* ptr_to_core_host_object,
	ptrdiff_t size);

uint8_t core_host_context_contract_get_property_value__(
	const void* context_contract,
	const uint8_t* property_name,
	uint16_t property_name_length, void* output);
uint8_t core_host_context_contract_set_property_value__(
	const void* ptr_to_host_policy_object,
	const void* context_contract,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count, void* output);
uint8_t core_host_context_contract_get_properties__(
	const void* context_contract, void* output);
uint8_t core_host_context_contract_load_runtime__(
	const void* context_contract, void* output);
uint8_t core_host_context_contract_run_app__(
	const void* context_contract,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count, void* output);
uint8_t core_host_context_contract_get_runtime_delegate__(
	void* context_contract, const uint8_t* delegate_type,
	uint16_t delegate_type_length, void* output);
uint8_t core_host_initialize__(
	const void* ptr_to_host_policy_object,
	const void* init_request,
	void* context_contract,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count, void* output);
uint8_t core_host_main__(
	const void* ptr_to_host_policy_object,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count, void* output);
uint8_t core_host_main_with_output_buffer__(
	const void* ptr_to_host_policy_object,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count, void* output);
uint8_t core_host_resolve_component_dependencies__(
	const void* ptr_to_host_policy_object,
	const uint8_t* component_main_assembly_path,
	ptrdiff_t component_main_assembly_path_length,
	void* output);
uint8_t core_host_set_error_writer__(
	const void* ptr_to_host_policy_object,
	const uint8_t* error_writer_file_name,
	uint16_t error_writer_file_name_length,
	void* output);

#endif
