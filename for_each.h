/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2021 TheVice
 *
 */

#ifndef _FOR_EACH_H_
#define _FOR_EACH_H_

#include <stdint.h>

struct buffer;

uint8_t for_each_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t for_each_evaluate_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	struct buffer* task_arguments,
	uint8_t fail_on_error, uint8_t verbose);
uint8_t do_evaluate_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	struct buffer* task_arguments, uint8_t verbose);

#endif
