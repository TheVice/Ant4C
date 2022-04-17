/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "host_policy.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "shared_object.h"
#if defined(_WIN32)
#include "text_encoding.h"
#include <wchar.h>
#endif

#include <string.h>

#if defined(_WIN32)
extern void* shared_object_load_wchar_t(const wchar_t* path);
#endif

typedef int32_t(calling_convention* corehost_initialize_type)(
	const void* init_request,
	int32_t options, void* context_contract);
typedef int32_t(calling_convention* corehost_load_type)(void* host_interface);
typedef int32_t(calling_convention* corehost_main_type)(
	const int32_t argc, const type_of_element** argv);
typedef int32_t(calling_convention* corehost_main_with_output_buffer_type)(
	const int32_t argc, const type_of_element** argv,
	type_of_element* out,
	int32_t buffer_size, int32_t* required_buffer_size);
typedef int32_t(
	calling_convention* corehost_resolve_component_dependencies_type)(
		const type_of_element* component_main_assembly_path,
		corehost_resolve_component_dependencies_result_type result);
typedef error_writer_type(calling_convention* corehost_set_error_writer_type)(
	error_writer_type error_writer);
typedef int32_t(calling_convention* corehost_unload_type)();

struct host_policy
{
	void* shared_object;
	/**/
	corehost_initialize_type corehost_initialize;
	corehost_load_type corehost_load;
	corehost_main_type corehost_main;
	corehost_main_with_output_buffer_type corehost_main_with_output_buffer;
	corehost_resolve_component_dependencies_type corehost_resolve_component_dependencies;
	corehost_set_error_writer_type corehost_set_error_writer;
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
	const type_of_element* path_to_host_policy,
	void* ptr_to_host_policy_object, ptrdiff_t size)
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
	struct host_policy* ptr_to_host_policy_object_ =
		(struct host_policy*)ptr_to_host_policy_object;

	for (uint8_t i = 0, count = COUNT_OF(host_policy_functions_string); i < count; ++i)
	{
		void* address = shared_object_get_procedure_address(
							host_policy_object, host_policy_functions_string[i]);

		if (!address)
		{
			continue;
		}

		switch (i)
		{
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4055)
#endif

			case host_policy_corehost_initialize_:
				ptr_to_host_policy_object_->corehost_initialize =
					(corehost_initialize_type)address;
				break;

			case host_policy_corehost_load_:
				ptr_to_host_policy_object_->corehost_load =
					(corehost_load_type)address;
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
					(corehost_set_error_writer_type)address;
				break;

			case host_policy_corehost_unload_:
				ptr_to_host_policy_object_->corehost_unload =
					(corehost_unload_type)address;
				break;
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(default: 4055)
#endif

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

uint8_t core_host_is_function_exists(
	const void* ptr_to_host_policy_object,
	const uint8_t* function_name,
	uint8_t function_name_length)
{
	static const uint8_t core_host_functions_lengths[] =
	{
		19, 13, 13, 32, 39, 25, 15
	};

	if (!ptr_to_host_policy_object ||
		!function_name ||
		!function_name_length)
	{
		return 0;
	}

	const struct host_policy* ptr_to_host_policy_object_ = (const struct host_policy*)ptr_to_host_policy_object;

	for (uint8_t i = 0, count = COUNT_OF(host_policy_functions_string); i < count; ++i)
	{
		if (core_host_functions_lengths[i] != function_name_length ||
			0 != memcmp(host_policy_functions_string[i], function_name, function_name_length))
		{
			continue;
		}

		switch (i)
		{
			case host_policy_corehost_initialize_:
				return NULL != ptr_to_host_policy_object_->corehost_initialize;

			case host_policy_corehost_load_:
				return NULL != ptr_to_host_policy_object_->corehost_load;

			case host_policy_corehost_main_:
				return NULL != ptr_to_host_policy_object_->corehost_main;

			case host_policy_corehost_main_with_output_buffer_:
				return NULL != ptr_to_host_policy_object_->corehost_main_with_output_buffer;

			case host_policy_corehost_resolve_component_dependencies_:
				return NULL != ptr_to_host_policy_object_->corehost_resolve_component_dependencies;

			case host_policy_corehost_set_error_writer_:
				return NULL != ptr_to_host_policy_object_->corehost_set_error_writer;

			case host_policy_corehost_unload_:
				return NULL != ptr_to_host_policy_object_->corehost_unload;

			default:
				break;
		}
	}

	return 0;
}

int32_t core_host_initialize(
	const void* ptr_to_host_policy_object,
	const void* init_request,
	int32_t options,
	void* context_contract)
{
	if (!ptr_to_host_policy_object ||
		!context_contract)
	{
		return -1;
	}

	const struct host_policy* ptr_to_host_policy_object_ = (const struct host_policy*)ptr_to_host_policy_object;

	if (!ptr_to_host_policy_object_->corehost_initialize)
	{
		return -1;
	}

	return ptr_to_host_policy_object_->corehost_initialize(init_request, options, context_contract);
}

int32_t core_host_load(
	const void* ptr_to_host_policy_object,
	void* host_interface)
{
	if (!ptr_to_host_policy_object ||
		!host_interface)
	{
		return -1;
	}

	const struct host_policy* ptr_to_host_policy_object_ = (const struct host_policy*)ptr_to_host_policy_object;

	if (!ptr_to_host_policy_object_->corehost_load)
	{
		return -1;
	}

	return ptr_to_host_policy_object_->corehost_load(host_interface);
}

int32_t core_host_main(
	const void* ptr_to_host_policy_object,
	const int32_t argc,
	const type_of_element** argv)
{
	if (!ptr_to_host_policy_object)
	{
		return -1;
	}

	const struct host_policy* ptr_to_host_policy_object_ = (const struct host_policy*)ptr_to_host_policy_object;

	if (!ptr_to_host_policy_object_->corehost_main)
	{
		return -1;
	}

	return ptr_to_host_policy_object_->corehost_main(argc, argv);
}

uint8_t core_host_main_with_output_buffer(
	const void* ptr_to_host_policy_object,
	const int32_t argc,
	const type_of_element** argv,
	struct buffer* output)
{
	if (!ptr_to_host_policy_object ||
		!output)
	{
		return 0;
	}

	const struct host_policy* ptr_to_host_policy_object_ = (const struct host_policy*)ptr_to_host_policy_object;

	if (!ptr_to_host_policy_object_->corehost_main_with_output_buffer)
	{
		return 0;
	}

	type_of_element* out = (type_of_element*)buffer_data(output, 0);
	int32_t required_size = 0;
	int32_t result = ptr_to_host_policy_object_->corehost_main_with_output_buffer(
						 argc, argv, out, 0, &required_size);

	if (IS_HOST_FAILED(result))
	{
		if ((int32_t)net_HostApiBufferTooSmall != result ||
			required_size < 1)
		{
			if (!buffer_resize(output, 0) ||
				!buffer_push_back(output, ' ') ||
				!int_to_string(result, output))
			{
				return 0;
			}

			return 1;
		}

#if defined(_WIN32)

		if (!buffer_resize(output, sizeof(uint32_t) + required_size * sizeof(type_of_element)))
#else
		if (!buffer_resize(output, required_size * sizeof(type_of_element)))
#endif
		{
			return 0;
		}

#if defined(_WIN32)
		out = (type_of_element*)buffer_data(output, sizeof(uint32_t));
#else
		out = (type_of_element*)buffer_data(output, 0);
#endif
		result = ptr_to_host_policy_object_->corehost_main_with_output_buffer(
					 argc, argv, out, required_size, &required_size);
#if defined(_WIN32)
		const type_of_element* finish = (type_of_element*)(buffer_data(output, 0) + buffer_size(output));

		if (!buffer_resize(output, 0) ||
			!text_encoding_UTF16LE_to_UTF8(out, finish, output))
		{
			return 0;
		}

#endif

		if (IS_HOST_FAILED(result))
		{
			if (!buffer_push_back(output, ' ') ||
				!int_to_string(result, output))
			{
				return 0;
			}
		}
	}

	return 1;
}

int32_t core_host_resolve_component_dependencies(
	const void* ptr_to_host_policy_object,
	const type_of_element* component_main_assembly_path,
	corehost_resolve_component_dependencies_result_type core_host_resolve_component_dependencies_callback)
{
	if (!ptr_to_host_policy_object ||
		!component_main_assembly_path ||
		!core_host_resolve_component_dependencies_callback)
	{
		return -1;
	}

	const struct host_policy* ptr_to_host_policy_object_ =
		(const struct host_policy*)ptr_to_host_policy_object;

	if (!ptr_to_host_policy_object_->corehost_resolve_component_dependencies)
	{
		return -1;
	}

	return ptr_to_host_policy_object_->corehost_resolve_component_dependencies(
			   component_main_assembly_path, core_host_resolve_component_dependencies_callback);
}

error_writer_type core_host_set_error_writer(
	const void* ptr_to_host_policy_object, error_writer_type writer)
{
	error_writer_type result = NULL;

	if (!ptr_to_host_policy_object)
	{
		return result;
	}

	const struct host_policy* ptr_to_host_policy_object_ = (const struct host_policy*)ptr_to_host_policy_object;

	if (ptr_to_host_policy_object_->corehost_set_error_writer)
	{
		result = ptr_to_host_policy_object_->corehost_set_error_writer(writer);
	}

	return result;
}

int32_t core_host_unload(const void* ptr_to_host_policy_object)
{
	if (!ptr_to_host_policy_object)
	{
		return -1;
	}

	const struct host_policy* ptr_to_host_policy_object_ = (const struct host_policy*)ptr_to_host_policy_object;

	if (!ptr_to_host_policy_object_->corehost_unload)
	{
		return -1;
	}

	return ptr_to_host_policy_object_->corehost_unload();
}
