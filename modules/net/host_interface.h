/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __HOST_INTERFACE_H__
#define __HOST_INTERFACE_H__

#include "net.common.h"

#include <stddef.h>
#include <stdint.h>

struct buffer;

uint8_t host_interface_init(
	void* host_interface, size_t size, size_t version_high);

uint8_t host_interface_set_config(
	void* host_interface,
	const type_of_element** keys, const type_of_element** values,
	size_t count);
uint8_t host_interface_set_framework_directory(
	void* host_interface, const type_of_element* directory);
uint8_t host_interface_set_framework_name(
	void* host_interface, const type_of_element* name);
uint8_t host_interface_set_dependency_file(
	void* host_interface, const type_of_element* dependency_file);
uint8_t host_interface_set_framework_dependent(
	void* host_interface, size_t is_framework_dependent);
uint8_t host_interface_set_paths_for_probing(
	void* host_interface, const type_of_element** paths, size_t count);
uint8_t host_interface_set_patch_roll_forward(
	void* host_interface, size_t patch_roll_forward);
uint8_t host_interface_set_prerelease_roll_forward(
	void* host_interface, size_t prerelease_roll_forward);
uint8_t host_interface_set_host_mode(
	void* host_interface, size_t host_mode);
uint8_t host_interface_set_target_framework_moniker(
	void* host_interface, const type_of_element* target_framework_moniker);
uint8_t host_interface_set_additional_dependency_serialized(
	void* host_interface,
	const type_of_element* additional_dependency_serialized);
uint8_t host_interface_set_framework_version(
	void* host_interface, const type_of_element* framework_version);
uint8_t host_interface_set_framework_names(
	void* host_interface, const type_of_element** names, size_t count);
uint8_t host_interface_set_framework_directories(
	void* host_interface, const type_of_element** directories, size_t count);
uint8_t host_interface_set_framework_requested_versions(
	void* host_interface, const type_of_element** versions, size_t count);
uint8_t host_interface_set_framework_found_versions(
	void* host_interface, const type_of_element** versions, size_t count);
uint8_t host_interface_set_host_command(
	void* host_interface, const type_of_element* host_command);
uint8_t host_interface_set_host_information(
	void* host_interface,
	const type_of_element* host_path,
	const type_of_element* dotnet_root,
	const type_of_element* application_path);
uint8_t host_interface_set_file_bundle_header_offset(
	void* host_interface, size_t single_file_bundle_header_offset);

uint8_t host_interface_exec_function(
	uint8_t function,
	const struct buffer* arguments, uint8_t arguments_count);

void* host_interface_get();

#endif
