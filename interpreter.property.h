/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#ifndef _INTERPRETER_PROPERTY_H_
#define _INTERPRETER_PROPERTY_H_

#include <stdint.h>

uint8_t property_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t property_evaluate_task(
	void* the_project, void* properties, const void* task_arguments, uint8_t verbose);

uint8_t property_get_id_of_get_value_function();
uint8_t property_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t property_exec_function(
	const void* the_project, uint8_t function, const void* arguments,
	uint8_t arguments_count, const void** the_property, void* output, uint8_t verbose);

#endif
