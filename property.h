/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

enum data_type { property_value_is_byte_array, property_value_is_integer, property_value_is_double };

uint8_t property_get_by_pointer(const void* the_property, struct buffer* output);

uint8_t property_exists(const struct buffer* properties,
						const uint8_t* name, uint8_t name_length, void** the_property);
uint8_t property_is_dynamic(const void* the_property, uint8_t* dynamic);
uint8_t property_is_readonly(const void* the_property, uint8_t* read_only);

uint8_t property_set_by_name(struct buffer* properties,
							 const uint8_t* name, uint8_t name_length,
							 const uint8_t* value, ptrdiff_t value_length,
							 enum data_type type_of_value,
							 uint8_t dynamic, uint8_t over_write,
							 uint8_t read_only, uint8_t verbose);
uint8_t property_set_by_pointer(void* the_property,
								const void* value, ptrdiff_t value_length,
								enum data_type type_of_value,
								uint8_t dynamic, uint8_t read_only, uint8_t verbose);
uint8_t property_set_from_file(void* the_property,
							   const uint8_t* file_name, uint16_t encoding,
							   uint8_t dynamic, uint8_t read_only, uint8_t verbose);

uint8_t property_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t property_evaluate_task(void* the_project, struct buffer* properties,
							   const struct buffer* task_arguments, uint8_t verbose);

uint8_t property_add_at_project(void* the_project, const struct buffer* properties,
								uint8_t(*except_filter)(const uint8_t*, uint8_t), uint8_t verbose);
void property_release_inner(struct buffer* properties);
void property_release(struct buffer* properties);

uint8_t property_get_id_of_get_value_function();
uint8_t property_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t property_exec_function(const void* the_project, uint8_t function, const struct buffer* arguments,
							   uint8_t arguments_count, const void** the_property, struct buffer* output, uint8_t verbose);

#endif
