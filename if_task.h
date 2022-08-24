/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, 2022 TheVice
 *
 */

#ifndef _IF_TASK_H_
#define _IF_TASK_H_

#include <stdint.h>

uint8_t if_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t if_evaluate_task(
	void* the_project, const void* the_target, void* task_arguments,
	const uint8_t* attributes_finish, const uint8_t* element_finish, uint8_t verbose);

#endif
