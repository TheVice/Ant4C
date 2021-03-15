/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 https://github.com/TheVice/
 *
 */

#include "host_fxr.h"
#include "buffer.h"
#include "common.h"
#include "shared_object.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
extern void* shared_object_load_wchar_t(const wchar_t* path);
#endif

struct _paths
{
	size_t size;
	const type_of_element* path_to_assembly;
	const type_of_element* path_to_dot_net_root;
};

static struct _paths paths;

#define SET_PARAMETERS(PARAMETERS, PATH_TO_ASSEMBLY, PATH_TO_DOT_NET_ROOT, PARAMETERS_OBJECT)	\
	\
	if ((PATH_TO_ASSEMBLY) || (PATH_TO_DOT_NET_ROOT))											\
	{																							\
		(PARAMETERS_OBJECT).size = sizeof(struct _paths);										\
		(PARAMETERS_OBJECT).path_to_assembly = (PATH_TO_ASSEMBLY);								\
		(PARAMETERS_OBJECT).path_to_dot_net_root = (PATH_TO_DOT_NET_ROOT);						\
		(PARAMETERS) = (const void*)&(PARAMETERS_OBJECT);										\
	}

typedef int32_t(*get_hostfxr_path_type)(type_of_element* output, size_t* output_size, const void* parameters);

uint8_t net_host_load(const type_of_element* path_to_net_host_object,
					  const type_of_element* path_to_assembly, const type_of_element* path_to_dot_net_root,
					  void** net_host_object, struct buffer* path_to_host_fxr)
{
	if (!path_to_net_host_object || !path_to_host_fxr)
	{
		return 0;
	}

	void* net_host_object_ = NULL;
#if defined(_WIN32)
	net_host_object_ = shared_object_load_wchar_t(path_to_net_host_object);
#else
	net_host_object_ = shared_object_load(path_to_net_host_object);
#endif

	if (!net_host_object_)
	{
		return 0;
	}

	get_hostfxr_path_type get_hostfxr_path =
		(get_hostfxr_path_type)shared_object_get_procedure_address(
			net_host_object_, (const uint8_t*)"get_hostfxr_path");

	if (!get_hostfxr_path)
	{
		shared_object_unload(net_host_object_);
		return 0;
	}

	const void* parameters = NULL;
	SET_PARAMETERS(parameters, path_to_assembly, path_to_dot_net_root, paths);
	const ptrdiff_t size = buffer_size(path_to_host_fxr);

	if (!buffer_append(path_to_host_fxr, NULL, FILENAME_MAX * sizeof(type_of_element)))
	{
		shared_object_unload(net_host_object_);
		return 0;
	}

	int32_t result = 0;
	size_t required_size = FILENAME_MAX;

	if (host_fxr_Success != (result = get_hostfxr_path((type_of_element*)buffer_data(
										  path_to_host_fxr, size), &required_size, parameters)))
	{
		if ((int32_t)host_fxr_HostApiBufferTooSmall != result ||
			required_size < FILENAME_MAX ||
			!buffer_resize(path_to_host_fxr, size) ||
			!buffer_append(path_to_host_fxr, NULL, required_size * sizeof(type_of_element)) ||
			host_fxr_Success != (/*result = */get_hostfxr_path((type_of_element*)buffer_data(
					path_to_host_fxr, size), &required_size, parameters)))
		{
			shared_object_unload(net_host_object_);
			return 0;
		}
	}

	if (net_host_object)
	{
		*net_host_object = net_host_object_;
	}
	else
	{
		shared_object_unload(net_host_object_);
	}

	required_size *= sizeof(type_of_element);
	return buffer_resize(path_to_host_fxr, size + required_size);
}

typedef int32_t(calling_convention* hostfxr_close_type)(const void* context);
typedef int32_t(calling_convention* hostfxr_get_available_sdks_type)(
	const type_of_element* exe_dir, hostfxr_get_available_sdks_result_type result);
typedef int32_t(calling_convention* hostfxr_get_native_search_directories_type)(
	const int32_t argc, const type_of_element** argv, type_of_element* output,
	int32_t output_size, int32_t* required_output_size);
typedef int32_t(calling_convention* hostfxr_get_runtime_delegate_type)(
	const void* context, int32_t type_of_delegate, hostfxr_delegate_function_type* the_delegate);
typedef int32_t(calling_convention* hostfxr_get_runtime_properties_type)(
	const void* context, size_t* count,
	type_of_element** keys, type_of_element** values);
typedef int32_t(calling_convention* hostfxr_get_runtime_property_value_type)(
	const void* context, const type_of_element* name, const type_of_element** value);
typedef int32_t(calling_convention* hostfxr_initialize_for_dotnet_command_line_type)(
	int32_t argc, const type_of_element** argv,
	const void* parameters, void** context);
typedef int32_t(calling_convention* hostfxr_initialize_for_runtime_config_type)(
	const type_of_element* runtime_config_path,
	const void* parameters, void** context);
typedef int32_t(calling_convention* hostfxr_main_type)(const int32_t argc, const type_of_element** argv);
typedef int32_t(calling_convention* hostfxr_main_bundle_startupinfo_type)(
	const int32_t argc, const type_of_element** argv, const type_of_element* host_path,
	const type_of_element* dotnet_root, const type_of_element* app_path, int64_t bundle_header_offset);
typedef int32_t(calling_convention* hostfxr_main_startupinfo_type)(
	const int32_t argc, const type_of_element** argv, const type_of_element* host_path,
	const type_of_element* dotnet_root, const type_of_element* app_path);
typedef int32_t(calling_convention* hostfxr_resolve_sdk_type)(
	const type_of_element* exe_dir, const type_of_element* working_dir,
	type_of_element* output, int32_t output_size);
typedef int32_t(calling_convention* hostfxr_resolve_sdk2_type)(
	const type_of_element* exe_dir, const type_of_element* working_dir,
	int32_t flags, hostfxr_resolve_sdk2_result_type result);
typedef int32_t(calling_convention* hostfxr_run_app_type)(const void* context);
typedef hostfxr_error_writer_type(calling_convention* hostfxr_set_error_writer_type)(
	hostfxr_error_writer_type writer);
typedef int32_t(calling_convention* hostfxr_set_runtime_property_value_type)(
	const void* context, const type_of_element* name, const type_of_element* value);

struct host_fxr
{
	void* shared_object;
	/**/
	hostfxr_close_type hostfxr_close;
	hostfxr_get_available_sdks_type hostfxr_get_available_sdks;
	hostfxr_get_native_search_directories_type hostfxr_get_native_search_directories;
	hostfxr_get_runtime_delegate_type hostfxr_get_runtime_delegate;
	hostfxr_get_runtime_properties_type hostfxr_get_runtime_properties;
	hostfxr_get_runtime_property_value_type hostfxr_get_runtime_property_value;
	hostfxr_initialize_for_dotnet_command_line_type hostfxr_initialize_for_dotnet_command_line;
	hostfxr_initialize_for_runtime_config_type hostfxr_initialize_for_runtime_config;
	hostfxr_main_type hostfxr_main;
	hostfxr_main_bundle_startupinfo_type hostfxr_main_bundle_startupinfo;
	hostfxr_main_startupinfo_type hostfxr_main_startupinfo;
	hostfxr_resolve_sdk_type hostfxr_resolve_sdk;
	hostfxr_resolve_sdk2_type hostfxr_resolve_sdk2;
	hostfxr_run_app_type hostfxr_run_app;
	hostfxr_set_error_writer_type hostfxr_set_error_writer;
	hostfxr_set_runtime_property_value_type hostfxr_set_runtime_property_value;
};

enum host_fxr_functions
{
	hostfxr_close_, hostfxr_get_available_sdks_, hostfxr_get_native_search_directories_,
	hostfxr_get_runtime_delegate_, hostfxr_get_runtime_properties_,
	hostfxr_get_runtime_property_value_, hostfxr_initialize_for_dotnet_command_line_,
	hostfxr_initialize_for_runtime_config_, hostfxr_main_, hostfxr_main_bundle_startupinfo_,
	hostfxr_main_startupinfo_, hostfxr_resolve_sdk_, hostfxr_resolve_sdk2_, hostfxr_run_app_,
	hostfxr_set_error_writer_, hostfxr_set_runtime_property_value_
};

static const uint8_t* host_fxr_functions_string[] =
{
	(const uint8_t*)"hostfxr_close",
	(const uint8_t*)"hostfxr_get_available_sdks",
	(const uint8_t*)"hostfxr_get_native_search_directories",
	(const uint8_t*)"hostfxr_get_runtime_delegate",
	(const uint8_t*)"hostfxr_get_runtime_properties",
	(const uint8_t*)"hostfxr_get_runtime_property_value",
	(const uint8_t*)"hostfxr_initialize_for_dotnet_command_line",
	(const uint8_t*)"hostfxr_initialize_for_runtime_config",
	(const uint8_t*)"hostfxr_main",
	(const uint8_t*)"hostfxr_main_bundle_startupinfo",
	(const uint8_t*)"hostfxr_main_startupinfo",
	(const uint8_t*)"hostfxr_resolve_sdk",
	(const uint8_t*)"hostfxr_resolve_sdk2",
	(const uint8_t*)"hostfxr_run_app",
	(const uint8_t*)"hostfxr_set_error_writer",
	(const uint8_t*)"hostfxr_set_runtime_property_value"
};

uint8_t host_fx_resolver_load(
	const type_of_element* path_to_host_fxr, void* ptr_to_host_fxr_object, ptrdiff_t size)
{
	if (!path_to_host_fxr ||
		!ptr_to_host_fxr_object ||
		size < (ptrdiff_t)sizeof(struct host_fxr))
	{
		return 0;
	}

	void* host_fx_object = NULL;
#if defined(_WIN32)
	host_fx_object = shared_object_load_wchar_t(path_to_host_fxr);
#else
	host_fx_object = shared_object_load(path_to_host_fxr);
#endif

	if (!host_fx_object)
	{
		return 0;
	}

	memset(ptr_to_host_fxr_object, 0, sizeof(struct host_fxr));
	struct host_fxr* ptr_to_host_fxr_object_ = (struct host_fxr*)ptr_to_host_fxr_object;

	for (uint8_t i = 0, count = COUNT_OF(host_fxr_functions_string); i < count; ++i)
	{
		void* address = shared_object_get_procedure_address(host_fx_object, host_fxr_functions_string[i]);

		if (!address)
		{
			continue;
		}

		switch (i)
		{
			case hostfxr_close_:
				ptr_to_host_fxr_object_->hostfxr_close =
					(hostfxr_close_type)address;
				break;

			case hostfxr_get_available_sdks_:
				ptr_to_host_fxr_object_->hostfxr_get_available_sdks =
					(hostfxr_get_available_sdks_type)address;
				break;

			case hostfxr_get_native_search_directories_:
				ptr_to_host_fxr_object_->hostfxr_get_native_search_directories =
					(hostfxr_get_native_search_directories_type)address;
				break;

			case hostfxr_get_runtime_delegate_:
				ptr_to_host_fxr_object_->hostfxr_get_runtime_delegate =
					(hostfxr_get_runtime_delegate_type)address;
				break;

			case hostfxr_get_runtime_properties_:
				ptr_to_host_fxr_object_->hostfxr_get_runtime_properties =
					(hostfxr_get_runtime_properties_type)address;
				break;

			case hostfxr_get_runtime_property_value_:
				ptr_to_host_fxr_object_->hostfxr_get_runtime_property_value =
					(hostfxr_get_runtime_property_value_type)address;
				break;

			case hostfxr_initialize_for_dotnet_command_line_:
				ptr_to_host_fxr_object_->hostfxr_initialize_for_dotnet_command_line =
					(hostfxr_initialize_for_dotnet_command_line_type)address;
				break;

			case hostfxr_initialize_for_runtime_config_:
				ptr_to_host_fxr_object_->hostfxr_initialize_for_runtime_config =
					(hostfxr_initialize_for_runtime_config_type)address;
				break;

			case hostfxr_main_:
				ptr_to_host_fxr_object_->hostfxr_main =
					(hostfxr_main_type)address;
				break;

			case hostfxr_main_bundle_startupinfo_:
				ptr_to_host_fxr_object_->hostfxr_main_bundle_startupinfo =
					(hostfxr_main_bundle_startupinfo_type)address;
				break;

			case hostfxr_main_startupinfo_:
				ptr_to_host_fxr_object_->hostfxr_main_startupinfo =
					(hostfxr_main_startupinfo_type)address;
				break;

			case hostfxr_resolve_sdk_:
				ptr_to_host_fxr_object_->hostfxr_resolve_sdk =
					(hostfxr_resolve_sdk_type)address;
				break;

			case hostfxr_resolve_sdk2_:
				ptr_to_host_fxr_object_->hostfxr_resolve_sdk2 =
					(hostfxr_resolve_sdk2_type)address;
				break;

			case hostfxr_run_app_:
				ptr_to_host_fxr_object_->hostfxr_run_app =
					(hostfxr_run_app_type)address;
				break;

			case hostfxr_set_error_writer_:
				ptr_to_host_fxr_object_->hostfxr_set_error_writer =
					(hostfxr_set_error_writer_type)address;
				break;

			case hostfxr_set_runtime_property_value_:
				ptr_to_host_fxr_object_->hostfxr_set_runtime_property_value =
					(hostfxr_set_runtime_property_value_type)address;
				break;

			default:
				break;
		}
	}

	ptr_to_host_fxr_object_->shared_object = host_fx_object;
	return 1;
}

void host_fx_resolver_unload(void* ptr_to_host_fxr_object)
{
	if (!ptr_to_host_fxr_object)
	{
		return;
	}

	struct host_fxr* ptr_to_host_fxr_object_ = (struct host_fxr*)(ptr_to_host_fxr_object);

	shared_object_unload(ptr_to_host_fxr_object_->shared_object);

	ptr_to_host_fxr_object_->shared_object = NULL;
}

uint8_t host_fx_resolver_is_function_exists(
	const void* ptr_to_host_fxr_object, const uint8_t* function_name, uint8_t function_name_length)
{
	static const uint8_t host_fxr_functions_lengths[] =
	{
		13,
		26,
		37,
		28,
		30,
		34,
		42,
		37,
		12,
		31,
		24,
		19,
		20,
		15,
		24,
		34
	};

	if (!ptr_to_host_fxr_object ||
		!function_name ||
		!function_name_length)
	{
		return 0;
	}

	struct host_fxr* ptr_to_host_fxr_object_ = (struct host_fxr*)ptr_to_host_fxr_object;

	for (uint8_t i = 0, count = COUNT_OF(host_fxr_functions_string); i < count; ++i)
	{
		if (host_fxr_functions_lengths[i] != function_name_length ||
			0 != memcmp(host_fxr_functions_string[i], function_name, function_name_length))
		{
			continue;
		}

		switch (i)
		{
			case hostfxr_close_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_close;

			case hostfxr_get_available_sdks_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_available_sdks;

			case hostfxr_get_native_search_directories_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_native_search_directories;

			case hostfxr_get_runtime_delegate_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_runtime_delegate;

			case hostfxr_get_runtime_properties_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_runtime_properties;

			case hostfxr_get_runtime_property_value_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_get_runtime_property_value;

			case hostfxr_initialize_for_dotnet_command_line_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_initialize_for_dotnet_command_line;

			case hostfxr_initialize_for_runtime_config_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_initialize_for_runtime_config;

			case hostfxr_main_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_main;

			case hostfxr_main_bundle_startupinfo_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_main_bundle_startupinfo;

			case hostfxr_main_startupinfo_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_main_startupinfo;

			case hostfxr_resolve_sdk_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_resolve_sdk;

			case hostfxr_resolve_sdk2_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_resolve_sdk2;

			case hostfxr_run_app_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_run_app;

			case hostfxr_set_error_writer_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_set_error_writer;

			case hostfxr_set_runtime_property_value_:
				return NULL != ptr_to_host_fxr_object_->hostfxr_set_runtime_property_value;

			default:
				break;
		}
	}

	return 0;
}

int32_t host_fxr_close(void* ptr_to_host_fxr_object, const void* context)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_close)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_close(context);
	}

	return -1;
}

int32_t host_fxr_get_available_sdks(
	void* ptr_to_host_fxr_object, const type_of_element* exe_dir, hostfxr_get_available_sdks_result_type result)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_available_sdks)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_available_sdks(exe_dir, result);
	}

	return -1;
}

int32_t host_fxr_get_native_search_directories(
	void* ptr_to_host_fxr_object, const int32_t argc, const type_of_element** argv,
	type_of_element* output, int32_t output_size, int32_t* required_output_size)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_native_search_directories)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_native_search_directories(
				   argc, argv, output, output_size, required_output_size);
	}

	return -1;
}

int32_t host_fxr_get_runtime_delegate(
	void* ptr_to_host_fxr_object, const void* context,
	int32_t type_of_delegate, hostfxr_delegate_function_type* the_delegate)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_delegate)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_delegate(
				   context, type_of_delegate, the_delegate);
	}

	return -1;
}

int32_t host_fxr_get_runtime_properties(
	void* ptr_to_host_fxr_object, const void* context,
	size_t* count, type_of_element** keys, type_of_element** values)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_properties)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_properties(
				   context, count, keys, values);
	}

	return -1;
}

int32_t host_fxr_get_runtime_property_value(
	void* ptr_to_host_fxr_object, const void* context, const type_of_element* name, const type_of_element** value)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_property_value)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_get_runtime_property_value(
				   context, name, value);
	}

	return -1;
}

int32_t host_fxr_initialize_for_dotnet_command_line(
	void* ptr_to_host_fxr_object, int32_t argc, const type_of_element** argv,
	const void* parameters, void** context)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_initialize_for_dotnet_command_line)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_initialize_for_dotnet_command_line(
				   argc, argv, parameters, context);
	}

	return -1;
}

int32_t host_fxr_initialize_for_dotnet_command_line_parameters_in_parts(
	void* ptr_to_host_fxr_object, int32_t argc, const type_of_element** argv,
	const type_of_element* path_to_assembly, const type_of_element* path_to_dot_net_root, void** context)
{
	const void* parameters = NULL;
	SET_PARAMETERS(parameters, path_to_assembly, path_to_dot_net_root, paths);
	return host_fxr_initialize_for_dotnet_command_line(ptr_to_host_fxr_object, argc, argv, parameters, context);
}

int32_t host_fxr_initialize_for_runtime_config(
	void* ptr_to_host_fxr_object, const type_of_element* runtime_config_path,
	const void* parameters, void** context)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_initialize_for_runtime_config)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_initialize_for_runtime_config(
				   runtime_config_path, parameters, context);
	}

	return -1;
}

int32_t host_fxr_initialize_for_runtime_config_parameters_in_parts(
	void* ptr_to_host_fxr_object, const type_of_element* runtime_config_path,
	const type_of_element* path_to_assembly, const type_of_element* path_to_dot_net_root, void** context)
{
	const void* parameters = NULL;
	SET_PARAMETERS(parameters, path_to_assembly, path_to_dot_net_root, paths);
	return host_fxr_initialize_for_runtime_config(
			   ptr_to_host_fxr_object, runtime_config_path, parameters, context);
}

int32_t host_fxr_main(void* ptr_to_host_fxr_object, const int32_t argc, const type_of_element** argv)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main(argc, argv);
	}

	return -1;
}

int32_t host_fxr_main_bundle_startupinfo(
	void* ptr_to_host_fxr_object, const int32_t argc, const type_of_element** argv,
	const type_of_element* host_path, const type_of_element* dotnet_root,
	const type_of_element* app_path, int64_t bundle_header_offset)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main_bundle_startupinfo)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main_bundle_startupinfo(
				   argc, argv, host_path, dotnet_root, app_path, bundle_header_offset);
	}

	return -1;
}

int32_t host_fxr_main_startupinfo(
	void* ptr_to_host_fxr_object, const int32_t argc, const type_of_element** argv,
	const type_of_element* host_path, const type_of_element* dotnet_root,
	const type_of_element* app_path)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main_startupinfo)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_main_startupinfo(
				   argc, argv, host_path, dotnet_root, app_path);
	}

	return -1;
}

int32_t host_fxr_resolve_sdk(
	void* ptr_to_host_fxr_object, const type_of_element* exe_dir,
	const type_of_element* working_dir, type_of_element* output, int32_t output_size)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_resolve_sdk)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_resolve_sdk(
				   exe_dir, working_dir, output, output_size);
	}

	return -1;
}

int32_t host_fxr_resolve_sdk2(
	void* ptr_to_host_fxr_object, const type_of_element* exe_dir,
	const type_of_element* working_dir, int32_t flags,
	hostfxr_resolve_sdk2_result_type result)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_resolve_sdk2)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_resolve_sdk2(exe_dir, working_dir, flags, result);
	}

	return -1;
}

int32_t host_fxr_run_app(void* ptr_to_host_fxr_object, const void* context)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_run_app)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_run_app(context);
	}

	return -1;
}

hostfxr_error_writer_type host_fxr_set_error_writer(
	void* ptr_to_host_fxr_object, hostfxr_error_writer_type writer)
{
	hostfxr_error_writer_type result = NULL;

	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_set_error_writer)
	{
		result = ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_set_error_writer(writer);
	}

	return result;
}

int32_t host_fxr_set_runtime_property_value(
	void* ptr_to_host_fxr_object, const void* context, const type_of_element* name, const type_of_element* value)
{
	if (ptr_to_host_fxr_object &&
		((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_set_runtime_property_value)
	{
		return ((struct host_fxr*)ptr_to_host_fxr_object)->hostfxr_set_runtime_property_value(context, name, value);
	}

	return -1;
}
