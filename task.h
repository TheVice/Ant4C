/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>

struct buffer;

uint8_t task_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t task_exec_function(const void* the_project, uint8_t function, const struct buffer* arguments,
						   uint8_t arguments_count, struct buffer* output);

#endif
