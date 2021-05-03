/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#ifndef _IF_TASK_H_
#define _IF_TASK_H_

#include <stdint.h>

struct buffer;

uint8_t if_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t if_evaluate_task(
	void* the_project, const void* the_target, struct buffer* task_arguments,
	const uint8_t* attributes_finish, const uint8_t* element_finish, uint8_t verbose);

#endif
