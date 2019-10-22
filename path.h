/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _PATH_H_
#define _PATH_H_

#include <stdint.h>

struct buffer;
struct range;

char path_delimiter();
char path_posix_delimiter();
char path_windows_delimiter();
uint8_t path_change_extension(const char* path_start, const char* path_finish,
							  const char* ext_start, const char* ext_finish, struct buffer* path);
uint8_t path_combine(const char* path1_start, const char* path1_finish,
					 const char* path2_start, const char* path2_finish, struct buffer* path);
uint8_t path_get_directory_name(const char* path_start, const char* path_finish, struct range* directory);
uint8_t path_get_extension(const char* path_start, const char* path_finish, struct range* ext);
uint8_t path_get_file_name(const char* path_start, const char* path_finish, struct range* file_name);
uint8_t path_get_file_name_without_extension(
	const char* path_start, const char* path_finish, struct range* file_name);
uint8_t path_get_full_path(const char* root_start, const char* root_finish,
						   const char* path_start, const char* path_finish, struct buffer* full_path);
uint8_t path_get_path_root(const char* path_start, const char* path_finish, struct range* root);
uint8_t path_get_temp_file_name(struct buffer* temp_file_name);
uint8_t path_get_temp_path(struct buffer* temp_path);
uint8_t path_has_extension(const char* path_start, const char* path_finish);
uint8_t path_is_path_rooted(const char* path_start, const char* path_finish);

uint8_t path_get_directory_for_current_process(struct buffer* path);
uint8_t path_get_directory_for_current_image(struct buffer* path);

uint8_t cygpath_get_dos_path(const char* path_start, const char* path_finish, struct buffer* path);
uint8_t cygpath_get_unix_path(char* path_start, char* path_finish);
uint8_t cygpath_get_windows_path(char* path_start, char* path_finish);

uint8_t path_get_function(const char* name_start, const char* name_finish);
uint8_t path_exec_function(const void* project,
						   uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						   struct buffer* output);
uint8_t cygpath_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output);

#endif
