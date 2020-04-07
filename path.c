/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "path.h"
#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "range.h"

#if defined(_WIN32)
#include <windows.h>
#else

#define _POSIXSOURCE 1

#include <unistd.h>
#include <stdlib.h>
#endif

#include <stdio.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#ifndef L_tmpnam_s
#define L_tmpnam_s L_tmpnam
#endif

static const uint8_t point = '.';
static const uint8_t tilde = '~';

#if defined(_WIN32)
static const uint8_t upper_level[] = { '\\', '.', '.' };
#else
static const uint8_t upper_level[] = { '/', '.', '.' };
#endif
#define UPPER_LEVEL_LENGTH COUNT_OF(upper_level)

uint8_t path_combine_in_place(struct buffer* path1, const ptrdiff_t size,
							  const uint8_t* path2_start, const uint8_t* path2_finish)
{
	if (NULL == path1 ||
		buffer_size(path1) < size ||
		path2_finish < path2_start)
	{
		return 0;
	}

	if (size < buffer_size(path1) && !buffer_push_back(path1, PATH_DELIMITER))
	{
		return 0;
	}

	if (!buffer_append(path1, path2_start, path2_finish - path2_start))
	{
		return 0;
	}

	ptrdiff_t new_size = buffer_size(path1) - size;

	if (new_size)
	{
#if defined(_WIN32)

		if (!cygpath_get_windows_path(buffer_data(path1, size), buffer_data(path1, size) + new_size))
#else
		if (!cygpath_get_unix_path(buffer_data(path1, size), buffer_data(path1, size) + new_size))
#endif
		{
			return 0;
		}

		if (!common_replace_double_byte_by_single(buffer_data(path1, size), &new_size, PATH_DELIMITER))
		{
			return 0;
		}
	}

	return new_size ? buffer_resize(path1, size + new_size) : 1;
}

uint8_t path_combine(const uint8_t* path1_start, const uint8_t* path1_finish,
					 const uint8_t* path2_start, const uint8_t* path2_finish, struct buffer* output)
{
	if (path1_finish < path1_start ||
		path2_finish < path2_start ||
		NULL == output)
	{
		return 0;
	}

	ptrdiff_t new_size = path1_finish - path1_start + path2_finish - path2_start;

	if (path1_start < path1_finish && path2_start < path2_finish)
	{
		++new_size;
	}

	if (!new_size)
	{
		return 1;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, new_size))
	{
		return 0;
	}

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	if (!buffer_append(output, path1_start, path1_finish - path1_start))
	{
		return 0;
	}

	return path_combine_in_place(output, size, path2_start, path2_finish);
}

uint8_t path_get_temp_file_name(struct buffer* temp_file_name)
{
	if (NULL == temp_file_name)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(temp_file_name);
#if !defined(_WIN32)
	int fd = -1;

	if (!buffer_append_char(temp_file_name, "/tmp/fileXXXXXX", 15) ||
		!buffer_push_back(temp_file_name, 0))
	{
		return 0;
	}

	char* temp_file_path = (char*)buffer_data(temp_file_name, size);

	if (-1 == (fd = mkstemp(temp_file_path)))
	{
		return 0;
	}

	close(fd);
	return buffer_resize(temp_file_name, size + 15);
#else

	if (!buffer_append_char(temp_file_name, NULL, L_tmpnam_s))
	{
		return 0;
	}

	uint8_t* temp_file_path = buffer_data(temp_file_name, size);
#if __STDC_SEC_API__

	if (0 != tmpnam_s((char*)temp_file_path, L_tmpnam_s))
	{
		return 0;
	}

#else
	tmpnam((char*)temp_file_path);
#endif
	ptrdiff_t length = common_count_bytes_until(temp_file_path, 0);

	if (length < 1)
	{
		return 0;
	}

	if (point == temp_file_path[length - 1])
	{
		const uint8_t* ptr = find_any_symbol_like_or_not_like_that(
								 temp_file_path + length - 2, temp_file_path, &point, 1, 0, -1);

		if (ptr == temp_file_path || PATH_DELIMITER == *ptr)
		{
			return 0;
		}

		++ptr;
		length = ptr - temp_file_path;
	}

	if (path_is_path_rooted(temp_file_path, temp_file_path + length))
	{
		if (!buffer_resize(temp_file_name, size + length) ||
			!buffer_push_back(temp_file_name, 0))
		{
			return 0;
		}

		return buffer_resize(temp_file_name, size + length);
	}

	return 0;
#if 0
	const ptrdiff_t temp_path_start = buffer_size(temp_file_name);

	if (!path_get_temp_path(temp_file_name))
	{
		return 0;
	}

	const ptrdiff_t temp_path_finish = buffer_size(temp_file_name);

	if (!buffer_append(temp_file_name, NULL, (ptrdiff_t)2 + temp_path_finish - size))
	{
		return 0;
	}

	const uint8_t* temp_path = buffer_data(temp_file_name, temp_path_start);
	const uint8_t* temp_path_ = buffer_data(temp_file_name, temp_path_finish);
	temp_file_path = buffer_data(temp_file_name, size);

	if (!buffer_resize(temp_file_name, temp_path_finish) ||
		!path_combine(
			temp_path, temp_path_,
			temp_file_path, temp_file_path + length,
			temp_file_name))
	{
		return 0;
	}

	length = buffer_size(temp_file_name) - temp_path_finish;
#if __STDC_SEC_API__

	if (0 != memcpy_s(temp_file_path, length, temp_path_, length))
	{
		return 0;
	}

#else
	memcpy(temp_file_path, temp_path_, length);
#endif

	if (!buffer_resize(temp_file_name, size + length) ||
		!buffer_push_back(temp_file_name, 0))
	{
		return 0;
	}

	return buffer_resize(temp_file_name, size + length);
#endif
#endif
}

uint8_t path_get_temp_path(struct buffer* temp_path)
{
	if (NULL == temp_path)
	{
		return 0;
	}

#if 0
	defined(_WIN32)
	const ptrdiff_t size = buffer_size(temp_path);

	if (!buffer_append(temp_path, NULL, 6 * FILENAME_MAX + sizeof(uint32_t)))
	{
		return 0;
	}

	wchar_t* pathW = (wchar_t*)buffer_data(temp_path, size);
	DWORD length = GetTempPathW(sizeof(wchar_t), pathW);

	if (length < sizeof(wchar_t) + 1)
	{
		return 0;
	}

	if (!buffer_resize(temp_path, size) ||
		!buffer_append(temp_path, NULL, (ptrdiff_t)6 * length + sizeof(uint32_t)))
	{
		return 0;
	}

	pathW = (wchar_t*)buffer_data(temp_path,
								  buffer_size(temp_path) - sizeof(uint32_t) - sizeof(uint16_t) * length - sizeof(uint16_t));
	length = GetTempPathW(length, pathW);

	if (!buffer_resize(temp_path, size) ||
		!text_encoding_UTF16LE_to_UTF8(pathW, pathW + length, temp_path))
	{
		return 0;
	}

	struct range directory;

	if (!path_get_directory_name(buffer_data(temp_path, size), buffer_data(temp_path, 0) + buffer_size(temp_path),
								 &directory))
	{
		return 0;
	}

	return buffer_resize(temp_path, size + range_size(&directory));
#else
	return buffer_append_char(temp_path, "/tmp", 4);
#endif
}

uint8_t path_is_path_rooted(const uint8_t* path_start, const uint8_t* path_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

#if defined(_WIN32)

	if (1 < path_finish - path_start)
	{
		return ':' == (path_start)[1];
	}

#endif
	return (path_posix_delimiter == path_start[0] || PATH_DELIMITER == path_start[0]);
}

uint8_t cygpath_get_unix_path(uint8_t* path_start, uint8_t* path_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	for (; path_start < path_finish; ++path_start)
	{
		if (path_windows_delimiter == (*path_start))
		{
			(*path_start) = path_posix_delimiter;
		}
	}

	return 1;
}

uint8_t cygpath_get_windows_path(uint8_t* path_start, uint8_t* path_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	for (; path_start < path_finish; ++path_start)
	{
		if (path_posix_delimiter == (*path_start))
		{
			(*path_start) = path_windows_delimiter;
		}
	}

	return 1;
}
