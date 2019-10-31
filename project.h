/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

uint8_t project_property_get_pointer(const void* project,
									 const uint8_t* property_name, uint8_t property_name_length,
									 void** the_property);
uint8_t project_property_set_value(void* project, const void* target,
								   const uint8_t* property_name, uint8_t property_name_length,
								   const uint8_t* property_value, ptrdiff_t property_value_length,
								   uint8_t dynamic, uint8_t overwrite,
								   uint8_t readonly, uint8_t verbose);

uint8_t project_target_exists(const void* project, const uint8_t* name, uint8_t name_length);

uint8_t project_get_base_directory(const void* project, const void* target, struct buffer* base_directory);
uint8_t project_get_buildfile_path(const void* project, const void* target, struct buffer* build_file);
uint8_t project_get_buildfile_uri(const void* project, const void* target, struct buffer* build_file_uri);
uint8_t project_get_default_target(const void* project, const void* target, struct buffer* default_target);
uint8_t project_get_name(const void* project, const void* target, struct buffer* project_name);

uint8_t project_new(void** project);
uint8_t project_add_properties(void* project, const void* target,
							   const void* properties, uint8_t verbose);
uint8_t project_load_from_content(const uint8_t* content_start, const uint8_t* content_finish,
								  const uint8_t* base_dir, ptrdiff_t base_dir_length,
								  void* project, uint8_t verbose);
uint8_t project_load_from_build_file(const uint8_t* path_to_build_file, void* project, uint8_t verbose);
void project_unload(void* project);

uint8_t project_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t project_exec_function(const void* project, const void* target,
							  uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output);
uint8_t program_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output);

#endif
