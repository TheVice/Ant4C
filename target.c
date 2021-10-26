/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "target.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "echo.h"
#include "interpreter.h"
#include "listener.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"
#include "xml.h"

#include <stddef.h>
#include <string.h>

static const uint8_t depends_delimiter = ',';

struct target
{
	uint8_t name[UINT8_MAX + 1];
	uint8_t name_length;
	/**/
	struct range attributes;
	/**/
	struct buffer depends;
	struct buffer tasks;
	/**/
	uint8_t has_executed;
};

uint8_t buffer_append_target(struct buffer* targets, const struct target* data, ptrdiff_t data_count)
{
	return buffer_append(targets, (const uint8_t*)data, sizeof(struct target) * data_count);
}

struct target* buffer_target_data(const struct buffer* targets, ptrdiff_t data_position)
{
	return (struct target*)buffer_data(targets, sizeof(struct target) * data_position);
}

uint8_t target_exists(const struct buffer* targets,
					  const uint8_t* name, uint8_t name_length)
{
	return target_get(targets, name, name_length, NULL);
}

uint8_t target_get_current_target(const void* the_target, const uint8_t** name, uint8_t* name_length)
{
	if (NULL == the_target ||
		NULL == name)
	{
		return 0;
	}

	/*TODO: add validation at project.*/
	const struct target* the_real_target = (const struct target*)the_target;
	*name = the_real_target->name;

	if (NULL != name_length)
	{
		*name_length = the_real_target->name_length;
	}

	return 1;
}

uint8_t target_has_executed(const struct buffer* targets,
							const uint8_t* name, uint8_t name_length)
{
	void* the_target = NULL;

	if (!target_get(targets, name, name_length, &the_target))
	{
		return 0;
	}

	return 0 < ((struct target*)the_target)->has_executed;
}

uint8_t target_print_description(void* the_project, struct target* the_target,
								 struct buffer* target_description,
								 const uint8_t* attributes_finish, const uint8_t* element_finish,
								 const struct buffer* sub_nodes_names, uint8_t verbose)
{
	if (NULL == the_target ||
		NULL == target_description)
	{
		return 0;
	}

	if (!echo(0, UTF8, NULL, Info, the_target->name, the_target->name_length, 0, verbose))
	{
		return 0;
	}

	if (buffer_size(target_description))
	{
		if (!echo(0, Default, NULL, Info, (const uint8_t*)"\t", 1, 0, verbose))
		{
			return 0;
		}

		return echo(0, UTF8, NULL, Info, buffer_data(target_description, 0),
					buffer_size(target_description), 1, verbose);
	}

	if (range_in_parts_is_null_or_empty(attributes_finish, element_finish))
	{
		return echo(0, Default, NULL, Info, (const uint8_t*)"\n", 1, 0, verbose);
	}

	if (!range_from_string((const uint8_t*)"description\0", 12, 1, target_description) ||
		!buffer_resize(&(the_target->tasks), 0))
	{
		return 0;
	}

	if (!xml_get_sub_nodes_elements(attributes_finish, element_finish, target_description, &(the_target->tasks)))
	{
		return echo(0, Default, NULL, Info, (const uint8_t*)"\n", 1, 0, verbose);
	}

	if (!echo(0, Default, NULL, Info, (const uint8_t*)"\t", 1, 0, verbose))
	{
		return 0;
	}

	return interpreter_evaluate_tasks(the_project, the_target, &(the_target->tasks), sub_nodes_names, 1, verbose);
}

uint8_t target_set_name(struct target* the_target, const uint8_t* name, uint8_t name_length)
{
	if (NULL == the_target ||
		NULL == name ||
		name_length < 1)
	{
		return 0;
	}

#if __STDC_LIB_EXT1__

	if (0 != memcpy_s(the_target->name, UINT8_MAX, name, name_length))
	{
		return 0;
	}

#else
	memcpy(the_target, name, name_length);
#endif
	the_target->name[name_length] = '\0';
	the_target->name_length = name_length;
	/**/
	return 1;
}

uint8_t target_new(void* the_project,
				   const uint8_t* name, uint8_t name_length,
				   const uint8_t* depends, uint16_t depends_length,
				   struct buffer* description,
				   const uint8_t* attributes_start, const uint8_t* attributes_finish,
				   const uint8_t* element_finish,
				   struct buffer* targets,
				   const struct buffer* sub_nodes_names, uint8_t verbose)
{
	(void)verbose;

	if (NULL == name ||
		name_length < 1 ||
		NULL == targets)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(targets);
	struct target* the_target = NULL;

	if (!buffer_append_target(targets, NULL, 1) ||
		NULL == (the_target = (struct target*)buffer_data(targets, size)))
	{
		return 0;
	}

	SET_NULL_TO_BUFFER(the_target->depends);
	SET_NULL_TO_BUFFER(the_target->tasks);

	if (!target_set_name(the_target, name, name_length))
	{
		return 0;
	}

	the_target->has_executed = 0;

	if (depends && 0 < depends_length)
	{
		ptrdiff_t index = 0;
		uint16_t depends_count = 1;
		const uint8_t* depend_name = depends;
		const uint8_t* depends_finish = depends + depends_length;

		while (depends_finish !=
			   (depend_name = find_any_symbol_like_or_not_like_that(
								  depend_name, depends_finish, &depends_delimiter, 1, 1, 1)))
		{
			++depend_name;
			++depends_count;
		}

		if (!buffer_append_range(&(the_target->depends), NULL, depends_count) ||
			!buffer_append(&(the_target->depends), depends, depends_length))
		{
			return 0;
		}

		depend_name = (const uint8_t*)buffer_range_data(&(the_target->depends), depends_count);
		const uint8_t* max_ptr =
			(const uint8_t*)(buffer_data(&(the_target->depends), 0) + buffer_size(&(the_target->depends)));

		if (!buffer_resize(&(the_target->depends), depends_count * sizeof(struct range)))
		{
			return 0;
		}

		index = 0;
		struct range* depend = NULL;

		while (NULL != (depend = buffer_range_data(&(the_target->depends), index++)))
		{
			depend->start = depend_name;
			depend->finish = find_any_symbol_like_or_not_like_that(depend_name, max_ptr, &depends_delimiter, 1, 1, 1);
			depend_name = find_any_symbol_like_or_not_like_that(depend->finish + 1, max_ptr, &depends_delimiter, 1, 0, 1);

			if (!string_trim(depend) ||
				!range_size(depend))
			{
				return 0;
			}

			if (max_ptr < depend_name)
			{
				break;
			}
		}
	}

	if (description)
	{
		the_target->attributes.start = the_target->attributes.finish = NULL;

		if (!target_print_description(the_project, the_target, description, attributes_start, element_finish,
									  sub_nodes_names, verbose))
		{
			return 0;
		}
	}
	else
	{
		if (!xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, &(the_target->tasks)))
		{
			if (!buffer_resize(&(the_target->tasks), 0))
			{
				return 0;
			}
		}

		if (range_in_parts_is_null_or_empty(attributes_start, attributes_finish))
		{
			the_target->attributes.start = the_target->attributes.finish = NULL;
		}
		else
		{
			the_target->attributes.start = attributes_start;
			the_target->attributes.finish = attributes_finish;
		}
	}

	return 1;
}

uint8_t target_get(const struct buffer* targets, const uint8_t* name,
				   uint8_t name_length, void** the_target)
{
	if (NULL == targets ||  NULL == name || 0 == name_length)
	{
		return 0;
	}

	ptrdiff_t i = 0;
	struct target* the_real_target = NULL;

	while (NULL != (the_real_target = buffer_target_data(targets, i++)))
	{
		if (name_length == the_real_target->name_length &&
			0 == memcmp(&the_real_target->name, name, name_length))
		{
			if (NULL != the_target)
			{
				*the_target = the_real_target;
			}

			return 1;
		}
	}

	return 0;
}

const struct range* target_get_depend(const void* the_target, uint16_t index)
{
	if (NULL == the_target)
	{
		return 0;
	}

	return buffer_range_data(&((const struct target*)the_target)->depends, index);
}

void target_release_inner(struct buffer* targets)
{
	if (NULL == targets)
	{
		return;
	}

	ptrdiff_t i = 0;
	struct target* the_target = NULL;

	while (NULL != (the_target = buffer_target_data(targets, i++)))
	{
		buffer_release(&the_target->depends);
		buffer_release(&the_target->tasks);
	}
}

void target_release(struct buffer* targets)
{
	if (NULL == targets)
	{
		return;
	}

	target_release_inner(targets);
	buffer_release(targets);
}

const uint8_t* target_attributes[] =
{
	(const uint8_t*)"name",
	(const uint8_t*)"depends",
	(const uint8_t*)"description",
};

const uint8_t target_attributes_lengths[] =
{
	4,
	7,
	11
};

#define NAME_POSITION			0
#define DEPENDS_POSITION		1
#define DESCRIPTION_POSITION	2

uint8_t target_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   target_attributes, target_attributes_lengths,
			   COUNT_OF(target_attributes_lengths),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t target_evaluate_task(void* the_project, struct buffer* task_arguments, uint8_t target_help,
							 const uint8_t* attributes_start, const uint8_t* attributes_finish,
							 const uint8_t* element_finish,
							 const struct buffer* sub_nodes_names, uint8_t verbose)
{
	if (NULL == the_project || NULL == task_arguments)
	{
		return 0;
	}

	const struct buffer* name = buffer_buffer_data(task_arguments, NAME_POSITION);

	if (!buffer_size(name))
	{
		return 0;
	}

	const struct buffer* depends = buffer_buffer_data(task_arguments, DEPENDS_POSITION);
	struct buffer* description = target_help ? buffer_buffer_data(task_arguments, DESCRIPTION_POSITION) : NULL;
	/**/
	return project_target_new(the_project,
							  buffer_data(name, 0), (uint8_t)buffer_size(name),
							  buffer_data(depends, 0), (uint16_t)buffer_size(depends),
							  description, attributes_start, attributes_finish, element_finish,
							  sub_nodes_names, verbose);
}

uint8_t target_is_in_stack(const struct buffer* stack, const void* the_target)
{
	if (NULL == stack ||
		NULL == the_target)
	{
		return 0;
	}

	ptrdiff_t index = 0;
	const void** ptr = NULL;

	while (NULL != (ptr = (const void**)buffer_data(stack, index)))
	{
		if (the_target == *ptr)
		{
			return 1;
		}

		index += sizeof(void**);
	}

	return 0;
}

uint8_t target_evaluate(void* the_project, void* the_target, struct buffer* stack,
						uint8_t cascade, uint8_t verbose)
{
	listener_target_started(NULL, 0, the_project, the_target, verbose);

	if (NULL == the_project ||
		NULL == the_target ||
		NULL == stack)
	{
		listener_target_finished(NULL, 0, the_project, the_target, 0, verbose);
		return 0;
	}

	struct target* the_real_target = (struct target*)the_target;

	if (target_is_in_stack(stack, the_target))
	{
		listener_target_finished(NULL, 0, the_project, the_target, 0, verbose);
		return 2 == the_real_target->has_executed ? 0 : 1;
	}

	uint8_t skip = 0;

	if (cascade)
	{
		skip = the_real_target->has_executed;
		the_real_target->has_executed = 2;

		if (!buffer_append(stack, (const uint8_t*)&the_target, sizeof(void**)))
		{
			listener_target_finished(NULL, 0, the_project, the_target, 0, verbose);
			return 0;
		}

		uint16_t index = 0;
		const struct range* depend = NULL;

		while (NULL != (depend = target_get_depend(the_target, index++)))
		{
			void* target_dep = NULL;

			if (!project_target_get(the_project, depend->start, (uint8_t)range_size(depend), &target_dep, verbose))
			{
				listener_target_finished(NULL, 0, the_project, the_target, 0, verbose);
				return 0;
			}

			if (the_target == target_dep)
			{
				listener_target_finished(NULL, 0, the_project, the_target, 0, verbose);
				return 0;
			}

			if (!target_evaluate(the_project, target_dep, stack, cascade, verbose))
			{
				listener_target_finished(NULL, 0, the_project, the_target, 0, verbose);
				return 0;
			}
		}

		the_real_target->has_executed = skip;
	}

	skip = 0;
	struct buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	if (!interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
			the_project, the_target, the_real_target->attributes.start,
			the_real_target->attributes.finish, &skip, &tmp, verbose))
	{
		buffer_release_with_inner_buffers(&tmp);
		listener_target_finished(NULL, 0, the_project, the_target, 0, verbose);
		return 0;
	}

	buffer_release_with_inner_buffers(&tmp);

	if (skip)
	{
		listener_target_finished(NULL, 0, the_project, the_target, 1, verbose);
		return 1;
	}

	if (!interpreter_evaluate_tasks(the_project, the_target, &(the_real_target->tasks), NULL, 0, verbose))
	{
		listener_target_finished(NULL, 0, the_project, the_target, 0, verbose);
		return 0;
	}

	the_real_target->has_executed = 1;
	skip = cascade ? 1 : buffer_append(stack, (const uint8_t*)&the_target, sizeof(void**));
	listener_target_finished(NULL, 0, the_project, the_target, skip, verbose);
	/**/
	return skip;
}

uint8_t target_evaluate_by_name(void* the_project, const struct range* target_name, uint8_t verbose)
{
	if (NULL == the_project ||
		range_is_null_or_empty(target_name))
	{
		return 0;
	}

	void* the_target = NULL;
	const uint8_t name_length = (uint8_t)range_size(target_name);

	if (!project_target_get(the_project, target_name->start, name_length, &the_target, verbose))
	{
		static const uint8_t asterisk = '*';

		if (!project_target_get(the_project, &asterisk, 1, &the_target, verbose))
		{
			return 0;
		}

		if (!target_set_name((struct target*)the_target, target_name->start, name_length))
		{
			return 0;
		}
	}

	struct buffer stack;

	SET_NULL_TO_BUFFER(stack);

	if (!target_evaluate(the_project, the_target, &stack, 1, verbose))
	{
		buffer_release(&stack);
		return 0;
	}

	buffer_release(&stack);
	return 1;
}

enum target_function
{
	target_exists_,
	get_current_target_,
	has_executed_,
	UNKNOWN_TARGET_FUNCTION
};

static const uint8_t* target_function_str[] =
{
	(const uint8_t*)"exists",
	(const uint8_t*)"get-current-target",
	(const uint8_t*)"has-executed"
};

uint8_t target_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, target_function_str, UNKNOWN_TARGET_FUNCTION);
}

uint8_t target_exec_function(const void* the_project,
							 const void* the_target,
							 uint8_t function, const struct buffer* arguments,
							 uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_TARGET_FUNCTION <= function ||
		NULL == arguments ||
		(0 != arguments_count && 1 != arguments_count) ||
		NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (arguments_count && !common_get_arguments(arguments, arguments_count, &argument, 0))
	{
		return 0;
	}

	switch (function)
	{
		case target_exists_:
			return arguments_count &&
				   bool_to_string(project_target_exists(the_project, argument.start, (uint8_t)range_size(&argument)),
								  output);

		case get_current_target_:
			return !arguments_count &&
				   target_get_current_target(the_target, &argument.start, &function) &&
				   buffer_append(output, argument.start, function);

		case has_executed_:
			return arguments_count && bool_to_string(
					   project_target_has_executed(the_project, argument.start, (uint8_t)range_size(&argument)), output);

		case UNKNOWN_TARGET_FUNCTION:
		default:
			break;
	}

	return 0;
}

#define CALL_TARGET_POSITION		0
#define CALL_CASCADE_POSITION		1

static const uint8_t* call_attributes[] =
{
	(const uint8_t*)"target",
	(const uint8_t*)"cascade"
};

static const uint8_t call_attributes_lengths[] = { 6, 7 };

uint8_t call_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   call_attributes, call_attributes_lengths,
			   COUNT_OF(call_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t call_evaluate_task(void* the_project, struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == the_project ||
		NULL == task_arguments)
	{
		return 0;
	}

	const struct buffer* target_name_in_a_buffer = buffer_buffer_data(task_arguments, CALL_TARGET_POSITION);

	if (!buffer_size(target_name_in_a_buffer))
	{
		return 0;
	}

	void* the_target = NULL;

	if (!project_target_get(the_project, buffer_data(target_name_in_a_buffer, 0),
							(uint8_t)buffer_size(target_name_in_a_buffer), &the_target, verbose))
	{
		return 0;
	}

	uint8_t cascade_value = 1;
	struct buffer* cascade_in_a_buffer = buffer_buffer_data(task_arguments, CALL_CASCADE_POSITION);

	if (buffer_size(cascade_in_a_buffer))
	{
		if (!bool_parse(buffer_data(cascade_in_a_buffer, 0), buffer_size(cascade_in_a_buffer), &cascade_value) ||
			!buffer_resize(cascade_in_a_buffer, 0))
		{
			return 0;
		}
	}

	return target_evaluate(the_project, the_target, cascade_in_a_buffer, cascade_value, verbose);
}
