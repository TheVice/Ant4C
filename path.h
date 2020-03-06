/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _PATH_H_
#define _PATH_H_

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

uint8_t path_change_extension(const uint8_t* path_start, const uint8_t* path_finish,
							  const uint8_t* ext_start, const uint8_t* ext_finish, struct buffer* path);
uint8_t path_combine(const uint8_t* path1_start, const uint8_t* path1_finish,
					 const uint8_t* path2_start, const uint8_t* path2_finish, struct buffer* output);
uint8_t path_get_directory_name(const uint8_t* path_start, const uint8_t* path_finish,
								struct range* directory);
uint8_t path_get_extension(const uint8_t* path_start, const uint8_t* path_finish, struct range* ext);
uint8_t path_get_file_name(const uint8_t* path_start, const uint8_t* path_finish, struct range* file_name);
uint8_t path_get_file_name_without_extension(
	const uint8_t* path_start, const uint8_t* path_finish, struct range* file_name);
uint8_t path_get_full_path(const uint8_t* root_start, const uint8_t* root_finish,
						   const uint8_t* path_start, const uint8_t* path_finish, struct buffer* full_path);
uint8_t path_get_path_root(const uint8_t* path_start, const uint8_t* path_finish, struct range* root);
uint8_t path_get_temp_file_name(struct buffer* temp_file_name);
uint8_t path_get_temp_path(struct buffer* temp_path);
uint8_t path_has_extension(const uint8_t* path_start, const uint8_t* path_finish);
uint8_t path_is_path_rooted(const uint8_t* path_start, const uint8_t* path_finish);

uint8_t path_get_directory_for_current_process(struct buffer* path);
uint8_t path_get_directory_for_current_image(struct buffer* path);

uint8_t cygpath_get_dos_path(const uint8_t* path_start, const uint8_t* path_finish, struct buffer* path);
uint8_t cygpath_get_unix_path(uint8_t* path_start, uint8_t* path_finish);
uint8_t cygpath_get_windows_path(uint8_t* path_start, uint8_t* path_finish);

uint8_t path_get_id_of_get_full_path_function();
uint8_t path_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t path_exec_function(const void* project,
						   uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						   struct buffer* output);
uint8_t cygpath_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output);

#endif
