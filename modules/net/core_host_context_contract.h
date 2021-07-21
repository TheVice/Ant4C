/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_CORE_HOST_CONTEXT_CONTRACT_H__
#define __MODULES_NET_CORE_HOST_CONTEXT_CONTRACT_H__

#include <stddef.h>
#include <stdint.h>

#include "net.common.h"

#define CORE_HOST_CONTEXT_CONTRACT_SIZE 64

uint8_t core_host_context_contract_init(
	void* context_contract, size_t size);

int32_t core_host_context_contract_get_property_value(
	const void* context_contract,
	const type_of_element* key,
	const type_of_element** value);
int32_t core_host_context_contract_set_property_value(
	const void* context_contract,
	const type_of_element* key,
	const type_of_element* value);
int32_t core_host_context_contract_get_properties(
	const void* context_contract,
	size_t* count,
	const type_of_element** keys,
	const type_of_element** values);
int32_t core_host_context_contract_load_runtime(
	const void* context_contract);
int32_t core_host_context_contract_run_app(
	const void* context_contract,
	const int32_t argc,
	const type_of_element** argv);
int32_t core_host_context_contract_get_runtime_delegate(
	const void* context_contract,
	uint32_t core_clr_delegate_type,
	void** the_delegate);

void* core_host_context_contract_get();

#endif
