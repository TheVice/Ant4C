/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "runtime_info.h"
#include "buffer.h"
#include "text_encoding.h"

#include <wchar.h>
#include <stddef.h>
#include <string.h>

#include <metahost.h>

uint8_t runtime_info_get_version_string(void* info, struct buffer* output)
{
	if (!info ||
		!output)
	{
		return 0;
	}

	DWORD required_size = 0;
	HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->GetVersionString(info, NULL, &required_size);

	if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != result || !required_size)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (ptrdiff_t)4 * required_size + sizeof(uint32_t)))
	{
		return 0;
	}

	uint16_t* start = (uint16_t*)buffer_data(output, size + required_size);
	result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->GetVersionString(info, start, &required_size);

	if (FAILED(result))
	{
		return 0;
	}

	const uint16_t* finish = start + required_size;

	if (!buffer_resize(output, size) ||
		!text_encoding_UTF16LE_to_UTF8(start, finish, output))
	{
		return 0;
	}

	return 1;
}

uint8_t runtime_info_get_runtime_directory(void* info, struct buffer* output)
{
	if (!info ||
		!output)
	{
		return 0;
	}

	DWORD required_size = 0;
	HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->GetRuntimeDirectory(info, NULL, &required_size);

	if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != result || !required_size)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (ptrdiff_t)4 * required_size + sizeof(uint32_t)))
	{
		return 0;
	}

	uint16_t* start = (uint16_t*)buffer_data(output, size + required_size);
	result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->GetRuntimeDirectory(info, start, &required_size);

	if (FAILED(result))
	{
		return 0;
	}

	const uint16_t* finish = start + required_size;

	if (!buffer_resize(output, size) ||
		!text_encoding_UTF16LE_to_UTF8(start, finish, output))
	{
		return 0;
	}

	return 1;
}

uint8_t runtime_info_is_loaded(void* info, void* process, uint8_t* loaded)
{
	if (!info ||
		!process ||
		!loaded)
	{
		return 0;
	}

	BOOL loaded_ = 0;
	const HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->IsLoaded(info, process, &loaded_);
	*loaded = 0 < loaded_;
	/**/
	return SUCCEEDED(result);
}

uint8_t runtime_info_load_error_string(
	void* info, uint32_t resource_id,
	long locale_id, struct buffer* output)
{
	if (!info ||
		!output)
	{
		return 0;
	}

	DWORD required_size = 0;
	HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->LoadErrorString(info, resource_id, NULL,
					 &required_size, locale_id);

	if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != result || !required_size)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (ptrdiff_t)4 * required_size + sizeof(uint32_t)))
	{
		return 0;
	}

	uint16_t* start = (uint16_t*)buffer_data(output, size + required_size);
	result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->LoadErrorString(
				 info, resource_id, start, &required_size, locale_id);

	if (FAILED(result))
	{
		return 0;
	}

	const uint16_t* finish = start + required_size;

	if (!buffer_resize(output, size) ||
		!text_encoding_UTF16LE_to_UTF8(start, finish, output))
	{
		return 0;
	}

	return 1;
}

/*uint8_t runtime_info_load_library(void* info)
{
	if (!info)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->LoadLibrary(info, ...);
	return SUCCEEDED(result);
}*/

/*uint8_t runtime_info_get_proc_address(void* info)
{
	if (!info)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->GetProcAddress(info, ...);
	return SUCCEEDED(result);
}*/

uint8_t runtime_info_get_interface(void* info, const void* class_id, const void* iid, void** the_interface)
{
	if (!info ||
		!class_id ||
		!iid ||
		!the_interface)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->GetInterface(
							   info, class_id, iid, the_interface);
	return SUCCEEDED(result);
}
/*CLSID_CorMetaDataDispenser IID_IMetaDataDispenser, IID_IMetaDataDispenserEx*/
/*CLSID_CorMetaDataDispenserRuntime IID_IMetaDataDispenser, IID_IMetaDataDispenserEx*/
uint8_t runtime_info_get_interface_of_cor_runtime_host(void* info, void** cor_host)
{
	if (!info ||
		!cor_host)
	{
		return 0;
	}

	struct ICorRuntimeHost* output = NULL;

	const HRESULT result = runtime_info_get_interface(info, &CLSID_CorRuntimeHost, &IID_ICorRuntimeHost, &output);

	*cor_host = output;

	return SUCCEEDED(result);
}

uint8_t runtime_info_get_interface_of_clr_runtime_host(void* info, void** clr_host)
{
	if (!info ||
		!clr_host)
	{
		return 0;
	}

	struct ICLRRuntimeHost* output = NULL;

	const HRESULT result = runtime_info_get_interface(info, &CLSID_CLRRuntimeHost, &IID_ICLRRuntimeHost, &output);

	*clr_host = output;

	return SUCCEEDED(result);
}

uint8_t runtime_info_get_interface_of_type_name_factory(void* info, void** type_name_factory)
{
	if (!info ||
		!type_name_factory)
	{
		return 0;
	}

	struct ITypeNameFactory* output = NULL;

	const HRESULT result = runtime_info_get_interface(info, &CLSID_TypeNameFactory, &IID_ITypeNameFactory,
						   &output);

	*type_name_factory = output;

	return SUCCEEDED(result);
}
/*CLSID_CLRDebuggingLegacy IID_ICorDebug*/
uint8_t runtime_info_get_interface_of_strong_name(void* info, void** strong_name)
{
	if (!info ||
		!strong_name)
	{
		return 0;
	}

	struct ICLRStrongName* output = NULL;

	const HRESULT result = runtime_info_get_interface(info, &CLSID_CLRStrongName, &IID_ICLRStrongName, &output);

	*strong_name = output;

	return SUCCEEDED(result);
}

uint8_t runtime_info_is_loadable(void* info, uint8_t* loadable)
{
	if (!info ||
		!loadable)
	{
		return 0;
	}

	BOOL loadable_ = 0;
	const HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->IsLoadable(info, &loadable_);
	*loadable = 0 < loadable_;
	/**/
	return SUCCEEDED(result);
}

uint8_t runtime_info_set_default_startup_flags_wchar_t(
	void* info, unsigned long startup_flags,
	const wchar_t* host_config_file)
{
	if (!info ||
		!host_config_file)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->SetDefaultStartupFlags(
							   info, startup_flags, host_config_file);
	/**/
	return SUCCEEDED(result);
}

uint8_t runtime_info_set_default_startup_flags(void* info, unsigned long startup_flags,
		const uint8_t* host_config_file)
{
	if (!info ||
		!host_config_file)
	{
		return 0;
	}

	struct buffer host_config_file_W;

	SET_NULL_TO_BUFFER(host_config_file_W);

	if (!text_encoding_UTF8_to_UTF16LE(host_config_file, host_config_file + strlen((const char*)host_config_file),
									   &host_config_file_W) ||
		!buffer_push_back_uint16(&host_config_file_W, 0))
	{
		buffer_release(&host_config_file_W);
		return 0;
	}

	const HRESULT result = runtime_info_set_default_startup_flags_wchar_t(
							   info, startup_flags,
							   buffer_wchar_t_data(&host_config_file_W, 0));
	buffer_release(&host_config_file_W);
	/**/
	return SUCCEEDED(result);
}

uint8_t runtime_info_get_default_startup_flags(void* info, struct buffer* output)
{
	if (!info ||
		!output)
	{
		return 0;
	}

	DWORD startup_flags = 0;
	DWORD required_size = 0;
	HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->GetDefaultStartupFlags(
						 info, &startup_flags, NULL, &required_size);

	if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != result || !required_size)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (ptrdiff_t)4 * required_size + sizeof(uint32_t)))
	{
		return 0;
	}

	uint16_t* start = (uint16_t*)buffer_data(output, size + required_size);
	result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->GetDefaultStartupFlags(
				 info, &startup_flags, start, &required_size);

	if (FAILED(result))
	{
		return 0;
	}

	const uint16_t* finish = start + required_size;

	if (!buffer_resize(output, size) ||
		!text_encoding_UTF16LE_to_UTF8(start, finish, output))
	{
		return 0;
	}

	return 1;
}
/*BindAsLegacyV2Runtime*/
uint8_t runtime_info_is_started(void* info, uint8_t* started, unsigned long* startup_flags)
{
	if (!info ||
		!started ||
		!startup_flags)
	{
		return 0;
	}

	BOOL started_ = 0;
	const HRESULT result = ((struct ICLRRuntimeInfo*)info)->lpVtbl->IsStarted(info, &started_, startup_flags);
	*started = 0 < started_;
	/**/
	return SUCCEEDED(result);
}
