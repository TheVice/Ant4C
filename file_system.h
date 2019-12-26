/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#if defined(_WIN32)
#include <wchar.h>
#endif

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

#if defined(_WIN32)
uint8_t file_system_append_pre_root(const struct range* path, struct buffer* output);
uint8_t file_system_get_position_after_pre_root(struct range* path);
#endif

#if defined(_WIN32)
uint8_t directory_exists_wchar_t(const wchar_t* path);
#endif
uint8_t directory_exists(const uint8_t* path);
int64_t directory_get_creation_time(const uint8_t* path);
int64_t directory_get_creation_time_utc(const uint8_t* path);
uint8_t directory_get_current_directory(
	const void* project, const void** the_property, struct buffer* current_directory);
uint8_t directory_get_directory_root(const uint8_t* path, struct range* output);
int64_t directory_get_last_access_time(const uint8_t* path);
int64_t directory_get_last_access_time_utc(const uint8_t* path);
int64_t directory_get_last_write_time(const uint8_t* path);
int64_t directory_get_last_write_time_utc(const uint8_t* path);
uint8_t directory_get_logical_drives(struct buffer* drives);
uint8_t directory_get_parent_directory(
	const uint8_t* path_start, const uint8_t* path_finish, struct range* parent);

#if defined(_WIN32)
uint8_t file_exists_wchar_t(const wchar_t* path);
#endif
uint8_t file_exists(const uint8_t* path);
int64_t file_get_creation_time(const uint8_t* path);
int64_t file_get_creation_time_utc(const uint8_t* path);
int64_t file_get_last_access_time(const uint8_t* path);
int64_t file_get_last_access_time_utc(const uint8_t* path);
int64_t file_get_last_write_time(const uint8_t* path);
int64_t file_get_last_write_time_utc(const uint8_t* path);
uint8_t file_open(const uint8_t* path, const uint8_t* mode, void** output);
uint64_t file_get_length(const uint8_t* path);
uint8_t file_up_to_date(const uint8_t* src_file, const uint8_t* target_file);

#endif
