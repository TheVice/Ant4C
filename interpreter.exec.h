/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#ifndef _INTERPRETER_EXEC_H_
#define _INTERPRETER_EXEC_H_

#include <stdint.h>

struct buffer;

uint8_t interpreter_get_environments(
	const void* the_project,
	const void* the_target,
	const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	struct buffer* environments,
	uint8_t verbose);

uint8_t exec_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t exec_evaluate_task(
	void* the_project, const void* the_target, const struct buffer* task_arguments, uint8_t verbose);

#endif
