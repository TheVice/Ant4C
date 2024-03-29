/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2022 TheVice
 *
 */

#include "if_task.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "interpreter.h"
#include "xml.h"

#include <stddef.h>

#define IF_TEST_POSITION		0

uint8_t if_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	static const uint8_t* test = (const uint8_t*)"test";
	static const uint8_t test_length = 4;
	return common_get_attributes_and_arguments_for_task(
			   &test, &test_length, 1,
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t if_evaluate_task(
	void* the_project, const void* the_target, void* task_arguments,
	const uint8_t* attributes_finish, const uint8_t* element_finish, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	void* test_in_a_buffer = buffer_buffer_data(task_arguments, IF_TEST_POSITION);

	if (!buffer_size(test_in_a_buffer))
	{
		return 1;
	}

	const uint8_t* value = buffer_uint8_t_data(test_in_a_buffer, 0);
	uint8_t test_value = (uint8_t)buffer_size(test_in_a_buffer);

	if (!bool_parse(value, value + test_value, &test_value))
	{
		return 0;
	}

	if (!test_value)
	{
		return 1;
	}

	if (!buffer_resize(test_in_a_buffer, 0))
	{
		return 0;
	}

	if (!xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, test_in_a_buffer))
	{
		return 1;
	}

	return interpreter_evaluate_tasks(the_project, the_target, test_in_a_buffer, NULL, 0, verbose);
}
