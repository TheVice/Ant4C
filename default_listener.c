/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "default_listener.h"

#include <stdio.h>

void listener_project_started(const uint8_t* source, const uint8_t* the_project)
{
	printf("The project '%s' is started from source '%s'.\n", the_project, source);
}

void listener_project_finished(const uint8_t* source, const uint8_t* the_project,
							   uint8_t result)
{
	printf("The project '%s' is finished from source '%s' with result '%i'.\n", the_project, source, result);
}

void listener_target_started(const uint8_t* source, ptrdiff_t offset,
							 const uint8_t* the_project, const uint8_t* the_target)
{
	printf("\tThe target '%s' is started from project '%s' at offset '%td' from source '%s'.\n",
		   the_target, the_project, offset, source);
}

void listener_target_finished(const uint8_t* source, ptrdiff_t offset,
							  const uint8_t* the_project, const uint8_t* the_target, uint8_t result)
{
	printf("\tThe target '%s' is finished from project '%s' at offset '%td' from source '%s' with result '%i'.\n",
		   the_target, the_project, offset, source, result);
}

void listener_task_started(const uint8_t* source, ptrdiff_t offset,
						   const uint8_t* the_project, const uint8_t* the_target, const uint8_t* task)
{
	printf("%sThe task '%s' is started (current target is '%s') from project '%s' at offset '%td' from source '%s'.\n",
		   NULL == the_target ? "\t" : "\t\t", task, the_target, the_project, offset, source);
}

void listener_task_finished(const uint8_t* source, ptrdiff_t offset,
							const uint8_t* the_project, const uint8_t* the_target, const uint8_t* task, uint8_t result)
{
	printf("%sThe task '%s' is finished (current target is '%s') from project '%s' at offset '%td' from source '%s' with result '%i'.\n",
		   NULL == the_target ? "\t" : "\t\t", task, the_target, the_project, offset, source, result);
}
