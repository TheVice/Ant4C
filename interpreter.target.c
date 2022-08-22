/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "target.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "project.h"
#include "range.h"

#include <stdint.h>

#define NAME_POSITION			0
#define DEPENDS_POSITION		1
#define DESCRIPTION_POSITION	2

uint8_t target_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	static const uint8_t* target_attributes[] =
	{
		(const uint8_t*)"name",
		(const uint8_t*)"depends",
		(const uint8_t*)"description",
	};
	/**/
	static const uint8_t target_attributes_lengths[] = { 4, 7, 11 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   target_attributes, target_attributes_lengths,
			   COUNT_OF(target_attributes_lengths),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t target_evaluate_task(
	void* the_project, void* task_arguments, uint8_t target_help,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	const struct range* sub_nodes_names, uint8_t verbose)
{
	if (NULL == the_project || NULL == task_arguments)
	{
		return 0;
	}

	const void* name = buffer_buffer_data(task_arguments, NAME_POSITION);

	if (!buffer_size(name))
	{
		return 0;
	}

	const void* depends = buffer_buffer_data(task_arguments, DEPENDS_POSITION);
	void* description = target_help ? buffer_buffer_data(task_arguments, DESCRIPTION_POSITION) : NULL;
	/**/
	const uint8_t* target_name_start = buffer_uint8_t_data(name, 0);
	const uint8_t* target_name_finish = target_name_start + buffer_size(name);
	/**/
	const uint8_t* target_depends_start = buffer_uint8_t_data(depends, 0);
	const uint8_t* target_depends_finish = target_depends_start + buffer_size(depends);
	/**/
	return project_target_new(
			   the_project,
			   target_name_start, target_name_finish,
			   target_depends_start, target_depends_finish,
			   description,
			   attributes_start, attributes_finish, element_finish,
			   sub_nodes_names, verbose);
}

enum target_function
{
	target_exists_,
	get_current_target_,
	has_executed_,
	UNKNOWN_TARGET_FUNCTION
};

uint8_t target_get_function(
	const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* target_function_str[] =
	{
		(const uint8_t*)"exists",
		(const uint8_t*)"get-current-target",
		(const uint8_t*)"has-executed"
	};
	/**/
	return common_string_to_enum(
			   name_start, name_finish, target_function_str, UNKNOWN_TARGET_FUNCTION);
}

uint8_t target_exec_function(
	const void* the_project, const void* the_target,
	uint8_t function, const void* arguments,
	uint8_t arguments_count, void* output, uint8_t verbose)
{
	if (UNKNOWN_TARGET_FUNCTION <= function ||
		NULL == arguments ||
		(0 != arguments_count && 1 != arguments_count) ||
		NULL == output)
	{
		return 0;
	}

	struct range argument;

	argument.start = argument.finish = NULL;

	if (arguments_count && !common_get_arguments(arguments, arguments_count, &argument, 0))
	{
		return 0;
	}

	switch (function)
	{
		case target_exists_:
			return arguments_count &&
				   bool_to_string(project_target_exists(the_project, argument.start, argument.finish, verbose),
								  output);

		case get_current_target_:
			return !arguments_count &&
				   target_get_current_target(the_target, &argument.start, &function) &&
				   buffer_append(output, argument.start, function);

		case has_executed_:
			return arguments_count && bool_to_string(
					   project_target_has_executed(the_project, argument.start, argument.finish, verbose), output);

		case UNKNOWN_TARGET_FUNCTION:
		default:
			break;
	}

	return 0;
}

#define CALL_TARGET_POSITION		0
#define CALL_CASCADE_POSITION		1

uint8_t call_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	static const uint8_t* call_attributes[] =
	{
		(const uint8_t*)"target",
		(const uint8_t*)"cascade"
	};
	/**/
	static const uint8_t call_attributes_lengths[] = { 6, 7 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   call_attributes, call_attributes_lengths,
			   COUNT_OF(call_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t call_evaluate_task(
	void* the_project, void* task_arguments, uint8_t verbose)
{
	if (NULL == the_project ||
		NULL == task_arguments)
	{
		return 0;
	}

	const void* target_name_in_a_buffer = buffer_buffer_data(
			task_arguments, CALL_TARGET_POSITION);
	uint8_t cascade_value = (uint8_t)buffer_size(target_name_in_a_buffer);

	if (!cascade_value)
	{
		return 0;
	}

	void* the_target;
	const uint8_t* name_start = buffer_uint8_t_data(target_name_in_a_buffer, 0);
	const uint8_t* name_finish = name_start + cascade_value;

	if (!project_target_get(the_project,
							name_start, name_finish, &the_target, verbose))
	{
		return 0;
	}

	void* cascade_in_a_buffer = buffer_buffer_data(
									task_arguments, CALL_CASCADE_POSITION);
	cascade_value = (uint8_t)buffer_size(cascade_in_a_buffer);

	if (cascade_value)
	{
		const uint8_t* value = buffer_uint8_t_data(cascade_in_a_buffer, 0);

		if (!bool_parse(value, value + cascade_value, &cascade_value) ||
			!buffer_resize(cascade_in_a_buffer, 0))
		{
			return 0;
		}
	}
	else
	{
		cascade_value = 1;
	}

	return target_evaluate(
			   the_project, the_target, cascade_in_a_buffer, cascade_value, verbose);
}
