/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _EXEC_H_
#define _EXEC_H_

#include <stdint.h>

struct range;

uint8_t exec(
	uint8_t append,
	const struct range* program,
	const struct range* base_dir,
	const struct range* command_line,
	const struct range* output_file,
	void* pid_property,
	void* result_property,
	const struct range* working_dir,
	const struct range* environment_variables,
	uint8_t spawn,
	uint32_t time_out,
	uint8_t verbose);
uint8_t exec_evaluate_task(void* project, const void* target,
						   const uint8_t* attributes_start, const uint8_t* attributes_finish,
						   const uint8_t* element_finish);

#endif
