/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _TARGET_H_
#define _TARGET_H_

#include <stdint.h>

struct buffer;
struct range;

uint8_t target_exists(
	const struct buffer* targets, const uint8_t* name, uint8_t name_length);
uint8_t target_get_current_target(
	const void* the_target, const uint8_t** name, uint8_t* name_length);
uint8_t target_has_executed(
	const struct buffer* targets, const uint8_t* name, uint8_t name_length);

uint8_t target_new(
	void* the_project,
	const uint8_t* name, uint8_t name_length,
	const uint8_t* depends, uint16_t depends_length,
	struct buffer* description,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	struct buffer* targets,
	const struct range* sub_nodes_names, uint8_t verbose);
uint8_t target_get(
	const struct buffer* targets,
	const uint8_t* name, uint8_t name_length,
	void** the_target);
const struct range* target_get_depend(const void* the_target, uint16_t index);
void target_release_inner(struct buffer* targets);
void target_release(struct buffer* targets);

uint8_t target_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t target_evaluate_task(
	void* the_project, struct buffer* task_arguments, uint8_t target_help,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	const struct range* sub_nodes_names, uint8_t verbose);

uint8_t target_evaluate(
	void* the_project, void* the_target, struct buffer* stack,
	uint8_t cascade, uint8_t verbose);
uint8_t target_evaluate_by_name(
	void* the_project, const struct range* target_name, uint8_t verbose);

uint8_t target_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t target_exec_function(
	const void* the_project, const void* the_target,
	uint8_t function, const struct buffer* arguments,
	uint8_t arguments_count, struct buffer* output);

uint8_t call_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t call_evaluate_task(
	void* the_project, struct buffer* task_arguments, uint8_t verbose);

#endif
