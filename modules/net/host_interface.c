/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "host_interface.h"
#include "arguments.h"
#include "net.common.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

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

static uint8_t is_buffer_init = 0;

static uint8_t b_config_keys_buffer[BUFFER_SIZE_OF];
static void* b_config_keys_ = (void*)b_config_keys_buffer;
static uint8_t b_config_values_buffer[BUFFER_SIZE_OF];
static void* b_config_values_ = (void*)b_config_values_buffer;
static uint8_t b_fx_dir_buffer[BUFFER_SIZE_OF];
static void* b_fx_dir_ = (void*)b_fx_dir_buffer;
static uint8_t b_fx_name_buffer[BUFFER_SIZE_OF];
static void* b_fx_name_ = (void*)b_fx_name_buffer;
static uint8_t b_dependency_file_buffer[BUFFER_SIZE_OF];
static void* b_dependency_file_ = (void*)b_dependency_file_buffer;
static uint8_t b_paths_for_probing_buffer[BUFFER_SIZE_OF];
static void* b_paths_for_probing_ = (void*)b_paths_for_probing_buffer;
static uint8_t b_target_framework_moniker_buffer[BUFFER_SIZE_OF];
static void* b_target_framework_moniker_ = (void*)b_target_framework_moniker_buffer;
static uint8_t b_additional_dependency_serialized_buffer[BUFFER_SIZE_OF];
static void* b_additional_dependency_serialized_ = (void*)b_additional_dependency_serialized_buffer;
static uint8_t b_fx_version_buffer[BUFFER_SIZE_OF];
static void* b_fx_version_ = (void*)b_fx_version_buffer;
static uint8_t b_fx_names_buffer[BUFFER_SIZE_OF];
static void* b_fx_names_ = (void*)b_fx_names_buffer;
static uint8_t b_fx_directories_buffer[BUFFER_SIZE_OF];
static void* b_fx_directories_ = (void*)b_fx_directories_buffer;
static uint8_t b_fx_requested_versions_buffer[BUFFER_SIZE_OF];
static void* b_fx_requested_versions_ = (void*)b_fx_requested_versions_buffer;
static uint8_t b_fx_found_versions_buffer[BUFFER_SIZE_OF];
static void* b_fx_found_versions_ = (void*)b_fx_found_versions_buffer;
static uint8_t b_host_command_buffer[BUFFER_SIZE_OF];
static void* b_host_command_ = (void*)b_host_command_buffer;
static uint8_t b_host_information_host_path_buffer[BUFFER_SIZE_OF];
static void* b_host_information_host_path_ = (void*)b_host_information_host_path_buffer;
static uint8_t b_host_information_dotnet_root_buffer[BUFFER_SIZE_OF];
static void* b_host_information_dotnet_root_ = (void*)b_host_information_dotnet_root_buffer;
static uint8_t b_host_information_application_path_buffer[BUFFER_SIZE_OF];
static void* b_host_information_application_path_ = (void*)b_host_information_application_path_buffer;

void host_interface_init_buffers()
{
	if (!is_buffer_init)
	{
		b_config_keys_ = (void*)b_config_keys_buffer;
		b_config_values_ = (void*)b_config_values_buffer;
		b_fx_dir_ = (void*)b_fx_dir_buffer;
		b_fx_name_ = (void*)b_fx_name_buffer;
		b_dependency_file_ = (void*)b_dependency_file_buffer;
		b_paths_for_probing_ = (void*)b_paths_for_probing_buffer;
		b_target_framework_moniker_ = (void*)b_target_framework_moniker_buffer;
		b_additional_dependency_serialized_ = (void*)b_additional_dependency_serialized_buffer;
		b_fx_version_ = (void*)b_fx_version_buffer;
		b_fx_names_ = (void*)b_fx_names_buffer;
		b_fx_directories_ = (void*)b_fx_directories_buffer;
		b_fx_requested_versions_ = (void*)b_fx_requested_versions_buffer;
		b_fx_found_versions_ = (void*)b_fx_found_versions_buffer;
		b_host_command_ = (void*)b_host_command_buffer;
		b_host_information_host_path_ = (void*)b_host_information_host_path_buffer;
		b_host_information_dotnet_root_ = (void*)b_host_information_dotnet_root_buffer;
		b_host_information_application_path_ = (void*)b_host_information_application_path_buffer;
		/**/
		is_buffer_init = buffer_init(b_config_keys_, BUFFER_SIZE_OF) &&
						 buffer_init(b_config_values_, BUFFER_SIZE_OF) &&
						 buffer_init(b_fx_dir_, BUFFER_SIZE_OF) &&
						 buffer_init(b_fx_name_, BUFFER_SIZE_OF) &&
						 buffer_init(b_dependency_file_, BUFFER_SIZE_OF) &&
						 buffer_init(b_paths_for_probing_, BUFFER_SIZE_OF) &&
						 buffer_init(b_target_framework_moniker_, BUFFER_SIZE_OF) &&
						 buffer_init(b_additional_dependency_serialized_, BUFFER_SIZE_OF) &&
						 buffer_init(b_fx_version_, BUFFER_SIZE_OF) &&
						 buffer_init(b_fx_names_, BUFFER_SIZE_OF) &&
						 buffer_init(b_fx_directories_, BUFFER_SIZE_OF) &&
						 buffer_init(b_fx_requested_versions_, BUFFER_SIZE_OF) &&
						 buffer_init(b_fx_found_versions_, BUFFER_SIZE_OF) &&
						 buffer_init(b_host_command_, BUFFER_SIZE_OF) &&
						 buffer_init(b_host_information_host_path_, BUFFER_SIZE_OF) &&
						 buffer_init(b_host_information_dotnet_root_, BUFFER_SIZE_OF) &&
						 buffer_init(b_host_information_application_path_, BUFFER_SIZE_OF);
	}
}

void host_interface_release_buffers()
{
	if (is_buffer_init)
	{
		buffer_release(b_config_keys_);
		buffer_release(b_config_values_);
		buffer_release(b_fx_dir_);
		buffer_release(b_fx_name_);
		buffer_release(b_dependency_file_);
		buffer_release(b_paths_for_probing_);
		buffer_release(b_target_framework_moniker_);
		buffer_release(b_additional_dependency_serialized_);
		buffer_release(b_fx_version_);
		buffer_release(b_fx_names_);
		buffer_release(b_fx_directories_);
		buffer_release(b_fx_requested_versions_);
		buffer_release(b_fx_found_versions_);
		buffer_release(b_host_command_);
		buffer_release(b_host_information_host_path_);
		buffer_release(b_host_information_dotnet_root_);
		buffer_release(b_host_information_application_path_);
	}
}

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
#if defined(_WIN32)
#define HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(THE_INTERFACE, VALUE, LENGTH, MEMBER, THE_BUFFER)		\
	host_interface_init_buffers();																		\
	\
	if (!THE_INTERFACE ||																				\
		!VALUE ||																						\
		!buffer_resize(THE_BUFFER, 0))																	\
	{																									\
		return 0;																						\
	}																									\
	\
	struct host_interface_type* THE_INTERFACE_ = (struct host_interface_type*)THE_INTERFACE;			\
	\
	THE_INTERFACE_->MEMBER = empty;																		\
	if (!text_encoding_UTF8_to_UTF16LE(VALUE, VALUE + LENGTH, THE_BUFFER) ||							\
		!buffer_push_back_uint16_t(THE_BUFFER, 0))														\
	{																									\
		return 0;																						\
	}																									\
	\
	THE_INTERFACE_->MEMBER = (const type_of_element*)buffer_data(THE_BUFFER, 0);						\
	return 1;

#define HOST_INTERFACE_SET_SYSTEM_PATH_TO_MEMBER_WIN(THE_INTERFACE, VALUE, LENGTH, MEMBER, THE_BUFFER)	\
	host_interface_init_buffers();																		\
	\
	if (!THE_INTERFACE ||																				\
		!VALUE ||																						\
		!buffer_resize(THE_BUFFER, 0))																	\
	{																									\
		return 0;																						\
	}																									\
	\
	struct host_interface_type* THE_INTERFACE_ = (struct host_interface_type*)THE_INTERFACE;			\
	\
	THE_INTERFACE_->MEMBER = empty;																		\
	if (!value_to_system_path(VALUE, LENGTH, THE_BUFFER))												\
	{																									\
		return 0;																						\
	}																									\
	\
	THE_INTERFACE_->MEMBER = (const type_of_element*)buffer_data(THE_BUFFER, LENGTH + 1);				\
	return 1;
#else
#define HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(THE_INTERFACE, VALUE, LENGTH, MEMBER, THE_BUFFER)		\
	host_interface_init_buffers();																		\
	\
	if (!THE_INTERFACE ||																				\
		!VALUE ||																						\
		!buffer_resize(THE_BUFFER, 0))																	\
	{																									\
		return 0;																						\
	}																									\
	\
	struct host_interface_type* THE_INTERFACE_ = (struct host_interface_type*)THE_INTERFACE;			\
	\
	THE_INTERFACE_->MEMBER = empty;																		\
	if (!buffer_append(THE_BUFFER, VALUE, LENGTH) ||													\
		!buffer_push_back(THE_BUFFER, 0))																\
	{																									\
		return 0;																						\
	}																									\
	\
	THE_INTERFACE_->MEMBER = (const type_of_element*)buffer_data(THE_BUFFER, 0);						\
	return 1;
#endif

#define HOST_INTERFACE_SET_SIZE_T_MEMBER(THE_INTERFACE, VALUE, MEMBER)							\
	if (!THE_INTERFACE)																			\
	{																							\
		return 0;																				\
	}																							\
	\
	struct host_interface_type* THE_INTERFACE_ = (struct host_interface_type*)THE_INTERFACE;	\
	THE_INTERFACE_->MEMBER = VALUE;																\
	return 1;

#define HOST_INTERFACE_SET_SYSTEM_PATHS_TO_MEMBER(THE_INTERFACE, VALUES, LENGTHS, COUNT, MEMBER, THE_BUFFER)	\
	host_interface_init_buffers();																				\
	\
	if (!THE_INTERFACE ||																						\
		!VALUES ||																								\
		!LENGTHS ||																								\
		!buffer_resize(THE_BUFFER, 0))																			\
	{																											\
		return 0;																								\
	}																											\
	\
	struct host_interface_type* THE_INTERFACE_ = (struct host_interface_type*)THE_INTERFACE;					\
	THE_INTERFACE_->MEMBER.arguments = NULL;																	\
	THE_INTERFACE_->MEMBER.length = COUNT;																		\
	return values_to_system_paths_(																				\
			VALUES, LENGTHS,																					\
			COUNT, THE_BUFFER,																					\
			&THE_INTERFACE_->MEMBER.arguments);

uint8_t host_interface_set_additional_dependency_serialized(
	void* host_interface,
	const uint8_t* additional_dependency_serialized,
	uint16_t additional_dependency_serialized_length)
{
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		additional_dependency_serialized,
		additional_dependency_serialized_length,
		additional_dependency_serialized,
		b_additional_dependency_serialized_);
}

uint8_t host_interface_set_application_path(
	void* host_interface,
	const uint8_t* application_path,
	uint16_t application_path_length)
{
#if defined(_WIN32)
	HOST_INTERFACE_SET_SYSTEM_PATH_TO_MEMBER_WIN(
		host_interface,
		application_path,
		application_path_length,
		host_information_application_path,
		b_host_information_application_path_);
#else
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		application_path,
		application_path_length,
		host_information_application_path,
		b_host_information_application_path_);
#endif
}

uint8_t host_interface_set_config_keys(
	void* host_interface,
	const uint8_t** keys, const uint16_t* keys_lengths,
	uint8_t count)
{
	host_interface_init_buffers();
	SET_DATA_FOR_STRING_MEMBER_OF_STRUCTURE(
		host_interface_type,
		host_interface,
		keys,
		keys_lengths,
		count,
		config_keys,
		b_config_keys_);
}

uint8_t host_interface_set_config_values(
	void* host_interface,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t count)
{
	host_interface_init_buffers();
	SET_DATA_FOR_STRING_MEMBER_OF_STRUCTURE(
		host_interface_type,
		host_interface,
		values,
		values_lengths,
		count,
		config_values,
		b_config_values_);
}

uint8_t host_interface_set_dependency_file(
	void* host_interface,
	const uint8_t* dependency_file,
	uint16_t dependency_file_length)
{
#if defined(_WIN32)
	HOST_INTERFACE_SET_SYSTEM_PATH_TO_MEMBER_WIN(
		host_interface,
		dependency_file,
		dependency_file_length,
		dependency_file,
		b_dependency_file_);
#else
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		dependency_file,
		dependency_file_length,
		dependency_file,
		b_dependency_file_);
#endif
}

uint8_t host_interface_set_dotnet_root(
	void* host_interface,
	const uint8_t* dotnet_root,
	uint16_t dotnet_root_length)
{
#if defined(_WIN32)
	HOST_INTERFACE_SET_SYSTEM_PATH_TO_MEMBER_WIN(
		host_interface,
		dotnet_root,
		dotnet_root_length,
		host_information_dotnet_root,
		b_host_information_dotnet_root_);
#else
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		dotnet_root,
		dotnet_root_length,
		host_information_dotnet_root,
		b_host_information_dotnet_root_);
#endif
}

uint8_t host_interface_set_file_bundle_header_offset(
	void* host_interface, size_t single_file_bundle_header_offset)
{
	HOST_INTERFACE_SET_SIZE_T_MEMBER(
		host_interface,
		single_file_bundle_header_offset,
		single_file_bundle_header_offset);
}

uint8_t host_interface_set_framework_dependent(
	void* host_interface, size_t is_framework_dependent)
{
	HOST_INTERFACE_SET_SIZE_T_MEMBER(
		host_interface,
		is_framework_dependent,
		is_framework_dependent);
}

uint8_t host_interface_set_framework_directories(
	void* host_interface,
	const uint8_t** directories, const uint16_t* directories_lengths,
	uint8_t count)
{
	HOST_INTERFACE_SET_SYSTEM_PATHS_TO_MEMBER(
		host_interface,
		directories,
		directories_lengths,
		count,
		fx_directories,
		b_fx_directories_);
}

uint8_t host_interface_set_framework_directory(
	void* host_interface, const uint8_t* directory, uint16_t directory_length)
{
#if defined(_WIN32)
	HOST_INTERFACE_SET_SYSTEM_PATH_TO_MEMBER_WIN(
		host_interface,
		directory,
		directory_length,
		fx_dir,
		b_fx_dir_);
#else
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		directory,
		directory_length,
		fx_dir,
		b_fx_dir_);
#endif
}

uint8_t host_interface_set_framework_found_versions(
	void* host_interface,
	const uint8_t** versions, const uint16_t* versions_lengths,
	uint8_t count)
{
	host_interface_init_buffers();
	SET_DATA_FOR_STRING_MEMBER_OF_STRUCTURE(
		host_interface_type,
		host_interface,
		versions,
		versions_lengths,
		count,
		fx_found_versions,
		b_fx_found_versions_);
}

uint8_t host_interface_set_framework_name(
	void* host_interface, const uint8_t* name, uint16_t name_length)
{
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		name,
		name_length,
		fx_name,
		b_fx_name_);
}

uint8_t host_interface_set_framework_names(
	void* host_interface,
	const uint8_t** names, const uint16_t* names_lengths,
	uint8_t count)
{
	host_interface_init_buffers();
	SET_DATA_FOR_STRING_MEMBER_OF_STRUCTURE(
		host_interface_type,
		host_interface,
		names,
		names_lengths,
		count,
		fx_names,
		b_fx_names_);
}

uint8_t host_interface_set_framework_requested_versions(
	void* host_interface,
	const uint8_t** versions, const uint16_t* versions_lengths,
	uint8_t count)
{
	host_interface_init_buffers();
	SET_DATA_FOR_STRING_MEMBER_OF_STRUCTURE(
		host_interface_type,
		host_interface,
		versions,
		versions_lengths,
		count,
		fx_requested_versions,
		b_fx_requested_versions_);
}

uint8_t host_interface_set_framework_version(
	void* host_interface,
	const uint8_t* framework_version, uint16_t framework_version_length)
{
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		framework_version,
		framework_version_length,
		fx_version,
		b_fx_version_);
}

uint8_t host_interface_set_host_command(
	void* host_interface,
	const uint8_t* host_command, uint16_t host_command_length)
{
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		host_command,
		host_command_length,
		host_command,
		b_host_command_);
}

uint8_t host_interface_set_host_mode(
	void* host_interface,
	const uint8_t* mode, uint16_t mode_length)
{
	static const uint8_t* modes[] =
	{
		(const uint8_t*)"invalid",
		(const uint8_t*)"muxer",
		(const uint8_t*)"apphost",
		(const uint8_t*)"split_fx",
		(const uint8_t*)"libhost"
	};
	/**/
	const uint8_t count = COUNT_OF(modes);
	size_t index = common_string_to_enum(
					   mode, mode + mode_length, modes, count);

	if (count == index)
	{
		index = (size_t)uint64_parse(mode, mode + mode_length);
	}

	HOST_INTERFACE_SET_SIZE_T_MEMBER(
		host_interface,
		index,
		host_mode);
}

uint8_t host_interface_set_host_path(
	void* host_interface,
	const uint8_t* path, uint16_t path_length)
{
#if defined(_WIN32)
	HOST_INTERFACE_SET_SYSTEM_PATH_TO_MEMBER_WIN(
		host_interface,
		path,
		path_length,
		host_information_host_path,
		b_host_information_host_path_);
#else
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		path,
		path_length,
		host_information_host_path,
		b_host_information_host_path_);
#endif
}

uint8_t host_interface_set_patch_roll_forward(
	void* host_interface, size_t patch_roll_forward)
{
	HOST_INTERFACE_SET_SIZE_T_MEMBER(
		host_interface,
		patch_roll_forward,
		patch_roll_forward);
}

uint8_t host_interface_set_paths_for_probing(
	void* host_interface,
	const uint8_t** paths, const uint16_t* paths_lengths,
	uint8_t count)
{
	HOST_INTERFACE_SET_SYSTEM_PATHS_TO_MEMBER(
		host_interface,
		paths,
		paths_lengths,
		count,
		paths_for_probing,
		b_paths_for_probing_);
}

uint8_t host_interface_set_prerelease_roll_forward(
	void* host_interface, size_t prerelease_roll_forward)
{
	HOST_INTERFACE_SET_SIZE_T_MEMBER(
		host_interface,
		prerelease_roll_forward,
		prerelease_roll_forward);
}

uint8_t host_interface_set_target_framework_moniker(
	void* host_interface,
	const uint8_t* target_framework_moniker,
	uint16_t target_framework_moniker_length)
{
	HOST_INTERFACE_SET_TYPE_OF_ELEMENT_MEMBER(
		host_interface,
		target_framework_moniker,
		target_framework_moniker_length,
		target_framework_moniker,
		b_target_framework_moniker_);
}

static uint8_t g_host_interface[HOST_INTERFACE_SIZE];

void* host_interface_get()
{
	return (void*)g_host_interface;
}
