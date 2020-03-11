/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "file_system.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#if !defined(_WIN32)
#include "date_time.h"
#endif
#include "path.h"
#include "property.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"
#if defined(_WIN32)
#include "text_encoding.h"
#endif

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#define _POSIXSOURCE 1

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
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

uint8_t file_system_append_pre_root(const struct range* path, struct buffer* output)
{
	if (range_is_null_or_empty(path) || NULL == output)
	{
		return 0;
	}

	if (path_is_path_rooted(path->start, path->finish) &&
		!string_starts_with(path->start, path->finish, pre_root_path, pre_root_path + pre_root_path_length) &&
		!buffer_append(output, pre_root_path, pre_root_path_length))
	{
		return 0;
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
	struct buffer* output)
{
	if (NULL == pattern ||
		all_entries < entry_type ||
		(0 != recurse && 1 != recurse) ||
		NULL == output)
	{
		return 0;
	}

	WIN32_FIND_DATAW file_data;
	const wchar_t* start = buffer_wchar_t_data(pattern, 0);
	const HANDLE file_handle = FindFirstFileW(start, &file_data);

	if (INVALID_HANDLE_VALUE == file_handle)
	{
		return 0;
	}

	const ptrdiff_t size = wcslen(start);
	const wchar_t* finish = start + size;
	const ptrdiff_t index =
		find_any_symbol_like_or_not_like_that_wchar_t(finish - 1, start, L"\\", 1, 1, -1) - start;
	const ptrdiff_t delta = size - index + 1;

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
				if (!buffer_append_wchar_t(pattern, NULL, delta) ||
					!directory_enumerate_file_system_entries_wchar_t(pattern, entry_type, recurse, output_encoding, output))
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

		if (UTF8 == output_encoding)
		{
			start = buffer_wchar_t_data(pattern, 0);
			finish = (const wchar_t*)(buffer_data(pattern, 0) + sizeof(wchar_t) * new_index);
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
	return 1;
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

uint8_t file_system_path_to_pathW(const uint8_t* path, struct buffer* pathW)
{
	if (NULL == path ||
		NULL == pathW)
	{
		return 0;
	}

	const ptrdiff_t length = common_count_bytes_until(path, 0);

	if (!length)
	{
		return 0;
	}

	if (path_is_path_rooted(path, path + length) &&
		!string_starts_with(path, path + length, pre_root_path, pre_root_path + pre_root_path_length) &&
		!buffer_append_wchar_t(pathW, pre_root_path_wchar_t, pre_root_path_length))
	{
		return 0;
	}

	return text_encoding_UTF8_to_UTF16LE(path, path + length, pathW) && buffer_push_back_uint16(pathW, 0);
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
	struct buffer* path, const uint8_t* wildcard,
	const uint8_t entry_type, const uint8_t recurse,
	struct buffer* output)
{
	if (NULL == path ||
		all_entries < entry_type ||
		(0 != recurse && 1 != recurse) ||
		NULL == output)
	{
		return 0;
	}

	DIR* directory = opendir(buffer_char_data(path, 0));

	if (NULL == directory)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(path);
	struct dirent* entry = NULL;

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
				if (!directory_enumerate_file_system_entries_(path, wildcard, entry_type, recurse, output))
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

		(void)wildcard;/*TODO:*/

		if (!buffer_append_data_from_buffer(output, path) ||
			!buffer_resize(path, size - 1) ||
			!buffer_push_back(path, 0))
		{
			closedir(directory);
			return 0;
		}
	}

	return 0 == closedir(directory);
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

	const uint8_t returned = directory_create_wchar_t(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	return returned;
#else
	static const mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
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

		if (!directory_enumerate_file_system_entries_wchar_t(&pathW, entry, 1, UTF16LE, &entries))
#else
		if (!directory_enumerate_file_system_entries(&pathW, entry, 1, &entries))
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

				if (0 != DeleteFileW(start))
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
	struct buffer* path, const uint8_t entry_type, const uint8_t recurse, struct buffer* output)
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
								 &patternW, entry_type, recurse, UTF8, output);
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

	struct buffer wildcard;

	SET_NULL_TO_BUFFER(wildcard);

	if (!buffer_append_data_from_range(&wildcard, &file_name))
	{
		buffer_release(&wildcard);
		return 0;
	}

	file_name.start = buffer_data(path, 0);
	file_name.finish = file_name.start + buffer_size(path);

	if (!path_get_directory_name(file_name.start, file_name.finish, &file_name) ||
		!buffer_resize(path, range_size(&file_name)) ||
		!buffer_push_back(path, 0))
	{
		buffer_release(&wildcard);
		return 0;
	}

	const uint8_t returned = directory_enumerate_file_system_entries_(
								 path, buffer_data(&wildcard, 0), entry_type, recurse, output);
	buffer_release(&wildcard);
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
	int64_t result = directory_status.st_atim.tv_sec;
	result = directory_status.st_mtim.tv_sec < result ? directory_status.st_mtim.tv_sec : result;
	result = directory_status.st_ctim.tv_sec < result ? directory_status.st_ctim.tv_sec : result;
	return result;
#endif
}

uint8_t directory_get_current_directory(const void* project, const void** the_property,
										struct buffer* output)
{
	if (!project_get_base_directory(project, the_property))
	{
		if (NULL != the_property)
		{
			(*the_property) = NULL;
		}

		return path_get_directory_for_current_process(output);
	}

	return property_get_by_pointer(the_property, output);
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
	return directory_status.st_atim.tv_sec;
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
	return directory_status.st_mtim.tv_sec;
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

#if defined(_WIN32)
int32_t _file_fileno(void* stream)
{
	return _fileno(stream);
}
#endif
uint8_t file_close(void* stream)
{
	return NULL != stream &&
		   0 == fclose(stream);
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
	int64_t result = file_status.st_atim.tv_sec;
	result = file_status.st_mtim.tv_sec < result ? file_status.st_mtim.tv_sec : result;
	result = file_status.st_ctim.tv_sec < result ? file_status.st_ctim.tv_sec : result;
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
	return file_status.st_atim.tv_sec;
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
	return file_status.st_mtim.tv_sec;
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

#if __STDC_SEC_API__
	return (0 == _wfopen_s((FILE**)output, path, mode) && NULL != (*output));
#else
	(*output) = (void*)_wfopen(path, mode);
	return (NULL != (*output));
#endif
}
#endif
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
#if __STDC_SEC_API__ && defined(_MSC_VER)
	return fread_s(content, count_of_elements, size_of_content_element, count_of_elements, stream);
#else
	return fread(content, size_of_content_element, count_of_elements, stream);
#endif
}

uint8_t file_read_all_bytes(const uint8_t* path, struct buffer* output)
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

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 4096))
	{
		file_close(stream);
		return 0;
	}

	uint64_t readed = 0;
	uint8_t* ptr = buffer_data(output, size);

	while (0 < (readed = file_read(ptr, sizeof(uint8_t), 4096, stream)))
	{
		size += (ptrdiff_t)readed;

		if (!buffer_resize(output, size) ||
			!buffer_append(output, NULL, 4096))
		{
			file_close(stream);
			return 0;
		}

		ptr = buffer_data(output, size);
	}

	return file_close(stream) && buffer_resize(output, size);
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
	exists,
	get_creation_time,
	get_creation_time_utc,
	get_current_directory,
	get_directory_root,
	get_last_access_time,
	get_last_access_time_utc,
	get_last_write_time,
	get_last_write_time_utc,
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

	struct range argument1;

	struct range argument2;

	struct range argument3;

	argument1.start = argument2.start = argument1.finish = argument2.finish =
											argument3.start = argument3.finish = NULL;

	if (1 == arguments_count)
	{
		if (!common_get_one_argument(arguments, &argument1, 1))
		{
			return 0;
		}

		argument1.finish--;
	}
	else if (2 == arguments_count)
	{
		if (!common_get_two_arguments(arguments, &argument1, &argument2, 1))
		{
			return 0;
		}

		argument1.finish--;
		argument2.finish--;
	}
	else if (3 == arguments_count)
	{
		if (!common_get_three_arguments(arguments, &argument1, &argument2, &argument3, 1))
		{
			return 0;
		}

		argument1.finish--;
		argument2.finish--;
		argument3.finish--;
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
										   argument2.start, argument2.finish, entry_types_str, UNKNOWN_ENTRY_TYPE);

			if (UNKNOWN_ENTRY_TYPE == entry_type)
			{
				break;
			}

			uint8_t recurse = 0;

			if (3 == arguments_count &&
				!bool_parse(argument3.start, range_size(&argument3), &recurse))
			{
				break;
			}

			return directory_enumerate_file_system_entries(buffer_buffer_data(arguments, 0), entry_type, recurse, output);
		}

		case exists:
			return 1 == arguments_count && bool_to_string(directory_exists(argument1.start), output);

		case get_creation_time:
			return 1 == arguments_count && int64_to_string(directory_get_creation_time(argument1.start), output);

		case get_creation_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_creation_time_utc(argument1.start), output);

		/*case get_current_directory:
			return 0 == arguments_count && directory_get_current_directory(project, the_property, output);*/

		case get_directory_root:
			return 1 == arguments_count &&
				   directory_get_directory_root(argument1.start, &argument2) &&
				   buffer_append_data_from_range(output, &argument2);

		case get_last_access_time:
			return 1 == arguments_count && int64_to_string(directory_get_last_access_time(argument1.start), output);

		case get_last_access_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_last_access_time_utc(argument1.start), output);

		case get_last_write_time:
			return 1 == arguments_count && int64_to_string(directory_get_last_write_time(argument1.start), output);

		case get_last_write_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_last_write_time_utc(argument1.start), output);

		case get_logical_drives:
			return 0 == arguments_count && directory_get_logical_drives(output);

		case get_parent_directory:
			return 1 == arguments_count &&
				   directory_get_parent_directory(argument1.start, argument1.finish, &argument2) &&
				   buffer_append_data_from_range(output, &argument2);

		case UNKNOWN_DIR_FUNCTION:
			break;
	}

	return 0;
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
