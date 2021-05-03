/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 TheVice
 *
 */

#ifndef _LOAD_FILE_H_
#define _LOAD_FILE_H_

#include <stdint.h>

enum FileEncoding
{
	ISO_8859_1 = 8861, ISO_8859_2, /*ISO_8859_3, ISO_8859_4,
	ISO_8859_5, ISO_8859_6,*/ ISO_8859_7, /*ISO_8859_8,*/ ISO_8859_9,
	ISO_8859_11, ISO_8859_13
};

#define FILE_ENCODING_UNKNOWN (ISO_8859_13 + 1)

struct buffer;

uint8_t load_file_to_buffer(const uint8_t* path, uint16_t encoding, struct buffer* output, uint8_t verbose);
uint16_t load_file_get_encoding(struct buffer* encoding_name);

uint8_t load_file_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t load_file_evaluate_task(void* project, struct buffer* task_arguments, uint8_t verbose);

#endif
