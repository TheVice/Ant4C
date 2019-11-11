/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "file_system.h"
#include "buffer.h"
#include "common.h"
#include "path.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"

#if defined(_WIN32)
#include <stdio.h>
#include <string.h>

#include <windows.h>

#define FIND_FILE_OBJECT_DATA(PATHW, DATA, RETURN, CLOSE_RESULT)\
	const HANDLE file_handle = FindFirstFileW((PATHW), (DATA));	\
	\
	if (INVALID_HANDLE_VALUE == file_handle)					\
	{															\
		return (RETURN);										\
	}															\
	\
	(CLOSE_RESULT) = FindClose(file_handle);

uint8_t directory_exists_by_wchar_path(const wchar_t* path)
{
	if (NULL == path || L'\0' == *path)
	{
		return 0;
	}

	WIN32_FIND_DATAW file_data;
	BOOL close_return = 0;
	FIND_FILE_OBJECT_DATA(path, &file_data, 0, close_return)
	const uint8_t is_directory = (0 != (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	return close_return && is_directory;
}

uint8_t path_to_pathW(const uint8_t* path, struct buffer* pathW)
{
	(void)path;
	(void)pathW;
#if 0

	if (NULL == path ||
		'\0' == *path ||
		NULL == pathW)
	{
		return 0;
	}

	int32_t length = common_count_bytes_until(path, 0);

	if (path_is_path_rooted(path, path + length) &&
		!buffer_append_wchar_t((pathW), L"\\\\?\\", 4))
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(pathW);

	if (!buffer_append_wchar_t(pathW, NULL, length) ||
		!buffer_push_back_uint16(pathW, 0))
	{
		return 0;
	}

	wchar_t* w = (wchar_t*)buffer_data(pathW, size);
	MULTI2WIDE(path, w, length)
	return 0 < length;
#endif
	return 0;
}
#else

#define _POSIXSOURCE 1

#include <dirent.h>
#endif
uint8_t directory_exists(const uint8_t* path)
{
	if (NULL == path || '\0' == *path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t is_exists = directory_exists_by_wchar_path(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	return is_exists;
#else
	DIR* dir = opendir((const char*)path);

	if (NULL == dir)
	{
		return 0;
	}

	closedir(dir);
	return 1;
#endif
}
#if 0
uint8_t directory_get_current_directory(const void* project, struct buffer* current_directory)
{
	if (!project_get_base_directory(project, NULL, current_directory))
	{
		return path_get_directory_for_current_process(current_directory);
	}

	return 1;
}
#endif
uint8_t directory_get_logical_drives(struct buffer* drives)
{
#if defined(_WIN32)

	if (NULL == drives)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(drives);
	const DWORD count_of_characters = GetLogicalDriveStringsW(0, NULL);

	if (!count_of_characters ||
		!buffer_append_char(drives, NULL, count_of_characters) ||
		!buffer_append_wchar_t(drives, NULL, count_of_characters))
	{
		return 0;
	}

	wchar_t* w = (wchar_t*)buffer_data(drives, size + count_of_characters);

	if (!GetLogicalDriveStringsW(count_of_characters, w))
	{
		return 0;
	}

	DWORD count = count_of_characters;
	char* m = (char*)buffer_data(drives, size);
	WIDE2MULTI(w, m, count);

	if (!count)
	{
		return 0;
	}

	return buffer_resize(drives, size + count_of_characters);
#else
	return buffer_push_back(drives, PATH_DELIMITER);
#endif
}

uint8_t directory_get_parent_directory(const uint8_t* path_start, const uint8_t* path_finish,
									   struct range* parent)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) || NULL == parent)
	{
		return 0;
	}

	parent->start = path_start;
	parent->finish = find_any_symbol_like_or_not_like_that(path_finish - 1, path_start, &PATH_DELIMITER, 1, 1,
					 -1);

	if (!string_trim(parent))
	{
		return 0;
	}

	return range_is_null_or_empty(parent);
}
#if defined(_WIN32)
uint8_t file_exists_by_wchar_path(const wchar_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

	WIN32_FIND_DATAW file_data;
	BOOL close_return = 0;
	FIND_FILE_OBJECT_DATA(path, &file_data, 0, close_return)
	const uint8_t is_file = (0 == (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	return close_return && is_file;
}
#else
#include <sys/stat.h>
#endif
uint8_t file_exists(const uint8_t* path)
{
	if (NULL == path || '\0' == *path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t is_exists = file_exists_by_wchar_path(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	return is_exists;
#else
	struct stat file_status;
	file_status.st_mode = 0;
	return -1 != stat((const char*)path, &file_status) && (S_IFDIR != (file_status.st_mode & S_IFDIR));
#endif
}

uint64_t file_get_length(const uint8_t* path)
{
	if (NULL == path || '\0' == *path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	WIN32_FIND_DATAW file_data;
	const HANDLE file_handle = FindFirstFileW(buffer_wchar_t_data(&pathW, 0), &file_data);
	buffer_release(&pathW);

	if (INVALID_HANDLE_VALUE == file_handle)
	{
		return 0;
	}

	if (!FindClose(file_handle))
	{
		return 0;
	}

	const uint8_t is_file = (0 == (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

	if (!is_file)
	{
		return 0;
	}

	uint64_t length = file_data.nFileSizeHigh;
	length *= ((uint64_t)(MAXDWORD) + (uint64_t)1);
	length += file_data.nFileSizeLow;
	return length;
#else
	struct stat file_status;
	file_status.st_size = 0;

	if (stat((const char*)path, &file_status) ||
		!(S_IFDIR != (file_status.st_mode & S_IFDIR)))
	{
		return 0;
	}

	return file_status.st_size;
#endif
}
