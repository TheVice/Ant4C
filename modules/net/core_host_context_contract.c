/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "core_host_context_contract.h"

#include <string.h>

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

uint8_t core_host_context_contract_init(
	void* context_contract, size_t size)
{
	if (!context_contract || size < sizeof(struct context_contract_type))
	{
		return 0;
	}

	memset(context_contract, 0, sizeof(struct context_contract_type));
	struct context_contract_type* context_contract_ = (struct context_contract_type*)context_contract;
	context_contract_->version = sizeof(context_contract);
	/**/
	return 1;
}

int32_t core_host_context_contract_get_property_value(
	const void* context_contract,
	const type_of_element* key,
	const type_of_element** value)
{
	if (!context_contract)
	{
		return -1;
	}

	struct context_contract_type* context_contract_ = (struct context_contract_type*)context_contract;

	if (!context_contract_->get_property_value)
	{
		return 1;
	}

	return context_contract_->get_property_value(key, value);
}

int32_t core_host_context_contract_set_property_value(
	const void* context_contract,
	const type_of_element* key,
	const type_of_element* value)
{
	if (!context_contract)
	{
		return -1;
	}

	struct context_contract_type* context_contract_ = (struct context_contract_type*)context_contract;

	if (!context_contract_->set_property_value)
	{
		return 1;
	}

	return context_contract_->set_property_value(key, value);
}

int32_t core_host_context_contract_get_properties(
	const void* context_contract,
	size_t* count,
	const type_of_element** keys,
	const type_of_element** values)
{
	if (!context_contract)
	{
		return -1;
	}

	struct context_contract_type* context_contract_ = (struct context_contract_type*)context_contract;

	if (!context_contract_->get_properties)
	{
		return 1;
	}

	return context_contract_->get_properties(count, keys, values);
}

int32_t core_host_context_contract_load_runtime(
	const void* context_contract)
{
	if (!context_contract)
	{
		return -1;
	}

	struct context_contract_type* context_contract_ = (struct context_contract_type*)context_contract;

	if (!context_contract_->load_runtime)
	{
		return 1;
	}

	return context_contract_->load_runtime();
}

int32_t core_host_context_contract_run_app(
	const void* context_contract,
	const int32_t argc,
	const type_of_element** argv)
{
	if (!context_contract)
	{
		return -1;
	}

	struct context_contract_type* context_contract_ = (struct context_contract_type*)context_contract;

	if (!context_contract_->run_app)
	{
		return 1;
	}

	return context_contract_->run_app(argc, argv);
}

int32_t core_host_context_contract_get_runtime_delegate(
	const void* context_contract,
	uint32_t core_clr_delegate_type,
	void** the_delegate)
{
	if (!context_contract)
	{
		return -1;
	}

	struct context_contract_type* context_contract_ = (struct context_contract_type*)context_contract;

	if (!context_contract_->get_runtime_delegate)
	{
		return 1;
	}

	return context_contract_->get_runtime_delegate(core_clr_delegate_type, the_delegate);
}

static uint8_t g_core_host_context_contract[CORE_HOST_CONTEXT_CONTRACT_SIZE];

void* core_host_context_contract_get()
{
	return g_core_host_context_contract;
}
