/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef _COPY_MOVE_H_
#define _COPY_MOVE_H_

#include <stdint.h>

struct buffer;

uint8_t copy_move_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t copy_evaluate_task(
	const void* the_project, const void* the_target,
	struct buffer* task_arguments, uint8_t verbose);
uint8_t move_evaluate_task(
	const void* the_project, const void* the_target,
	struct buffer* task_arguments, uint8_t verbose);

#endif
