/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020, 2022 TheVice
 *
 */

#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include <stddef.h>
#include <stdint.h>

enum data_type { property_value_is_byte_array, property_value_is_integer, property_value_is_double };

uint8_t property_get_by_pointer(const void* the_property, void* output);

uint8_t property_exists(
	const void* properties, const uint8_t* name, uint8_t name_length, void** the_property);
uint8_t property_is_dynamic(const void* the_property, uint8_t* dynamic);
uint8_t property_is_readonly(const void* the_property, uint8_t* read_only);

uint8_t property_set_by_name(
	void* properties, const uint8_t* name, uint8_t name_length,
	const uint8_t* value, ptrdiff_t value_length,
	enum data_type type_of_value, uint8_t dynamic, uint8_t over_write,
	uint8_t read_only, uint8_t verbose);
uint8_t property_set_by_pointer(
	void* the_property, const void* value, ptrdiff_t value_length,
	enum data_type type_of_value, uint8_t dynamic, uint8_t read_only, uint8_t verbose);
uint8_t property_set_from_file(
	void* the_property, const uint8_t* file_name, uint16_t encoding,
	uint8_t dynamic, uint8_t read_only, uint8_t verbose);
uint8_t property_set_from_stream(
	void* the_property, void* stream, uint8_t dynamic, uint8_t read_only, uint8_t verbose);

uint8_t property_add_at_project(
	void* the_project, const void* properties, uint8_t verbose);
void property_release_inner(void* properties);
void property_release(void* properties);

#endif
