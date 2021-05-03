/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#ifndef _SLEEP_UNIT_H_
#define _SLEEP_UNIT_H_

#include <stdint.h>

struct buffer;

uint8_t sleep_for(uint32_t milliseconds);

uint8_t sleep_unit_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t sleep_unit_evaluate_task(struct buffer* task_arguments, uint8_t verbose);

#endif
