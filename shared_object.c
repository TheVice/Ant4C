/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "shared_object.h"
/*#ifndef NDEBUG
#include "echo.h"
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
	/*if (!file_exists_wchar_t(path))
	{
		return NULL;
	}*/
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
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return NULL;
	}

	void* object = shared_object_load_wchar_t(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	/**/
	return object;
#else
	/*if (!file_exists(path))
	{
		return NULL;
	}*/

/*#ifndef NDEBUG
	void* ptr = dlopen((const char*)path, RTLD_NOW);

	if (NULL == ptr)
	{
		const char* error_str = dlerror();
		const uint8_t message_length = (uint8_t)common_count_bytes_until(ptr, 0);
		echo(0, UTF8, NULL, Error, (const uint8_t*)error_str, message_length, 1, 0);
	}

	return ptr;
#else*/
	return dlopen((const char*)path, RTLD_NOW);
/*#endif*/
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
	return (void*)GetProcAddress(object, (const char*)procedure_name);
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
