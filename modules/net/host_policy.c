/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "host_policy.h"

#include "common.h"
#include "shared_object.h"

#include <string.h>

#if defined(_WIN32)
extern void* shared_object_load_wchar_t(const wchar_t* path);
#endif

typedef void(*corehost_resolve_component_dependencies_result_type)(
	const type_of_element* assembly_paths,
	const type_of_element* native_search_paths,
	const type_of_element* resource_search_paths);

typedef int32_t(*corehost_initialize_type)(void* host_interface);
/*TODO: typedef (*corehost_load_type);*/
typedef int32_t(*corehost_main_type)(
	const int32_t argc, const type_of_element** argv);
typedef int32_t(*corehost_main_with_output_buffer_type)(
	const int32_t argc, const type_of_element** argv,
	type_of_element* the_buffer,
	int32_t buffer_size, int32_t* required_buffer_size);
typedef int32_t(*corehost_resolve_component_dependencies_type)(
	const type_of_element* component_main_assembly_path,
	corehost_resolve_component_dependencies_result_type result);
/*TODO: typedef (*corehost_set_error_writer_type);*/
typedef int32_t(*corehost_unload_type)();

struct host_policy
{
	void* shared_object;
	/**/
	corehost_initialize_type corehost_initialize;
	/*TODO: corehost_load_type*/void* corehost_load;
	corehost_main_type corehost_main;
	corehost_main_with_output_buffer_type corehost_main_with_output_buffer;
	corehost_resolve_component_dependencies_type corehost_resolve_component_dependencies;
	/*TODO: corehost_set_error_writer_type*/void* corehost_set_error_writer;
	corehost_unload_type corehost_unload;
};

enum host_policy_functions
{
	host_policy_corehost_initialize_, host_policy_corehost_load_,
	host_policy_corehost_main_, host_policy_corehost_main_with_output_buffer_,
	host_policy_corehost_resolve_component_dependencies_,
	host_policy_corehost_set_error_writer_, host_policy_corehost_unload_
};

static const uint8_t* host_policy_functions_string[] =
{
	(const uint8_t*)"corehost_initialize",
	(const uint8_t*)"corehost_load",
	(const uint8_t*)"corehost_main",
	(const uint8_t*)"corehost_main_with_output_buffer",
	(const uint8_t*)"corehost_resolve_component_dependencies",
	(const uint8_t*)"corehost_set_error_writer",
	(const uint8_t*)"corehost_unload"
};

uint8_t host_policy_load(
	const type_of_element* path_to_host_policy, void* ptr_to_host_policy_object, ptrdiff_t size)
{
	if (!path_to_host_policy ||
		!ptr_to_host_policy_object ||
		size < (ptrdiff_t)sizeof(struct host_policy))
	{
		return 0;
	}

	void* host_policy_object = NULL;
#if defined(_WIN32)
	host_policy_object = shared_object_load_wchar_t(path_to_host_policy);
#else
	host_policy_object = shared_object_load(path_to_host_policy);
#endif

	if (!host_policy_object)
	{
		return 0;
	}

	memset(ptr_to_host_policy_object, 0, sizeof(struct host_policy));
	struct host_policy* ptr_to_host_policy_object_ = (struct host_policy*)ptr_to_host_policy_object;

	for (uint8_t i = 0, count = COUNT_OF(host_policy_functions_string); i < count; ++i)
	{
		void* address = shared_object_get_procedure_address(host_policy_object, host_policy_functions_string[i]);

		if (!address)
		{
			continue;
		}

		switch (i)
		{
			case host_policy_corehost_initialize_:
				ptr_to_host_policy_object_->corehost_initialize =
					(corehost_initialize_type)address;
				break;

			case host_policy_corehost_load_:
				ptr_to_host_policy_object_->corehost_load =
					/*TODO: (corehost_load_type)*/address;
				break;

			case host_policy_corehost_main_:
				ptr_to_host_policy_object_->corehost_main  =
					(corehost_main_type)address;
				break;

			case host_policy_corehost_main_with_output_buffer_:
				ptr_to_host_policy_object_->corehost_main_with_output_buffer =
					(corehost_main_with_output_buffer_type)address;
				break;

			case host_policy_corehost_resolve_component_dependencies_:
				ptr_to_host_policy_object_->corehost_resolve_component_dependencies =
					(corehost_resolve_component_dependencies_type)address;
				break;

			case host_policy_corehost_set_error_writer_:
				ptr_to_host_policy_object_->corehost_set_error_writer =
					/*TODO: (corehost_set_error_writer_type)*/address;
				break;

			case host_policy_corehost_unload_:
				ptr_to_host_policy_object_->corehost_unload =
					(corehost_unload_type)address;
				break;

			default:
				break;
		}
	}

	ptr_to_host_policy_object_->shared_object = host_policy_object;
	return 1;
}

void host_policy_unload(void* ptr_to_host_policy_object)
{
	if (!ptr_to_host_policy_object)
	{
		return;
	}

	struct host_policy* ptr_to_host_policy_object_ = (struct host_policy*)(ptr_to_host_policy_object);

	shared_object_unload(ptr_to_host_policy_object_->shared_object);

	memset(ptr_to_host_policy_object, 0, sizeof(struct host_policy));
}
