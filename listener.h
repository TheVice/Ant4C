/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef _LISTENER_H_
#define _LISTENER_H_

#include <stddef.h>
#include <stdint.h>

void listener_project_started(const uint8_t* source, const void* the_project);
void listener_project_finished(const uint8_t* source, const void* the_project);

void listener_target_started(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							 const void* the_target);
void listener_target_finished(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							  const void* the_target);

void listener_task_started(const uint8_t* source, ptrdiff_t offset, const void* the_project,
						   const void* the_target, uint8_t task);
void listener_task_finished(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							const void* the_target, uint8_t task, uint8_t result);

void listener_set_on_project_started(void (*listener_on_project_started)(const uint8_t* source,
									 const void* the_project));
void listener_set_on_project_finished(void (*listener_on_project_finished)(const uint8_t* source,
									  const void* the_project));

void listener_set_on_target_started(void (*listener_on_target_started)(const uint8_t* source,
									ptrdiff_t offset, const void* the_project, const void* the_target));
void listener_set_on_target_finished(void (*listener_on_target_finished)(const uint8_t* source,
									 ptrdiff_t offset, const void* the_project, const void* the_target));

void listener_set_on_task_started(void (*listener_on_task_started)(const uint8_t* source, ptrdiff_t offset,
								  const void* the_project, const void* the_target, uint8_t task));
void listener_set_on_task_finished(void (*listener_on_task_finished)(const uint8_t* source, ptrdiff_t offset,
								   const void* the_project, const void* the_target, uint8_t task, uint8_t result));

#endif
