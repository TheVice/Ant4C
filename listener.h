/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#ifndef _LISTENER_H_
#define _LISTENER_H_

#include <stddef.h>
#include <stdint.h>

struct range;

void listener_project_started(const uint8_t* source, const uint8_t* the_project, uint8_t verbose);
void listener_project_finished(
	const uint8_t* source, const uint8_t* the_project, uint8_t result, uint8_t verbose);

void listener_target_started(const uint8_t* source, ptrdiff_t offset,
							 const uint8_t* the_project, const uint8_t* the_target, uint8_t verbose);
void listener_target_finished(const uint8_t* source, ptrdiff_t offset,
							  const uint8_t* the_project, const uint8_t* the_target, uint8_t result, uint8_t verbose);

void listener_task_started(const uint8_t* source, ptrdiff_t offset,
						   const uint8_t* the_project, const uint8_t* the_target,
						   const struct range* task_name, ptrdiff_t task_id,
						   const uint8_t* the_module, uint8_t verbose);
void listener_task_finished(const uint8_t* source, ptrdiff_t offset,
							const uint8_t* the_project, const uint8_t* the_target,
							const struct range* task_name, ptrdiff_t task_id,
							const uint8_t* the_module, uint8_t result, uint8_t verbose);

void listener_set_on_project_started(
	void (*listener_on_project_started)(const uint8_t* source, const uint8_t* the_project));
void listener_set_on_project_finished(
	void (*listener_on_project_finished)(const uint8_t* source, const uint8_t* the_project, uint8_t result));

void listener_set_on_target_started(
	void (*listener_on_target_started)(const uint8_t* source, ptrdiff_t offset,
									   const uint8_t* the_project, const uint8_t* the_target));
void listener_set_on_target_finished(
	void (*listener_on_target_finished)(const uint8_t* source, ptrdiff_t offset,
										const uint8_t* the_project, const uint8_t* the_target, uint8_t result));

void listener_set_on_task_started(
	void (*listener_on_task_started)(const uint8_t* source, ptrdiff_t offset,
									 const uint8_t* the_project, const uint8_t* the_target, const uint8_t* the_task));
void listener_set_on_task_finished(
	void (*listener_on_task_finished)(const uint8_t* source, ptrdiff_t offset,
									  const uint8_t* the_project, const uint8_t* the_target,
									  const uint8_t* the_task, uint8_t result));

uint8_t load_listener(const uint8_t* listener, void** object);

#endif
