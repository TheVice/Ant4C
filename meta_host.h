/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef __META_HOST_H__
#define __META_HOST_H__

#include <stdint.h>

struct buffer;

uint8_t meta_host_init();

uint8_t meta_host_get_runtime(
	const uint8_t* version_start, const uint8_t* version_finish, void** the_runtime);
uint8_t meta_host_get_runtime_v2(void** the_runtime);
uint8_t meta_host_get_runtime_v4(void** the_runtime);
uint8_t meta_host_get_version_from_file(
	const uint8_t* file_path_start, const uint8_t* file_path_finish, struct buffer* output);
uint8_t meta_host_enumerate_installed_runtimes(struct buffer* output);
uint8_t meta_host_enumerate_loaded_runtimes(void* process, struct buffer* output);

uint8_t meta_host_exit_process(int32_t exit_code);

void meta_host_release();

#endif
