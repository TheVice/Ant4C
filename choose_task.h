/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, 2022 TheVice
 *
 */

#ifndef _CHOOSE_TASK_H_
#define _CHOOSE_TASK_H_

#include <stdint.h>

uint8_t choose_evaluate_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	void* task_arguments, uint8_t verbose);

#endif
