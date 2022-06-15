/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020, 2022 TheVice
 *
 */

#ifndef _EXEC_H_
#define _EXEC_H_

#include <stdint.h>

struct buffer;
struct range;

uint8_t exec(
	const void* the_project,
	const void* the_target,
	uint8_t append,
	struct buffer* program,
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

#endif
