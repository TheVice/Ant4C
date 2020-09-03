/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "fail_task.h"
#include "buffer.h"
#include "common.h"
#include "echo.h"
#include "interpreter.h"
#include "text_encoding.h"

#include <stddef.h>

#define FAIL_MESSAGE_POSITION		0

uint8_t fail_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	static const uint8_t* fail_attributes = (const uint8_t*)"message";
	static const uint8_t fail_attributes_lengths = 7;
	/**/
	return common_get_attributes_and_arguments_for_task(
			   &fail_attributes, &fail_attributes_lengths, 1,
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t fail_evaluate_task(
	const void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	struct buffer* task_arguments, uint8_t verbose)
{
	struct buffer* message = buffer_buffer_data(task_arguments, FAIL_MESSAGE_POSITION);

	if (!buffer_size(message))
	{
		if (!interpreter_get_xml_element_value(the_project, the_target, attributes_finish, element_finish, message,
											   verbose))
		{
			return 0;
		}
	}

	if (buffer_size(message))
	{
		echo(0, UTF8, NULL, Error, buffer_data(message, 0), buffer_size(message), 1, verbose);
	}

	return 0;
}
