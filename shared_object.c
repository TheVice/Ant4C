/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2022 TheVice
 *
 */

#if !defined(_WIN32)
#define _POSIX_SOURCE 1
#endif

#include "shared_object.h"
/*#ifndef NDEBUG
#include "common.h"
#include "echo.h"
#include "text_encoding.h"
#endif*/

#include <stddef.h>

#if defined(_WIN32)
#include "buffer.h"
#include "file_system.h"

#include <wchar.h>
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined(_WIN32)

void* shared_object_load_wchar_t(const wchar_t* path)
{
	return (void*)LoadLibraryExW(path, NULL, 0);
}

#endif

void* shared_object_load(const uint8_t* path)
{
	if (NULL == path)
	{
		return NULL;
	}

#if defined(_WIN32)
	uint8_t pathW_buffer[BUFFER_SIZE_OF];
	void* pathW = (void*)pathW_buffer;

	if (!buffer_init(pathW, BUFFER_SIZE_OF))
	{
		return NULL;
	}

	if (!file_system_path_to_pathW(path, pathW))
	{
		buffer_release(pathW);
		return NULL;
	}

	void* object = shared_object_load_wchar_t(buffer_wchar_t_data(pathW, 0));
	buffer_release(pathW);
	/**/
	return object;
#else
	/*
#ifndef NDEBUG
	uint8_t verbose = 1;
	void* object = dlopen((const char*)path, RTLD_NOW);

	if (verbose && NULL == object)
	{
		const uint8_t* error_str = (const uint8_t*)dlerror();
		const uint8_t message_length = (uint8_t)common_count_bytes_until(error_str, 0);
		echo(0, UTF8, NULL, Error, error_str, message_length, 1, 0);
	}

	return object;
#else*/
	return dlopen((const char*)path, RTLD_NOW);
	/*
#endif
	*/
#endif
}

void* shared_object_get_procedure_address(void* object, const uint8_t* procedure_name)
{
	if (NULL == object ||
		NULL == procedure_name)
	{
		return NULL;
	}

#if defined(_WIN32)
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(suppress: 4054)
	return (void*)GetProcAddress((HMODULE)object, (const char*)procedure_name);
#else
	return (void*)GetProcAddress((HMODULE)object, (const char*)procedure_name);
#endif
#else
	return dlsym(object, (const char*)procedure_name);
#endif
}

void shared_object_unload(void* object)
{
	if (NULL != object)
	{
#if defined(_WIN32)
		/*TODO:*/FreeLibrary((HMODULE)object);
#else
		dlclose(object);
#endif
	}
}
