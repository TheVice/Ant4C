/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, 2022 TheVice
 *
 */

#ifndef _COPY_MOVE_H_
#define _COPY_MOVE_H_

#include <stdint.h>

uint8_t copy_move_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t copy_evaluate_task(
	const void* the_project, const void* the_target,
	void* task_arguments, uint8_t verbose);
uint8_t move_evaluate_task(
	const void* the_project, const void* the_target,
	void* task_arguments, uint8_t verbose);

#endif
