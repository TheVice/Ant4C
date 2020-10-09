/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "task.h"

#include "common.h"
#include "conversion.h"
#include "interpreter.h"
#include "project.h"
#include "range.h"

enum task_function
{
	task_exists,
	UNKNOWN_TASK_FUNCTION
};

static const uint8_t* task_function_str[] =
{
	(const uint8_t*)"exists"
};

uint8_t task_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, task_function_str, UNKNOWN_TASK_FUNCTION);
}

uint8_t task_exec_function(const void* the_project,
						   uint8_t function, const struct buffer* arguments,
						   uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_TASK_FUNCTION <= function ||
		NULL == arguments ||
		1 != arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_arguments(arguments, arguments_count, &argument, 0))
	{
		return 0;
	}

	arguments_count = (interpreter_get_unknown_task_id() != interpreter_get_task(
						   argument.start, argument.finish));

	if (!arguments_count)
	{
		arguments_count = NULL != project_get_task_from_module(the_project, &argument, NULL, NULL);
	}

	return bool_to_string(arguments_count, output);
}
