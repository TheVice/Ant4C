/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _PATH_H_
#define _PATH_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

static const uint8_t path_posix_delimiter = '/';
static const uint8_t path_windows_delimiter = '\\';

#if defined(_WIN32)
#define PATH_DELIMITER path_windows_delimiter
#else
#define PATH_DELIMITER path_posix_delimiter
#endif

uint8_t path_get_temp_file_name(struct buffer* temp_file_name);
uint8_t path_get_temp_path(struct buffer* temp_path);
uint8_t path_is_path_rooted(const uint8_t* path_start, const uint8_t* path_finish);
uint8_t cygpath_get_unix_path(uint8_t* path_start, uint8_t* path_finish);
uint8_t cygpath_get_windows_path(uint8_t* path_start, uint8_t* path_finish);

#endif
