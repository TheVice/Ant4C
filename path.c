/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

/*#if !defined(_WIN32)
#if defined(__linux)
#define _POSIX_SOURCE 1
#define _DEFAULT_SOURCE 1
#else
#define _BSD_SOURCE 1
#endif
#endif*/

#include "stdc_secure_api.h"

#include "path.h"
#include "buffer.h"
#include "common.h"
#if !defined(_WIN32)
#include "environment.h"
#endif
#include "file_system.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#if defined(_WIN32)
#include <wchar.h>

#include <windows.h>
#else

#include <unistd.h>
#include <stdlib.h>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#endif

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/sysctl.h>
#include <errno.h>
#endif

#endif

#include <stdio.h>
#include <string.h>

#ifndef L_tmpnam_s
#define L_tmpnam_s L_tmpnam
#endif

static const uint8_t point = '.';
#if defined(_WIN32)
static const uint8_t colon_mark = ':';
#else
static const uint8_t tilde = '~';
#endif
static const uint8_t* upper_level = (const uint8_t*)"..";

uint8_t path_change_extension(
	const uint8_t* path_start, const uint8_t* path_finish,
	const uint8_t* ext_start, const uint8_t* ext_finish,
	struct buffer* path)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	const uint8_t* cur_ext_start = path_start;

	if (path_get_extension(&cur_ext_start, path_finish))
	{
		if (string_equal(cur_ext_start, path_finish, ext_start, ext_finish))
		{
			return buffer_append(path, path_start, path_finish - path_start);
		}

		if (!buffer_append(path, path_start, cur_ext_start - path_start))
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

uint8_t path_combine_in_place(
	struct buffer* path1, const ptrdiff_t size,
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

		if (!string_replace_double_char_with_single(
				buffer_data(path1, size),
				&new_size, &PATH_DELIMITER,
				&PATH_DELIMITER + 1))
		{
			return 0;
		}
	}

	return new_size ? buffer_resize(path1, size + new_size) : 1;
}

uint8_t path_combine(
	const uint8_t* path1_start, const uint8_t* path1_finish,
	const uint8_t* path2_start, const uint8_t* path2_finish,
	struct buffer* output)
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

uint8_t path_get_directory_name(
	const uint8_t* path_start, const uint8_t** path_finish)
{
	if (NULL == path_finish ||
		range_in_parts_is_null_or_empty(path_start, *path_finish))
	{
		return 0;
	}

	const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
							 *path_finish, path_start,
							 &PATH_DELIMITER, &PATH_DELIMITER + 1, 1, -1);
	uint32_t char_set;
	const uint8_t* pos_ = string_enumerate(pos, *path_finish, &char_set);

	if (PATH_DELIMITER == char_set && path_start == pos)
	{
		*path_finish = pos_;
	}
	else
	{
		*path_finish = pos;
	}

	return path_start != *path_finish;
}

uint8_t path_get_extension(
	const uint8_t** path_start, const uint8_t* path_finish)
{
	if (NULL == path_start ||
		range_in_parts_is_null_or_empty(*path_start, path_finish))
	{
		return 0;
	}

	const uint8_t* file_name_start =
		string_find_any_symbol_like_or_not_like_that(
			path_finish, *path_start,
			&PATH_DELIMITER, &PATH_DELIMITER + 1, 1, -1);
	file_name_start = string_find_any_symbol_like_or_not_like_that(
						  path_finish, file_name_start,
						  &point, &point + 1, 1, -1);
	const uint8_t* pos;
	uint32_t char_set;

	if (NULL == (pos = string_enumerate(file_name_start, path_finish, &char_set)))
	{
		return 0;
	}

	if (point != char_set)
	{
		return 0;
	}

	if (pos == path_finish)
	{
		file_name_start = path_finish;
	}

	*path_start = file_name_start;
	return file_name_start < path_finish;
}

uint8_t path_get_file_name(
	const uint8_t** path_start, const uint8_t* path_finish)
{
	if (NULL == path_start ||
		range_in_parts_is_null_or_empty(*path_start, path_finish))
	{
		return 0;
	}

	const uint8_t* file_name_start =
		string_find_any_symbol_like_or_not_like_that(
			path_finish, *path_start,
			&PATH_DELIMITER, &PATH_DELIMITER + 1, 1, -1);
	uint32_t char_set;
	const uint8_t* pos = string_enumerate(file_name_start, path_finish, &char_set);

	if (PATH_DELIMITER == char_set && NULL != pos)
	{
		file_name_start = pos;
	}

	*path_start = file_name_start;
	return 1;
}

uint8_t path_get_file_name_without_extension(
	const uint8_t* path_start, const uint8_t* path_finish,
	struct range* file_name)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		NULL == file_name)
	{
		return 0;
	}

	file_name->start = path_start;
	file_name->finish = path_finish;

	if (!path_get_file_name(&file_name->start, path_finish))
	{
		return 0;
	}

	const uint8_t* ext_start = file_name->start;
	const uint8_t has_extension = path_get_extension(&ext_start, file_name->finish);

	if (has_extension)
	{
		file_name->finish = ext_start;
	}

	return string_trim(file_name);
}

uint8_t path_is_start_valid(const uint8_t* start, const uint8_t* finish)
{
	struct range out;
	uint32_t char_set;
	const ptrdiff_t length = string_get_length(start, finish);

	if (string_starts_with(start, finish, upper_level, upper_level + 2))
	{
		if (2 == length)
		{
			return 0;
		}

		if (!string_substring(start, finish, 2, 1, &out))
		{
			return 0;
		}

		if (!string_enumerate(out.start, out.finish, &char_set))
		{
			return 0;
		}

		if (PATH_DELIMITER == char_set)
		{
			return 0;
		}
	}

#if defined(_WIN32)

	if (2 < length)
	{
		if (!string_substring(start, finish, 1, 1, &out))
		{
			return 0;
		}

		if (!string_enumerate(out.start, out.finish, &char_set))
		{
			return 0;
		}

		if (colon_mark == char_set)
		{
			if (!string_substring(start, finish, 3, -1, &out))
			{
				return 0;
			}

			if (string_starts_with(out.start, out.finish, upper_level, upper_level + 2))
			{
				if (4 == length)
				{
					return 0;
				}
				else if (4 < length)
				{
					if (!string_substring(start, finish, 5, 1, &out))
					{
						if (!string_enumerate(out.start, out.finish, &char_set))
						{
							return 0;
						}

						if (PATH_DELIMITER == char_set)
						{
							return 0;
						}
					}
				}
			}
		}
	}

#else

	if (length)
	{
		start = string_enumerate(start, finish, &char_set);

		if (!start)
		{
			return 0;
		}

		if (PATH_DELIMITER == char_set)
		{
			if (string_starts_with(start, finish, upper_level, upper_level + 2))
			{
				if (3 == length)
				{
					return 0;
				}
				else if (3 < length)
				{
					if (!string_substring(start, finish, 2, 1, &out))
					{
						return 0;
					}

					if (!string_enumerate(out.start, out.finish, &char_set))
					{
						return 0;
					}

					if (PATH_DELIMITER == char_set)
					{
						return 0;
					}
				}
			}
		}
	}

#endif
	return 1;
}

uint8_t path_get_full_path(
	const uint8_t* root_start, const uint8_t* root_finish,
	const uint8_t* path_start, const uint8_t* path_finish,
	struct buffer* full_path)
{
	if (NULL == path_start ||
		NULL == path_finish ||
		path_finish < path_start ||
		NULL == full_path)
	{
		return 0;
	}

	uint32_t char_set;
	const ptrdiff_t size = buffer_size(full_path);
#if !defined(_WIN32)

	if (!string_enumerate(path_start, path_finish, &char_set))
	{
		return 0;
	}

	if (tilde == char_set)
	{
		if (!environment_get_folder_path(UserProfile, full_path))
		{
			return 0;
		}
	}

#endif
	uint8_t is_path_rooted;

	if (!path_is_path_rooted(path_start, path_finish, &is_path_rooted))
	{
		return 0;
	}

	if (!is_path_rooted)
	{
		if (!string_replace(root_start, root_finish,
#if defined(_WIN32)
							&path_posix_delimiter, &path_posix_delimiter + 1,
#else
							&path_windows_delimiter, &path_windows_delimiter + 1,
#endif
							&PATH_DELIMITER, &PATH_DELIMITER + 1, full_path))
		{
			return 0;
		}
	}

	if (!path_combine_in_place(full_path, size, path_start, path_finish))
	{
		return 0;
	}

	const uint8_t* start = buffer_data(full_path, size);
	const uint8_t* finish = start + buffer_size(full_path);
	const uint8_t* pos = start;

	while (finish != (pos = string_find_any_symbol_like_or_not_like_that(
								pos, finish, &PATH_DELIMITER, &PATH_DELIMITER + 1, 1, 1)))
	{
		if (!path_is_start_valid(start, finish))
		{
			return 0;
		}

		const uint8_t* prev_pos = pos;
		pos = string_enumerate(pos, finish, NULL);

		if (!string_starts_with(pos, finish, upper_level, upper_level + 2))
		{
			continue;
		}

		is_path_rooted = 2;

		while (is_path_rooted)
		{
			pos = string_enumerate(pos, finish, NULL);
			--is_path_rooted;
		}

		if (!string_enumerate(pos, finish, &char_set))
		{
			return 0;
		}

		if (finish == pos || PATH_DELIMITER == char_set)
		{
			const uint8_t* start_1 =
				string_find_any_symbol_like_or_not_like_that(
					prev_pos, start, &PATH_DELIMITER, &PATH_DELIMITER + 1, 0, -1);
			start_1 =
				string_find_any_symbol_like_or_not_like_that(
					start_1, start, &PATH_DELIMITER, &PATH_DELIMITER + 1, 1, -1);
			start_1 =
				string_find_any_symbol_like_or_not_like_that(
					start_1, finish, &PATH_DELIMITER, &PATH_DELIMITER + 1, 0, 1);
			/**/
			const uint8_t* start_2 =
				string_find_any_symbol_like_or_not_like_that(
					pos, finish, &PATH_DELIMITER, &PATH_DELIMITER + 1, 0, 1);
			/**/
			uint8_t* dst = buffer_data(full_path, size + start_1 - start);

			for (; start_2 < finish; ++start_2, ++dst)
			{
				*dst = *start_2;
			}

			pos =
				string_find_any_symbol_like_or_not_like_that(
					start_1, start, &PATH_DELIMITER, &PATH_DELIMITER + 1, 1, -1);
			finish = dst;
		}
	}

	return buffer_resize(full_path, size + finish - start);
}

uint8_t path_get_path_root(
	const uint8_t* path_start, const uint8_t** path_finish)
{
	if (NULL == path_finish ||
		range_in_parts_is_null_or_empty(path_start, *path_finish))
	{
		return 0;
	}

#if defined(_WIN32)
	const ptrdiff_t length = string_get_length(path_start, *path_finish);

	if (1 < length)
	{
		struct range out;
		uint32_t char_set;

		if (!string_substring(path_start, *path_finish, 1, 1, &out))
		{
			return 0;
		}

		if (!string_enumerate(out.start, out.finish, &char_set))
		{
			return 0;
		}

		if (colon_mark == char_set)
		{
			if (2 < length)
			{
				if (!string_substring(path_start, *path_finish, 2, 1, &out))
				{
					return 0;
				}

				if (!string_enumerate(out.start, out.finish, &char_set))
				{
					return 0;
				}

				if (PATH_DELIMITER == char_set ||
					path_posix_delimiter == char_set)
				{
					char_set = 3;
				}
				else
				{
					char_set = 2;
				}
			}
			else
			{
				char_set = 2;
			}

			const uint8_t* pos = path_start;

			while (char_set)
			{
				pos = string_enumerate(pos, *path_finish, NULL);
				--char_set;
			}

			*path_finish = pos;
			return NULL != *path_finish;
		}
		else
		{
			*path_finish = path_start;
		}
	}

	return 0;
#else
	uint32_t char_set;
	*path_finish = string_enumerate(path_start, *path_finish, &char_set);

	if (NULL != *path_finish &&
		PATH_DELIMITER == char_set)
	{
		return 1;
	}
	else
	{
		*path_finish = path_start;
	}

	return 0;
#endif
}
#if defined(_MSC_VER)
uint8_t path_get_temp_file_name_wchar_t(struct buffer* temp_file_name)
{
	if (NULL == temp_file_name)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(temp_file_name);

	if (!buffer_append(temp_file_name, NULL, 4 * L_tmpnam_s + sizeof(uint32_t)))
	{
		return 0;
	}

	wchar_t* temp_file_path_wchar_t = (wchar_t*)buffer_data(temp_file_name, size + sizeof(uint32_t));

	if (0 != _wtmpnam_s(temp_file_path_wchar_t, L_tmpnam_s))
	{
		return 0;
	}

	const wchar_t* temp_file_path_wchar_t_finish = temp_file_path_wchar_t + wcslen(temp_file_path_wchar_t);

	if (!buffer_resize(temp_file_name, size) ||
		!text_encoding_UTF16LE_to_UTF8(temp_file_path_wchar_t, temp_file_path_wchar_t_finish, temp_file_name))
	{
		return 0;
	}

	const ptrdiff_t length = buffer_size(temp_file_name);

	if (!buffer_push_back(temp_file_name, 0) ||
		!buffer_resize(temp_file_name, length))
	{
		return 0;
	}

	return 1;
}
#endif
uint8_t path_get_temp_file_name(struct buffer* temp_file_name)
{
	if (NULL == temp_file_name)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(temp_file_name);
#if !defined(_WIN32)

	if (!path_get_temp_path(temp_file_name) ||
		!buffer_append_char(temp_file_name, "/fileXXXXXX\0", 12))
	{
		return 0;
	}

	char* temp_file_path = (char*)buffer_data(temp_file_name, size);
	int fd;

	if (-1 == (fd = mkstemp(temp_file_path)))
	{
		return 0;
	}

	close(fd);
	return buffer_resize(temp_file_name, buffer_size(temp_file_name) - 1);
#else

	if (!buffer_append_char(temp_file_name, NULL, L_tmpnam_s))
	{
		return 0;
	}

	uint8_t* temp_file_path = buffer_data(temp_file_name, size);
#if __STDC_LIB_EXT1__

	if (0 != tmpnam_s((char*)temp_file_path, L_tmpnam_s))
	{
		return 0;
	}

#else
	tmpnam((char*)temp_file_path);
#endif
	ptrdiff_t length = common_count_bytes_until(temp_file_path, 0);
	const uint8_t* temp_file_path_finish = temp_file_path + length;

	if (length < 1)
	{
		return 0;
	}

	if (string_ends_with(
			temp_file_path, temp_file_path_finish,
			&point, &point + 1))
	{
		uint32_t char_set;
		const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
								 temp_file_path_finish, temp_file_path,
								 &point, &point + 1, 0, -1);

		if (pos == temp_file_path)
		{
			return 0;
		}

		if (!string_enumerate(pos, temp_file_path_finish, &char_set))
		{
			return 0;
		}

		if (PATH_DELIMITER == char_set)
		{
			return 0;
		}

		pos = string_enumerate(pos, temp_file_path_finish, NULL);

		if (NULL == pos)
		{
			return 0;
		}

		length = pos - temp_file_path;
	}

	uint8_t is_path_rooted;

	if (!path_is_path_rooted(temp_file_path, temp_file_path + length, &is_path_rooted))
	{
		return 0;
	}

	if (is_path_rooted)
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
#if __STDC_LIB_EXT1__

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
#if defined(NTDDI_VERSION) && defined(NTDDI_WIN10_FE) && (NTDDI_VERSION >= NTDDI_WIN10_FE)
#define GET_TEMP_PATH GetTempPath2W
#else
#define GET_TEMP_PATH GetTempPathW
#endif

	if (NULL == temp_path)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(temp_path);
#if defined(_WIN32)

	if (!buffer_append(temp_path, NULL, (ptrdiff_t)4 * FILENAME_MAX + sizeof(uint32_t)))
	{
		return 0;
	}

	wchar_t* pathW = (wchar_t*)buffer_data(temp_path, size + sizeof(uint32_t));
	DWORD length = GET_TEMP_PATH(sizeof(wchar_t), pathW);

	if (length < sizeof(wchar_t) + 1)
	{
		return 0;
	}

	if (!buffer_resize(temp_path, size) ||
		!buffer_append(temp_path, NULL, (ptrdiff_t)4 * length + sizeof(uint32_t)))
	{
		return 0;
	}

	pathW = (wchar_t*)buffer_data(temp_path, size + sizeof(uint32_t));
	length = GET_TEMP_PATH(length, pathW);

	if (!buffer_resize(temp_path, size) ||
		!text_encoding_UTF16LE_to_UTF8((const uint16_t*)pathW, (const uint16_t*)(pathW + length), temp_path))
	{
		return 0;
	}

#else
	static const uint8_t* tmp_dir = (const uint8_t*)"TMPDIR";

	if (environment_get_variable(tmp_dir, tmp_dir + 6, temp_path))
	{
#endif
	const uint8_t* path_start = buffer_data(temp_path, size);
	const uint8_t* path_finish = buffer_data(temp_path, 0) + buffer_size(temp_path);

	if (!path_get_directory_name(path_start, &path_finish))
	{
		return 0;
	}

	return buffer_resize(temp_path, size + (path_finish - path_start));
#if !defined(_WIN32)
}

if (!buffer_resize(temp_path, size))
{
	return 0;
}

#if defined(__ANDROID__)
static const uint8_t* temp_path_ = (const uint8_t*)"/data/local/tmp";
#define TEMP_PATH_SIZE 15
#else
static const uint8_t* temp_path_ = (const uint8_t*)"/tmp";
#define TEMP_PATH_SIZE 4
#endif
return directory_exists(temp_path_) &&
	   buffer_append(temp_path, temp_path_, TEMP_PATH_SIZE);
#endif
}

uint8_t path_has_extension(
	const uint8_t* path_start, const uint8_t* path_finish)
{
	return path_get_extension(&path_start, path_finish);
}

uint8_t path_is_path_rooted(
	const uint8_t* path_start, const uint8_t* path_finish,
	uint8_t* is_path_rooted)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		!is_path_rooted)
	{
		return 0;
	}

	struct range out;

	uint32_t char_set;

#if defined(_WIN32)
	ptrdiff_t length = string_get_length(path_start, path_finish);

	if (2 < length)
	{
		length = file_system_get_position_after_pre_root(&path_start, path_finish);

		if (!length)
		{
			return 0;
		}
		else if (2 == length)
		{
			return path_is_path_rooted(path_start, path_finish, is_path_rooted);
		}

		length = 2;
	}

	if (1 < length)
	{
		if (!string_substring(path_start, path_finish, 1, 1, &out))
		{
			return 0;
		}

		if (!string_enumerate(out.start, out.finish, &char_set))
		{
			return 0;
		}

		*is_path_rooted = colon_mark == char_set;
		return 1;
	}

#endif

	if (!string_substring(path_start, path_finish, 0, 1, &out))
	{
		return 0;
	}

	if (!string_enumerate(out.start, out.finish, &char_set))
	{
		return 0;
	}

	*is_path_rooted =
		path_posix_delimiter == char_set ||
		PATH_DELIMITER == char_set;
	return 1;
}

uint8_t path_glob(
	const uint8_t* path_start, const uint8_t* path_finish,
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
	uint32_t wild_card_symbol;

	while (NULL != (wild_card_start = string_enumerate(wild_card_start, wild_card_finish, &wild_card_symbol)))
	{
		if (NULL == path_start && asterisk != wild_card_symbol)
		{
			return 0;
		}

		if (NULL != path_start && asterisk != wild_card_symbol)
		{
			uint32_t input_symbol;

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
	DWORD length = GetCurrentDirectoryW(2, pathW);

	if (length < 2)
	{
		return 0;
	}

	if (!buffer_resize(path, size) ||
		!buffer_append(path, NULL, (ptrdiff_t)6 * (length + (ptrdiff_t)1) + sizeof(uint32_t)))
	{
		return 0;
	}

	pathW = (wchar_t*)buffer_data(path,
								  buffer_size(path) - sizeof(uint32_t) - sizeof(wchar_t) * length - sizeof(wchar_t));
	length = GetCurrentDirectoryW(length + 1, pathW);

	if (length < 2)
	{
		return 0;
	}

	const wchar_t* start = pathW;
	const wchar_t* finish = pathW + length;
	file_system_set_position_after_pre_root_wchar_t(&start);
	return buffer_resize(path, size) &&
		   text_encoding_UTF16LE_to_UTF8((const uint16_t*)start, (const uint16_t*)finish, path);
#else

	for (;;)
	{
		if (!buffer_append_char(path, NULL, FILENAME_MAX))
		{
			return 0;
		}

		const uint8_t* path_ = buffer_data(path, size);
		const ptrdiff_t length = buffer_size(path) - size;

		if (!getcwd((char*)path_, (int)length))
		{
			continue;
		}

		if (!buffer_resize(path, size + common_count_bytes_until(path_, 0)))
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

	while (buffer_append(path, NULL, sizeof(wchar_t) * FILENAME_MAX + sizeof(uint32_t)))
	{
		wchar_t* pathW = (wchar_t*)buffer_data(path, size + sizeof(uint32_t));
		const ptrdiff_t expected_size = (buffer_size(path) - size) / sizeof(wchar_t) - sizeof(uint32_t);
		const DWORD real_size = GetModuleFileNameW(NULL, pathW, (DWORD)expected_size);

		if (!real_size)
		{
			return 0;
		}

		if (expected_size < (ptrdiff_t)real_size)
		{
			continue;
		}

		if (!buffer_resize(path, size) ||
			!text_encoding_UTF16LE_to_UTF8((const uint16_t*)pathW, (const uint16_t*)(pathW + real_size), path))
		{
			return 0;
		}

		break;
	}

#elif defined(__APPLE__) && defined(__MACH__)

	while (buffer_append(path, NULL, FILENAME_MAX))
	{
		uint8_t* ptr = buffer_data(path, size);
		uint32_t expected_size = (uint32_t)(buffer_size(path) - size);

		if (0 != _NSGetExecutablePath((char*)ptr, &expected_size))
		{
			continue;
		}

		const uint8_t* finish = buffer_data(path, 0) + buffer_size(path);
		static const uint8_t zero = 0;
		finish = string_find_any_symbol_like_or_not_like_that(
					 ptr, finish, &zero, &zero + 1, 1, 1);

		if (!buffer_resize(path, finish - buffer_data(path, 0)))
		{
			return 0;
		}

		break;
	}

#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#define MIB_COUNT 4
	int mib[MIB_COUNT];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;

	while (buffer_append(path, NULL, FILENAME_MAX))
	{
		size_t expected_size = (size_t)(buffer_size(path) - size);
		uint8_t* ptr = buffer_data(path, size);

		if (0 == sysctl(mib, MIB_COUNT, ptr, &expected_size, NULL, 0))
		{
			if (!buffer_resize(path, size + expected_size))
			{
				return 0;
			}

			break;
		}

		if (ENOMEM != errno)
		{
			return 0;
		}
	}

#elif defined(__OpenBSD__)
	/*TODO:*/
#elif defined(__linux__) || defined(BSD)
#if defined(__linux__)
	static const char* path_ = "/proc/self/exe";
#elif defined(__NetBSD__)
	static const char* path_ = "/proc/curproc/exe";
#elif defined(__DragonFly__)
	static const char* path_ = "/proc/curproc/file";
#endif

	while (buffer_append(path, NULL, FILENAME_MAX))
	{
		uint8_t* ptr = buffer_data(path, size);
		const ptrdiff_t expected_size = buffer_size(path) - size;
		const ptrdiff_t real_size = readlink(path_, (char*)ptr, expected_size);

		if (-1 == real_size)
		{
			return 0;
		}

		if (expected_size < real_size)
		{
			continue;
		}

		if (!buffer_resize(path, size + real_size))
		{
			return 0;
		}

		break;
	}

#else
	/*NOTE: place for platforms not listed above.*/
#endif
	const uint8_t* path_start = buffer_data(path, size);
	const uint8_t* path_finish = buffer_data(path, 0) + buffer_size(path);

	if (!path_get_directory_name(path_start, &path_finish))
	{
		return 0;
	}

	return buffer_resize(path, size + (path_finish - path_start));
}

const uint8_t* path_try_to_get_absolute_path(
	const void* the_project, const void* the_target,
	struct buffer* input, struct buffer* tmp, uint8_t verbose)
{
	const uint8_t* path_start = buffer_data(input, 0);
	const uint8_t* path_finish = path_start + buffer_size(input);
	uint8_t is_path_rooted;

	if (!path_is_path_rooted(path_start, path_finish, &is_path_rooted))
	{
		return 0;
	}

	if (!is_path_rooted)
	{
		if (!project_get_current_directory(the_project, the_target, tmp, 0, verbose))
		{
			return NULL;
		}

		if (!path_combine_in_place(tmp, 0, path_start, path_finish) ||
			!buffer_push_back(tmp, 0))
		{
			return NULL;
		}

		path_start = buffer_data(tmp, 0);

		if (file_exists(path_start))
		{
			path_finish = NULL;
		}

		if (NULL != path_finish)
		{
			path_start = buffer_data(input, 0);
			path_finish = path_start + buffer_size(input);

			if (!buffer_resize(tmp, 0))
			{
				return NULL;
			}

			if (!path_get_directory_for_current_process(tmp))
			{
				return NULL;
			}

			if (!path_combine_in_place(tmp, 0, path_start, path_finish) ||
				!buffer_push_back(tmp, 0))
			{
				return NULL;
			}

			path_start = buffer_data(tmp, 0);

			if (file_exists(path_start))
			{
				path_finish = NULL;
			}
		}

		if (NULL != path_finish)
		{
			path_start = buffer_data(input, 0);
			path_finish = path_start + buffer_size(input);

			if (!buffer_resize(tmp, 0))
			{
				return NULL;
			}

			if (file_get_full_path(path_start, path_finish, tmp))
			{
				path_start = buffer_data(tmp, 0);
				path_finish = NULL;
			}
		}

		if (NULL != path_finish)
		{
			if (!buffer_push_back(input, 0))
			{
				return NULL;
			}

			path_start = buffer_data(input, 0);
			path_finish = NULL;
		}
	}

	return path_start;
}

uint8_t cygpath_get_dos_path(
	const uint8_t* path_start, const uint8_t* path_finish,
	struct buffer* path)
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
						  buffer_size(path) - sizeof(uint32_t) - sizeof(wchar_t) * count - sizeof(wchar_t));
	/**/
	const DWORD returned_count = GetShortPathNameW(long_path, short_path, (DWORD)(count + 1));
	/**/
	return returned_count &&
		   buffer_resize(path, size) &&
		   text_encoding_UTF16LE_to_UTF8((const uint16_t*)short_path, (const uint16_t*)(short_path + returned_count),
										 path);
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

	uint32_t char_set;
	const uint8_t* pos = path_start;

	while (NULL != (pos = string_enumerate(pos, path_finish, &char_set)))
	{
		if (path_windows_delimiter == char_set &&
			1 == pos - path_start)
		{
			*path_start = path_posix_delimiter;
		}

		path_start += pos - path_start;
	}

	return 1;
}

uint8_t cygpath_get_windows_path(uint8_t* path_start, uint8_t* path_finish)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish))
	{
		return 0;
	}

	uint32_t char_set;
	const uint8_t* pos = path_start;

	while (NULL != (pos = string_enumerate(pos, path_finish, &char_set)))
	{
		if (path_posix_delimiter == char_set &&
			1 == pos - path_start)
		{
			*path_start = path_windows_delimiter;
		}

		path_start += pos - path_start;
	}

	return 1;
}
