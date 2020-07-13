/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "listener.h"

static void (*on_project_started)(const uint8_t* source, const void* the_project) = NULL;
static void (*on_project_finished)(const uint8_t* source, const void* the_project) = NULL;

static void (*on_target_started)(const uint8_t* source, ptrdiff_t offset, const void* the_project,
								 const void* the_target) = NULL;
static void (*on_target_finished)(const uint8_t* source, ptrdiff_t offset, const void* the_project,
								  const void* the_target) = NULL;

static void (*on_task_started)(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							   const void* the_target, uint8_t task) = NULL;
static void (*on_task_finished)(const uint8_t* source, ptrdiff_t offset, const void* the_project,
								const void* the_target, uint8_t task, uint8_t result) = NULL;

void listener_project_started(const uint8_t* source, const void* the_project)
{
	if (NULL != on_project_started)
	{
		on_project_started(source, the_project);
	}
}

void listener_project_finished(const uint8_t* source, const void* the_project)
{
	if (NULL != on_project_finished)
	{
		on_project_finished(source, the_project);
	}
}

void listener_target_started(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							 const void* the_target)
{
	if (NULL != on_target_started)
	{
		on_target_started(source, offset, the_project, the_target);
	}
}

void listener_target_finished(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							  const void* the_target)
{
	if (NULL != on_target_finished)
	{
		on_target_finished(source, offset, the_project, the_target);
	}
}

void listener_task_started(const uint8_t* source, ptrdiff_t offset, const void* the_project,
						   const void* the_target, uint8_t task)
{
	if (NULL != on_task_started)
	{
		on_task_started(source, offset, the_project, the_target, task);
	}
}

void listener_task_finished(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							const void* the_target, uint8_t task, uint8_t result)
{
	if (NULL != on_task_finished)
	{
		on_task_finished(source, offset, the_project, the_target, task, result);
	}
}

void listener_set_on_project_started(void (*listener_on_project_started)(const uint8_t* source,
									 const void* the_project))
{
	on_project_started = listener_on_project_started;
}

void listener_set_on_project_finished(void (*listener_on_project_finished)(const uint8_t* source,
									  const void* the_project))
{
	on_project_finished = listener_on_project_finished;
}

void listener_set_on_target_started(void (*listener_on_target_started)(const uint8_t* source,
									ptrdiff_t offset, const void* the_project, const void* the_target))
{
	on_target_started = listener_on_target_started;
}

void listener_set_on_target_finished(void (*listener_on_target_finished)(const uint8_t* source,
									 ptrdiff_t offset, const void* the_project, const void* the_target))
{
	on_target_finished = listener_on_target_finished;
}

void listener_set_on_task_started(void (*listener_on_task_started)(const uint8_t* source, ptrdiff_t offset,
								  const void* the_project, const void* the_target, uint8_t task))
{
	on_task_started = listener_on_task_started;
}

void listener_set_on_task_finished(void (*listener_on_task_finished)(const uint8_t* source, ptrdiff_t offset,
								   const void* the_project, const void* the_target, uint8_t task, uint8_t result))
{
	on_task_finished = listener_on_task_finished;
}
