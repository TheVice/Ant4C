/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#ifndef _PATH_H_
#define _PATH_H_

#include <stddef.h>
#include <stdint.h>

struct range;

static const uint8_t path_posix_delimiter = '/';
static const uint8_t path_windows_delimiter = '\\';

#if defined(_WIN32)
#define PATH_DELIMITER path_windows_delimiter
#else
#define PATH_DELIMITER path_posix_delimiter
#endif

uint8_t path_change_extension(
	const uint8_t* path_start, const uint8_t* path_finish,
	const uint8_t* ext_start, const uint8_t* ext_finish,
	void* path);
uint8_t path_combine_in_place(
	void* path1, const ptrdiff_t size,
	const uint8_t* path2_start, const uint8_t* path2_finish);
uint8_t path_combine(
	const uint8_t* path1_start, const uint8_t* path1_finish,
	const uint8_t* path2_start, const uint8_t* path2_finish,
	void* output);
uint8_t path_get_directory_name(
	const uint8_t* path_start, const uint8_t** path_finish);
uint8_t path_get_extension(
	const uint8_t** path_start, const uint8_t* path_finish);
uint8_t path_get_file_name(
	const uint8_t** path_start, const uint8_t* path_finish);
uint8_t path_get_file_name_without_extension(
	const uint8_t* path_start, const uint8_t* path_finish,
	struct range* file_name);
uint8_t path_get_full_path(
	const uint8_t* root_start, const uint8_t* root_finish,
	const uint8_t* path_start, const uint8_t* path_finish,
	void* full_path);
uint8_t path_get_path_root(
	const uint8_t* path_start, const uint8_t** path_finish);
uint8_t path_get_temp_file_name(void* temp_file_name);
uint8_t path_get_temp_path(void* temp_path);
uint8_t path_has_extension(
	const uint8_t* path_start, const uint8_t* path_finish);
uint8_t path_is_path_rooted(
	const uint8_t* path_start, const uint8_t* path_finish,
	uint8_t* is_path_rooted);
uint8_t path_glob(
	const uint8_t* path_start, const uint8_t* path_finish,
	const uint8_t* wild_card_start, const uint8_t* wild_card_finish);

uint8_t path_get_directory_for_current_process(void* path);
uint8_t path_get_directory_for_current_image(void* path);

const uint8_t* path_try_to_get_absolute_path(
	const void* the_project, const void* the_target,
	void* input, void* tmp, uint8_t verbose);

uint8_t cygpath_get_dos_path(
	const uint8_t* path_start, const uint8_t* path_finish,
	void* path);
uint8_t cygpath_get_unix_path(uint8_t* path_start, uint8_t* path_finish);
uint8_t cygpath_get_windows_path(uint8_t* path_start, uint8_t* path_finish);

#endif
