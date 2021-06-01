/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "host_interface.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"

#include <string.h>

struct host_interface_type
{
	size_t version_low;
	size_t version_high;
	struct string_arguments_type config_keys;
	struct string_arguments_type config_values;
	const type_of_element* fx_dir;
	const type_of_element* fx_name;
	const type_of_element* dependency_file;
	size_t is_framework_dependent;
	struct string_arguments_type paths_for_probing;
	size_t patch_roll_forward;
	size_t prerelease_roll_forward;
	size_t host_mode;
	const type_of_element* target_framework_moniker;
	const type_of_element* additional_dependency_serialized;
	const type_of_element* fx_version;
	struct string_arguments_type fx_names;
	struct string_arguments_type fx_directories;
	struct string_arguments_type fx_requested_versions;
	struct string_arguments_type fx_found_versions;
	const type_of_element* host_command;
	const type_of_element* host_information_host_path;
	const type_of_element* host_information_dotnet_root;
	const type_of_element* host_information_application_path;
	size_t single_file_bundle_header_offset;
};

#if defined(_WIN32)
static const type_of_element* empty = L"";
#else
static const type_of_element* empty = (const type_of_element*)"";
#endif

uint8_t host_interface_init(
	void* host_interface, size_t size, size_t version_high)
{
	if (!host_interface || size < sizeof(struct host_interface_type))
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	memset(host_interface_, 0, sizeof(struct host_interface_type));

	host_interface_->version_low = sizeof(struct host_interface_type);

	host_interface_->version_high = version_high;

	host_interface_->fx_dir = empty;

	host_interface_->fx_name = empty;

	host_interface_->dependency_file = empty;

	host_interface_->patch_roll_forward = 1;

	host_interface_->target_framework_moniker = empty;

	host_interface_->additional_dependency_serialized = empty;

	host_interface_->fx_version = empty;

	host_interface_->host_command = empty;

	host_interface_->host_information_host_path = empty;

	host_interface_->host_information_dotnet_root = empty;

	host_interface_->host_information_application_path = empty;

	return 1;
}

uint8_t host_interface_set_config(
	void* host_interface,
	const type_of_element** keys,
	const type_of_element** values,
	size_t count)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->config_keys.arguments = keys;

	host_interface_->config_keys.length = count;

	host_interface_->config_values.arguments = values;

	host_interface_->config_values.length = count;

	return 1;
}

uint8_t host_interface_set_framework_directory(
	void* host_interface, const type_of_element* directory)
{
	if (!host_interface ||
		!directory)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->fx_dir = directory;

	return 1;
}

uint8_t host_interface_set_framework_name(
	void* host_interface, const type_of_element* name)
{
	if (!host_interface ||
		!name)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->fx_name = name;

	return 1;
}

uint8_t host_interface_set_dependency_file(
	void* host_interface, const type_of_element* dependency_file)
{
	if (!host_interface ||
		!dependency_file)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->dependency_file = dependency_file;

	return 1;
}

uint8_t host_interface_set_framework_dependent(
	void* host_interface, size_t is_framework_dependent)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->is_framework_dependent = is_framework_dependent;

	return 1;
}

uint8_t host_interface_set_paths_for_probing(
	void* host_interface,
	const type_of_element** paths,
	size_t count)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->paths_for_probing.arguments = paths;

	host_interface_->paths_for_probing.length = count;

	return 1;
}

uint8_t host_interface_set_patch_roll_forward(
	void* host_interface, size_t patch_roll_forward)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->patch_roll_forward = patch_roll_forward;

	return 1;
}

uint8_t host_interface_set_prerelease_roll_forward(
	void* host_interface, size_t prerelease_roll_forward)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->prerelease_roll_forward = prerelease_roll_forward;

	return 1;
}

uint8_t host_interface_set_host_mode(
	void* host_interface, size_t host_mode)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->host_mode = host_mode;

	return 1;
}

uint8_t host_interface_set_target_framework_moniker(
	void* host_interface, const type_of_element* target_framework_moniker)
{
	if (!host_interface ||
		!target_framework_moniker)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->target_framework_moniker = target_framework_moniker;

	return 1;
}

uint8_t host_interface_set_additional_dependency_serialized(
	void* host_interface, const type_of_element* additional_dependency_serialized)
{
	if (!host_interface ||
		!additional_dependency_serialized)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->additional_dependency_serialized = additional_dependency_serialized;

	return 1;
}

uint8_t host_interface_set_framework_version(
	void* host_interface, const type_of_element* framework_version)
{
	if (!host_interface ||
		!framework_version)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->fx_version = framework_version;

	return 1;
}

uint8_t host_interface_set_framework_names(
	void* host_interface,
	const type_of_element** names,
	size_t count)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->fx_names.arguments = names;

	host_interface_->fx_names.length = count;

	return 1;
}

uint8_t host_interface_set_framework_directories(
	void* host_interface,
	const type_of_element** directories,
	size_t count)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->fx_directories.arguments = directories;

	host_interface_->fx_directories.length = count;

	return 1;
}

uint8_t host_interface_set_framework_requested_versions(
	void* host_interface,
	const type_of_element** versions,
	size_t count)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->fx_requested_versions.arguments = versions;

	host_interface_->fx_requested_versions.length = count;

	return 1;
}

uint8_t host_interface_set_framework_found_versions(
	void* host_interface,
	const type_of_element** versions,
	size_t count)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->fx_found_versions.arguments = versions;

	host_interface_->fx_found_versions.length = count;

	return 1;
}

uint8_t host_interface_set_host_command(
	void* host_interface,
	const type_of_element* host_command)
{
	if (!host_interface ||
		!host_command)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->host_command = host_command;

	return 1;
}

uint8_t host_interface_set_host_information(
	void* host_interface,
	const type_of_element* host_path,
	const type_of_element* dotnet_root,
	const type_of_element* application_path)
{
	if (!host_interface ||
		!host_path ||
		!dotnet_root ||
		!application_path)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->host_information_host_path = host_path;

	host_interface_->host_information_dotnet_root = dotnet_root;

	host_interface_->host_information_application_path = application_path;

	return 1;
}

uint8_t host_interface_set_file_bundle_header_offset(
	void* host_interface, size_t single_file_bundle_header_offset)
{
	if (!host_interface)
	{
		return 0;
	}

	struct host_interface_type* host_interface_ = (struct host_interface_type*)host_interface;

	host_interface_->single_file_bundle_header_offset = single_file_bundle_header_offset;

	return 1;
}

static uint8_t host_interface[UINT8_MAX];

uint8_t host_interface_exec_function(
	uint8_t function,
	const struct buffer* arguments, uint8_t arguments_count)
{
	if (!arguments ||
		!arguments_count)
	{
		return 0;
	}

	struct range arguments_in_range[10];

	if (!common_get_arguments(arguments, arguments_count, arguments_in_range, 1))
	{
		return 0;
	}

	switch (function)
	{
		case host_interface_initialize_:
			return 1 == arguments_count &&
				   host_interface_init(
					   host_interface,
					   UINT8_MAX,
					   int_parse(arguments_in_range[0].start));

		case host_interface_set_additional_dependency_serialized_:
			break;

		case host_interface_set_configuration_:
			break;

		case host_interface_set_dependency_file_:
			break;

		case host_interface_set_file_bundle_header_offset_:
			break;

		case host_interface_set_framework_dependent_:
			break;

		case host_interface_set_framework_directories_:
			break;

		case host_interface_set_framework_directory_:
			break;

		case host_interface_set_framework_found_versions_:
			break;

		case host_interface_set_framework_name_:
			break;

		case host_interface_set_framework_names_:
			break;

		case host_interface_set_framework_requested_versions_:
			break;

		case host_interface_set_framework_version_:
			break;

		case host_interface_set_host_command_:
			break;

		case host_interface_set_host_information_:
			break;

		case host_interface_set_host_mode_:
			break;

		case host_interface_set_patch_roll_forward_:
			break;

		case host_interface_set_paths_for_probing_:
			break;

		case host_interface_set_prerelease_roll_forward_:
			break;

		case host_interface_set_target_framework_moniker_:
			break;

		default:
			break;
	}

	return 0;
}

void* host_interface_get()
{
	return (void*)host_interface;
}
