/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

enum data_type { property_value_is_byte_array, property_value_is_integer, property_value_is_double };

uint8_t property_get_by_name(const void* project, const uint8_t* property_name, uint8_t property_name_length,
							 struct buffer* output);
uint8_t property_get_by_pointer(const void* the_property, struct buffer* output);

uint8_t property_exists(const struct buffer* properties,
						const uint8_t* name, uint8_t name_length, void** the_property);
uint8_t property_is_dynamic(const void* the_property, uint8_t* is_dynamic);
uint8_t property_is_readonly(const void* the_property, uint8_t* is_readonly);

uint8_t property_set_by_name(struct buffer* properties,
							 const uint8_t* name, uint8_t name_length,
							 const uint8_t* value, ptrdiff_t value_length,
							 enum data_type type_of_value,
							 uint8_t dynamic, uint8_t overwrite,
							 uint8_t readonly, uint8_t verbose);
uint8_t property_set_by_pointer(void* the_property,
								const void* value, ptrdiff_t value_length,
								enum data_type type_of_value,
								uint8_t dynamic,
								uint8_t readonly, uint8_t verbose);

uint8_t property_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t property_evaluate_task(void* project, const struct buffer* task_arguments);

uint8_t property_add_at_project(void* project, const struct buffer* properties, uint8_t verbose);
void property_clear(struct buffer* properties);

uint8_t property_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t property_exec_function(const void* project, uint8_t function, const struct buffer* arguments,
							   uint8_t arguments_count, void** the_property, struct buffer* output);
uint8_t property_actualize_value(const void* project, const void* target,
								 uint8_t property_function_id, void* the_property,
								 ptrdiff_t size, struct buffer* return_of_function);

#endif
