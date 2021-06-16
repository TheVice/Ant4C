/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_HOST_INTERFACE_H__
#define __MODULES_NET_HOST_INTERFACE_H__

#include <stddef.h>
#include <stdint.h>

struct buffer;

#define HOST_INTERFACE_SIZE UINT8_MAX

void host_interface_release_buffers();

uint8_t host_interface_init(
	void* host_interface, size_t size, size_t version_high);

uint8_t host_interface_set_additional_dependency_serialized(
	void* host_interface,
	const uint8_t* additional_dependency_serialized,
	uint16_t additional_dependency_serialized_length);
uint8_t host_interface_set_application_path(
	void* host_interface,
	const uint8_t* application_path,
	uint16_t application_path_length);
uint8_t host_interface_set_config_keys(
	void* host_interface,
	const uint8_t** keys, const uint16_t* keys_lengths,
	uint8_t count);
uint8_t host_interface_set_config_values(
	void* host_interface,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t count);
uint8_t host_interface_set_dependency_file(
	void* host_interface,
	const uint8_t* dependency_file,
	uint16_t dependency_file_length);
uint8_t host_interface_set_dotnet_root(
	void* host_interface,
	const uint8_t* dotnet_root,
	uint16_t dotnet_root_length);
uint8_t host_interface_set_file_bundle_header_offset(
	void* host_interface, size_t single_file_bundle_header_offset);
uint8_t host_interface_set_framework_dependent(
	void* host_interface, size_t is_framework_dependent);
uint8_t host_interface_set_framework_directories(
	void* host_interface,
	const uint8_t** directories, const uint16_t* directories_lengths,
	uint8_t count);
uint8_t host_interface_set_framework_directory(
	void* host_interface, const uint8_t* directory, uint16_t directory_length);
uint8_t host_interface_set_framework_found_versions(
	void* host_interface,
	const uint8_t** versions, const uint16_t* versions_lengths,
	uint8_t count);
uint8_t host_interface_set_framework_name(
	void* host_interface, const uint8_t* name, uint16_t name_length);
uint8_t host_interface_set_framework_names(
	void* host_interface,
	const uint8_t** names, const uint16_t* names_lengths,
	uint8_t count);
uint8_t host_interface_set_framework_requested_versions(
	void* host_interface,
	const uint8_t** versions, const uint16_t* versions_lengths,
	uint8_t count);
uint8_t host_interface_set_framework_version(
	void* host_interface,
	const uint8_t* framework_version, uint16_t framework_version_length);
uint8_t host_interface_set_host_command(
	void* host_interface,
	const uint8_t* host_command, uint16_t host_command_length);
uint8_t host_interface_set_host_mode(
	void* host_interface,
	const uint8_t* mode, uint16_t mode_length);
uint8_t host_interface_set_host_path(
	void* host_interface,
	const uint8_t* path, uint16_t path_length);
uint8_t host_interface_set_patch_roll_forward(
	void* host_interface, size_t patch_roll_forward);
uint8_t host_interface_set_paths_for_probing(
	void* host_interface,
	const uint8_t** paths, const uint16_t* paths_lengths,
	uint8_t count);
uint8_t host_interface_set_prerelease_roll_forward(
	void* host_interface, size_t prerelease_roll_forward);
uint8_t host_interface_set_target_framework_moniker(
	void* host_interface,
	const uint8_t* target_framework_moniker,
	uint16_t target_framework_moniker_length);

void* host_interface_get();

#endif
