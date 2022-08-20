/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include "interpreter.property.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "project.h"
#include "property.h"
#include "range.h"

#define DYNAMIC_POSITION		0
#define NAME_POSITION			1
#define OVER_WRITE_POSITION		2
#define READ_ONLY_POSITION		3
#define VALUE_POSITION			4

uint8_t property_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	static const uint8_t* property_attributes[] =
	{
		(const uint8_t*)"dynamic",
		(const uint8_t*)"name",
		(const uint8_t*)"overwrite",
		(const uint8_t*)"readonly",
		(const uint8_t*)"value"
	};
	/**/
	static const uint8_t property_attributes_lengths[] = { 7, 4, 9, 8, 5 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   property_attributes, property_attributes_lengths,
			   COUNT_OF(property_attributes_lengths),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t property_evaluate_task(
	void* the_project, void* properties, const void* task_arguments, uint8_t verbose)
{
	if ((NULL == the_project && NULL == properties) ||
		NULL == task_arguments)
	{
		return 0;
	}

	const void* name = buffer_buffer_data(task_arguments, NAME_POSITION);
	const uint8_t name_length = (uint8_t)buffer_size(name);

	if (!name_length)
	{
		return 0;
	}

	const void* dynamic_in_a_buffer = buffer_buffer_data(task_arguments, DYNAMIC_POSITION);
	const void* over_write_in_a_buffer = buffer_buffer_data(task_arguments, OVER_WRITE_POSITION);
	const void* read_only_in_a_buffer = buffer_buffer_data(task_arguments, READ_ONLY_POSITION);
	/**/
	const uint8_t* value = buffer_data(dynamic_in_a_buffer, 0);
	uint8_t dynamic = (uint8_t)buffer_size(dynamic_in_a_buffer);

	if (dynamic && !bool_parse(value, value + dynamic, &dynamic))
	{
		return 0;
	}

	value = buffer_data(over_write_in_a_buffer, 0);
	uint8_t over_write = (uint8_t)buffer_size(over_write_in_a_buffer);

	if (over_write && !bool_parse(value, value + over_write, &over_write))
	{
		return 0;
	}
	else
	{
		over_write = 1;
	}

	value = buffer_data(read_only_in_a_buffer, 0);
	uint8_t read_only = (uint8_t)buffer_size(read_only_in_a_buffer);

	if (read_only && !bool_parse(value, value + read_only, &read_only))
	{
		return 0;
	}

	const void* value_in_a_buffer = buffer_buffer_data(task_arguments, VALUE_POSITION);
	value = buffer_data(value_in_a_buffer, 0);

	if (NULL == value)
	{
		value = (const uint8_t*)&value;
	}

	if (the_project)
	{
		return project_property_set_value(
				   the_project,
				   buffer_data(name, 0), name_length,
				   value, buffer_size(value_in_a_buffer),
				   dynamic, over_write, read_only, verbose);
	}

	return property_set_by_name(
			   properties,
			   buffer_data(name, 0), name_length,
			   value, buffer_size(value_in_a_buffer),
			   property_value_is_byte_array,
			   dynamic, over_write, read_only, verbose);
}

enum property_function
{
	property_exists_function,
	property_get_value_function,
	property_is_dynamic_function,
	property_is_readonly_function,
	UNKNOWN_PROPERTY_FUNCTION
};

uint8_t property_get_id_of_get_value_function()
{
	return property_get_value_function;
}

uint8_t property_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* property_str[] =
	{
		(const uint8_t*)"exists",
		(const uint8_t*)"get-value",
		(const uint8_t*)"is-dynamic",
		(const uint8_t*)"is-readonly"
	};
	/**/
	return common_string_to_enum(name_start, name_finish, property_str, UNKNOWN_PROPERTY_FUNCTION);
}

uint8_t property_exec_function(
	const void* the_project, uint8_t function, const void* arguments,
	uint8_t arguments_count, const void** the_property, void* output, uint8_t verbose)
{
	if (UNKNOWN_PROPERTY_FUNCTION <= function ||
		NULL == arguments ||
		1 != arguments_count ||
		NULL == the_property ||
		NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_arguments(arguments, arguments_count, &argument, 0))
	{
		return 0;
	}

	void* non_const_prop = NULL;
	const uint8_t is_exists = project_property_exists(the_project,
							  argument.start, (uint8_t)range_size(&argument), &non_const_prop, verbose);
	*the_property = is_exists ? non_const_prop : NULL;
	const void* prop = *the_property;

	if (function != property_exists_function && !is_exists)
	{
		return 0;
	}

	uint8_t value;

	switch (function)
	{
		case property_exists_function:
			return bool_to_string(is_exists, output);

		case property_get_value_function:
			return is_exists && property_get_by_pointer(prop, output);

		case property_is_dynamic_function:
			return NULL != prop &&
				   property_is_dynamic(prop, &value) &&
				   bool_to_string(value, output);

		case property_is_readonly_function:
			return NULL != prop &&
				   property_is_readonly(prop, &value) &&
				   bool_to_string(value, output);

		case UNKNOWN_PROPERTY_FUNCTION:
		default:
			break;
	}

	return 0;
}
