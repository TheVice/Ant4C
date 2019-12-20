/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _TARGET_H_
#define _TARGET_H_

#include <stdint.h>

struct buffer;
struct range;

uint8_t target_exists(const struct buffer* targets, const uint8_t* name, uint8_t name_length);
uint8_t target_get_current_target(const void* target, const uint8_t** name, uint8_t* name_length);
uint8_t target_has_executed(const struct buffer* targets, const uint8_t* name, uint8_t name_length);

uint8_t target_new(const struct range* name, const struct range* depends, const struct range* content,
				   struct buffer* targets);
uint8_t target_get(const struct buffer* targets, const uint8_t* name, uint8_t name_length, void** target);
void target_clear(struct buffer* targets);

uint8_t target_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t target_evaluate_task(void* project, const struct buffer* task_arguments,
							 const uint8_t* attributes_finish, const uint8_t* element_finish,
							 uint8_t verbose);

uint8_t target_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t target_exec_function(
	const void* project, const void* target,
	uint8_t function, const struct buffer* arguments,
	uint8_t arguments_count, struct buffer* output);

#endif
