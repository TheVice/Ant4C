/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, 2022 TheVice
 *
 */

#ifndef _TRY_CATCH_H_
#define _TRY_CATCH_H_

#include <stdint.h>

uint8_t try_catch_evaluate_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	void* task_arguments, uint8_t verbose);

#endif
