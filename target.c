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

uint8_t target_exists(
	const struct buffer* targets,
	const uint8_t* name_start, const uint8_t* name_finish, uint8_t verbose)
{
	return target_get(targets, name_start, name_finish, NULL, verbose);
}

uint8_t target_get_current_target(
	const void* the_target, const uint8_t** name, uint8_t* name_length)
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

uint8_t target_has_executed(
	const struct buffer* targets,
	const uint8_t* name_start, const uint8_t* name_finish, uint8_t verbose)
{
	void* the_target;

	if (!target_get(targets, name_start, name_finish, &the_target, verbose))
	{
		return 0;
	}

	return 0 < ((struct target*)the_target)->has_executed;
}

uint8_t target_print_description(
	void* the_project, struct target* the_target,
	struct buffer* target_description,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	const struct range* sub_nodes_names, uint8_t verbose)
{
	static const uint8_t* n = (const uint8_t*)"\n";
	static const uint8_t* t = (const uint8_t*)"\t";

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
		if (!echo(0, Default, NULL, Info, t, 1, 0, verbose))
		{
			return 0;
		}

		return echo(0, UTF8, NULL, Info, buffer_data(target_description, 0),
					buffer_size(target_description), 1, verbose);
	}

	if (range_in_parts_is_null_or_empty(attributes_finish, element_finish))
	{
		return echo(0, Default, NULL, Info, n, 1, 0, verbose);
	}

	static const uint8_t* description_str = (const uint8_t*)"description\0";
	struct range sub_node_name;
	sub_node_name.start = description_str;
	sub_node_name.finish = sub_node_name.start + 12;

	if (!xml_get_sub_nodes_elements(attributes_finish, element_finish, &sub_node_name, &the_target->tasks))
	{
		return echo(0, Default, NULL, Info, n, 1, 0, verbose);
	}

	if (!echo(0, Default, NULL, Info, t, 1, 0, verbose))
	{
		return 0;
	}

	return interpreter_evaluate_tasks(the_project, the_target, &the_target->tasks, sub_nodes_names, 1, verbose);
}

uint8_t target_set_name(struct target* the_target, const uint8_t* name, ptrdiff_t name_length)
{
	if (NULL == the_target ||
		NULL == name ||
		name_length < 1 ||
		UINT8_MAX < name_length)
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
	the_target->name_length = (uint8_t)name_length;
	/**/
	return 1;
}

uint8_t target_new(
	void* the_project,
	const uint8_t* name_start, const uint8_t* name_finish,
	const uint8_t* depends_start, const uint8_t* depends_finish,
	struct buffer* description,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	struct buffer* targets,
	const struct range* sub_nodes_names, uint8_t verbose)
{
	(void)verbose;

	if (range_in_parts_is_null_or_empty(name_start, name_finish) ||
		NULL == targets)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(targets);
	struct target* the_target;

	if (!buffer_append_target(targets, NULL, 1) ||
		NULL == (the_target = (struct target*)buffer_data(targets, size)))
	{
		return 0;
	}

	SET_NULL_TO_BUFFER(the_target->depends);
	SET_NULL_TO_BUFFER(the_target->tasks);

	if (!target_set_name(the_target, name_start, name_finish - name_start))
	{
		return 0;
	}

	the_target->has_executed = 0;

	if (!range_in_parts_is_null_or_empty(depends_start, depends_finish))
	{
		ptrdiff_t index = 0;
		ptrdiff_t depends_count = 1;
		const uint8_t* pos = depends_start;

		while (depends_finish !=
			   (pos = string_find_any_symbol_like_or_not_like_that(
						  pos, depends_finish, &depends_delimiter, &depends_delimiter + 1, 1, 1)))
		{
			pos = string_enumerate(pos, depends_finish, NULL);
			++depends_count;
		}

		if (!buffer_append_range(&the_target->depends, NULL, depends_count) ||
			!buffer_append(&the_target->depends, depends_start, depends_finish - depends_start))
		{
			return 0;
		}

		pos = (const uint8_t*)buffer_range_data(&the_target->depends, depends_count);
		const uint8_t* max_ptr =
			(const uint8_t*)(buffer_data(&the_target->depends, 0) + buffer_size(&the_target->depends));

		if (!buffer_resize(&the_target->depends, depends_count * sizeof(struct range)))
		{
			return 0;
		}

		index = 0;
		struct range* depend;

		while (NULL != (depend = buffer_range_data(&the_target->depends, index++)))
		{
			depend->start = pos;
			depend->finish = string_find_any_symbol_like_or_not_like_that(
								 pos, max_ptr, &depends_delimiter,
								 &depends_delimiter + 1, 1, 1);
			pos = string_enumerate(depend->finish, max_ptr, NULL);
			pos = string_find_any_symbol_like_or_not_like_that(
					  pos, max_ptr, &depends_delimiter,
					  &depends_delimiter + 1, 0, 1);

			if (!string_trim(depend) ||
				!range_size(depend))
			{
				return 0;
			}

			if (max_ptr < pos)
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
		if (!xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, &the_target->tasks))
		{
			if (!buffer_resize(&the_target->tasks, 0))
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

uint8_t target_get(
	const struct buffer* targets,
	const uint8_t* name_start, const uint8_t* name_finish,
	void** the_target, uint8_t verbose)
{
	if (NULL == targets ||
		range_in_parts_is_null_or_empty(name_start, name_finish))
	{
		return 0;
	}

	ptrdiff_t i = 0;
	struct target* the_real_target;

	while (NULL != (the_real_target = buffer_target_data(targets, i++)))
	{
		const uint8_t* pos =
			the_real_target->name + the_real_target->name_length;

		if (string_equal(the_real_target->name, pos, name_start, name_finish))
		{
			if (NULL != the_target)
			{
				*the_target = the_real_target;
			}

			return 1;
		}
	}

	(void)verbose;/*TODO*/
	return 0;
}

const struct range* target_get_depend(const void* the_target, ptrdiff_t index)
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
	struct target* the_target;

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

uint8_t target_is_in_a_stack(const struct buffer* stack, const void* the_target)
{
	if (NULL == stack ||
		NULL == the_target)
	{
		return 0;
	}

	ptrdiff_t index = 0;
	const void** target_in_a_stack;

	while (NULL != (target_in_a_stack = (const void**)buffer_data(stack, index)))
	{
		if (the_target == *target_in_a_stack)
		{
			return 1;
		}

		index += sizeof(void**);
	}

	return 0;
}

uint8_t target_evaluate(
	void* the_project, void* the_target, struct buffer* stack,
	uint8_t cascade, uint8_t verbose)
{
	listener_target_started(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, verbose);

	if (NULL == the_project ||
		NULL == the_target ||
		NULL == stack)
	{
		listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 0, verbose);
		return 0;
	}

	struct target* the_real_target = (struct target*)the_target;

	if (target_is_in_a_stack(stack, the_target))
	{
		listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 0, verbose);
		return 2 == the_real_target->has_executed ? 0 : 1;
	}

	uint8_t skip = 0;

	if (cascade)
	{
		skip = the_real_target->has_executed;
		the_real_target->has_executed = 2;

		if (!buffer_append(stack, (const uint8_t*)&the_target, sizeof(void**)))
		{
			listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 0, verbose);
			return 0;
		}

		ptrdiff_t index = 0;
		const struct range* depend;

		while (NULL != (depend = target_get_depend(the_target, index++)))
		{
			void* target_dep;

			if (!project_target_get(the_project, depend->start, depend->finish, &target_dep, verbose))
			{
				listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 0, verbose);
				return 0;
			}

			if (the_target == target_dep)
			{
				listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 0, verbose);
				return 0;
			}

			if (!target_evaluate(the_project, target_dep, stack, cascade, verbose))
			{
				listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 0, verbose);
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
		listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 0, verbose);
		return 0;
	}

	buffer_release_with_inner_buffers(&tmp);

	if (skip)
	{
		listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 1, verbose);
		return 1;
	}

	if (!interpreter_evaluate_tasks(the_project, the_target, &the_real_target->tasks, NULL, 0, verbose))
	{
		listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, 0, verbose);
		return 0;
	}

	the_real_target->has_executed = 1;
	skip = cascade ? 1 : buffer_append(stack, (const uint8_t*)&the_target, sizeof(void**));
	listener_target_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, skip, verbose);
	/**/
	return skip;
}

uint8_t target_evaluate_by_name(
	void* the_project,
	const uint8_t* name_start, const uint8_t* name_finish,
	uint8_t verbose)
{
	if (NULL == the_project ||
		range_in_parts_is_null_or_empty(name_start, name_finish))
	{
		return 0;
	}

	void* the_target;

	if (!project_target_get(the_project, name_start, name_finish, &the_target, verbose))
	{
		static const uint8_t asterisk = '*';

		if (!project_target_get(the_project, &asterisk, &asterisk + 1, &the_target, verbose))
		{
			return 0;
		}

		if (!target_set_name((struct target*)the_target, name_start, name_finish - name_start))
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
