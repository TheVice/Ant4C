/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "file_system.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "environment.h"
#include "hash.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

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
#include <sys/types.h>
#endif

enum entry_types { directory_entry, file_entry, all_entries, UNKNOWN_ENTRY_TYPE };

#if defined(_WIN32)

#define FIND_FILE_OBJECT_DATA(PATHW, DATA, RETURN, CLOSE_RESULT)\
	const HANDLE file_handle = FindFirstFileW((PATHW), (DATA));	\
	\
	if (INVALID_HANDLE_VALUE == file_handle)					\
	{															\
		return (RETURN);										\
	}															\
	\
	(CLOSE_RESULT) = FindClose(file_handle);

#define FIND_FILE_OBJECT_DATA_FROM_BUFFER(PATH)												\
	struct buffer pathW;																	\
	SET_NULL_TO_BUFFER(pathW);																\
	\
	if (!file_system_path_to_pathW((PATH), &pathW))											\
	{																						\
		buffer_release(&pathW);																\
		return 0;																			\
	}																						\
	\
	WIN32_FIND_DATAW file_data;																\
	const HANDLE file_handle = FindFirstFileW(buffer_wchar_t_data(&pathW, 0), &file_data);	\
	buffer_release(&pathW);																	\
	\
	if (INVALID_HANDLE_VALUE == file_handle)												\
	{																						\
		return 0;																			\
	}																						\
	\
	if (!FindClose(file_handle))															\
	{																						\
		return 0;																			\
	}

static const uint8_t* pre_root_path = (const uint8_t*)"\\\\?\\";
static const wchar_t* pre_root_path_wchar_t = L"\\\\?\\";
static const uint8_t pre_root_path_length = 4;

uint8_t _buffer_append_pre(struct buffer* the_buffer, const uint8_t* data, ptrdiff_t size)
{
	if (NULL == the_buffer || size < 0)
	{
		return 0;
	}

	if (!size)
	{
		return 1;
	}

	const ptrdiff_t current_size = buffer_size(the_buffer);

	if (!buffer_append(the_buffer, NULL, size))
	{
		return 0;
	}

	for (ptrdiff_t i = current_size; 0 < i;)
	{
		uint8_t* dst = NULL;
		const uint8_t* src = NULL;

		if (i < size)
		{
			dst = buffer_data(the_buffer, size);
			src = buffer_data(the_buffer, 0);
			MEM_CPY(dst, src, i);
			/**/
			break;
		}

		dst = buffer_data(the_buffer, i);
		i -= size;
		src = buffer_data(the_buffer, i);
		MEM_CPY(dst, src, size);
	}

	if (NULL != data)
	{
		uint8_t* dst = buffer_data(the_buffer, 0);
#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(dst, size, data, size))
		{
			return 0;
		}

#else
		memcpy(dst, data, size);
#endif
	}

	return 1;
}

uint8_t file_system_append_pre_root(struct buffer* path)
{
	const ptrdiff_t size = buffer_size(path);

	if (!size)
	{
		return 0;
	}

	struct range path_in_the_range;

	BUFFER_TO_RANGE(path_in_the_range, path);

	if (path_is_path_rooted(path_in_the_range.start, path_in_the_range.finish))
	{
		if (!string_starts_with(path_in_the_range.start, path_in_the_range.finish,
								pre_root_path, pre_root_path + pre_root_path_length) &&
			!_buffer_append_pre(path, pre_root_path, pre_root_path_length))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t file_system_get_position_after_pre_root(struct range* path)
{
	if (range_is_null_or_empty(path))
	{
		return 0;
	}

	const ptrdiff_t index = string_index_of(path->start, path->finish,
											pre_root_path, pre_root_path + pre_root_path_length);
	path->start += (-1 == index ? 0 : index + pre_root_path_length);
	return 1;
}

void file_system_set_position_after_pre_root_wchar_t(const wchar_t** path)
{
	if (NULL != path && NULL != (*path))
	{
		const wchar_t* ptr = wcsstr((*path), pre_root_path_wchar_t);

		if (NULL != ptr)
		{
			ptr += pre_root_path_length;
			(*path) = ptr;
		}
	}
}

uint8_t directory_create_wchar_t(const wchar_t* path)
{
	/*return 0 != CreateDirectoryExW(%HOME%, path, NULL);*/
	return 0 != CreateDirectoryW(path, NULL);
}

uint8_t directory_delete_wchar_t(const wchar_t* path)
{
	return 0 != RemoveDirectoryW(path);
}

uint8_t directory_enumerate_file_system_entries_wchar_t(
	struct buffer* pattern,
	const uint8_t entry_type, const uint8_t recurse, uint8_t output_encoding,
	struct buffer* output, uint8_t fail_on_error)
{
	if (NULL == pattern ||
		all_entries < entry_type ||
		(0 != recurse && 1 != recurse) ||
		(UTF8 != output_encoding && UTF16LE != output_encoding) ||
		NULL == output)
	{
		return 0;
	}

	WIN32_FIND_DATAW file_data;
	const wchar_t* start = buffer_wchar_t_data(pattern, 0);
	const HANDLE file_handle = FindFirstFileW(start, &file_data);

	if (INVALID_HANDLE_VALUE == file_handle)
	{
		return fail_on_error ? 0 : FAIL_WITH_OUT_ERROR;
	}

	const ptrdiff_t size = wcslen(start);
	const wchar_t* finish = start + size;
	const ptrdiff_t index =
		find_any_symbol_like_or_not_like_that_wchar_t(finish - 1, start, L"\\", 1, 1, -1) - start;
	const ptrdiff_t delta = size - index + 1;
	uint8_t result = 1;

	do
	{
		const size_t name_length = wcslen(file_data.cFileName);

		if (0 != (FILE_ATTRIBUTE_DIRECTORY & file_data.dwFileAttributes))
		{
			if ((1 == name_length && 0 == wmemcmp(L".", file_data.cFileName, name_length)) ||
				(2 == name_length && 0 == wmemcmp(L"..", file_data.cFileName, name_length)))
			{
				continue;
			}
		}

		if (!buffer_append_wchar_t(pattern, NULL, 1 + name_length + delta))
		{
			FindClose(file_handle);
			return 0;
		}

		const ptrdiff_t new_index = 1 + index + name_length;

		if (!buffer_resize(pattern, sizeof(wchar_t) * new_index))
		{
			FindClose(file_handle);
			return 0;
		}

		start = buffer_wchar_t_data(pattern, index);

		if (!buffer_append_wchar_t(pattern, start, delta))
		{
			FindClose(file_handle);
			return 0;
		}

		if (!buffer_resize(pattern, index * sizeof(wchar_t)) ||
			!buffer_push_back_uint16(pattern, PATH_DELIMITER) ||
			!buffer_append_wchar_t(pattern, file_data.cFileName, name_length))
		{
			FindClose(file_handle);
			return 0;
		}

		if (0 != (FILE_ATTRIBUTE_DIRECTORY & file_data.dwFileAttributes))
		{
			if (recurse)
			{
				if (!buffer_append_wchar_t(pattern, NULL, delta))
				{
					FindClose(file_handle);
					return 0;
				}

				result = directory_enumerate_file_system_entries_wchar_t(pattern, entry_type, recurse, output_encoding,
						 output, fail_on_error);

				if (!result)
				{
					FindClose(file_handle);
					return 0;
				}
			}

			if (file_entry == entry_type)
			{
				finish = (const wchar_t*)(buffer_data(pattern, 0) + sizeof(wchar_t) * new_index);

				if (!buffer_resize(pattern, index * sizeof(wchar_t)) ||
					!buffer_append_wchar_t(pattern, finish, delta))
				{
					FindClose(file_handle);
					return 0;
				}

				continue;
			}
		}
		else if (directory_entry == entry_type)
		{
			finish = (const wchar_t*)(buffer_data(pattern, 0) + sizeof(wchar_t) * new_index);

			if (!buffer_resize(pattern, index * sizeof(wchar_t)) ||
				!buffer_append_wchar_t(pattern, finish, delta))
			{
				FindClose(file_handle);
				return 0;
			}

			continue;
		}

		finish = (const wchar_t*)(buffer_data(pattern, 0) + sizeof(wchar_t) * new_index);

		if (UTF8 == output_encoding)
		{
			start = buffer_wchar_t_data(pattern, 0);
			file_system_set_position_after_pre_root_wchar_t(&start);

			if (!text_encoding_UTF16LE_to_UTF8(start, finish, output) ||
				!buffer_push_back(output, 0))
			{
				FindClose(file_handle);
				return 0;
			}
		}
		else
		{
			if (!buffer_append(output, buffer_data(pattern, 0), sizeof(wchar_t) * new_index) ||
				!buffer_push_back_uint16(output, 0))
			{
				FindClose(file_handle);
				return 0;
			}
		}

		if (!buffer_resize(pattern, index * sizeof(wchar_t)) ||
			!buffer_append_wchar_t(pattern, finish, delta))
		{
			FindClose(file_handle);
			return 0;
		}
	}
	while (FindNextFileW(file_handle, &file_data));

	FindClose(file_handle);
	return result;
}

uint8_t directory_exists_wchar_t(const wchar_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

	WIN32_FIND_DATAW file_data;
	BOOL close_return = 0;
	FIND_FILE_OBJECT_DATA(path, &file_data, 0, close_return)
	const uint8_t is_directory = (0 != (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	return close_return && is_directory;
}

uint8_t file_system_path_in_range_to_pathW(
	const uint8_t* path_start, const uint8_t* path_finish, struct buffer* pathW)
{
	if (range_in_parts_is_null_or_empty(path_start, path_finish) ||
		NULL == pathW)
	{
		return 0;
	}

	if (path_is_path_rooted(path_start, path_finish) &&
		!string_starts_with(path_start, path_finish, pre_root_path, pre_root_path + pre_root_path_length) &&
		!buffer_append_wchar_t(pathW, pre_root_path_wchar_t, pre_root_path_length))
	{
		return 0;
	}

	return text_encoding_UTF8_to_UTF16LE(path_start, path_finish, pathW) && buffer_push_back_uint16(pathW, 0);
}

uint8_t file_system_path_to_pathW(const uint8_t* path, struct buffer* pathW)
{
	if (NULL == path ||
		NULL == pathW)
	{
		return 0;
	}

	const ptrdiff_t length = common_count_bytes_until(path, 0);
	return file_system_path_in_range_to_pathW(path, path + length, pathW);
}
#else
#define FILE_STAT(PATH)									\
	struct stat file_status;							\
	file_status.st_size = 0;							\
	\
	if (stat((const char*)(PATH), &file_status) ||		\
		!(S_IFDIR != (file_status.st_mode & S_IFDIR)))	\
	{													\
		return 0;										\
	}

#define DIRECTORY_STAT(PATH)								\
	struct stat directory_status;							\
	directory_status.st_size = 0;							\
	\
	if (stat((const char*)(PATH), &directory_status) ||		\
		S_IFDIR != (directory_status.st_mode & S_IFDIR))	\
	{														\
		return 0;											\
	}

uint8_t directory_delete_(const char* path)
{
	return 0 == rmdir(path);
}

uint8_t directory_enumerate_file_system_entries_(
	struct buffer* path,
	const uint8_t* wild_card_start,
	const uint8_t* wild_card_finish,
	const uint8_t entry_type, const uint8_t recurse,
	struct buffer* output, uint8_t fail_on_error)
{
	if (NULL == path ||
		range_in_parts_is_null_or_empty(wild_card_start, wild_card_finish) ||
		all_entries < entry_type ||
		(0 != recurse && 1 != recurse) ||
		NULL == output)
	{
		return 0;
	}

	DIR* directory = opendir(buffer_char_data(path, 0));

	if (NULL == directory)
	{
		return fail_on_error ? 0 : FAIL_WITH_OUT_ERROR;
	}

	const ptrdiff_t size = buffer_size(path);
	struct dirent* entry = NULL;
	uint8_t result = 1;

	while (NULL != (entry = readdir(directory)))
	{
		const size_t name_length = common_count_bytes_until((const uint8_t*)entry->d_name, 0);

		if (DT_DIR == entry->d_type)
		{
			if ((1 == name_length && 0 == memcmp(".", entry->d_name, name_length)) ||
				(2 == name_length && 0 == memcmp("..", entry->d_name, name_length)))
			{
				continue;
			}
		}

		if (!buffer_resize(path, size - 1) ||
			!path_combine_in_place(path, size - 2, (const uint8_t*)entry->d_name,
								   (const uint8_t*)(entry->d_name + name_length)) ||
			!buffer_push_back(path, 0))
		{
			closedir(directory);
			return 0;
		}

		if (DT_DIR == entry->d_type)
		{
			if (recurse)
			{
				if (!(result = directory_enumerate_file_system_entries_(
								   path, wild_card_start, wild_card_finish,
								   entry_type, recurse, output, fail_on_error)))
				{
					closedir(directory);
					return 0;
				}
			}

			if (file_entry == entry_type)
			{
				if (!buffer_resize(path, size - 1) ||
					!buffer_push_back(path, 0))
				{
					closedir(directory);
					return 0;
				}

				continue;
			}
		}
		else if (directory_entry == entry_type)
		{
			if (!buffer_resize(path, size - 1) ||
				!buffer_push_back(path, 0))
			{
				closedir(directory);
				return 0;
			}

			continue;
		}

		const uint8_t* path_start = buffer_data(path, 0);
		const uint8_t* path_finish = path_start + buffer_size(path);

		if (!path_glob(path_start, path_finish,
					   wild_card_start, wild_card_finish))
		{
			continue;
		}

		if (!buffer_append_data_from_buffer(output, path) ||
			!buffer_resize(path, size - 1) ||
			!buffer_push_back(path, 0))
		{
			closedir(directory);
			return 0;
		}
	}

	if (0 != closedir(directory))
	{
		return 0;
	}

	return result;
}
#endif
uint8_t directory_create(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	wchar_t* path_start = buffer_wchar_t_data(&pathW, 0);
	/**/
	const wchar_t* start = buffer_wchar_t_data(&pathW, 0);
	const wchar_t* finish = (const wchar_t*)(buffer_data(&pathW, 0) + buffer_size(&pathW));
	/**/
	file_system_set_position_after_pre_root_wchar_t(&start);

	while (finish != (start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\\", 1, 1, 1)))
	{
		wchar_t* pos = path_start + (start - path_start);
		++start;

		if (L':' == *(pos - 1))
		{
			continue;
		}

		(*pos) = L'\0';

		if (!directory_exists_wchar_t(path_start))
		{
			if (!directory_create_wchar_t(path_start))
			{
				buffer_release(&pathW);
				return 0;
			}
		}

		(*pos) = L'\\';
	}

	const uint8_t returned = directory_create_wchar_t(path_start);
	buffer_release(&pathW);
	return returned;
#else
	static const mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	/**/
	const uint8_t* start = path;
	const uint8_t* finish = path + common_count_bytes_until(path, 0);
	/**/
	struct buffer current_directory;
	SET_NULL_TO_BUFFER(current_directory);

	if (!buffer_append(&current_directory, NULL, finish - start) ||
		!buffer_resize(&current_directory, 0))
	{
		buffer_release(&current_directory);
		return 0;
	}

	while (finish != (start = find_any_symbol_like_or_not_like_that(start, finish, &path_posix_delimiter, 1, 1,
							  1)))
	{
		if (start == path)
		{
			++start;
			continue;
		}

		if (!buffer_resize(&current_directory, 0) ||
			!buffer_append(&current_directory, path, start - path) ||
			!buffer_push_back(&current_directory, 0))
		{
			buffer_release(&current_directory);
			return 0;
		}

		++start;

		if (!directory_exists(buffer_data(&current_directory, 0)))
		{
			if (0 != mkdir((const char*)buffer_data(&current_directory, 0), mode))
			{
				buffer_release(&current_directory);
				return 0;
			}
		}
	}

	buffer_release(&current_directory);
	return 0 == mkdir((const char*)path, mode);
#endif
}

uint8_t directory_delete(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

	struct buffer pathW;

	SET_NULL_TO_BUFFER(pathW);

#if defined(_WIN32)
	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const ptrdiff_t size = buffer_size(&pathW);

	if (!buffer_resize(&pathW, size - sizeof(wchar_t)) ||
		!buffer_append_wchar_t(&pathW, L"\\*\0", 3))
	{
		buffer_release(&pathW);
		return 0;
	}

#else

	if (!buffer_append(&pathW, path, common_count_bytes_until(path, 0)) ||
		!buffer_push_back(&pathW, 0))
	{
		buffer_release(&pathW);
		return 0;
	}

#endif
	struct buffer entries;
	SET_NULL_TO_BUFFER(entries);

	for (uint8_t entry = file_entry; ; entry = directory_entry)
	{
		if (!buffer_resize(&entries, 0))
		{
			buffer_release(&entries);
			buffer_release(&pathW);
			return 0;
		}

#if defined(_WIN32)

		if (!directory_enumerate_file_system_entries_wchar_t(&pathW, entry, 1, UTF16LE, &entries, 1))
#else
		if (!directory_enumerate_file_system_entries(&pathW, entry, 1, &entries, 1))
#endif
		{
			buffer_release(&entries);
			buffer_release(&pathW);
			return 0;
		}

#if defined(_WIN32)
		const wchar_t* start = buffer_wchar_t_data(&entries, 0);
		const wchar_t* finish = (const wchar_t*)(buffer_data(&entries, 0) + buffer_size(&entries));
#else
		const uint8_t* start = buffer_data(&entries, 0);
		const uint8_t* finish = start + buffer_size(&entries);
#endif

		if (file_entry == entry)
		{
			while (start != finish)
			{
#if defined(_WIN32)

				if (0 == DeleteFileW(start))
#else
				if (0 != remove((const char*)start))
#endif
				{
					buffer_release(&entries);
					buffer_release(&pathW);
					return 0;
				}

#if defined(_WIN32)
				start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 1, 1);
				start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 0, 1);
#else
				start = find_any_symbol_like_or_not_like_that(start, finish, (const uint8_t*)"\0", 1, 1, 1);
				start = find_any_symbol_like_or_not_like_that(start, finish, (const uint8_t*)"\0", 1, 0, 1);
#endif
			}
		}
		else
		{
			while (start != finish)
			{
#if defined(_WIN32)

				if (!directory_delete_wchar_t(start))
#else
				if (!directory_delete_((const char*)start))
#endif
				{
					buffer_release(&entries);
					buffer_release(&pathW);
					return 0;
				}

#if defined(_WIN32)
				start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 1, 1);
				start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 0, 1);
#else
				start = find_any_symbol_like_or_not_like_that(start, finish, (const uint8_t*)"\0", 1, 1, 1);
				start = find_any_symbol_like_or_not_like_that(start, finish, (const uint8_t*)"\0", 1, 0, 1);
#endif
			}

			break;
		}
	}

	buffer_release(&entries);
#if defined(_WIN32)

	if (!buffer_resize(&pathW, size - sizeof(wchar_t)) ||
		!buffer_push_back_uint16(&pathW, 0))
	{
		buffer_release(&pathW);
		return 0;
	}

	if (!directory_delete_wchar_t(buffer_wchar_t_data(&pathW, 0)))
#else
	if (!directory_delete_(buffer_char_data(&pathW, 0)))
#endif
	{
		buffer_release(&pathW);
		return 0;
	}

	buffer_release(&pathW);
	return 1;
}

uint8_t directory_enumerate_file_system_entries(
	struct buffer* path, const uint8_t entry_type, const uint8_t recurse,
	struct buffer* output, uint8_t fail_on_error)
{
	if (NULL == path ||
		all_entries < entry_type ||
		(0 != recurse && 1 != recurse) ||
		NULL == output)
	{
		return 0;
	}

	struct range file_name;

	file_name.start = buffer_data(path, 0);

	file_name.finish = file_name.start + buffer_size(path);

	if (!path_get_file_name(file_name.start, file_name.finish, &file_name))
	{
		return 0;
	}

	if (file_name.finish == find_any_symbol_like_or_not_like_that(
			file_name.start, file_name.finish, (const uint8_t*)"?*", 2, 1, 1))
	{
		const ptrdiff_t index = file_name.start - buffer_data(path, 0);
		static const uint8_t asterisk = '*';

		if (!buffer_resize(path, buffer_size(path) - 1) ||
			!path_combine_in_place(path, buffer_size(path) < index ? index - 1 : index, &asterisk, &asterisk + 1) ||
			!buffer_push_back(path, 0))
		{
			return 0;
		}
	}

#if defined(_WIN32)
	struct buffer patternW;
	SET_NULL_TO_BUFFER(patternW);

	if (!file_system_path_to_pathW(buffer_data(path, 0), &patternW))
	{
		buffer_release(&patternW);
		return 0;
	}

	const uint8_t returned = directory_enumerate_file_system_entries_wchar_t(
								 &patternW, entry_type, recurse, UTF8, output, fail_on_error);
	/**/
	buffer_release(&patternW);
	return returned;
#else
	file_name.start = buffer_data(path, 0);
	file_name.finish = file_name.start + buffer_size(path);

	if (!path_get_file_name(file_name.start, file_name.finish, &file_name))
	{
		return 0;
	}

	struct buffer wild_card;

	SET_NULL_TO_BUFFER(wild_card);

	if (!buffer_append_data_from_range(&wild_card, &file_name))
	{
		buffer_release(&wild_card);
		return 0;
	}

	file_name.start = buffer_data(path, 0);
	file_name.finish = file_name.start + buffer_size(path);

	if (!path_get_directory_name(file_name.start, file_name.finish, &file_name) ||
		!buffer_resize(path, range_size(&file_name)) ||
		!buffer_push_back(path, 0))
	{
		buffer_release(&wild_card);
		return 0;
	}

	const uint8_t* wild_card_start = buffer_data(&wild_card, 0);
	const uint8_t* wild_card_finish = wild_card_start + buffer_size(&wild_card);
	/**/
	const uint8_t returned = directory_enumerate_file_system_entries_(
								 path, wild_card_start, wild_card_finish,
								 entry_type, recurse, output, fail_on_error);
	buffer_release(&wild_card);
	return returned;
#endif
}

uint8_t directory_exists(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = directory_exists_wchar_t(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	return returned;
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

int64_t file_system_win32_time_to_datetime(int64_t input)
{
	if (input < 1)
	{
		return 0;
	}

	input = input / 10000000;
	input -= 11644473600;/*1970 - 1601*/
	return input;
}

int64_t file_system_datetime_to_win32_time(int64_t input)
{
	if (input < 1)
	{
		return 0;
	}

	input += 11644473600;
	input *= 10000000;
	return input;
}

#define INT64_T_TO_FILETIME(I, O)						\
	(I) = file_system_datetime_to_win32_time(I);		\
	(O).dwHighDateTime = (I) >> 32;						\
	(O).dwLowDateTime = (int32_t)(I);/* && INT32_MAX;*/

long file_system_get_bias()
{
	static uint8_t loaded = 0;
	static long result = 0;

	if (!loaded)
	{
#if defined(_WIN32)
#if defined(_MSC_VER)
		DYNAMIC_TIME_ZONE_INFORMATION tz;

		if (TIME_ZONE_ID_INVALID == GetDynamicTimeZoneInformation(&tz))
#else
		TIME_ZONE_INFORMATION tz;

		if (TIME_ZONE_ID_INVALID == GetTimeZoneInformation(&tz))
#endif
		{
			return 0;
		}

		loaded = 1;
		result = tz.Bias + tz.DaylightBias;
#else
		loaded = 1;
		result = datetime_get_bias();
#endif
	}

	return result;
}

int64_t directory_get_creation_time(const uint8_t* path)
{
	return directory_get_creation_time_utc(path) - (int64_t)60 * file_system_get_bias();
}

int64_t directory_get_creation_time_utc(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	FIND_FILE_OBJECT_DATA_FROM_BUFFER(path);
	const uint8_t is_directory = (0 != (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

	if (!is_directory)
	{
		return 0;
	}

	return file_system_win32_time_to_datetime(
			   ((int64_t)file_data.ftCreationTime.dwHighDateTime << 32) + file_data.ftCreationTime.dwLowDateTime);
#else
	DIRECTORY_STAT(path);
	int64_t result = directory_status.st_atime;
	result = directory_status.st_mtime < result ? directory_status.st_mtime : result;
	result = directory_status.st_ctime < result ? directory_status.st_ctime : result;
	return result;
#endif
}

uint8_t directory_get_current_directory(const void* project, const void** the_property,
										struct buffer* output, uint8_t verbose)
{
	if (!project_get_base_directory(project, the_property, verbose))
	{
		if (NULL != the_property)
		{
			(*the_property) = NULL;
		}

		return path_get_directory_for_current_process(output);
	}

	return property_get_by_pointer(*the_property, output);
}

uint8_t directory_get_directory_root(const uint8_t* path, struct range* root)
{
	return NULL != path && path_get_path_root(path, path + common_count_bytes_until(path, 0), root);
}

int64_t directory_get_last_access_time(const uint8_t* path)
{
	return directory_get_last_access_time_utc(path) - (int64_t)60 * file_system_get_bias();
}

int64_t directory_get_last_access_time_utc(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	FIND_FILE_OBJECT_DATA_FROM_BUFFER(path);
	const uint8_t is_directory = (0 != (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

	if (!is_directory)
	{
		return 0;
	}

	return file_system_win32_time_to_datetime(
			   ((int64_t)file_data.ftLastAccessTime.dwHighDateTime << 32) + file_data.ftLastAccessTime.dwLowDateTime);
#else
	DIRECTORY_STAT(path);
	return directory_status.st_atime;
#endif
}

int64_t directory_get_last_write_time(const uint8_t* path)
{
	return directory_get_last_write_time_utc(path) - (int64_t)60 * file_system_get_bias();
}

int64_t directory_get_last_write_time_utc(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	FIND_FILE_OBJECT_DATA_FROM_BUFFER(path);
	const uint8_t is_directory = (0 != (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

	if (!is_directory)
	{
		return 0;
	}

	return file_system_win32_time_to_datetime(
			   ((int64_t)file_data.ftLastWriteTime.dwHighDateTime << 32) + file_data.ftLastWriteTime.dwLowDateTime);
#else
	DIRECTORY_STAT(path);
	return directory_status.st_mtime;
#endif
}

uint8_t directory_get_logical_drives(struct buffer* drives)
{
#if defined(_WIN32)

	if (NULL == drives)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(drives);
	const DWORD logical_drives = GetLogicalDrives();

	if (0 == logical_drives ||
		!buffer_append(drives, NULL, 4 * 26 + 1) ||
		!buffer_resize(drives, size))
	{
		return 0;
	}

	for (uint32_t drive = 'A', i = 1, count = 'Z' + 1; drive < count; ++drive, i = i << 1)
	{
		if (i & logical_drives)
		{
			if (!buffer_push_back(drives, (uint8_t)drive) ||
				!buffer_push_back(drives, ':') ||
				!buffer_push_back(drives, '\\') ||
				!buffer_push_back(drives, '\0'))
			{
				return 0;
			}
		}
	}

	return buffer_push_back(drives, '\0');
#else
	return buffer_push_back(drives, PATH_DELIMITER) && buffer_push_back_uint16(drives, 0);
#endif
}

uint8_t directory_get_parent_directory(const uint8_t* path_start, const uint8_t* path_finish,
									   struct range* parent)
{
	return path_get_directory_name(path_start, path_finish, parent);
}
#if !defined(_WIN32)
uint8_t file_system_move_entry(const uint8_t* current_path, const uint8_t* new_path)
{
	return 0 == rename((const char*)current_path, (const char*)new_path);
}
#endif
uint8_t directory_move(const uint8_t* current_path, const uint8_t* new_path)
{
	if (NULL == current_path ||
		NULL == new_path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(current_path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	if (!directory_exists_wchar_t(buffer_wchar_t_data(&pathW, 0)))
	{
		buffer_release(&pathW);
		return 0;
	}

	const ptrdiff_t size = buffer_size(&pathW);

	if (!file_system_path_to_pathW(new_path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	if (file_exists_wchar_t((const wchar_t*)buffer_data(&pathW, size)))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = (0 != MoveFileExW(buffer_wchar_t_data(&pathW, 0),
							  (const wchar_t*)buffer_data(&pathW, size),
							  MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH));
	buffer_release(&pathW);
	return returned;
#else

	if (!directory_exists(current_path) ||
		file_exists(new_path))
	{
		return 0;
	}

	return file_system_move_entry(current_path, new_path);
#endif
}

enum file_function
{
	file_exists_,
	file_get_checksum_,
	file_get_creation_time_,
	file_get_creation_time_utc_,
	file_get_last_access_time_,
	file_get_last_access_time_utc_,
	file_get_last_write_time_,
	file_get_last_write_time_utc_,
	file_get_length_,
	file_up_to_date_,
	file_replace_,
	UNKNOWN_FILE_FUNCTION,
	file_set_creation_time_,
	file_set_creation_time_utc_,
	file_set_last_access_time_,
	file_set_last_access_time_utc_,
	file_set_last_write_time_,
	file_set_last_write_time_utc_
};

#if defined(_WIN32)
uint8_t file_system_set_time_utc_wchar_t(const wchar_t* path, const FILETIME* creationTime,
		const FILETIME* lastAccessTime, const FILETIME* lastWriteTime)
{
	if (NULL == path)
	{
		return 0;
	}

	static FILETIME localLastAccessTime;

	if (NULL == lastAccessTime)
	{
		localLastAccessTime.dwLowDateTime = localLastAccessTime.dwHighDateTime = UINT32_MAX;
		lastAccessTime = &localLastAccessTime;
	}

#if defined(_WIN32_WINNT_WIN8) && defined(_WIN32_WINNT) && (_WIN32_WINNT_WIN8 <= _WIN32_WINNT)
	const HANDLE file_handle = CreateFile2(path, FILE_WRITE_ATTRIBUTES, 0, OPEN_EXISTING, NULL);
#else
	const HANDLE file_handle = CreateFileW(path, FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING,
										   FILE_ATTRIBUTE_NORMAL, NULL);
#endif

	if (INVALID_HANDLE_VALUE == file_handle)
	{
		return 0;
	}

	if (0 == SetFileTime(file_handle, creationTime, lastAccessTime, lastWriteTime))
	{
		CloseHandle(file_handle);
		return 0;
	}

	return 0 != CloseHandle(file_handle);
}

uint8_t file_system_set_time_utc(const uint8_t* path, int64_t time, uint8_t function)
{
	if (NULL == path)
	{
		return 0;
	}

	struct buffer pathW;

	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	FILETIME win32_time;
	INT64_T_TO_FILETIME(time, win32_time);

	if (0 == time)
	{
		buffer_release(&pathW);
		return 0;
	}

	const wchar_t* ptr = buffer_wchar_t_data(&pathW, 0);
	uint8_t returned = 0;

	if (file_set_creation_time_utc_ == function ||
		file_set_creation_time_ == function)
	{
		returned = file_system_set_time_utc_wchar_t(ptr, &win32_time, NULL, NULL);
	}
	else if (file_set_last_access_time_utc_ == function ||
			 file_set_last_access_time_ == function)
	{
		returned = file_system_set_time_utc_wchar_t(ptr, NULL, &win32_time, NULL);
	}
	else if (file_set_last_write_time_utc_ == function ||
			 file_set_last_write_time_ == function)
	{
		returned = file_system_set_time_utc_wchar_t(ptr, NULL, NULL, &win32_time);
	}

	buffer_release(&pathW);
	return returned;
}

uint8_t file_system_set_time(const uint8_t* path, int64_t time, uint8_t function)
{
	return file_system_set_time_utc(path, time + (int64_t)60 * file_system_get_bias(), function);
}
#endif
uint8_t directory_set_current_directory(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = 0 != SetCurrentDirectoryW(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	return returned;
#else
	return -1 != chdir((const char*)path);
#endif
}
#if defined(_WIN32)
int32_t _file_fileno(void* stream)
{
	return _fileno(stream);
}
#endif

uint8_t file_append(const uint8_t* path, const struct range* data, uint16_t encoding)
{
	if (NULL == path ||
		range_is_null_or_empty(data))
	{
		return 0;
	}

	void* file = NULL;

	if (!file_open(path, (const uint8_t*)"ab", &file))
	{
		return 0;
	}

	if (!file_write_with_encoding(data, encoding, file))
	{
		file_close(file);
		return 0;
	}

	return file_close(file);
}

uint8_t file_close(void* stream)
{
	return NULL != stream &&
		   0 == fclose(stream);
}
#if defined(_WIN32)
uint8_t file_copy_wchar_t(const wchar_t* current_path, const wchar_t* new_path)
{
#if defined(_WIN32_WINNT_WIN8) && defined(_WIN32_WINNT) && (_WIN32_WINNT_WIN8 <= _WIN32_WINNT)
	return SUCCEEDED(CopyFile2(current_path, new_path, NULL));
#else
	return 0 != CopyFileExW(current_path, new_path, NULL, NULL, FALSE, COPY_FILE_FAIL_IF_EXISTS);
#endif
}
#endif
uint8_t file_copy(const uint8_t* exists_file, const uint8_t* new_file)
{
	if (NULL == exists_file ||
		NULL == new_file)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(exists_file, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	if (!file_exists_wchar_t((const wchar_t*)buffer_data(&pathW, 0)))
	{
		buffer_release(&pathW);
		return 0;
	}

	const ptrdiff_t size = buffer_size(&pathW);

	if (!file_system_path_to_pathW(new_file, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	if (directory_exists_wchar_t(buffer_wchar_t_data(&pathW, size)))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned =
		file_copy_wchar_t(buffer_wchar_t_data(&pathW, 0),
						  (const wchar_t*)buffer_data(&pathW, size));
	buffer_release(&pathW);
	return returned;
#else

	if (!file_exists(exists_file))
	{
		return 0;
	}

	if (directory_exists(new_file))
	{
		return 0;
	}

	void* input = NULL;

	if (!file_open(exists_file, (const uint8_t*)"rb", &input))
	{
		return 0;
	}

	struct buffer content;

	SET_NULL_TO_BUFFER(content);

	if (!buffer_resize(&content, 4096))
	{
		buffer_release(&content);
		file_close(input);
		return 0;
	}

	void* output = NULL;

	if (!file_open(new_file, (const uint8_t*)"wb", &output))
	{
		buffer_release(&content);
		file_close(input);
		return 0;
	}

	size_t size = 0;

	while (0 < (size = file_read(buffer_data(&content, 0), sizeof(uint8_t), 4096, input)))
	{
		if (size != file_write(buffer_data(&content, 0), sizeof(uint8_t), size, output))
		{
			file_close(input);
			file_close(output);
			buffer_release(&content);
			return 0;
		}
	}

	buffer_release(&content);
	size = file_close(input);
	size = size && file_close(output);
	return 0 < size && file_get_length(exists_file) == file_get_length(new_file);
#endif
}

uint8_t file_create(const uint8_t* path)
{
	void* stream = NULL;

	if (!file_open(path, (const uint8_t*)"wb", &stream))
	{
		return 0;
	}

	return file_close(stream);
}

uint8_t file_delete(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = (0 != DeleteFileW(buffer_wchar_t_data(&pathW, 0)));
	buffer_release(&pathW);
	return returned;
#else
	return 0 == remove((const char*)path);
#endif
}
#if defined(_WIN32)
uint8_t file_exists_wchar_t(const wchar_t* path)
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
#endif
uint8_t file_exists(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = file_exists_wchar_t(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	return returned;
#else
	struct stat file_status;
	file_status.st_mode = 0;
	return -1 != stat((const char*)path, &file_status) && (S_IFDIR != (file_status.st_mode & S_IFDIR));
#endif
}

uint8_t file_flush(void* stream)
{
	return NULL != stream && 0 == fflush(stream);
}

uint8_t file_seek(void* stream, long offset, int32_t origin)
{
	return NULL != stream && 0 == fseek(stream, offset, origin);
}

long file_tell(void* stream)
{
	return NULL != stream ? ftell(stream) : 0;
}

size_t file_write(const void* content, const size_t size_of_content_element,
				  const size_t count_of_elements, void* stream)
{
	return fwrite(content, size_of_content_element, count_of_elements, stream);
}
#if defined(_WIN32)
uint8_t file_get_attributes_wchar_t(const wchar_t* path, unsigned long* attributes)
{
	if (NULL == path ||
		NULL == attributes)
	{
		return 0;
	}

	(*attributes) = GetFileAttributesW(path);
	return INVALID_FILE_ATTRIBUTES != (*attributes);
}
#endif
uint8_t file_get_attributes(const uint8_t* path, unsigned long* attributes)
{
	if (NULL == path ||
		NULL == attributes)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = file_get_attributes_wchar_t(buffer_wchar_t_data(&pathW, 0), attributes);
	buffer_release(&pathW);
	return returned;
#else
	(*attributes) = 0;
	return 0;
#endif
}

int64_t file_get_creation_time(const uint8_t* path)
{
	return file_get_creation_time_utc(path) - (int64_t)60 * file_system_get_bias();
}

int64_t file_get_creation_time_utc(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	FIND_FILE_OBJECT_DATA_FROM_BUFFER(path);
	const uint8_t is_file = (0 == (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

	if (!is_file)
	{
		return 0;
	}

	return file_system_win32_time_to_datetime(
			   ((int64_t)file_data.ftCreationTime.dwHighDateTime << 32) + file_data.ftCreationTime.dwLowDateTime);
#else
	FILE_STAT(path);
	int64_t result = file_status.st_atime;
	result = file_status.st_mtime < result ? file_status.st_mtime : result;
	result = file_status.st_ctime < result ? file_status.st_ctime : result;
	return result;
#endif
}

int64_t file_get_last_access_time(const uint8_t* path)
{
	return file_get_last_access_time_utc(path) - (int64_t)60 * file_system_get_bias();
}

int64_t file_get_last_access_time_utc(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	FIND_FILE_OBJECT_DATA_FROM_BUFFER(path);
	const uint8_t is_file = (0 == (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

	if (!is_file)
	{
		return 0;
	}

	return file_system_win32_time_to_datetime(
			   ((int64_t)file_data.ftLastAccessTime.dwHighDateTime << 32) + file_data.ftLastAccessTime.dwLowDateTime);
#else
	FILE_STAT(path);
	return file_status.st_atime;
#endif
}

int64_t file_get_last_write_time(const uint8_t* path)
{
	return file_get_last_write_time_utc(path) - (int64_t)60 * file_system_get_bias();
}

int64_t file_get_last_write_time_utc(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	FIND_FILE_OBJECT_DATA_FROM_BUFFER(path);
	const uint8_t is_file = (0 == (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

	if (!is_file)
	{
		return 0;
	}

	return file_system_win32_time_to_datetime(
			   ((int64_t)file_data.ftLastWriteTime.dwHighDateTime << 32) + file_data.ftLastWriteTime.dwLowDateTime);
#else
	FILE_STAT(path);
	return file_status.st_mtime;
#endif
}
#if defined(_WIN32)
uint8_t file_open_wchar_t(const wchar_t* path, const wchar_t* mode, void** output)
{
	if (NULL == path ||
		NULL == mode ||
		NULL == output)
	{
		return 0;
	}

#if __STDC_LIB_EXT1__
	return (0 == _wfopen_s((FILE**)output, path, mode) && NULL != (*output));
#else
	(*output) = (void*)_wfopen(path, mode);
	return (NULL != (*output));
#endif
}
#endif
uint8_t file_move(const uint8_t* current_path, const uint8_t* new_path)
{
	if (NULL == current_path ||
		NULL == new_path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(current_path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	if (!file_exists_wchar_t(buffer_wchar_t_data(&pathW, 0)))
	{
		buffer_release(&pathW);
		return 0;
	}

	const ptrdiff_t size = buffer_size(&pathW);

	if (!file_system_path_to_pathW(new_path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	if (directory_exists_wchar_t((const wchar_t*)buffer_data(&pathW, size)))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = (0 != MoveFileExW(buffer_wchar_t_data(&pathW, 0),
							  (const wchar_t*)buffer_data(&pathW, size),
							  MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH));
	buffer_release(&pathW);
	return returned;
#else

	if (!file_exists(current_path) ||
		directory_exists(new_path))
	{
		return 0;
	}

	return file_system_move_entry(current_path, new_path);
#endif
}

uint8_t file_open(const uint8_t* path, const uint8_t* mode, void** output)
{
	if (NULL == path ||
		NULL == mode ||
		NULL == output)
	{
		return 0;
	}

#if defined(_WIN32)
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
#if __STDC_LIB_EXT1__
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

#if __STDC_LIB_EXT1__ && defined(_MSC_VER)
	return fread_s(content, count_of_elements, size_of_content_element, count_of_elements, stream);
#else
	return fread(content, size_of_content_element, count_of_elements, stream);
#endif
}

uint8_t file_read_with_several_steps(void* stream, struct buffer* content)
{
	if (NULL == stream ||
		NULL == content)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(content);

	if (!buffer_append(content, NULL, 4096))
	{
		return 0;
	}

	size_t readed = 0;
	uint8_t* ptr = buffer_data(content, size);

	while (0 < (readed = file_read(ptr, sizeof(uint8_t), 4096, stream)))
	{
		size += (ptrdiff_t)readed;

		if (!buffer_resize(content, size) ||
			(4096 == readed && !buffer_append(content, NULL, 4096)))
		{
			return 0;
		}

		ptr = buffer_data(content, size);
	}

	return buffer_resize(content, size);
}

uint8_t file_read_all(const uint8_t* path, struct buffer* output)
{
	if (NULL == path ||
		NULL == output)
	{
		return 0;
	}

	void* stream = NULL;

	if (!file_open(path, (const uint8_t*)"rb", &stream))
	{
		return 0;
	}

	if (!file_read_with_several_steps(stream, output))
	{
		file_close(stream);
		return 0;
	}

	return file_close(stream);
}

uint8_t file_read_lines(const uint8_t* path, struct buffer* output)
{
	if (NULL == path ||
		NULL == output)
	{
		return 0;
	}

	void* stream = NULL;

	if (!file_open(path, (const uint8_t*)"rb", &stream) ||
		!buffer_resize(output, 4096))
	{
		return 0;
	}

	ptrdiff_t size = 0;
	ptrdiff_t readed = 0;
	uint16_t count_of_lines = 1;
	static const uint8_t n = '\n';
	uint8_t* ptr = buffer_data(output, 0);

	while (0 < (readed = (ptrdiff_t)file_read(ptr, sizeof(uint8_t), 4096, stream)))
	{
		size += (ptrdiff_t)readed;
		path = ptr + readed;

		while (-1 != (readed = string_index_of(ptr, path, &n, &n + 1)))
		{
			ptr += readed + 1;
			++count_of_lines;
		}

		ptr = buffer_data(output, 0);
	}

	if (!size)
	{
		return file_close(stream) && buffer_resize(output, 0);
	}

	count_of_lines = count_of_lines ? count_of_lines * sizeof(struct range) : sizeof(struct range);
	size += count_of_lines;

	if (!buffer_resize(output, size))
	{
		file_close(stream);
		return 0;
	}

	if (!file_seek(stream, 0, SEEK_SET))
	{
		file_close(stream);
		return 0;
	}

	if (!buffer_resize(output, count_of_lines))
	{
		file_close(stream);
		return 0;
	}

	if (!file_read_with_several_steps(stream, output))
	{
		file_close(stream);
		return 0;
	}

	if (!file_close(stream))
	{
		return 0;
	}

	ptr = buffer_data(output, count_of_lines);
	path = buffer_data(output, 0) + buffer_size(output);

	if (!buffer_resize(output, count_of_lines))
	{
		return 0;
	}

	count_of_lines = 0;
	struct range* range_of_line = NULL;

	while (-1 != (size = string_index_of(ptr, path, &n, &n + 1)) &&
		   NULL != (range_of_line = buffer_range_data(output, count_of_lines++)))
	{
		range_of_line->start = ptr;
		ptr += size;
		range_of_line->finish = ptr;
		++ptr;
	}

	if (ptr < path)
	{
		range_of_line = buffer_range_data(output, count_of_lines++);
		range_of_line->start = ptr;
		range_of_line->finish = path;
	}
	else
	{
		if (!buffer_resize(output, buffer_size(output) - sizeof(struct range)))
		{
			return 0;
		}
	}

	if (!count_of_lines)
	{
		if (NULL == range_of_line)
		{
			range_of_line = buffer_range_data(output, count_of_lines);
		}

		range_of_line->start = buffer_data(output, 0) + buffer_size(output);
		range_of_line->finish = path;
	}

	return 1;
}

uint64_t file_get_length(const uint8_t* path)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	FIND_FILE_OBJECT_DATA_FROM_BUFFER(path);
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
	FILE_STAT(path);
	return file_status.st_size;
#endif
}

uint8_t file_up_to_date(const uint8_t* src_file, const uint8_t* target_file)
{
	return file_get_last_write_time_utc(src_file) <= file_get_last_write_time_utc(target_file);
}
#if defined(_WIN32)
uint8_t file_set_attributes_wchar_t(const wchar_t* path,
									uint8_t archive, uint8_t hidden, uint8_t normal, uint8_t readonly, uint8_t system_attribute)
{
	if (NULL == path)
	{
		return 0;
	}

	unsigned long attributes = 0;

	if (!file_get_attributes_wchar_t(path, &attributes))
	{
		return 0;
	}

	if (archive)
	{
		attributes |= FILE_ATTRIBUTE_ARCHIVE;
	}
	else
	{
		attributes &= ~FILE_ATTRIBUTE_ARCHIVE;
	}

	if (hidden)
	{
		attributes |= FILE_ATTRIBUTE_HIDDEN;
	}
	else
	{
		attributes &= ~FILE_ATTRIBUTE_HIDDEN;
	}

	if (normal)
	{
		attributes |= FILE_ATTRIBUTE_NORMAL;
	}
	else
	{
		attributes &= ~FILE_ATTRIBUTE_NORMAL;
	}

	if (readonly)
	{
		attributes |= FILE_ATTRIBUTE_READONLY;
	}
	else
	{
		attributes &= ~FILE_ATTRIBUTE_READONLY;
	}

	if (system_attribute)
	{
		attributes |= FILE_ATTRIBUTE_SYSTEM;
	}
	else
	{
		attributes &= ~FILE_ATTRIBUTE_SYSTEM;
	}

	return 0 != SetFileAttributesW(path, attributes);
}
#endif
uint8_t file_set_attributes(const uint8_t* path,
							uint8_t archive, uint8_t hidden, uint8_t normal, uint8_t readonly, uint8_t system_attribute)
{
	if (NULL == path)
	{
		return 0;
	}

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = file_set_attributes_wchar_t(
								 buffer_wchar_t_data(&pathW, 0), archive, hidden, normal, readonly, system_attribute);
	buffer_release(&pathW);
	return returned;
#else
	(void)archive;
	(void)hidden;
	(void)normal;
	(void)readonly;
	(void)system_attribute;
	return 1;
#endif
}
#if defined(_WIN32)
uint8_t file_set_creation_time(const uint8_t* path, int64_t time)
{
	return file_system_set_time(path, time, file_set_creation_time_);
}

uint8_t file_set_creation_time_utc(const uint8_t* path, int64_t time)
{
	return file_system_set_time_utc(path, time, file_set_creation_time_utc_);
}

uint8_t file_set_last_access_time(const uint8_t* path, int64_t time)
{
	return file_system_set_time(path, time, file_set_last_access_time_);
}

uint8_t file_set_last_access_time_utc(const uint8_t* path, int64_t time)
{
	return file_system_set_time_utc(path, time, file_set_last_access_time_utc_);
}

uint8_t file_set_last_write_time(const uint8_t* path, int64_t time)
{
	return file_system_set_time(path, time, file_set_last_write_time_);
}

uint8_t file_set_last_write_time_utc(const uint8_t* path, int64_t time)
{
	return file_system_set_time_utc(path, time, file_set_last_write_time_utc_);
}
#else
uint8_t file_set_time_utc(const uint8_t* path, int64_t time, uint8_t function)
{
	if (NULL == path)
	{
		return 0;
	}

	struct utimbuf times;

	if (file_set_last_access_time_ == function ||
		file_set_last_access_time_utc_ == function)
	{
		times.actime = time;
		times.modtime = file_get_last_write_time_utc(path);
	}
	else if (file_set_last_write_time_ == function ||
			 file_set_last_write_time_utc_ == function)
	{
		times.actime = file_get_last_access_time_utc(path);
		times.modtime = time;
	}
	else if (file_set_creation_time_ == function ||
			 file_set_creation_time_utc_ == function)
	{
		return 1;
	}
	else
	{
		return 0;
	}

	return 0 == utime((const char*)path, &times);
}

uint8_t file_set_time(const uint8_t* path, int64_t time, uint8_t function)
{
	return file_set_time_utc(path, time + (int64_t)60 * file_system_get_bias(), function);
}

uint8_t file_set_creation_time(const uint8_t* path, int64_t time)
{
	return file_set_time(path, time, file_set_creation_time_);
}

uint8_t file_set_creation_time_utc(const uint8_t* path, int64_t time)
{
	return file_set_time_utc(path, time, file_set_creation_time_utc_);
}

uint8_t file_set_last_access_time(const uint8_t* path, int64_t time)
{
	return file_set_time(path, time, file_set_last_access_time_);
}

uint8_t file_set_last_access_time_utc(const uint8_t* path, int64_t time)
{
	return file_set_time_utc(path, time, file_set_last_access_time_utc_);
}

uint8_t file_set_last_write_time(const uint8_t* path, int64_t time)
{
	return file_set_time(path, time, file_set_last_write_time_);
}

uint8_t file_set_last_write_time_utc(const uint8_t* path, int64_t time)
{
	return file_set_time_utc(path, time, file_set_last_write_time_utc_);
}
#endif

uint8_t file_write_with_several_steps(const struct buffer* content, void* stream)
{
	ptrdiff_t size = buffer_size(content);

	if (0 < size)
	{
		size_t written = 0;
		uint8_t* ptr = buffer_data(content, 0);

		while (0 < (written = file_write(ptr, sizeof(uint8_t), MIN(4096, size), stream)))
		{
			size -= (ptrdiff_t)written;
			ptr += written;
		}
	}

	return 0 == size;
}

uint8_t file_write_all(const uint8_t* path, const struct buffer* content)
{
	if (NULL == path ||
		NULL == content)
	{
		return 0;
	}

	void* stream = NULL;

	if (!file_open(path, (const uint8_t*)"wb", &stream))
	{
		return 0;
	}

	if (!file_write_with_several_steps(content, stream))
	{
		file_close(stream);
		return 0;
	}

	return file_close(stream);
}

uint8_t file_write_with_encoding(const struct range* data, uint16_t encoding, void* stream)
{
	if (UTF8 == encoding || Default == encoding)
	{
		const ptrdiff_t size = range_size(data);

		if (size != (ptrdiff_t)file_write(data->start, sizeof(uint8_t), size, stream))
		{
			return 0;
		}
	}
	else
	{
		struct buffer output;
		SET_NULL_TO_BUFFER(output);

		switch (encoding)
		{
			case ASCII:
				encoding = text_encoding_UTF_to_ASCII(data->start, data->finish, UTF8, &output);
				break;

			case BigEndianUnicode:
			case UTF16BE:
				encoding = text_encoding_UTF8_to_UTF16BE(data->start, data->finish, &output);
				break;

			case Unicode:
			case UTF16LE:
				encoding = text_encoding_UTF8_to_UTF16LE(data->start, data->finish, &output);
				break;

			case UTF32BE:
				encoding = text_encoding_UTF8_to_UTF32BE(data->start, data->finish, &output);
				break;

			case UTF32:
			case UTF32LE:
				encoding = text_encoding_decode_UTF8(data->start, data->finish, &output);
				break;

			case Windows_874:
			case Windows_1250:
			case Windows_1251:
			case Windows_1252:
			case Windows_1253:
			case Windows_1254:
			case Windows_1255:
			case Windows_1256:
			case Windows_1257:
			case Windows_1258:
				encoding = text_encoding_UTF8_to_code_page(data->start, data->finish, encoding, &output);
				break;

			default:
				encoding = 0;
				break;
		}

		if (!encoding)
		{
			buffer_release(&output);
			return 0;
		}
		else
		{
			if (!file_write_with_several_steps(&output, stream))
			{
				buffer_release(&output);
				return 0;
			}
		}

		buffer_release(&output);
	}

	return 1;
}

uint8_t file_replace_with_same_length(
	uint8_t* content, ptrdiff_t size, void* stream,
	const uint8_t* to_be_replaced, const uint8_t* by_replacement, ptrdiff_t length)
{
	ptrdiff_t readed = 0;
	const uint8_t* to_be_replaced_finish = to_be_replaced + length;

	while (0 < (readed = file_read(content, sizeof(uint8_t), size, stream)))
	{
		long index;
		const uint8_t* sub_content = content;

		while (-1 != (index = (long)string_index_of(sub_content, content + readed, to_be_replaced,
							  to_be_replaced_finish)))
		{
			sub_content += index;
			index = (long)((sub_content - content) - readed);

			if (!file_seek(stream, index, SEEK_CUR))
			{
				return 0;
			}

			if (length != (ptrdiff_t)fwrite(by_replacement, sizeof(uint8_t), length, stream))
			{
				return 0;
			}

			index = (long)(readed - (sub_content - content) - length);

			if (!file_seek(stream, index, SEEK_CUR))
			{
				return 0;
			}

			sub_content += length;
		}

		if (readed < size)
		{
			break;
		}

		if (length < size)
		{
			index = (long)(-length);

			if (!file_seek(stream, index, SEEK_CUR))
			{
				return 0;
			}
		}
	}

	return 1;
}

uint8_t file_replace(const uint8_t* path,
					 const uint8_t* to_be_replaced_start, const uint8_t* to_be_replaced_finish,
					 const uint8_t* by_replacement_start, const uint8_t* by_replacement_finish)
{
	if (NULL == to_be_replaced_start || NULL == to_be_replaced_finish ||
		to_be_replaced_finish <= to_be_replaced_start)
	{
		return 0;
	}

	void* stream = NULL;

	if (!file_open(path, (const uint8_t*)"rb+", &stream))
	{
		return 0;
	}

	if (string_equal(to_be_replaced_start, to_be_replaced_finish, by_replacement_start, by_replacement_finish))
	{
		return file_close(stream);
	}

	const ptrdiff_t to_be_replaced_length = to_be_replaced_finish - to_be_replaced_start;
	const ptrdiff_t by_replacement_length = (NULL == by_replacement_start || NULL == by_replacement_finish ||
											by_replacement_finish < by_replacement_start) ? -1 : (by_replacement_finish - by_replacement_start);
	const ptrdiff_t size = MAX(by_replacement_length, MAX(to_be_replaced_length, 4096));
	/**/
	struct buffer input;
	SET_NULL_TO_BUFFER(input);

	if (!buffer_resize(&input, size))
	{
		buffer_release(&input);
		fclose(stream);
		return 0;
	}

	uint8_t* content = buffer_data(&input, 0);

	if (to_be_replaced_length == by_replacement_length)
	{
		if (!file_replace_with_same_length(content, size, stream,
										   to_be_replaced_start, by_replacement_start, to_be_replaced_length))
		{
			buffer_release(&input);
			file_close(stream);
			return 0;
		}
	}
	else
	{
		/*TODO:*/
		if (!buffer_resize(&input, 0) ||
			!file_read_with_several_steps(stream, &input))
		{
			buffer_release(&input);
			file_close(stream);
			return 0;
		}

		if (!file_close(stream))
		{
			buffer_release(&input);
			return 0;
		}

		struct buffer output;

		SET_NULL_TO_BUFFER(output);

		struct range input_in_range;

		BUFFER_TO_RANGE(input_in_range, &input);

		if (!string_replace(input_in_range.start, input_in_range.finish,
							to_be_replaced_start, to_be_replaced_finish,
							by_replacement_start, by_replacement_finish, &output))
		{
			buffer_release(&input);
			buffer_release(&output);
			return 0;
		}

		buffer_release(&input);
		stream = NULL;

		if (!file_open(path, (const uint8_t*)"wb", &stream))
		{
			buffer_release(&output);
			return 0;
		}

		if (!file_write_with_several_steps(&output, stream))
		{
			buffer_release(&output);
			file_close(stream);
			return 0;
		}

		buffer_release(&output);
		return file_close(stream);
	}

	buffer_release(&input);
	return file_close(stream);
}

uint8_t file_get_full_path(const struct range* partial_path, struct buffer* full_path)
{
	if (range_is_null_or_empty(partial_path) ||
		NULL == full_path)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(full_path);

	if (path_is_path_rooted(partial_path->start, partial_path->finish))
	{
		return buffer_append_data_from_range(full_path, partial_path) &&
			   buffer_push_back(full_path, 0) &&
			   file_exists(buffer_data(full_path, size));
	}

	static const uint8_t* path_variable = (const uint8_t*)"PATH";
	/**/
	struct buffer variable;
	SET_NULL_TO_BUFFER(variable);
	/**/
	struct range start_of_path;

	if (environment_get_variable(path_variable, path_variable + 4, &variable))
	{
		BUFFER_TO_RANGE(start_of_path, &variable);

		if (!buffer_resize(full_path, range_size(&start_of_path) + range_size(partial_path)))
		{
			buffer_release(&variable);
			return 0;
		}

		const uint8_t* ptr = start_of_path.start;

		while (start_of_path.finish != (ptr = find_any_symbol_like_or_not_like_that(ptr, start_of_path.finish,
											  &ENVIRONMENT_DELIMITER, 1, 1, 1)))
		{
			if (!buffer_resize(full_path, size))
			{
				buffer_release(&variable);
				return 0;
			}

			if (!buffer_append(full_path, start_of_path.start, ptr - start_of_path.start))
			{
				buffer_release(&variable);
				return 0;
			}

			if (!path_combine_in_place(full_path, size, partial_path->start, partial_path->finish) ||
				!buffer_push_back(full_path, 0))
			{
				buffer_release(&variable);
				return 0;
			}

			if (file_exists(buffer_data(full_path, size)))
			{
				buffer_release(&variable);
				return 1;
			}

			ptr = find_any_symbol_like_or_not_like_that(ptr + 1, start_of_path.finish, &ENVIRONMENT_DELIMITER, 1, 0, 1);
			start_of_path.start = ptr;
		}
	}

#if !defined(_WIN32)
	static const uint8_t* names[] =
	{
		(const uint8_t*)"PWD", (const uint8_t*)"OLDPWD"
	};
	/**/
	static const uint8_t names_lengths[] =
	{
		3, 6
	};

	for (uint8_t i = 0, count = COUNT_OF(names); i < count; ++i)
	{
		if (!buffer_resize(&variable, 0))
		{
			buffer_release(&variable);
			return 0;
		}

		if (environment_get_variable(names[i], names[i] + names_lengths[i], &variable))
		{
			if (!buffer_resize(full_path, size))
			{
				buffer_release(&variable);
				return 0;
			}

			BUFFER_TO_RANGE(start_of_path, &variable);

			if (!buffer_append_data_from_range(full_path, &start_of_path))
			{
				buffer_release(&variable);
				return 0;
			}

			if (!path_combine_in_place(full_path, size, partial_path->start, partial_path->finish) ||
				!buffer_push_back(full_path, 0))
			{
				buffer_release(&variable);
				return 0;
			}

			if (file_exists(buffer_data(full_path, size)))
			{
				buffer_release(&variable);
				return 1;
			}
		}
	}

#endif
	buffer_release(&variable);
	return 0;
}

static const uint8_t* entry_types_str[] =
{
	(const uint8_t*)"directory",
	(const uint8_t*)"file",
	(const uint8_t*)"all"
};

static const uint8_t* dir_function_str[] =
{
	(const uint8_t*)"enumerate-file-system-entries",
	(const uint8_t*)"exists",
	(const uint8_t*)"get-creation-time",
	(const uint8_t*)"get-creation-time-utc",
	(const uint8_t*)"get-current-directory",
	(const uint8_t*)"get-directory-root",
	(const uint8_t*)"get-last-access-time",
	(const uint8_t*)"get-last-access-time-utc",
	(const uint8_t*)"get-last-write-time",
	(const uint8_t*)"get-last-write-time-utc",
	(const uint8_t*)"get-logical-drives",
	(const uint8_t*)"get-parent-directory"
};

enum dir_function
{
	enumerate_file_system_entries,
	dir_exists,
	dir_get_creation_time,
	dir_get_creation_time_utc,
	get_current_directory,
	get_directory_root,
	dir_get_last_access_time,
	dir_get_last_access_time_utc,
	dir_get_last_write_time,
	dir_get_last_write_time_utc,
	get_logical_drives,
	get_parent_directory,
	UNKNOWN_DIR_FUNCTION
};

uint8_t dir_get_id_of_get_current_directory_function()
{
	return get_current_directory;
}

uint8_t dir_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, dir_function_str, UNKNOWN_DIR_FUNCTION);
}

uint8_t dir_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						  struct buffer* output)
{
	if (UNKNOWN_DIR_FUNCTION <= function ||
		get_current_directory == function ||
		NULL == arguments ||
		3 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range values[3];

	if (arguments_count && !common_get_arguments(arguments, arguments_count, values, 1))
	{
		return 0;
	}

	switch (function)
	{
		case enumerate_file_system_entries:
		{
			if (3 != arguments_count &&
				2 != arguments_count)
			{
				break;
			}

			const uint8_t entry_type = common_string_to_enum(
										   values[1].start, values[1].finish, entry_types_str, UNKNOWN_ENTRY_TYPE);

			if (UNKNOWN_ENTRY_TYPE == entry_type)
			{
				break;
			}

			uint8_t recurse = 0;

			if (3 == arguments_count &&
				!bool_parse(values[2].start, range_size(&values[2]), &recurse))
			{
				break;
			}

			return directory_enumerate_file_system_entries(buffer_buffer_data(arguments, 0), entry_type, recurse, output,
					1);
		}

		case dir_exists:
			return 1 == arguments_count && bool_to_string(directory_exists(values[0].start), output);

		case dir_get_creation_time:
			return 1 == arguments_count && int64_to_string(directory_get_creation_time(values[0].start), output);

		case dir_get_creation_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_creation_time_utc(values[0].start), output);

		case get_directory_root:
			return 1 == arguments_count &&
				   directory_get_directory_root(values[0].start, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case dir_get_last_access_time:
			return 1 == arguments_count && int64_to_string(directory_get_last_access_time(values[0].start), output);

		case dir_get_last_access_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_last_access_time_utc(values[0].start), output);

		case dir_get_last_write_time:
			return 1 == arguments_count && int64_to_string(directory_get_last_write_time(values[0].start), output);

		case dir_get_last_write_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_last_write_time_utc(values[0].start), output);

		case get_logical_drives:
			return 0 == arguments_count && directory_get_logical_drives(output);

		case get_parent_directory:
			return 1 == arguments_count &&
				   directory_get_parent_directory(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case UNKNOWN_DIR_FUNCTION:
		default:
			break;
	}

	return 0;
}

static const uint8_t* file_function_str[] =
{
	(const uint8_t*)"exists",
	(const uint8_t*)"get-checksum",
	(const uint8_t*)"get-creation-time",
	(const uint8_t*)"get-creation-time-utc",
	(const uint8_t*)"get-last-access-time",
	(const uint8_t*)"get-last-access-time-utc",
	(const uint8_t*)"get-last-write-time",
	(const uint8_t*)"get-last-write-time-utc",
	(const uint8_t*)"get-length",
	(const uint8_t*)"up-to-date",
	(const uint8_t*)"replace"
};

uint8_t file_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, file_function_str, UNKNOWN_FILE_FUNCTION);
}

uint8_t file_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						   struct buffer* output)
{
	if (UNKNOWN_FILE_FUNCTION <= function ||
		NULL == arguments ||
		!arguments_count ||
		3 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range values[3];

	if (!common_get_arguments(arguments, arguments_count, values, 1))
	{
		return 0;
	}

	for (uint8_t i = arguments_count, count = COUNT_OF(values); i < count; ++i)
	{
		values[i].start = values[i].finish = NULL;
	}

	switch (function)
	{
		case file_exists_:
			return 1 == arguments_count &&
				   bool_to_string(file_exists(values[0].start), output);

		case file_get_checksum_:
			return (2 == arguments_count || 3 == arguments_count) &&
				   file_get_checksum(values[0].start, &values[1], &values[2], output);

		case file_get_creation_time_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_creation_time(values[0].start), output);

		case file_get_creation_time_utc_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_creation_time_utc(values[0].start), output);

		case file_get_last_access_time_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_last_access_time(values[0].start), output);

		case file_get_last_access_time_utc_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_last_access_time_utc(values[0].start), output);

		case file_get_last_write_time_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_last_write_time(values[0].start), output);

		case file_get_last_write_time_utc_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_last_write_time_utc(values[0].start), output);

		case file_get_length_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_length(values[0].start), output);

		case file_up_to_date_:
			return 2 == arguments_count &&
				   bool_to_string(file_up_to_date(values[0].start, values[1].start), output);

		case file_replace_:
			return 3 == arguments_count &&
				   bool_to_string(
					   file_replace(values[0].start,
									values[1].start, values[1].finish,
									values[2].start, values[2].finish), output);

		case UNKNOWN_FILE_FUNCTION:
		default:
			break;
	}

	return 0;
}

#define ATTRIB_ARCHIVE_POSITION		0
#define ATTRIB_FILE_POSITION		1
#define ATTRIB_HIDDEN_POSITION		2
#define ATTRIB_NORMAL_POSITION		3
#define ATTRIB_READ_ONLY_POSITION	4
#define ATTRIB_SYSTEM_POSITION		5

#define ATTRIB_MAX_POSITION			(ATTRIB_SYSTEM_POSITION + 1)

static const uint8_t* attrib_attributes[] =
{
	(const uint8_t*)"archive",
	(const uint8_t*)"file",
	(const uint8_t*)"hidden",
	(const uint8_t*)"normal",
	(const uint8_t*)"readonly",
	(const uint8_t*)"system"
};

static const uint8_t attrib_attributes_lengths[] = { 7, 4, 6, 6, 8, 6 };

uint8_t attrib_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   attrib_attributes, attrib_attributes_lengths,
			   COUNT_OF(attrib_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t attrib_evaluate_task(struct buffer* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* file_path_in_a_buffer = buffer_buffer_data(task_arguments, ATTRIB_FILE_POSITION);

	if (buffer_size(file_path_in_a_buffer))
	{
		if (!buffer_push_back(file_path_in_a_buffer, 0))
		{
			return 0;
		}
	}
	else
	{
		return 1;
	}

	uint8_t attributes[ATTRIB_MAX_POSITION];

	for (uint8_t i = 0, count = ATTRIB_MAX_POSITION; i < count; ++i)
	{
		if (ATTRIB_FILE_POSITION == i)
		{
			continue;
		}

		const struct buffer* data = buffer_buffer_data(task_arguments, i);

		if (NULL == data)
		{
			return 0;
		}

		attributes[ATTRIB_FILE_POSITION] = (uint8_t)buffer_size(data);

		if (!attributes[ATTRIB_FILE_POSITION])
		{
			attributes[i] = 0;
			continue;
		}

		if (!bool_parse(buffer_data(data, 0), attributes[ATTRIB_FILE_POSITION], &(attributes[i])))
		{
			return 0;
		}
	}

	return file_set_attributes(buffer_data(file_path_in_a_buffer, 0),
							   attributes[ATTRIB_ARCHIVE_POSITION], attributes[ATTRIB_HIDDEN_POSITION],
							   attributes[ATTRIB_NORMAL_POSITION], attributes[ATTRIB_READ_ONLY_POSITION],
							   attributes[ATTRIB_SYSTEM_POSITION]);
}

#define DELETE_DIR_POSITION		0
#define DELETE_FILE_POSITION	1

static const uint8_t* delete_attributes[] =
{
	(const uint8_t*)"dir",
	(const uint8_t*)"file"
};

static const uint8_t delete_attributes_lengths[] = { 3, 4 };

uint8_t delete_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   delete_attributes, delete_attributes_lengths,
			   COUNT_OF(delete_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t delete_evaluate_task(struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* dir_path_in_buffer = buffer_buffer_data(task_arguments, DELETE_DIR_POSITION);

	struct buffer* file_path_in_buffer = buffer_buffer_data(task_arguments, DELETE_FILE_POSITION);

	const uint8_t* dir_ = NULL;

	if (buffer_size(dir_path_in_buffer))
	{
		if (!buffer_push_back(dir_path_in_buffer, 0))
		{
			return 0;
		}

		dir_ = buffer_data(dir_path_in_buffer, 0);

		if (file_exists(dir_))
		{
			return 0;
		}
	}

	const uint8_t* file = NULL;

	if (buffer_size(file_path_in_buffer))
	{
		if (!buffer_push_back(file_path_in_buffer, 0))
		{
			return 0;
		}

		file = buffer_data(file_path_in_buffer, 0);

		if (directory_exists(file))
		{
			return 0;
		}
	}

	if (NULL == dir_ && NULL == file)
	{
		return 0;
	}

	if (file && file_exists(file))
	{
		verbose = file_delete(file);
	}
	else
	{
		verbose = 1;
	}

	if (dir_ && directory_exists(dir_))
	{
		verbose = directory_delete(dir_) && verbose;
	}

	return verbose;
}

#define MKDIR_DIR_POSITION		0

uint8_t mkdir_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   delete_attributes, delete_attributes_lengths, 1,
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t mkdir_evaluate_task(struct buffer* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* dir_path_in_buffer = buffer_buffer_data(task_arguments, MKDIR_DIR_POSITION);

	if (!buffer_size(dir_path_in_buffer))
	{
		return 0;
	}

	if (!buffer_push_back(dir_path_in_buffer, 0))
	{
		return 0;
	}

	const uint8_t* dir_ = buffer_data(dir_path_in_buffer, 0);

	if (file_exists(dir_))
	{
		return 0;
	}

	return directory_create(dir_);
}

#define TOUCH_DATE_TIME_POSITION	0
#define TOUCH_FILE_POSITION			1
#define TOUCH_MILLIS_POSITION		2

static const uint8_t* touch_attributes[] =
{
	(const uint8_t*)"datetime",
	(const uint8_t*)"file",
	(const uint8_t*)"millis"
};

static const uint8_t touch_attributes_lengths[] = { 8, 4, 6 };

uint8_t touch_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   touch_attributes, touch_attributes_lengths,
			   COUNT_OF(touch_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t touch_evaluate_task(struct buffer* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* file_path_in_buffer = buffer_buffer_data(task_arguments, TOUCH_FILE_POSITION);

	if (!buffer_size(file_path_in_buffer))
	{
		return 1;
	}

	if (!buffer_push_back(file_path_in_buffer, 0) ||
		directory_exists(buffer_data(file_path_in_buffer, 0)))
	{
		return 0;
	}

	struct buffer* date_time_in_buffer = buffer_buffer_data(task_arguments, TOUCH_DATE_TIME_POSITION);

	struct buffer* millis_in_buffer = buffer_buffer_data(task_arguments, TOUCH_MILLIS_POSITION);

	if (!file_exists(buffer_data(file_path_in_buffer, 0)))
	{
		if (!file_create(buffer_data(file_path_in_buffer, 0)))
		{
			return 0;
		}

		if (!buffer_size(date_time_in_buffer) &&
			!buffer_size(millis_in_buffer))
		{
			return 1;
		}
	}

	int64_t seconds = 0;

	if (buffer_size(date_time_in_buffer))
	{
		if (!datetime_parse_buffer(date_time_in_buffer))
		{
			return 0;
		}

		seconds = buffer_size(date_time_in_buffer);
		seconds -= sizeof(int64_t);

		if (seconds < 1)
		{
			return 0;
		}

		seconds = *(const int64_t*)buffer_data(date_time_in_buffer, (ptrdiff_t)seconds);
	}
	else if (buffer_size(millis_in_buffer))
	{
		if (!buffer_push_back(millis_in_buffer, 0))
		{
			return 0;
		}

		seconds = int64_parse(buffer_data(millis_in_buffer, 0));
		seconds = date_time_millisecond_to_second(seconds);
	}
	else
	{
		seconds = datetime_now();
	}

	return file_set_last_write_time(buffer_data(file_path_in_buffer, 0), seconds);
}
