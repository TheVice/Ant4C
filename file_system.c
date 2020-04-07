/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "file_system.h"
#include "buffer.h"
#include "common.h"
#include "hash.h"
#include "range.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#define _POSIXSOURCE 1

#include <utime.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

uint8_t file_close(void* stream)
{
	return NULL != stream &&
		   0 == fclose(stream);
}

uint8_t file_open(const uint8_t* path, const uint8_t* mode, void** output)
{
	if (NULL == path ||
		NULL == mode ||
		NULL == output)
	{
		return 0;
	}

#if 0
	defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const ptrdiff_t size = buffer_size(&pathW);

	if (!text_encoding_UTF8_to_UTF16LE(mode, mode + common_count_bytes_until(mode, '\0'), &pathW) ||
		!buffer_push_back_uint16(&pathW, 0))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned =
		file_open_wchar_t(buffer_wchar_t_data(&pathW, 0), (const wchar_t*)buffer_data(&pathW, size), output);
	buffer_release(&pathW);
	return returned;
#else
#if __STDC_SEC_API__
	return (0 == fopen_s((FILE**)output, (const char*)path, (const char*)mode) && NULL != (*output));
#else
	(*output) = (void*)fopen((const char*)path, (const char*)mode);
	return (NULL != (*output));
#endif
#endif
}

size_t file_read(void* content, const size_t size_of_content_element,
				 const size_t count_of_elements, void* stream)
{
	if (NULL == content ||
		NULL == stream)
	{
		return 0;
	}

#if __STDC_SEC_API__ && defined(_MSC_VER)
	return fread_s(content, count_of_elements, size_of_content_element, count_of_elements, stream);
#else
	return fread(content, size_of_content_element, count_of_elements, stream);
#endif
}

size_t file_write(const void* content, const size_t size_of_content_element,
				  const size_t count_of_elements, void* stream)
{
	return fwrite(content, size_of_content_element, count_of_elements, stream);
}
