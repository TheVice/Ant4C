/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "net.host_policy.h"
#include "arguments.h"
#include "core_host_context_contract.h"
#include "error_writer.h"
#include "host_interface.h"
#include "host_policy.h"
#include "net.common.h"
#include "net_delegate.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

#if defined(_WIN32)
#include <wchar.h>
#endif

#include <string.h>

uint8_t host_policy_init(
	const uint8_t* path_to_core_host,
	uint16_t path_to_core_host_length,
	struct buffer* tmp,
	void* ptr_to_core_host_object,
	ptrdiff_t size)
{
	if (size < 96)
	{
		return 0;
	}

	return load_library(
			   path_to_core_host, path_to_core_host_length, tmp,
			   ptr_to_core_host_object, size, host_policy_load);
}

static struct buffer* dependencies = NULL;

void core_host_resolve_component_dependencies_callback(
	const type_of_element* assembly_paths,
	const type_of_element* native_search_paths,
	const type_of_element* resource_search_paths);

int32_t core_host_initialize_get_options(const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		return -1;
	}

	enum core_host_initialize_options
	{
		none,
		wait_for_initialized,
		get_contract,
		context_contract_version_set = 0x80000000
	};
	/**/
	static const uint8_t* core_host_initialize_options_[] =
	{
		(const uint8_t*)"none",
		(const uint8_t*)"wait_for_initialized",
		(const uint8_t*)"get_contract",
		(const uint8_t*)"context_contract_version_set"
	};
	/**/
	static const uint8_t core_host_initialize_options_max_value = 4;
	/**/
	int32_t options = common_string_to_enum(
						  value_start, value_finish,
						  core_host_initialize_options_, core_host_initialize_options_max_value);

	if (core_host_initialize_options_max_value == options)
	{
		options = -1;
	}
	else if (3 == options)
	{
		options = context_contract_version_set;
	}

	return options;
}

uint8_t core_host_context_contract_get_property_value__(
	const void* context_contract,
	const uint8_t* property_name, uint16_t property_name_length, struct buffer* output)
{
#if defined(_WIN32)

	if (!text_encoding_UTF8_to_UTF16LE(property_name, property_name + property_name_length, output) ||
		!buffer_push_back_uint16(output, 0))
	{
		return 0;
	}

#else

	if (!buffer_append(output, property_name, property_name_length) ||
		!buffer_push_back(output, 0))
	{
		return 0;
	}

#endif
	const type_of_element* key = (const type_of_element*)buffer_data(output, 0);
	const type_of_element* value = NULL;
	const int32_t result = core_host_context_contract_get_property_value(
							   context_contract, key, &value);

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	if (IS_HOST_FAILED(result))
	{
		if (!buffer_push_back(output, 0) ||
			!int_to_string(result, output))
		{
			return 0;
		}
	}
	else
	{
#if defined(_WIN32)
		key = value + wcslen(value);

		if ((value < key) && !text_encoding_UTF16LE_to_UTF8(value, key, output))
#else
		if (!buffer_append(output, value, common_count_bytes_until(value, 0)))
#endif
		{
			return 0;
		}
	}

	return 1;
}

uint8_t core_host_context_contract_set_property_value__(
	const void* ptr_to_host_policy_object, const void* context_contract,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_policy_object ||
		!context_contract ||
		!values ||
		!values_lengths ||
		values_count < 1 || 2 < values_count ||
		!output)
	{
		return 0;
	}

#if defined(_WIN32)

	if (!text_encoding_UTF8_to_UTF16LE(values[0], values[0] + values_lengths[0], output) ||
		!buffer_push_back_uint16(output, 0))
#else
	if (!buffer_append(output, values[0], values_lengths[0]) ||
		!buffer_push_back(output, 0))
#endif
	{
		return 0;
	}

	const ptrdiff_t index = buffer_size(output);

	if (2 == values_count && values_lengths[1])
	{
#if defined(_WIN32)

		if (!text_encoding_UTF8_to_UTF16LE(values[1], values[1] + values_lengths[1], output))
#else
		if (!buffer_append(output, values[1], values_lengths[1]))
#endif
		{
			return 0;
		}
	}

#if defined(_WIN32)

	if (!buffer_push_back_uint16(output, 0))
#else
	if (!buffer_push_back(output, 0))
#endif
	{
		return 0;
	}

	const type_of_element* name = (const type_of_element*)buffer_data(output, 0);
	const type_of_element* value = 2 == values_count ? (const type_of_element*)buffer_data(output, index) : NULL;
	/**/
	const int32_t result = core_host_context_contract_set_property_value(context_contract, name, value);

	if (!buffer_resize(output, 0) ||
		!int_to_string(result, output))
	{
		return 0;
	}

	return 1;
}

uint8_t core_host_context_contract_get_properties__(const void* context_contract, struct buffer* output)
{
	if (!context_contract || !output)
	{
		return 0;
	}

	size_t count = 0;
	const type_of_element** properties_keys = NULL;
	const type_of_element** properties_values = NULL;
	/**/
	int32_t result = core_host_context_contract_get_properties(
						 context_contract, &count, properties_keys, properties_values);

	if ((int32_t)net_HostApiBufferTooSmall == result)
	{
		if (!buffer_resize(output, 2 * count * sizeof(type_of_element*)))
		{
			return 0;
		}

		memset(buffer_data(output, 0), 0, buffer_size(output));
		/**/
		properties_keys = (const type_of_element**)buffer_data(output, 0);
		properties_values = (const type_of_element**)buffer_data(output, count * sizeof(type_of_element*));
		/**/
		result = core_host_context_contract_get_properties(
					 context_contract, &count, properties_keys, properties_values);
	}

	if (IS_HOST_FAILED(result))
	{
		return int_to_string(result, output);
	}

	const ptrdiff_t size = buffer_size(output);

	if (count && size < INT16_MAX)
	{
		if (!buffer_append(output, NULL, INT16_MAX) ||
			!buffer_resize(output, size))
		{
			return 0;
		}
	}

	for (size_t i = 0; i < count; ++i)
	{
		properties_keys = (const type_of_element**)buffer_data(output, 0);
		properties_values = (const type_of_element**)buffer_data(output, count * sizeof(type_of_element*));
		/**/
		const type_of_element* key_ = properties_keys[i];
		const type_of_element* value_ = properties_values[i];
#if defined(_WIN32)

		if (!buffer_push_back(output, '\'') ||
			!text_encoding_UTF16LE_to_UTF8(key_, key_ + wcslen(key_), output) ||
			!buffer_push_back(output, '\'') ||
			!common_append_string_to_buffer((const uint8_t*)" = ", output) ||
			!buffer_push_back(output, '\''))
		{
			return 0;
		}

		if (value_)
		{
			key_ = value_ + wcslen(value_);

			if (value_ < key_ &&
				!text_encoding_UTF16LE_to_UTF8(value_, key_, output))
			{
				return 0;
			}
		}

#else

		if (!buffer_push_back(output, '\'') ||
			!buffer_append(output, key_, common_count_bytes_until(key_, 0)) ||
			!buffer_push_back(output, '\'') ||
			!common_append_string_to_buffer((const uint8_t*)" = ", output) ||
			!buffer_push_back(output, '\''))
		{
			return 0;
		}

		if (value_)
		{
			if (!buffer_append(output, value_, common_count_bytes_until(value_, 0)))
			{
				return 0;
			}
		}

#endif

		if (!buffer_push_back(output, '\'') ||
			!buffer_push_back(output, '\n'))
		{
			return 0;
		}
	}

	if (0 < size && size < buffer_size(output))
	{
		const ptrdiff_t new_size = buffer_size(output) - size;
		const uint8_t* src = buffer_data(output, size);
		uint8_t* dst = buffer_data(output, 0);
		/**/
		MEM_CPY(dst, src, new_size);
		/**/
		dst = buffer_data(output, new_size);
		memset(dst, 0, size);
		/**/
		return buffer_resize(output, new_size);
	}

	return buffer_resize(output, 0) && int_to_string(result, output);
}

uint8_t core_host_context_contract_load_runtime__(
	const void* context_contract, struct buffer* output)
{
	if (!context_contract ||
		!output)
	{
		return 0;
	}

	return int_to_string(core_host_context_contract_load_runtime(context_contract), output);
}

uint8_t core_host_context_contract_run_app__(
	const void* context_contract,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!context_contract ||
		!values ||
		!values_lengths ||
		!output)
	{
		return 0;
	}

	const type_of_element** argv = NULL;

	if (!values_to_arguments(values, values_lengths, values_count, output, &argv))
	{
		return 0;
	}

	const int32_t result = core_host_context_contract_run_app(context_contract, values_count, argv);

	if (!buffer_resize(output, 0) ||
		!int_to_string(result, output))
	{
		return 0;
	}

	return 1;
}

uint8_t core_host_context_contract_get_runtime_delegate__(
	void* context_contract, const uint8_t* delegate_type,
	uint16_t delegate_type_length, struct buffer* output)
{
	int32_t type_of_delegate = common_string_to_enum(
								   delegate_type, delegate_type + delegate_type_length,
								   net_delegate_types_str, NET_DELEGATE_MAX_VALUE);

	if (NET_DELEGATE_MAX_VALUE == type_of_delegate)
	{
		if (!buffer_append(output, delegate_type, delegate_type_length) ||
			!buffer_push_back(output, 0))
		{
			return 0;
		}

		type_of_delegate = int_parse(buffer_data(output, 0));

		if (!buffer_resize(output, 0))
		{
			return 0;
		}
	}

	void* the_delegate;
	const int32_t result =
		core_host_context_contract_get_runtime_delegate(
			context_contract, type_of_delegate, &the_delegate);

	if (IS_HOST_FAILED(result))
	{
		return buffer_push_back(output, ' ') && int_to_string(result, output);
	}

	return pointer_to_string(the_delegate, output);
}

uint8_t core_host_initialize__(
	const void* ptr_to_host_policy_object,
	const void* init_request, void* context_contract,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	int32_t options = 0;

	if (values_count)
	{
		options = core_host_initialize_get_options(values[0], values[0] + values_lengths[0]);

		if (-1 == options)
		{
			if (!buffer_append(output, values[0], values_lengths[0]) ||
				!buffer_push_back(output, 0))
			{
				return 0;
			}

			options = int_parse(buffer_data(output, 0));

			if (!buffer_resize(output, 0))
			{
				return 0;
			}
		}
	}

	options = core_host_initialize(
				  ptr_to_host_policy_object, init_request,
				  options, context_contract);
	/**/
	return int_to_string(options, output);
}

uint8_t core_host_main__(
	const void* ptr_to_host_policy_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	const type_of_element** argv = NULL;

	if (!values_to_arguments(values, values_lengths, values_count, output, &argv))
	{
		return 0;
	}

	const int32_t result = core_host_main(ptr_to_host_policy_object, values_count, argv);

	if (!buffer_resize(output, 0) ||
		!int_to_string(result, output))
	{
		return 0;
	}

	return 1;
}

uint8_t core_host_main_with_output_buffer__(
	const void* ptr_to_host_policy_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	struct buffer arguments;
	SET_NULL_TO_BUFFER(arguments);
	const type_of_element** argv = NULL;

	if (!values_to_arguments(values, values_lengths, values_count, &arguments, &argv))
	{
		buffer_release(&arguments);
		return 0;
	}

	if (!core_host_main_with_output_buffer(ptr_to_host_policy_object, values_count, argv, output))
	{
		buffer_release(&arguments);
		return 0;
	}

	buffer_release(&arguments);
	return 1;
}

uint8_t core_host_resolve_component_dependencies__(
	const void* ptr_to_host_policy_object,
	const uint8_t* component_main_assembly_path,
	ptrdiff_t component_main_assembly_path_length,
	struct buffer* output)
{
	if (!ptr_to_host_policy_object ||
		!component_main_assembly_path ||
		!component_main_assembly_path_length ||
		!output)
	{
		return 0;
	}

	if (!value_to_system_path(component_main_assembly_path, component_main_assembly_path_length, output))
	{
		return 0;
	}

#if defined(_WIN32)
	const type_of_element* path = (const type_of_element*)buffer_data(output,
								  component_main_assembly_path_length + 1);
#else
	const type_of_element* path = (const type_of_element*)buffer_data(output, 0);
#endif
	struct buffer sub_output;
	SET_NULL_TO_BUFFER(sub_output);
	/**/
	dependencies = &sub_output;
	const int32_t result = core_host_resolve_component_dependencies(
							   ptr_to_host_policy_object, path,
							   core_host_resolve_component_dependencies_callback);
	dependencies = NULL;

	if (!buffer_resize(output, 0) ||
		!buffer_append_data_from_buffer(output, &sub_output))
	{
		buffer_release(&sub_output);
		return 0;
	}

	buffer_release(&sub_output);

	if (IS_HOST_FAILED(result))
	{
		if (!buffer_push_back(output, ' ') ||
			!int_to_string(result, output))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t core_host_set_error_writer__(
	const void* ptr_to_host_policy_object,
	const uint8_t* error_writer_file_name, uint16_t error_writer_file_name_length,
	struct buffer* output)
{
	const type_of_element* path = NULL;

	if (error_writer_file_name && error_writer_file_name_length)
	{
		if (!value_to_system_path(error_writer_file_name, error_writer_file_name_length, output))
		{
			return 0;
		}

#if defined(_WIN32)
		path = (const type_of_element*)buffer_data(output, (ptrdiff_t)1 + error_writer_file_name_length);
#else
		path = buffer_data(output, 0);
#endif
	}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4054)
	error_writer_file_name = (const uint8_t*)set_error_writer(
								 ptr_to_host_policy_object,
								 host_policy_error_writer, core_host_set_error_writer,
								 path, 1);
#pragma warning(default: 4054)
#else
	error_writer_file_name = (const uint8_t*)set_error_writer(
								 ptr_to_host_policy_object,
								 host_policy_error_writer, core_host_set_error_writer,
								 path, 1);
#endif

	if (!buffer_resize(output, 0) ||
		!pointer_to_string(error_writer_file_name, output))
	{
		return 0;
	}

	return 1;
}

void core_host_resolve_component_dependencies_callback(
	const type_of_element* assembly_paths,
	const type_of_element* native_search_paths,
	const type_of_element* resource_search_paths)
{
	if (!dependencies)
	{
		return;
	}

#if defined(_WIN32)

	if (!text_encoding_UTF16LE_to_UTF8(assembly_paths, assembly_paths + wcslen(assembly_paths), dependencies) ||
		!buffer_push_back(dependencies, '\n') ||
		!text_encoding_UTF16LE_to_UTF8(native_search_paths, native_search_paths + wcslen(native_search_paths),
									   dependencies) ||
		!buffer_push_back(dependencies, '\n') ||
		!text_encoding_UTF16LE_to_UTF8(resource_search_paths, resource_search_paths + wcslen(resource_search_paths),
									   dependencies) ||
		!buffer_push_back(dependencies, 0))
	{
		return;
	}

#else

	if (!buffer_append(dependencies, assembly_paths, common_count_bytes_until(assembly_paths, 0)) ||
		!buffer_push_back(dependencies, '\n') ||
		!buffer_append(dependencies, native_search_paths, common_count_bytes_until(native_search_paths, 0)) ||
		!buffer_push_back(dependencies, '\n') ||
		!buffer_append(dependencies, resource_search_paths, common_count_bytes_until(resource_search_paths, 0)) ||
		!buffer_push_back(dependencies, 0))
	{
		return;
	}

#endif
}
