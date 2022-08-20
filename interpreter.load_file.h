/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#ifndef _INTERPRETER_LOAD_FILE_H_
#define _INTERPRETER_LOAD_FILE_H_

#include <stdint.h>

uint16_t load_file_get_encoding(void* encoding_name);

uint8_t load_file_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t load_file_evaluate_task(
	void* project, void* task_arguments, uint8_t verbose);

#endif
