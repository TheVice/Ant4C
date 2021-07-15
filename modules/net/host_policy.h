/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_HOST_POLICY_H__
#define __MODULES_NET_HOST_POLICY_H__

#include "net.common.h"

#include <stddef.h>
#include <stdint.h>

struct buffer;

struct context_contract_type
{
	size_t version;
	int32_t(*get_property_value)(
		const type_of_element* key,
		const type_of_element** value);
	int32_t(*set_property_value)(
		const type_of_element* key,
		const type_of_element* value);
	int32_t(*get_properties)(
		size_t* count,
		const type_of_element** keys,
		const type_of_element** values);
	int32_t(*load_runtime)();
	int32_t(*run_app)(
		const int32_t argc,
		const type_of_element** argv);
	int32_t(*get_runtime_delegate)(
		uint32_t core_clr_delegate_type,
		void** the_delegate);
};

typedef void(*corehost_resolve_component_dependencies_result_type)(
	const type_of_element* assembly_paths,
	const type_of_element* native_search_paths,
	const type_of_element* resource_search_paths);

uint8_t host_policy_load(
	const type_of_element* path_to_host_policy,
	void* ptr_to_host_policy_object,
	ptrdiff_t size);
void host_policy_unload(void* ptr_to_host_policy_object);

uint8_t core_host_is_function_exists(
	const void* ptr_to_host_policy_object,
	const uint8_t* function_name,
	uint8_t function_name_length);

int32_t core_host_initialize(
	const void* ptr_to_host_policy_object,
	const void* init_request,
	int32_t options,
	struct context_contract_type* context_contract);
int32_t core_host_load(
	const void* ptr_to_host_policy_object,
	void* host_interface);
int32_t core_host_main(
	const void* ptr_to_host_policy_object,
	const int32_t argc,
	const type_of_element** argv);
uint8_t core_host_main_with_output_buffer(
	const void* ptr_to_host_policy_object,
	const int32_t argc,
	const type_of_element** argv,
	struct buffer* output);
int32_t core_host_resolve_component_dependencies(
	const void* ptr_to_host_policy_object,
	const type_of_element* component_main_assembly_path,
	corehost_resolve_component_dependencies_result_type core_host_resolve_component_dependencies_callback);
error_writer_type core_host_set_error_writer(
	const void* ptr_to_host_policy_object, error_writer_type writer);
int32_t core_host_unload(const void* ptr_to_host_policy_object);

#endif
