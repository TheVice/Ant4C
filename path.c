/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "path.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "environment.h"
#include "file_system.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"

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

#if defined(_WIN32)
static const uint8_t* upper_level = (const uint8_t*)"\\..";
#else
static const uint8_t* upper_level = (const uint8_t*)"/..";
#endif
static const ptrdiff_t upper_level_length = 3;

static const uint8_t point = '.';
static const uint8_t tilde = '~';

static const uint8_t posix_delimiter = '/';
static const uint8_t windows_delimiter = '\\';

#if defined(_WIN32)
static const uint8_t delimiter = '\\';
#else
static const uint8_t delimiter = '/';
#endif

uint8_t path_delimiter()
{
	return delimiter;
}

uint8_t path_posix_delimiter()
{
	return posix_delimiter;
}

uint8_t path_windows_delimiter()
{
	return windows_delimiter;
}

uint8_t path_change_extension(const uint8_t* path_start, const uint8_t* path_finish,
							  const uint8_t* ext_start, const uint8_t* ext_finish, struct buffer* path)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	struct range cur_ext;

	if (path_get_extension(path_start, path_finish, &cur_ext))
	{
		if (string_equal(cur_ext.start, cur_ext.finish, ext_start, ext_finish))
		{
			return buffer_append(path, path_start, path_finish - path_start);
		}

		if (!buffer_append(path, path_start, cur_ext.start - path_start))
		{
			return 0;
		}
	}
	else
	{
		if (!buffer_append(path, path_start, path_finish - path_start))
		{
			return 0;
		}
	}

	if (!range_in_parts_is_null_or_empty(ext_start, ext_finish))
	{
		if (!buffer_append(path, ext_start, ext_finish - ext_start))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t path_combine(const uint8_t* path1_start, const uint8_t* path1_finish,
					 const uint8_t* path2_start, const uint8_t* path2_finish, struct buffer* path)
{
	if (NULL == path1_start || NULL == path1_finish ||
		NULL == path2_start || NULL == path2_finish ||
		NULL == path ||
		path1_finish < path1_start || path2_finish < path2_start)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(path);

	if (!buffer_append(path, NULL,
					   path1_finish - path1_start + 1 + path2_finish - path2_start))
	{
		return 0;
	}

	if (!buffer_resize(path, size))
	{
		return 0;
	}

	if (!buffer_append(path, path1_start, path1_finish - path1_start))
	{
		return 0;
	}

	if (!buffer_push_back(path, delimiter))
	{
		return 0;
	}

	if (!buffer_append(path, path2_start, path2_finish - path2_start))
	{
		return 0;
	}

	ptrdiff_t new_size = buffer_size(path) - size;
	replace_double_char_by_single(buffer_data(path, size), &new_size, delimiter);
	return buffer_resize(path, size + new_size);
}

uint8_t path_get_directory_name(const uint8_t* path_start, const uint8_t* path_finish,
								struct range* directory)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||  NULL == directory)
	{
		return 0;
	}

	directory->finish = find_any_symbol_like_or_not_like_that(
							path_finish - 1, path_start, &delimiter, 1, 1, -1);
	directory->start = path_start;

	if (!string_trim(directory))
	{
		return 0;
	}

	if (delimiter == (*directory->start) && range_is_null_or_empty(directory))
	{
		++directory->finish;
	}

	return !range_is_null_or_empty(directory);
}

uint8_t path_get_extension(const uint8_t* path_start, const uint8_t* path_finish, struct range* ext)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) || NULL == ext)
	{
		return 0;
	}

	const uint8_t* file_name_start = find_any_symbol_like_or_not_like_that(
										 path_finish - 1, path_start, &delimiter, 1, 1, -1);
	ext->start = find_any_symbol_like_or_not_like_that(path_finish - 1, file_name_start, &point, 1, 1, -1);
	ext->finish = path_finish;

	if (!string_trim(ext) || point != (*ext->start))
	{
		return 0;
	}

	if (ext->start + 1 == ext->finish)
	{
		ext->start = ext->finish;
	}

	return ext->start < ext->finish;
}

uint8_t path_get_file_name(const uint8_t* path_start, const uint8_t* path_finish, struct range* file_name)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		NULL == file_name)
	{
		return 0;
	}

	const uint8_t* file_name_start = find_any_symbol_like_or_not_like_that(path_finish - 1, path_start,
									 &delimiter, 1, 1, -1);

	if (delimiter == (*file_name_start) && file_name_start < path_finish)
	{
		++file_name_start;
	}

	file_name->start = file_name_start;
	file_name->finish = path_finish;
	return string_trim(file_name);
}

uint8_t path_get_file_name_without_extension(const uint8_t* path_start, const uint8_t* path_finish,
		struct range* file_name)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		NULL == file_name)
	{
		return 0;
	}

	if (!path_get_file_name(path_start, path_finish, file_name))
	{
		return 0;
	}

	struct range ext;

	const uint8_t has_extension = path_get_extension(file_name->start, file_name->finish, &ext);

	if (has_extension)
	{
		file_name->finish = ext.start;
	}

	return string_trim(file_name);
}

uint8_t path_get_full_path(const uint8_t* root_start, const uint8_t* root_finish,
						   const uint8_t* path_start, const uint8_t* path_finish, struct buffer* full_path)
{
	if (NULL == path_start ||
		NULL == path_finish ||
		path_finish < path_start ||
		NULL == full_path)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(full_path);

	if (tilde == (*path_start))
	{
		if (!environment_get_folder_path(UserProfile, full_path))
		{
			return 0;
		}
	}
	else if (!path_is_path_rooted(path_start, path_finish))
	{
		if (range_in_parts_is_null_or_empty(root_start, root_finish))
		{
			return 0;
		}

		if (!buffer_append(full_path, root_start, root_finish - root_start))
		{
			return 0;
		}
	}

	const ptrdiff_t start_path_length = buffer_size(full_path) - size;

	if (0 < start_path_length)
	{
		if (!buffer_append(full_path, NULL, start_path_length + path_finish - path_start + 1) ||
			!buffer_resize(full_path, size + start_path_length))
		{
			return 0;
		}
	}

	uint8_t* start = NULL;
	uint8_t* finish = NULL;

	if (0 < start_path_length)
	{
		start = buffer_data(full_path, size);
		finish = start + start_path_length;

		if (!path_combine(start, finish, path_start, path_finish, full_path))
		{
			return 0;
		}

		start = buffer_data(full_path, size + 2 * start_path_length);
		finish = buffer_data(full_path, 0) + buffer_size(full_path);

		if (NULL == start || !buffer_resize(full_path, size + start_path_length))
		{
			return 0;
		}

		if (!buffer_append(full_path, start, finish - start))
		{
			return 0;
		}
	}

	start = buffer_data(full_path, size);
	finish = 0 < buffer_size(full_path) ? (buffer_data(full_path, 0) + buffer_size(full_path)) : NULL;
	ptrdiff_t index = 0;
	const uint8_t* real_start = start;

	while (-1 != (index = string_index_of(start, finish, upper_level, upper_level + upper_level_length)))
	{
		if (0 == index)
		{
			return 0;
		}

		const uint8_t* start_ = find_any_symbol_like_or_not_like_that(start + index - 1, real_start, &delimiter, 1, 1,
								-1);
		start = start + index + upper_level_length;

		if (!buffer_resize(full_path, size + start_ - real_start))
		{
			return 0;
		}

		if (start < finish)
		{
			if (!buffer_append(full_path, start, finish - start))
			{
				return 0;
			}

			start -= index + upper_level_length;
			finish = (buffer_data(full_path, 0) + buffer_size(full_path));
		}
		else
		{
			start = finish = NULL;
			break;
		}
	}

	if (size == buffer_size(full_path))
	{
		if (!buffer_append(full_path, path_start, path_finish - path_start))
		{
			return 0;
		}
	}

	start = buffer_data(full_path, size);
	finish = 0 < buffer_size(full_path) ? (buffer_data(full_path, 0) + buffer_size(full_path)) : NULL;

	if (!range_in_parts_is_null_or_empty(start, finish))
	{
		if (string_contains(start, finish, &tilde, &tilde + 1) ||
			finish != find_any_symbol_like_or_not_like_that(start, finish, &tilde, 1, 1, 1))
		{
			return 0;
		}
	}

	index = buffer_size(full_path) - size;

	if (0 < index)
	{
		const ptrdiff_t current_index = index;
		replace_double_char_by_single(start, &index, delimiter);

		if (current_index != index && !buffer_resize(full_path, size + index))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t path_get_path_root(const uint8_t* path_start, const uint8_t* path_finish, struct range* root)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) || NULL == root)
	{
		return 0;
	}

	root->start = path_start;
#if defined(_WIN32)
	const ptrdiff_t path_length = path_finish - path_start;

	if (1 < path_length && ':' == path_start[1])
	{
		if (2 < path_length &&
			(delimiter == path_start[2] ||
			 posix_delimiter == path_start[2]))
		{
			root->finish = 3 + root->start;
		}
		else
		{
			root->finish = 2 + root->start;
		}

		return 1;
	}

#else

	if (path_start[0] == delimiter)
	{
		root->finish = 1 + root->start;
		return 1;
	}

#endif
	return 0;
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

	if (!common_append_string_to_buffer((const uint8_t*)"/tmp/fileXXXXXX", temp_file_name) ||
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
	return buffer_resize(temp_file_name, size + strlen(temp_file_path));
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

		if (ptr == temp_file_path || delimiter == *ptr)
		{
			return 0;
		}

		++ptr;
		length = ptr - temp_file_path;
	}

	if (path_is_path_rooted(temp_file_path, temp_file_path + length))
	{
		return buffer_resize(temp_file_name, size + length);
	}

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
	return buffer_resize(temp_file_name, size + length) && buffer_push_back(temp_file_name, 0);
#endif
}

uint8_t path_get_temp_path(struct buffer* temp_path)
{
	if (NULL == temp_path)
	{
		return 0;
	}

#if defined(_WIN32)
#if 0
	const ptrdiff_t size = buffer_size(temp_path);

	if (!buffer_append_wchar_t(temp_path, NULL, 2))
	{
		return 0;
	}

	wchar_t* w = (wchar_t*)buffer_data(temp_path, size);
	DWORD length = GetTempPathW(2, w);

	if (length < 3)
	{
		return 0;
	}

	if (!buffer_append_char(temp_path, NULL, length) ||
		!buffer_append_wchar_t(temp_path, NULL, length))
	{
		return 0;
	}

	uint8_t* m = buffer_data(temp_path, size);
	w = (wchar_t*)buffer_data(temp_path, size + length);
	length = GetTempPathW(length, w);

	if (!length || !buffer_resize(temp_path, size + length))
	{
		return 0;
	}

	WIDE2MULTI(w, m, length);

	if (length < 1)
	{
		return 0;
	}

	ptrdiff_t length_ = buffer_size(temp_path) - size;
	const uint8_t* m_ = find_any_symbol_like_or_not_like_that(m + length_ - 1, m, &delimiter, 1, 0, -1);
	m_ = find_any_symbol_like_or_not_like_that(m_, m + length_, &delimiter, 1, 1, 1);
	length_ = m_ - m;
	return buffer_resize(temp_path, size + length_);
#endif
	/*TODO:*/
	return 0;
#else
	return common_append_string_to_buffer((const uint8_t*)"/tmp", temp_path);
#endif
}

uint8_t path_has_extension(const uint8_t* path_start, const uint8_t* path_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	struct range ext;

	return path_get_extension(path_start, path_finish, &ext);
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
	return (path_start[0] == posix_delimiter || path_start[0] == delimiter);
}

uint8_t path_get_directory_for_current_process(struct buffer* path)
{
	if (NULL == path)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(path);
#if defined(_WIN32)

	if (!buffer_append_wchar_t(path, NULL, 2))
	{
		return 0;
	}

	wchar_t* w = (wchar_t*)buffer_data(path, size);
	DWORD length = GetCurrentDirectoryW(2, w);

	if (length == 0)
	{
		return 0;
	}

	if (2 < length)
	{
		if (!buffer_resize(path, size) ||
			!buffer_append_char(path, NULL, (ptrdiff_t)length + 1) ||
			!buffer_append_wchar_t(path, NULL, (ptrdiff_t)length + 1))
		{
			return 0;
		}

		w = (wchar_t*)buffer_data(path, size + sizeof(char) * ((ptrdiff_t)length + 1));
		length = GetCurrentDirectoryW(length + 1, w);

		if (length == 0)
		{
			return 0;
		}
	}

	if (!buffer_resize(path, size + length))
	{
		return 0;
	}

	char* m = (char*)buffer_data(path, size);
	WIDE2MULTI(w, m, length);
	return 0 < length;
#else

	while (1)
	{
		if (!buffer_append_char(path, NULL, FILENAME_MAX))
		{
			return 0;
		}

		char* ptr = (char*)buffer_data(path, size);
		const ptrdiff_t length = buffer_size(path) - size;

		if (!getcwd(ptr, (int)length))
		{
			continue;
		}

		if (!buffer_resize(path, size + strlen(ptr)))
		{
			return 0;
		}

		break;
	}

	return 1;
#endif
}

uint8_t path_get_directory_for_current_image(struct buffer* path)
{
	if (NULL == path)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(path);
#if defined(_WIN32)
#if 0

	while (1)
	{
		if (!buffer_append_wchar_t(path, NULL, FILENAME_MAX))
		{
			return 0;
		}

		wchar_t* ptr = (wchar_t*)buffer_data(path, size);
		const ptrdiff_t size_ = (buffer_size(path) - size) / sizeof(wchar_t);
		const DWORD real_size = GetModuleFileNameW(NULL, ptr, (DWORD)size_);

		if (0 == real_size)
		{
			return 0;
		}

		if (real_size < (DWORD)size_) /*TODO: move condition to while.*/
		{
			if (!buffer_resize(path, size + real_size) ||
				!buffer_append_wchar_t(path, NULL, (ptrdiff_t)2 + real_size))
			{
				return 0;
			}

			ptr = (wchar_t*)buffer_data(path, size);

			if (!buffer_resize(path, size + real_size) ||
				!buffer_append_wchar_t(path, ptr, real_size))
			{
				return 0;
			}

			ptr = (wchar_t*)buffer_data(path, size + real_size);
			uint8_t* m = buffer_data(path, size);
			uint16_t c = (uint16_t)real_size;
			WIDE2MULTI(ptr, m, c);

			if (!c)
			{
				return 0;
			}

			struct range directory;

			if (!path_get_directory_name(m, m + real_size, &directory))
			{
				return 0;
			}

			if (!buffer_resize(path, size + range_size(&directory)))
			{
				return 0;
			}

			break;
		}
	}

	return 1;
#endif
	(void)size;
	/*TODO:*/
	return 0;
#elif __linux

	while (1)
	{
		if (!buffer_append_wchar_t(path, NULL, FILENAME_MAX))
		{
			return 0;
		}

		uint8_t* ptr = buffer_data(path, size);
		const ptrdiff_t size_ = buffer_size(path) - size;
		ptrdiff_t real_size = readlink("/proc/self/exe", (char*)ptr, size_);

		if (-1 == real_size)
		{
			return 0;
		}

		if (real_size < size_)
		{
			struct range directory;

			if (!path_get_directory_name(ptr, ptr + real_size, &directory) ||
				!buffer_resize(path, size + range_size(&directory)))
			{
				return 0;
			}

			break;
		}
	}

	return 1;
#else
	(void)size;
	/*TODO:*/
	return 0;
#endif
}

uint8_t cygpath_get_dos_path(const uint8_t* path_start, const uint8_t* path_finish, struct buffer* path)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	const ptrdiff_t size = buffer_size(path);
	uint16_t count = (uint16_t)(path_finish - path_start);

	if (!buffer_append_char(path, NULL, (ptrdiff_t)count + 1) ||
		!buffer_append_wchar_t(path, NULL, 2 * ((ptrdiff_t)count + 1)))
	{
		return 0;
	}

	wchar_t* long_path = (wchar_t*)buffer_data(path, size + 1 + count);

	for (uint16_t i = 0; i < count; ++i)/*TODO: Use WIDE2MULTI*/
	{
		long_path[i] = path_start[i];
	}

	long_path[count] = L'\0';
	ptrdiff_t index = size + 1 + count + sizeof(wchar_t) * ((ptrdiff_t)count + 1);
	wchar_t* short_path = (wchar_t*)buffer_data(path, index);
	count = (uint16_t)GetShortPathNameW(long_path, short_path, count + 1);

	if (!count)
	{
		return 0;
	}

	char* m = (char*)buffer_data(path, size);
	m[count] = '\0';
	index = count;
	WIDE2MULTI(short_path, m, count)

	if (!count)
	{
		return 0;
	}

	/*NOTE: for save terminate path in buffer + 1 to the index should be added.*/
	return buffer_resize(path, size + index);
#else
	return 0;
#endif
}

uint8_t cygpath_get_unix_path(uint8_t* path_start, uint8_t* path_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	for (; path_start < path_finish; ++path_start)
	{
		if (windows_delimiter == (*path_start))
		{
			(*path_start) = posix_delimiter;
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
		if (posix_delimiter == (*path_start))
		{
			(*path_start) = windows_delimiter;
		}
	}

	return 1;
}

static const uint8_t* path_function_str[] =
{
	(const uint8_t*)"change-extension",
	(const uint8_t*)"combine",
	(const uint8_t*)"get-directory-name",
	(const uint8_t*)"get-extension",
	(const uint8_t*)"get-file-name",
	(const uint8_t*)"get-file-name-without-extension",
	(const uint8_t*)"get-full-path",
	(const uint8_t*)"get-path-root",
	(const uint8_t*)"get-temp-file-name",
	(const uint8_t*)"get-temp-path",
	(const uint8_t*)"has-extension",
	(const uint8_t*)"is-path-rooted",
	(const uint8_t*)"get-dos-path",
	(const uint8_t*)"get-unix-path",
	(const uint8_t*)"get-windows-path"
};

enum path_function
{
	change_extension, combine, get_directory_name,
	get_extension, get_file_name,
	get_file_name_without_extension, get_full_path,
	get_path_root, get_temp_file_name,
	get_temp_path, has_extension, is_path_rooted,
	get_dos_path, get_unix_path, get_windows_path,
	UNKNOWN_PATH_FUNCTION
};

uint8_t path_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, path_function_str, UNKNOWN_PATH_FUNCTION);
}

uint8_t path_get_full_path_(const void* project, const struct range* path, struct buffer* output)
{
	const ptrdiff_t size = buffer_size(output);

	if (!directory_get_current_directory(project, output))
	{
		return 0;
	}

	if (range_is_null_or_empty(path) || (1 == range_size(path) && point == *(path->start)))
	{
		return 1;
	}

	const ptrdiff_t size_ = buffer_size(output);

	if (!buffer_append(output, NULL, size_ - size + range_size(path) + 4))
	{
		return 0;
	}

	struct range base_dir;

	base_dir.start = buffer_data(output, size);

	base_dir.finish = base_dir.start + size_ - size;

	return buffer_resize(output, size) &&
		   path_get_full_path(base_dir.start, base_dir.finish,
							  path->start, path->finish, output);
}

uint8_t path_exec_function(const void* project, uint8_t function, const struct buffer* arguments,
						   uint8_t arguments_count,
						   struct buffer* output)
{
	if (UNKNOWN_PATH_FUNCTION <= function ||
		NULL == arguments ||
		2 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range argument1;

	struct range argument2;

	argument1.start = argument2.start = argument1.finish = argument2.finish = NULL;

	switch (arguments_count)
	{
		case 0:
			break;

		case 1:
			if (!common_get_one_argument(arguments, &argument1, 0))
			{
				if (get_full_path == function)
				{
					argument1.start = argument1.finish = NULL;
					break;
				}

				return 0;
			}

			break;

		case 2:
			if (!common_get_two_arguments(arguments, &argument1, &argument2, 0))
			{
				return 0;
			}

			break;

		default:
			return 0;
	}

	switch (function)
	{
		case change_extension:
			return (2 == arguments_count) &&
				   path_change_extension(argument1.start, argument1.finish, argument2.start, argument2.finish, output);

		case combine:
			return (2 == arguments_count) &&
				   path_combine(argument1.start, argument1.finish, argument2.start, argument2.finish, output);

		case get_directory_name:
			return (1 == arguments_count) &&
				   path_get_directory_name(argument1.start, argument1.finish, &argument2) &&
				   buffer_append_data_from_range(output, &argument2);

		case get_extension:
			return (1 == arguments_count) &&
				   path_get_extension(argument1.start, argument1.finish, &argument2) &&
				   buffer_append_data_from_range(output, &argument2);

		case get_file_name:
			return (1 == arguments_count) &&
				   path_get_file_name(argument1.start, argument1.finish, &argument2) &&
				   buffer_append_data_from_range(output, &argument2);

		case get_file_name_without_extension:
			return (1 == arguments_count) &&
				   path_get_file_name_without_extension(argument1.start, argument1.finish, &argument2) &&
				   buffer_append_data_from_range(output, &argument2);

		case get_full_path:
			return (1 == arguments_count) && path_get_full_path_(project, &argument1, output);

		case get_path_root:
			return (1 == arguments_count) &&
				   path_get_path_root(argument1.start, argument1.finish, &argument2) &&
				   buffer_append_data_from_range(output, &argument2);

		case get_temp_file_name:
			return !arguments_count && path_get_temp_file_name(output);

		case get_temp_path:
			return !arguments_count && path_get_temp_path(output);

		case has_extension:
			return (1 == arguments_count) &&
				   bool_to_string(path_has_extension(argument1.start, argument1.finish), output);

		case is_path_rooted:
			return (1 == arguments_count) &&
				   bool_to_string(path_is_path_rooted(argument1.start, argument1.finish), output);

		case UNKNOWN_PATH_FUNCTION:
		default:
			break;
	}

	return 0;
}

uint8_t cygpath_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output)
{
	if (UNKNOWN_PATH_FUNCTION <= function ||
		NULL == arguments ||
		1 != arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_one_argument(arguments, &argument, 0))
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	switch (function)
	{
		case get_dos_path:
			return cygpath_get_dos_path(argument.start, argument.finish, output);

		case get_unix_path:
			return buffer_append_data_from_range(output, &argument) &&
				   cygpath_get_unix_path(buffer_data(output, size),
										 buffer_data(output, size) + range_size(&argument));

		case get_windows_path:
			return buffer_append_data_from_range(output, &argument) &&
				   cygpath_get_windows_path(buffer_data(output, size),
											buffer_data(output, size) + range_size(&argument));

		case UNKNOWN_PATH_FUNCTION:
		default:
			break;
	}

	return 0;
}
