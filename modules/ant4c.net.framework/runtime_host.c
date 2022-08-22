/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, 2022 TheVice
 *
 */

#include "runtime_host.h"

#include "buffer.h"
#include "common.h"
#include "text_encoding.h"

#include <metahost.h>

#include <wchar.h>
#include <stddef.h>

uint8_t runtime_host_start(void* host)
{
	if (!host)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->Start(host);
	return SUCCEEDED(result);
}

uint8_t runtime_host_stop(void* host)
{
	if (!host)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->Stop(host);
	return SUCCEEDED(result);
}

uint8_t runtime_host_set_host_control(void* host, void* control)
{
	if (!host ||
		!control)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->SetHostControl(host, control);
	return SUCCEEDED(result);
}

uint8_t runtime_host_get_clr_control(void* host, void** control)
{
	if (!host ||
		!control)
	{
		return 0;
	}

	struct ICLRControl* control_ = NULL;

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->GetCLRControl(host, &control_);

	*control = control_;

	return SUCCEEDED(result);
}

uint8_t runtime_host_unload_app_domain(void* host, unsigned long app_domain_id, uint8_t wait_untile_done)
{
	if (!host)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->UnloadAppDomain(
							   host, app_domain_id, wait_untile_done);
	return SUCCEEDED(result);
}

/*uint8_t runtime_host_execute_in_app_domain(
	void* host, unsigned long app_domain_id, void* call_back, void* cookie)
{
	if (!host ||
		!call_back ||
		!cookie)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->ExecuteInAppDomain(
							   host, app_domain_id, call_back, cookie);
	return SUCCEEDED(result);
	return 0;
}*/

uint8_t runtime_host_get_current_app_domain_id(void* host, unsigned long* app_domain_id)
{
	if (!host ||
		!app_domain_id)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->GetCurrentAppDomainId(host, app_domain_id);
	return SUCCEEDED(result);
}

/*uint8_t runtime_host_execute_application_wchar_t(
	void* host, const wchar_t* app_full_name,
	unsigned long manifest_paths_count, const wchar_t** manifest_paths,
	unsigned long activation_data_count, const wchar_t** activation_data,
	int32_t* return_value)
{
	if (!host ||
		!app_full_name ||
		!manifest_paths ||
		!activation_data ||
		!return_value)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->ExecuteApplication(
							   host, app_full_name,
							   manifest_paths_count, manifest_paths,
							   activation_data_count, activation_data, return_value);
	return SUCCEEDED(result);
}

uint8_t runtime_host_execute_application(
	void* host, const uint8_t* app_full_name,
	unsigned long manifest_paths_count, const uint8_t** manifest_paths,
	unsigned long activation_data_count, const uint8_t** activation_data,
	int32_t* return_value)
{
	if (!host ||
		!app_full_name ||
		!manifest_paths ||
		!activation_data ||
		!return_value)
	{
		return 0;
	}

	return 1;
}*/

uint8_t runtime_host_execute_in_default_app_domain_wchar_t(
	void* host, const wchar_t* assembly_path,
	const wchar_t* type_name, const wchar_t* method_name,
	const wchar_t* argument, unsigned long* return_value)
{
	if (!host ||
		!assembly_path ||
		!type_name ||
		!method_name ||
		!argument ||
		!return_value)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeHost*)host)->lpVtbl->ExecuteInDefaultAppDomain(host, assembly_path,
						   type_name, method_name, argument, return_value);
	return SUCCEEDED(result);
}

uint8_t runtime_host_execute_in_default_app_domain(
	void* host, const uint8_t* assembly_path,
	const uint8_t* type_name, const uint8_t* method_name,
	const uint8_t* argument, unsigned long* return_value)
{
	if (!host ||
		!assembly_path ||
		!type_name ||
		!method_name ||
		!argument ||
		!return_value)
	{
		return 0;
	}

	ptrdiff_t assembly_path_size = common_count_bytes_until(assembly_path, 0);
	ptrdiff_t type_name_size = common_count_bytes_until(type_name, 0);
	ptrdiff_t method_name_size = common_count_bytes_until(method_name, 0);
	ptrdiff_t argument_size = common_count_bytes_until(argument, 0);
	/**/
	const ptrdiff_t size = assembly_path_size + type_name_size +
						   method_name_size + argument_size + 4 * sizeof(uint32_t);
	/**/
	uint8_t arguments_W_buffer[BUFFER_SIZE_OF];
	void* arguments_W = (void*)arguments_W_buffer;

	if (!buffer_init(arguments_W, BUFFER_SIZE_OF))
	{
		return 0;
	}

	if (!buffer_append_wchar_t(arguments_W, NULL, size) ||
		!buffer_resize(arguments_W, 0))
	{
		return 0;
	}

	if (!text_encoding_UTF8_to_UTF16LE(assembly_path, assembly_path + assembly_path_size, arguments_W) ||
		!buffer_push_back_uint16_t(arguments_W, 0))
	{
		return 0;
	}

	assembly_path_size = buffer_size(arguments_W);

	if (!text_encoding_UTF8_to_UTF16LE(type_name, type_name + type_name_size, arguments_W) ||
		!buffer_push_back_uint16_t(arguments_W, 0))
	{
		return 0;
	}

	type_name_size = buffer_size(arguments_W);

	if (!text_encoding_UTF8_to_UTF16LE(method_name, method_name + method_name_size, arguments_W) ||
		!buffer_push_back_uint16_t(arguments_W, 0))
	{
		return 0;
	}

	method_name_size = buffer_size(arguments_W);

	if (!text_encoding_UTF8_to_UTF16LE(argument, argument + argument_size, arguments_W) ||
		!buffer_push_back_uint16_t(arguments_W, 0))
	{
		return 0;
	}

	const wchar_t* assembly_path_W = buffer_wchar_t_data(arguments_W, 0);
	const wchar_t* type_name_W = (const wchar_t*)buffer_data(arguments_W, assembly_path_size);
	const wchar_t* method_name_W = (const wchar_t*)buffer_data(arguments_W, type_name_size);
	const wchar_t* argument_W = (const wchar_t*)buffer_data(arguments_W, method_name_size);
	/**/
	argument_size = runtime_host_execute_in_default_app_domain_wchar_t(
						host, assembly_path_W, type_name_W,
						method_name_W, argument_W, return_value);
	/**/
	buffer_release(arguments_W);
	/**/
	return 0 < argument_size;
}
