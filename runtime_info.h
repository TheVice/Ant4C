/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef __RUN_TIME_INFO_H__
#define __RUN_TIME_INFO_H__

#include <stdint.h>

struct buffer;

uint8_t runtime_info_get_version_string(void* info, struct buffer* output);
uint8_t runtime_info_get_runtime_directory(void* info, struct buffer* output);
uint8_t runtime_info_is_loaded(void* info, void* process, uint8_t* loaded);
uint8_t runtime_info_load_error_string(void* info, uint32_t resource_id,
									   long locale_id, struct buffer* output);

uint8_t runtime_info_get_interface(
	void* info, const void* class_id, const void* iid, void** the_interface);
uint8_t runtime_info_get_interface_of_cor_runtime_host(void* info, void** cor_host);
uint8_t runtime_info_get_interface_of_clr_runtime_host(void* info, void** clr_host);
uint8_t runtime_info_get_interface_of_type_name_factory(void* info, void** type_name_factory);
uint8_t runtime_info_get_interface_of_strong_name(void* info, void** strong_name);

uint8_t runtime_info_is_loadable(void* info, uint8_t* loadable);
uint8_t runtime_info_set_default_startup_flags(
	void* info, unsigned long startup_flags, const uint8_t* host_config_file);
uint8_t runtime_info_get_default_startup_flags(void* info, struct buffer* output);
uint8_t runtime_info_is_started(void* info, uint8_t* started, unsigned long* startup_flags);

#endif
