/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "net.h"

#include "core_host_context_contract.h"
#include "core_host_initialize_request.h"
#include "error_writer.h"
#include "host_fxr.h"
#include "host_interface.h"
#include "host_policy.h"
#include "net.common.h"
#include "net.file.h"
#include "net.host_fxr.h"
#include "net.host_policy.h"
#include "net.host.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "string_unit.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

static const uint8_t space = ' ';

static const uint8_t* name_spaces[] =
{
	(const uint8_t*)"net",
	(const uint8_t*)"nethost",
	(const uint8_t*)"hostfxr",
	(const uint8_t*)"hostpolicy",
	(const uint8_t*)"hostinterface",
	(const uint8_t*)"corehost",
	(const uint8_t*)"corehost-context-contract",
	(const uint8_t*)"corehost-initialize-request",
	(const uint8_t*)"file"
};

static const uint8_t* all_functions =
	(const uint8_t*)"result-to-string\0" \
	"\0" \
	"get-hostfxr-path\0" \
	"\0" \
	"functions\0" \
	"initialize\0" \
	"is-function-exists\0" \
	"close\0" \
	"get-available-sdks\0" \
	"get-dotnet-environment-info\0" \
	"get-native-search-directories\0" \
	"get-runtime-delegate\0" \
	"get-runtime-properties\0" \
	"get-runtime-property-value\0" \
	"initialize-for-dotnet-command-line\0" \
	"initialize-for-runtime-config\0" \
	"main\0" \
	"main-bundle-startupinfo\0" \
	"main-startupinfo\0" \
	"resolve-sdk\0" \
	"resolve-sdk2\0" \
	"run-app\0" \
	"set-error-writer\0" \
	"set-runtime-property-value\0" \
	"\0" \
	"initialize\0" \
	"\0" \
	"initialize\0" \
	"set-additional-dependency-serialized\0" \
	"set-application-path\0" \
	"set-config-keys\0" \
	"set-config-values\0" \
	"set-dependency-file\0" \
	"set-dotnet-root\0" \
	"set-file-bundle-header-offset\0" \
	"set-framework-dependent\0" \
	"set-framework-directories\0" \
	"set-framework-directory\0" \
	"set-framework-found-versions\0" \
	"set-framework-name\0" \
	"set-framework-names\0" \
	"set-framework-requested-versions\0" \
	"set-framework-version\0" \
	"set-host-command\0" \
	"set-host-mode\0" \
	"set-host-path\0" \
	"set-patch-roll-forward\0" \
	"set-paths-for-probing\0" \
	"set-prerelease-roll-forward\0" \
	"set-target-framework-moniker\0" \
	"\0" \
	"functions\0" \
	"is-function-exists\0" \
	"initialize\0" \
	"load\0" \
	"main\0" \
	"main-with-output-buffer\0" \
	"resolve-component-dependencies\0" \
	"set-error-writer\0" \
	"unload\0" \
	"\0" \
	"initialize\0" \
	"get-property-value\0" \
	"set-property-value\0" \
	"get-properties\0" \
	"load-runtime\0" \
	"run-app\0" \
	"get-runtime-delegate\0" \
	"\0" \
	"initialize\0" \
	"set-config-keys\0" \
	"set-config-values\0" \
	"\0" \
	"is-assembly\0" \
	"\0";

#define COUNT_OF_ALL_FUNCTIONS 1241

enum ant4c_net_module_
{
	net_result_to_string_,
	/**/
	net_host_get_hostfxr_path_,
	/**/
	host_fxr_functions_,
	host_fxr_initialize_,
	host_fxr_is_function_exists_,
	/**/
	host_fxr_close_,
	host_fxr_get_available_sdks_,
	host_fxr_get_dotnet_environment_info_,
	host_fxr_get_native_search_directories_,
	host_fxr_get_runtime_delegate_,
	host_fxr_get_runtime_properties_,
	host_fxr_get_runtime_property_value_,
	host_fxr_initialize_for_dotnet_command_line_,
	host_fxr_initialize_for_runtime_config_,
	host_fxr_main_,
	host_fxr_main_bundle_startupinfo_,
	host_fxr_main_startupinfo_,
	host_fxr_resolve_sdk_,
	host_fxr_resolve_sdk2_,
	host_fxr_run_app_,
	host_fxr_set_error_writer_,
	host_fxr_set_runtime_property_value_,
	/**/
	host_policy_initialize_,
	/**/
	host_interface_initialize_,
	host_interface_set_additional_dependency_serialized_,
	host_interface_set_application_path_,
	host_interface_set_config_keys_,
	host_interface_set_config_values_,
	host_interface_set_dependency_file_,
	host_interface_set_dotnet_root_,
	host_interface_set_file_bundle_header_offset_,
	host_interface_set_framework_dependent_,
	host_interface_set_framework_directories_,
	host_interface_set_framework_directory_,
	host_interface_set_framework_found_versions_,
	host_interface_set_framework_name_,
	host_interface_set_framework_names_,
	host_interface_set_framework_requested_versions_,
	host_interface_set_framework_version_,
	host_interface_set_host_command_,
	host_interface_set_host_mode_,
	host_interface_set_host_path_,
	host_interface_set_patch_roll_forward_,
	host_interface_set_paths_for_probing_,
	host_interface_set_prerelease_roll_forward_,
	host_interface_set_target_framework_moniker_,
	/**/
	core_host_functions_,
	core_host_is_function_exists_,
	/**/
	core_host_initialize_,
	core_host_load_,
	core_host_main_,
	core_host_main_with_output_buffer_,
	core_host_resolve_component_dependencies_,
	core_host_set_error_writer_,
	core_host_unload_,
	/**/
	core_host_context_contract_initialize_,
	core_host_context_contract_get_property_value_,
	core_host_context_contract_set_property_value_,
	core_host_context_contract_get_properties_,
	core_host_context_contract_load_runtime_,
	core_host_context_contract_run_app_,
	core_host_context_contract_get_runtime_delegate_,
	/**/
	core_host_initialize_request_initialize_,
	core_host_initialize_request_set_config_keys_,
	core_host_initialize_request_set_config_values_,
	/**/
	file_is_assembly_
};

const uint8_t* enumerate_name_spaces(ptrdiff_t index)
{
	static const ptrdiff_t count = COUNT_OF(name_spaces);

	if (index < 0 || count <= index)
	{
		return NULL;
	}

	return name_spaces[index];
}

const uint8_t* enumerate_functions(const uint8_t* name_space, ptrdiff_t index)
{
	if (NULL == name_space)
	{
		return NULL;
	}

	ptrdiff_t i = 0;
	const uint8_t* ptr = NULL;

	while (NULL != (ptr = enumerate_name_spaces(i++)))
	{
		if (ptr == name_space)
		{
			break;
		}
	}

	if (NULL == ptr)
	{
		return NULL;
	}

	const ptrdiff_t name_space_number = i - 1;
	i = 0;

	while (NULL != (ptr = common_get_string_at(
							  all_functions, all_functions + COUNT_OF_ALL_FUNCTIONS,
							  name_space_number, i++)))
	{
		if (index == (i - 1))
		{
			return ptr;
		}
	}

	return NULL;
}

static uint8_t host_fxr_object[160];
static uint8_t is_host_fxr_object_initialized = 0;

static uint8_t host_policy_object[96];
static uint8_t is_host_policy_object_initialized = 0;

static uint8_t output_data_buffer[BUFFER_SIZE_OF];
static void* output_data = (void*)output_data_buffer;
static uint8_t is_buffer_initialized = 0;

uint8_t evaluate_function(
	const uint8_t* function,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	const uint8_t** output, uint16_t* output_length)
{
	static const uint16_t one = 1;

	if (NULL == function ||
		NULL == values ||
		NULL == values_lengths ||
		NULL == output ||
		NULL == output_length)
	{
		return 0;
	}

	if (is_buffer_initialized)
	{
		if (!buffer_resize(output_data, 0))
		{
			return 0;
		}
	}
	else
	{
		output_data = (void*)output_data_buffer;
		is_buffer_initialized = buffer_init(output_data, BUFFER_SIZE_OF);
	}

	const uint8_t* ptr = NULL;
	ptrdiff_t function_number = 0;

	for (uint8_t x = 0, count = COUNT_OF(name_spaces); x < count; ++x)
	{
		uint8_t y = 0;

		while (NULL != (ptr = common_get_string_at(
								  all_functions,
								  all_functions + COUNT_OF_ALL_FUNCTIONS,
								  x, y++)))
		{
			if (function == ptr)
			{
				x = count;
				break;
			}

			++function_number;
		}
	}

	if (NULL == ptr)
	{
		return 0;
	}

	const uint8_t* space_in_array[1];
	const void* ptr_to_host_fxr_object = is_host_fxr_object_initialized ? host_fxr_object : NULL;
	const void* ptr_to_host_policy_object = is_host_policy_object_initialized ? host_policy_object : NULL;

	switch (function_number)
	{
		case net_result_to_string_:
			if (1 != values_count ||
				!net_result_to_string(values[0], values_lengths[0], output_data))
			{
				return 0;
			}

			break;

		case net_host_get_hostfxr_path_:
			if (!net_host_get_hostfxr_path(values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_functions_:
			if (1 < values_count)
			{
				return 0;
			}

			if (!values_count)
			{
				space_in_array[0] = &space;
				values = space_in_array;
				values_lengths = &one;
			}

			if (!get_exists_functions(
					ptr_to_host_fxr_object,
					name_spaces[2],
					common_get_string_at(
						all_functions, all_functions + COUNT_OF_ALL_FUNCTIONS,
						2, 3),
					values[0],
					values_lengths[0],
					host_fx_resolver_is_function_exists,
					output_data))
			{
				return 0;
			}

			break;

		case host_fxr_initialize_:
			if (1 != values_count)
			{
				return 0;
			}

			if (is_host_fxr_object_initialized)
			{
				host_fx_resolver_unload(host_fxr_object);
				is_host_fxr_object_initialized = 0;
			}

			is_host_fxr_object_initialized = host_fx_resolver_init(
												 values[0], values_lengths[0],
												 output_data, host_fxr_object, sizeof(host_fxr_object));

			if (!buffer_resize(output_data, 0) ||
				!bool_to_string(is_host_fxr_object_initialized, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_is_function_exists_:
			if (1 != values_count)
			{
				return 0;
			}

			if (!is_function_exists(
					ptr_to_host_fxr_object, name_spaces[2],
					values[0], values_lengths[0],
					host_fx_resolver_is_function_exists, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_close_:
			if (1 != values_count ||
				!hostfxr_close(ptr_to_host_fxr_object, values[0], (uint8_t)values_lengths[0], output_data))
			{
				return 0;
			}

			break;

		case host_fxr_get_available_sdks_:
			if (1 < values_count)
			{
				return 0;
			}

			if (values_count)
			{
				values_count = hostfxr_get_available_sdks(
								   ptr_to_host_fxr_object, values[0], values_lengths[0], output_data);
			}
			else
			{
				values_count = hostfxr_get_available_sdks(
								   ptr_to_host_fxr_object, NULL, 0, output_data);
			}

			if (!values_count)
			{
				return 0;
			}

			break;

		case host_fxr_get_dotnet_environment_info_:
			if (!hostfxr_get_dotnet_environment_information(
					ptr_to_host_fxr_object, values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_get_native_search_directories_:
			if (!hostfxr_get_native_search_directories(ptr_to_host_fxr_object,
					values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_get_runtime_delegate_:
			if (!hostfxr_get_runtime_delegate(
					ptr_to_host_fxr_object, values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_get_runtime_properties_:
			if (!hostfxr_get_runtime_properties(
					ptr_to_host_fxr_object, values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_get_runtime_property_value_:
			if (!hostfxr_get_runtime_property_value(
					ptr_to_host_fxr_object, values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_initialize_for_dotnet_command_line_:
			if (!hostfxr_initialize_for_dotnet_command_line(
					ptr_to_host_fxr_object, values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_initialize_for_runtime_config_:
			if (!hostfxr_initialize_for_runtime_config(
					ptr_to_host_fxr_object, values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_main_:
			if (!hostfxr_main(
					ptr_to_host_fxr_object, values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_main_bundle_startupinfo_:
			if (!hostfxr_main_bundle_startupinfo(
					ptr_to_host_fxr_object, values,
					values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_main_startupinfo_:
			if (!hostfxr_main_startupinfo(
					ptr_to_host_fxr_object, values,
					values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_resolve_sdk_:
			if (!hostfxr_resolve_sdk(
					ptr_to_host_fxr_object, values,
					values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_resolve_sdk2_:
			if (!hostfxr_resolve_sdk2(
					ptr_to_host_fxr_object, values,
					values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case host_fxr_run_app_:
			if (1 != values_count ||
				!hostfxr_run_app(
					ptr_to_host_fxr_object, values[0], (uint8_t)values_lengths[0], output_data))
			{
				return 0;
			}

			break;

		case host_fxr_set_error_writer_:
			if (1 < values_count ||
				!hostfxr_set_error_writer(
					ptr_to_host_fxr_object,
					values_count ? values[0] : NULL,
					values_count ? values_lengths[0] : 0,
					output_data))
			{
				return 0;
			}

			break;

		case host_fxr_set_runtime_property_value_:
			if (!hostfxr_set_runtime_property_value(ptr_to_host_fxr_object,
													values, values_lengths,
													values_count, output_data))
			{
				return 0;
			}

			break;

		case host_policy_initialize_:
			if (1 != values_count)
			{
				return 0;
			}

			if (is_host_policy_object_initialized)
			{
				host_fx_resolver_unload(host_policy_object);
				is_host_policy_object_initialized = 0;
			}

			is_host_policy_object_initialized = host_policy_init(
													values[0], values_lengths[0],
													output_data, host_policy_object,
													sizeof(host_policy_object));

			if (!buffer_resize(output_data, 0) ||
				!bool_to_string(is_host_policy_object_initialized, output_data))
			{
				return 0;
			}

			break;

		case host_interface_initialize_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_init(
							   host_interface_get(), HOST_INTERFACE_SIZE,
							   (size_t)uint64_parse(values[0],
													values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_additional_dependency_serialized_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_additional_dependency_serialized(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_application_path_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_application_path(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_config_keys_:
			values_count = host_interface_set_config_keys(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_config_values_:
			values_count = host_interface_set_config_values(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_dependency_file_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_dependency_file(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_dotnet_root_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_dotnet_root(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_file_bundle_header_offset_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_file_bundle_header_offset(
							   host_interface_get(),
							   (size_t)uint64_parse(values[0],
													values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_dependent_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_framework_dependent(
							   host_interface_get(),
							   (size_t)uint64_parse(values[0],
													values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_directories_:
			values_count = host_interface_set_framework_directories(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_directory_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_framework_directory(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_found_versions_:
			values_count = host_interface_set_framework_found_versions(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_name_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_framework_name(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_names_:
			values_count = host_interface_set_framework_names(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_requested_versions_:
			values_count = host_interface_set_framework_requested_versions(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_framework_version_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_framework_version(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_host_command_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_host_command(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_host_mode_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_host_mode(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_host_path_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_host_path(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_patch_roll_forward_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_patch_roll_forward(
							   host_interface_get(),
							   (size_t)uint64_parse(values[0],
													values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_paths_for_probing_:
			values_count = host_interface_set_paths_for_probing(
							   host_interface_get(),
							   values, values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_prerelease_roll_forward_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_prerelease_roll_forward(
							   host_interface_get(),
							   (size_t)uint64_parse(values[0],
													values[0] + values_lengths[0]));

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case host_interface_set_target_framework_moniker_:
			if (1 != values_count)
			{
				return 0;
			}

			values_count = host_interface_set_target_framework_moniker(
							   host_interface_get(),
							   values[0], values_lengths[0]);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case core_host_functions_:
			if (1 < values_count)
			{
				return 0;
			}

			if (!values_count)
			{
				space_in_array[0] = &space;
				values = space_in_array;
				values_lengths = &one;
			}

			if (!get_exists_functions(
					ptr_to_host_policy_object,
					name_spaces[5],
					common_get_string_at(
						all_functions,
						all_functions + COUNT_OF_ALL_FUNCTIONS,
						5, 2),
					values[0],
					values_lengths[0],
					core_host_is_function_exists,
					output_data))
			{
				return 0;
			}

			break;

		case core_host_is_function_exists_:
			if (1 != values_count)
			{
				return 0;
			}

			if (!is_function_exists(
					ptr_to_host_policy_object,
					name_spaces[5],
					values[0], values_lengths[0],
					core_host_is_function_exists, output_data))
			{
				return 0;
			}

			break;

		case core_host_initialize_:
			if (2 < values_count ||
				!core_host_initialize__(
					ptr_to_host_policy_object,
					1 < values_count ? NULL : core_host_initialize_request_get(),
					core_host_context_contract_get(),
					values, values_lengths, values_count,
					output_data))
			{
				return 0;
			}

			break;

		case core_host_load_:
			if (values_count ||
				!int_to_string(core_host_load(
								   ptr_to_host_policy_object,
								   host_interface_get()),
							   output_data))
			{
				return 0;
			}

			break;

		case core_host_main_:
			if (!core_host_main__(
					ptr_to_host_policy_object,
					values, values_lengths, values_count,
					output_data))
			{
				return 0;
			}

			break;

		case core_host_main_with_output_buffer_:
			if (!core_host_main_with_output_buffer__(
					ptr_to_host_policy_object,
					values, values_lengths, values_count,
					output_data))
			{
				return 0;
			}

			break;

		case core_host_resolve_component_dependencies_:
			if (1 != values_count ||
				!core_host_resolve_component_dependencies__(
					ptr_to_host_policy_object,
					values[0], values_lengths[0],
					output_data))
			{
				return 0;
			}

			break;

		case core_host_set_error_writer_:
			if (1 < values_count ||
				!core_host_set_error_writer__(
					ptr_to_host_policy_object,
					values_count ? values[0] : NULL,
					values_count ? values_lengths[0] : 0,
					output_data))
			{
				return 0;
			}

			break;

		case core_host_unload_:
			if (values_count ||
				!int_to_string(
					core_host_unload(ptr_to_host_policy_object),
					output_data))
			{
				return 0;
			}

			break;

		case core_host_context_contract_initialize_:
			if (values_count ||
				!int_to_string(
					core_host_context_contract_init(
						core_host_context_contract_get(),
						CORE_HOST_CONTEXT_CONTRACT_SIZE),
					output_data))
			{
				return 0;
			}

			break;

		case core_host_context_contract_get_property_value_:
			if (1 != values_count ||
				!core_host_context_contract_get_property_value__(
					core_host_context_contract_get(),
					values[0], values_lengths[0], output_data))
			{
				return 0;
			}

			break;

		case core_host_context_contract_set_property_value_:
			if (!core_host_context_contract_set_property_value__(
					ptr_to_host_policy_object, core_host_context_contract_get(),
					values, values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case core_host_context_contract_get_properties_:
			if (values_count ||
				!core_host_context_contract_get_properties__(
					core_host_context_contract_get(), output_data))
			{
				return 0;
			}

			break;

		case core_host_context_contract_load_runtime_:
			if (values_count ||
				!core_host_context_contract_load_runtime__(
					core_host_context_contract_get(), output_data))
			{
				return 0;
			}

			break;

		case core_host_context_contract_run_app_:
			if (!core_host_context_contract_run_app__(
					core_host_context_contract_get(), values,
					values_lengths, values_count, output_data))
			{
				return 0;
			}

			break;

		case core_host_context_contract_get_runtime_delegate_:
			if (1 != values_count ||
				!core_host_context_contract_get_runtime_delegate__(
					core_host_context_contract_get(), values[0],
					values_lengths[0], output_data))
			{
				return 0;
			}

			break;

		case core_host_initialize_request_initialize_:
			if (values_count)
			{
				return 0;
			}

			values_count = core_host_initialize_request_init(
							   core_host_initialize_request_get(),
							   CORE_HOST_INITIALIZE_REQUEST_SIZE);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case core_host_initialize_request_set_config_keys_:
			values_count = core_host_initialize_request_set_config_keys(
							   core_host_initialize_request_get(), values,
							   values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case core_host_initialize_request_set_config_values_:
			values_count = core_host_initialize_request_set_config_values(
							   core_host_initialize_request_get(), values,
							   values_lengths, values_count);

			if (!bool_to_string(values_count, output_data))
			{
				return 0;
			}

			break;

		case file_is_assembly_:
			if (!file_is_assembly(ptr_to_host_fxr_object,
								  values, values_lengths,
								  values_count, output_data))
			{
				return 0;
			}

			break;

		default:
			return 0;
	}

	*output = buffer_uint8_t_data(output_data, 0);
	*output_length = (uint16_t)buffer_size(output_data);
	/**/
	return 1;
}

void module_release()
{
	host_interface_release_buffers();
	core_host_initialize_request_release_buffers();
	error_writer_release_buffers(
		is_host_fxr_object_initialized ? host_fxr_object : NULL,
		is_host_policy_object_initialized ? host_policy_object : NULL);

	if (is_host_fxr_object_initialized)
	{
		host_fx_resolver_unload(host_fxr_object);
		is_host_fxr_object_initialized = 0;
	}

	if (is_host_policy_object_initialized)
	{
		host_policy_unload(host_policy_object);
		is_host_policy_object_initialized = 0;
	}

	if (is_buffer_initialized)
	{
		buffer_release(output_data);
	}
}
