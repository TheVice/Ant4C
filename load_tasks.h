/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef _LOAD_TASKS_H_
#define _LOAD_TASKS_H_

#include <stdint.h>

struct buffer;
struct range;

uint8_t load_tasks_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);

uint8_t load_tasks_evaluate_task(void* the_project, const void* the_target,
								 struct buffer* task_arguments, uint8_t verbose);

uint8_t load_tasks_evaluate_loaded_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	const uint8_t* element_finish, struct buffer* task_arguments,
	const void* the_module, const uint8_t* pointer_to_the_task,
	uint8_t verbose);
uint8_t load_tasks_evaluate_loaded_function(const void* the_module, const uint8_t* pointer_to_the_function,
		struct buffer* arguments, uint8_t arguments_count, struct buffer* return_of_function, uint8_t verbose);

const uint8_t* load_tasks_get_task(
	const struct buffer* modules, const struct range* task_name, void** the_module_of_task);

const uint8_t* load_tasks_get_function(
	const struct buffer* modules, const struct range* name_space,
	const struct range* function_name, void** the_module_of_function, const uint8_t** name_space_at_module);

void load_tasks_unload(struct buffer* modules);

uint8_t load_tasks_copy_modules_with_out_objects(const struct buffer* the_source,
		struct buffer* the_destination);

#endif
