/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "meta_host.h"
#include "runtime_info.h"
#include "buffer.h"
#include "file_system.h"
#include "range.h"
#include "text_encoding.h"

#include <metahost.h>

#pragma comment(lib, "mscoree.lib")

static struct ICLRMetaHost* meta_host_instance = NULL;

uint8_t meta_host_init()
{
	if (meta_host_instance)
	{
		return 1;
	}

	return SUCCEEDED(CLRCreateInstance(&CLSID_CLRMetaHost, &IID_ICLRMetaHost, (LPVOID*)&meta_host_instance));
}

uint8_t meta_host_get_runtime_wchar_t(const wchar_t* version, void** the_runtime)
{
	if (!version ||
		!the_runtime ||
		!meta_host_init())
	{
		return 0;
	}

	struct ICLRRuntimeInfo* info = NULL;

	const HRESULT result = meta_host_instance->lpVtbl->GetRuntime(
							   meta_host_instance, version, &IID_ICLRRuntimeInfo, &info);

	*the_runtime = info;

	return SUCCEEDED(result);
}

uint8_t meta_host_get_runtime(const uint8_t* version_start, const uint8_t* version_finish, void** the_runtime)
{
	if (range_in_parts_is_null_or_empty(version_start, version_finish) ||
		!the_runtime ||
		!meta_host_init())
	{
		return 0;
	}

	struct buffer version_W;

	SET_NULL_TO_BUFFER(version_W);

	if (!text_encoding_UTF8_to_UTF16LE(
			version_start, version_finish, &version_W) ||
		!buffer_push_back_uint16(&version_W, 0))
	{
		buffer_release(&version_W);
		return 0;
	}

	const uint8_t result = meta_host_get_runtime_wchar_t(buffer_wchar_t_data(&version_W, 0), the_runtime);
	buffer_release(&version_W);
	return result;
}
#if 0
uint8_t meta_host_get_runtime_v1(void** the_runtime)
{
	static const wchar_t* version = L"v1.0.3705";
	return meta_host_get_runtime_wchar_t(version, the_runtime);
}

uint8_t meta_host_get_runtime_v1dot1(void** the_runtime)
{
	static const wchar_t* version = L"v1.1.4322";
	return meta_host_get_runtime_wchar_t(version, the_runtime);
}
#endif
uint8_t meta_host_get_runtime_v2(void** the_runtime)
{
	static const wchar_t* version = L"v2.0.50727";
	return meta_host_get_runtime_wchar_t(version, the_runtime);
}

uint8_t meta_host_get_runtime_v4(void** the_runtime)
{
	static const wchar_t* version = L"v4.0.30319";
	return meta_host_get_runtime_wchar_t(version, the_runtime);
}

uint8_t meta_host_get_version_from_file_wchar_t(const wchar_t* file_path, struct buffer* output)
{
	if (!file_path ||
		!file_exists_wchar_t(file_path) ||
		!output ||
		!meta_host_init())
	{
		return 0;
	}

	DWORD required_size = 0;
	HRESULT result = meta_host_instance->lpVtbl->GetVersionFromFile(
						 meta_host_instance, file_path, NULL, &required_size);

	if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != result || !required_size)
	{
		return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT) == result;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (ptrdiff_t)4 * required_size + sizeof(uint32_t)))
	{
		return 0;
	}

	uint16_t* start = (uint16_t*)buffer_data(output, size + required_size);
	result = meta_host_instance->lpVtbl->GetVersionFromFile(meta_host_instance, file_path, start, &required_size);

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

uint8_t meta_host_get_version_from_file(
	const uint8_t* file_path_start, const uint8_t* file_path_finish, struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(file_path_start, file_path_finish) ||
		!output ||
		!meta_host_init())
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, file_path_start, file_path_finish - file_path_start) ||
		!buffer_push_back(output, 0))
	{
		return 0;
	}

	struct buffer pathW;

	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(buffer_data(output, size), &pathW) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	const uint8_t result = meta_host_get_version_from_file_wchar_t(
							   buffer_wchar_t_data(&pathW, 0), output);
	buffer_release(&pathW);
	return result;
}

uint8_t meta_host_enumerate_installed_runtimes(struct buffer* output)
{
	if (!output ||
		!meta_host_init())
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);
	struct IEnumUnknown* enumerator = NULL;

	if (FAILED(meta_host_instance->lpVtbl->EnumerateInstalledRuntimes(meta_host_instance, &enumerator)))
	{
		return 0;
	}

	ULONG fetched = 0;
	static const ULONG celt = 1;
	struct ICLRRuntimeInfo* info = NULL;

	while (S_OK == enumerator->lpVtbl->Next(enumerator, celt, (IUnknown**)&info, &fetched))
	{
		if (!runtime_info_get_version_string(info, output) ||
			!buffer_push_back(output, ','))
		{
			return 0;
		}
	}

	enumerator->lpVtbl->Release(enumerator);
	enumerator = NULL;

	if (size < buffer_size(output))
	{
		if (!buffer_resize(output, buffer_size(output) - 1))
		{
			return buffer_push_back(output, 0);
		}
	}

	return 1;
}

uint8_t meta_host_enumerate_loaded_runtimes(void* process, struct buffer* output)
{
	if (!process ||
		!output ||
		!meta_host_init())
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);
	struct IEnumUnknown* enumerator = NULL;

	if (FAILED(meta_host_instance->lpVtbl->EnumerateLoadedRuntimes(meta_host_instance, process, &enumerator)))
	{
		return 0;
	}

	ULONG fetched = 0;
	static const ULONG celt = 1;
	struct ICLRRuntimeInfo* info = NULL;

	while (S_OK == enumerator->lpVtbl->Next(enumerator, celt, (IUnknown**)&info, &fetched))
	{
		if (!runtime_info_get_version_string(info, output) ||
			!buffer_push_back(output, ','))
		{
			return 0;
		}
	}

	enumerator->lpVtbl->Release(enumerator);
	enumerator = NULL;

	if (size < buffer_size(output))
	{
		if (!buffer_resize(output, buffer_size(output) - 1))
		{
			return buffer_push_back(output, 0);
		}
	}

	return 1;
}
/*RequestRuntimeLoadedNotification
QueryLegacyV2RuntimeBinding*/
uint8_t meta_host_exit_process(int32_t exit_code)
{
	if (!meta_host_init())
	{
		return 0;
	}

	const HRESULT result = meta_host_instance->lpVtbl->ExitProcess(meta_host_instance, exit_code);
	return SUCCEEDED(result);
}

void meta_host_release()
{
	if (meta_host_instance)
	{
		meta_host_instance->lpVtbl->Release(meta_host_instance);
		meta_host_instance = NULL;
	}
}
