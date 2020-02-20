/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "file_system.h"
#include "buffer.h"
#include "common.h"
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

#if defined(_WIN32)
#include <string.h>

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

uint8_t directory_create_wchar_t(const wchar_t* path)
{
	/*return 0 != CreateDirectoryExW(%HOME%, path, NULL);*/
	return 0 != CreateDirectoryW(path, NULL);
}

uint8_t directory_delete_wchar_t(const wchar_t* path)
{
	return 0 != RemoveDirectoryW(path);
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

#if defined(_WIN32)
	struct buffer pathW;
	SET_NULL_TO_BUFFER(pathW);

	if (!file_system_path_to_pathW(path, &pathW))
	{
		buffer_release(&pathW);
		return 0;
	}

	const uint8_t returned = directory_delete_wchar_t(buffer_wchar_t_data(&pathW, 0));
	buffer_release(&pathW);
	return returned;
#else
	return 0 == rmdir((const char*)path);
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
										struct buffer* current_directory)
{
	if (!project_get_base_directory(project, the_property))
	{
		if (NULL != the_property)
		{
			(*the_property) = NULL;
		}

		return path_get_directory_for_current_process(current_directory);
	}

	return property_get_by_pointer(the_property, current_directory);
}

uint8_t directory_get_directory_root(const uint8_t* path, struct range* output)
{
	return NULL != path && path_get_path_root(path, path + common_count_bytes_until(path, 0), output);
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

uint8_t file_fflush(void* stream)
{
	return NULL != stream && 0 == fflush(stream);
}

uint8_t file_fseek(void* stream, long offset, int32_t origin)
{
	return NULL != stream && 0 == fseek(stream, offset, origin);
}

long file_ftell(void* stream)
{
	return NULL == stream ? 0 : ftell(stream);
}

uint8_t file_fwrite(const void* content, const size_t size_of_content_element,
					const size_t count_of_elements, void* stream)
{
	return count_of_elements == fwrite(content, size_of_content_element, count_of_elements, stream);
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

uint64_t file_read(void* stream, uint64_t size, void* output)
{
	if (NULL == stream ||
		0 == size ||
		NULL == output)
	{
		return 0;
	}

#if __STDC_SEC_API__ && defined(_MSC_VER)
	return fread_s(output, (size_t)size, sizeof(uint8_t), (size_t)size, stream);
#else
	return fread(output, sizeof(uint8_t), (size_t)size, stream);
#endif
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

	/**/
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
