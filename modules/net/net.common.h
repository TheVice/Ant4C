/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __NET_COMMON_H__
#define __NET_COMMON_H__

#include <stddef.h>

#if defined(_WIN32)
#include <wchar.h>
#else
#include <stdint.h>
#endif

#if defined(_WIN32)
#define type_of_element wchar_t
#define calling_convention __cdecl
#define delegate_calling_convention __stdcall
#define host_fxr_PATH_DELIMITER ';'
#define host_fxr_PATH_DELIMITER_wchar_t L';'
#else
#define type_of_element uint8_t
#define calling_convention
#define delegate_calling_convention
#define host_fxr_PATH_DELIMITER ':'
#endif

typedef void(calling_convention* error_writer_type)(
	const type_of_element* message);

struct string_arguments_type
{
	size_t length;
	const type_of_element** arguments;
};

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
	host_interface_set_configuration_,
	host_interface_set_dependency_file_,
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
	host_interface_set_host_information_,
	host_interface_set_host_mode_,
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
	file_is_assembly_
};

#endif
