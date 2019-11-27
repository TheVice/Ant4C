/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _ECHO_H_
#define _ECHO_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;

enum Level { Debug, Error, Info, None, Verbose, Warning, NoLevel };

uint8_t echo(uint8_t append, uint8_t encoding, const uint8_t* file, uint8_t level,
			 const uint8_t* message, ptrdiff_t message_length, uint8_t new_line, uint8_t verbose);

uint8_t echo_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t echo_evaluate_task(struct buffer* task_arguments,
						   const uint8_t* attributes_finish, const uint8_t* element_finish);

#endif
