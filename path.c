/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
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
#include "text_encoding.h"

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

uint8_t path_combine_in_place(struct buffer* path1, const ptrdiff_t size,
							  const uint8_t* path2_start, const uint8_t* path2_finish)
{
	if (NULL == path1 ||
		buffer_size(path1) < size ||
		path2_finish < path2_start)
	{
		return 0;
	}

	if (size < buffer_size(path1) &&
		path2_start < path2_finish &&
		!buffer_push_back(path1, PATH_DELIMITER))
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

uint8_t path_get_directory_name(const uint8_t* path_start, const uint8_t* path_finish,
								struct range* directory)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) || NULL == directory)
	{
		return 0;
	}

	directory->finish = find_any_symbol_like_or_not_like_that(
							path_finish - 1, path_start, &PATH_DELIMITER, 1, 1, -1);
	directory->start = path_start;

	if (!string_trim(directory))
	{
		return 0;
	}

	if (PATH_DELIMITER == (*directory->start) && range_is_null_or_empty(directory))
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
										 path_finish - 1, path_start, &PATH_DELIMITER, 1, 1, -1);
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
									 &PATH_DELIMITER, 1, 1, -1);

	if (PATH_DELIMITER == (*file_name_start) && file_name_start < path_finish)
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

	while (-1 != (index = string_index_of(start, finish, upper_level, upper_level + UPPER_LEVEL_LENGTH)))
	{
		if (0 == index)
		{
			return 0;
		}

		const uint8_t* start_ = find_any_symbol_like_or_not_like_that(start + index - 1, real_start, &PATH_DELIMITER,
								1, 1,
								-1);
		start = start + index + UPPER_LEVEL_LENGTH;

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

			start -= index + UPPER_LEVEL_LENGTH;
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

		if (!common_replace_double_byte_by_single(start, &index, PATH_DELIMITER))
		{
			return 0;
		}

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
			(PATH_DELIMITER == path_start[2] ||
			 path_posix_delimiter == path_start[2]))
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

	if (PATH_DELIMITER == path_start[0])
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
}

uint8_t path_get_temp_path(struct buffer* temp_path)
{
	if (NULL == temp_path)
	{
		return 0;
	}

#if defined(_WIN32)
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
	return (path_posix_delimiter == path_start[0] || PATH_DELIMITER == path_start[0]);
}

uint8_t path_glob(const uint8_t* path_start, const uint8_t* path_finish,
				  const uint8_t* wild_card_start, const uint8_t* wild_card_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		range_in_parts_is_null_or_empty(wild_card_start, wild_card_finish))
	{
		return 0;
	}

	static const uint8_t asterisk = '*';
	static const uint8_t question_mark = '?';
	/**/
	uint8_t go_until = 0;
	uint32_t wild_card_symbol = 0;

	while (NULL != (wild_card_start = string_enumerate(wild_card_start, wild_card_finish, &wild_card_symbol)))
	{
		if (NULL == path_start && asterisk != wild_card_symbol)
		{
			return 0;
		}

		if (NULL != path_start && asterisk != wild_card_symbol)
		{
			uint32_t input_symbol = 0;

			if (go_until)
			{
				if (question_mark != wild_card_symbol)
				{
					while (NULL != (path_start = string_enumerate(path_start, path_finish, &input_symbol)))
					{
						if (wild_card_symbol == input_symbol)
						{
							break;
						}
					}

					if (NULL == path_start)
					{
						return 0;
					}
				}
				else
				{
					path_start = string_enumerate(path_start, path_finish, &input_symbol);
				}

				go_until = 0;
				continue;
			}

			path_start = string_enumerate(path_start, path_finish, &input_symbol);

			if (question_mark != wild_card_symbol && input_symbol != wild_card_symbol)
			{
				return 0;
			}

			continue;
		}

		if (asterisk == wild_card_symbol)
		{
			go_until = 1;
			continue;
		}
	}

	return 1;
}

uint8_t path_get_directory_for_current_process(struct buffer* path)
{
	if (NULL == path)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(path);
#if defined(_WIN32)

	if (!buffer_append(path, NULL, 6 * (FILENAME_MAX + 1) + sizeof(uint32_t)))
	{
		return 0;
	}

	wchar_t* pathW = (wchar_t*)buffer_data(path, size);
	const DWORD length = GetCurrentDirectoryW(2, pathW);

	if (length < 2)
	{
		return 0;
	}

	if (FILENAME_MAX < length)
	{
		if (!buffer_resize(path, size) ||
			!buffer_append(path, NULL, (ptrdiff_t)6 * (length + (ptrdiff_t)1) + sizeof(uint32_t)))
		{
			return 0;
		}
	}

	pathW = (wchar_t*)buffer_data(path,
								  buffer_size(path) - sizeof(uint32_t) - sizeof(uint16_t) * length - sizeof(uint16_t));
	const DWORD returned_length = GetCurrentDirectoryW(length + 1, pathW);

	if (returned_length < 2)
	{
		return 0;
	}

	const wchar_t* start = (const wchar_t*)buffer_data(path,
						   buffer_size(path) - sizeof(uint32_t) - sizeof(uint16_t) * length - sizeof(uint16_t));

	if (!buffer_resize(path, size))
	{
		return 0;
	}

	const wchar_t* finish = start + returned_length;
	file_system_set_position_after_pre_root_wchar_t(&start);
	return text_encoding_UTF16LE_to_UTF8(start, finish, path);
#else

	for (;;)
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

	while (buffer_append(path, NULL, sizeof(uint16_t) * FILENAME_MAX + sizeof(uint32_t)))
	{
		wchar_t* ptr = (wchar_t*)buffer_data(path, size + sizeof(uint32_t));
		const ptrdiff_t expected_size = (buffer_size(path) - size) / sizeof(uint16_t) - sizeof(uint32_t);
		const DWORD real_size = GetModuleFileNameW(NULL, ptr, (DWORD)expected_size);

		if (!real_size)
		{
			return 0;
		}

		if (expected_size < (ptrdiff_t)real_size)
		{
			continue;
		}

		if (!buffer_resize(path, size + real_size) ||
			!buffer_append(path, NULL, (ptrdiff_t)4 * (real_size + (ptrdiff_t)1) + sizeof(uint32_t)))
		{
			return 0;
		}

		ptr = (wchar_t*)buffer_data(path, size + sizeof(uint32_t));

		if (!buffer_resize(path, size))
		{
			return 0;
		}

		if (!text_encoding_UTF16LE_to_UTF8(ptr, ptr + real_size, path))
		{
			return 0;
		}

		struct range directory;

		if (!path_get_directory_name(buffer_data(path, size), buffer_data(path, 0) + buffer_size(path), &directory))
		{
			return 0;
		}

		return buffer_resize(path, size + range_size(&directory));
	}

#elif defined(__linux__)

	while (buffer_append_wchar_t(path, NULL, FILENAME_MAX))
	{
		uint8_t* ptr = buffer_data(path, size);
		const ptrdiff_t expected_size = buffer_size(path) - size;
		ptrdiff_t real_size = readlink("/proc/self/exe", (char*)ptr, expected_size);

		if (-1 == real_size)
		{
			return 0;
		}

		if (expected_size < real_size)
		{
			continue;
		}

		struct range directory;

		if (!path_get_directory_name(ptr, ptr + real_size, &directory))
		{
			return 0;
		}

		return buffer_resize(path, size + range_size(&directory));
	}

#else
	(void)size;
	/*TODO:*/
#endif
	return 0;
}

const uint8_t* path_try_to_get_absolute_path(const void* the_project, const void* the_target,
		struct buffer* input, struct buffer* tmp, uint8_t verbose)
{
	struct range path;
	BUFFER_TO_RANGE(path, input);

	if (!path_is_path_rooted(path.start, path.finish))
	{
		if (!project_get_current_directory(the_project, the_target, tmp, 0, verbose))
		{
			return NULL;
		}

		if (!path_combine_in_place(tmp, 0, path.start, path.finish) ||
			!buffer_push_back(tmp, 0))
		{
			return NULL;
		}

		path.start = buffer_data(tmp, 0);

		if (file_exists(path.start))
		{
			path.finish = NULL;
		}

		if (NULL != path.finish)
		{
			BUFFER_TO_RANGE(path, input);

			if (!buffer_resize(tmp, 0))
			{
				return NULL;
			}

			if (!path_get_directory_for_current_process(tmp))
			{
				return NULL;
			}

			if (!path_combine_in_place(tmp, 0, path.start, path.finish) ||
				!buffer_push_back(tmp, 0))
			{
				return NULL;
			}

			path.start = buffer_data(tmp, 0);

			if (file_exists(path.start))
			{
				path.finish = NULL;
			}
		}

		if (NULL != path.finish)
		{
			BUFFER_TO_RANGE(path, input);

			if (!buffer_resize(tmp, 0))
			{
				return NULL;
			}

			if (file_get_full_path(&path, tmp))
			{
				path.start = buffer_data(tmp, 0);
				path.finish = NULL;
			}
		}

		if (NULL != path.finish)
		{
			if (!buffer_push_back(input, 0))
			{
				return NULL;
			}

			path.start = buffer_data(input, 0);
			path.finish = NULL;
		}
	}

	return path.start;
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

	if (!text_encoding_UTF8_to_UTF16LE(path_start, path_finish, path) ||
		!buffer_push_back_uint16(path, 0))
	{
		return 0;
	}

	const ptrdiff_t count = path_finish - path_start;

	if (!buffer_append(path, NULL, 6 * (count + 1) + sizeof(uint32_t)))
	{
		return 0;
	}

	const wchar_t* long_path = (const wchar_t*)buffer_data(path, size);
	wchar_t* short_path = (wchar_t*)buffer_data(path,
						  buffer_size(path) - sizeof(uint32_t) - sizeof(uint16_t) * count - sizeof(uint16_t));
	/**/
	const DWORD returned_count = GetShortPathNameW(long_path, short_path, (DWORD)(count + 1));

	if (!returned_count)
	{
		return 0;
	}

	if (!buffer_resize(path, size))
	{
		return 0;
	}

	return text_encoding_UTF16LE_to_UTF8(short_path, short_path + returned_count, path);
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
	path_change_extension_function,
	path_combine_function,
	path_get_directory_name_function,
	path_get_extension_function,
	path_get_file_name_function,
	path_get_file_name_without_extension_function,
	path_get_full_path_function,
	path_get_path_root_function,
	path_get_temp_file_name_function,
	path_get_temp_path_function,
	path_has_extension_function,
	path_is_path_rooted_function,
	cygpath_get_dos_path_function,
	cygpath_get_unix_path_function,
	cygpath_get_windows_path_function,
	UNKNOWN_PATH_FUNCTION
};

uint8_t path_get_id_of_get_full_path_function()
{
	return path_get_full_path_function;
}

uint8_t path_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, path_function_str, UNKNOWN_PATH_FUNCTION);
}

uint8_t path_exec_function(const void* project, uint8_t function, const struct buffer* arguments,
						   uint8_t arguments_count,
						   struct buffer* output)
{
	(void)project;

	if (UNKNOWN_PATH_FUNCTION <= function ||
		NULL == arguments ||
		2 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range values[2];

	if (!common_get_arguments(arguments, arguments_count, values, 0))
	{
		return 0;
	}

	switch (function)
	{
		case path_change_extension_function:
			return (2 == arguments_count) &&
				   path_change_extension(values[0].start, values[0].finish, values[1].start, values[1].finish, output);

		case path_combine_function:
			return (2 == arguments_count) &&
				   path_combine(values[0].start, values[0].finish, values[1].start, values[1].finish, output);

		case path_get_directory_name_function:
			return (1 == arguments_count) &&
				   path_get_directory_name(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_extension_function:
			return (1 == arguments_count) &&
				   path_get_extension(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_file_name_function:
			return (1 == arguments_count) &&
				   path_get_file_name(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_file_name_without_extension_function:
			return (1 == arguments_count) &&
				   path_get_file_name_without_extension(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_path_root_function:
			return (1 == arguments_count) &&
				   path_get_path_root(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_temp_file_name_function:
#if defined(_WIN32)
			return !arguments_count && path_get_temp_file_name(output) && file_create(buffer_data(output, 0));
#else
			return !arguments_count && path_get_temp_file_name(output);
#endif

		case path_get_temp_path_function:
			return !arguments_count && path_get_temp_path(output);

		case path_has_extension_function:
			return (1 == arguments_count) &&
				   bool_to_string(path_has_extension(values[0].start, values[0].finish), output);

		case path_is_path_rooted_function:
			return (1 == arguments_count) &&
				   bool_to_string(path_is_path_rooted(values[0].start, values[0].finish), output);

		case path_get_full_path_function:
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
		case cygpath_get_dos_path_function:
			return cygpath_get_dos_path(argument.start, argument.finish, output);

		case cygpath_get_unix_path_function:
			return buffer_append_data_from_range(output, &argument) &&
				   cygpath_get_unix_path(buffer_data(output, size),
										 buffer_data(output, size) + range_size(&argument));

		case cygpath_get_windows_path_function:
			return buffer_append_data_from_range(output, &argument) &&
				   cygpath_get_windows_path(buffer_data(output, size),
											buffer_data(output, size) + range_size(&argument));

		case UNKNOWN_PATH_FUNCTION:
		default:
			break;
	}

	return 0;
}
