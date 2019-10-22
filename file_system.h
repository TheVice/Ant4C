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
uint8_t directory_exists_by_wchar_path(const wchar_t* path);

uint8_t file_exists_by_wchar_path(const wchar_t* path);
#endif

uint8_t directory_exists(const char* path);
#if 0
directory_get_creation_time
#endif
uint8_t directory_get_current_directory(const void* project, struct buffer* current_directory);
#if 0
directory_get_directory_root->path_root
directory_get_last_access_time
directory_get_last_write_time
#endif
uint8_t directory_get_logical_drives(struct buffer* drives);
uint8_t directory_get_parent_directory(const char* path_start, const char* path_finish, struct range* parent);

uint8_t file_exists(const char* path);
#if 0
file_get_creation_time
file_get_last_access_time
file_get_last_write_time
#endif
uint64_t file_get_length(const char* path);
#if 0
file_is_assembly
file_up_to_date
#endif

#endif
