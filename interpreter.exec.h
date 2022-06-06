/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#ifndef _INTERPRETER_EXEC_H_
#define _INTERPRETER_EXEC_H_

#include <stdint.h>

struct buffer;

uint8_t interpreter_get_environments(
	const void* the_project,
	const void* the_target,
	const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	struct buffer* environments,
	uint8_t verbose);

#endif
