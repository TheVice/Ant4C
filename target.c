/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "target.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"

#include <stddef.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

/*static const uint8_t depends_delimiter = ',';*/

struct target
{
	uint8_t name[UINT8_MAX + 1];
	uint8_t name_length;
	/**/
	struct buffer depends;
	struct buffer content;
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

uint8_t target_get_current_target(const void* target, const uint8_t** name, uint8_t* name_length)
{
	if (NULL == target)
	{
		return 0;
	}

	const struct target* target_ = (const struct target*)target;
	*name = target_->name;
	*name_length = target_->name_length;
	return 1;
}

uint8_t target_has_executed(const struct buffer* targets,
							const uint8_t* name, uint8_t name_length)
{
	void* target_ = NULL;

	if (!target_get(targets, name, name_length, &target_))
	{
		return 0;
	}

	return 0 < ((struct target*)target_)->has_executed;
}

uint8_t target_new(const struct range* name, const struct range* depends, const struct range* content,
				   struct buffer* targets)
{
	if (range_is_null_or_empty(name) ||
		UINT8_MAX < range_size(name) ||
		NULL == targets)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(targets);
	struct target* target = NULL;

	if (!buffer_append_target(targets, NULL, 1) ||
		NULL == (target = (struct target*)buffer_data(targets, size)))
	{
		return 0;
	}

	SET_NULL_TO_BUFFER(target->depends);
	SET_NULL_TO_BUFFER(target->content);
	target->name_length = (uint8_t)range_size(name);
#if __STDC_SEC_API__

	if (0 != memcpy_s(target->name, UINT8_MAX, name->start, target->name_length))
	{
		return 0;
	}

#else
	memcpy(target, name->start, target->name_length);
#endif
	target->name[target->name_length] = '\0';

	if (!range_is_null_or_empty(depends))
	{
		if (!buffer_append_data_from_range(&(target->depends), depends))
		{
			return 0;
		}
	}

	if (!range_is_null_or_empty(content))
	{
		if (!buffer_append_data_from_range(&(target->content), content))
		{
			return 0;
		}
	}

	target->has_executed = 0;
	/**/
	return 1;
}

uint8_t target_get(const struct buffer* targets, const uint8_t* name,
				   uint8_t name_length, void** target)
{
	if (NULL == targets ||  NULL == name || 0 == name_length)
	{
		return 0;
	}

	ptrdiff_t i = 0;
	struct target* target_ = NULL;

	while (NULL != (target_ = buffer_target_data(targets, i++)))
	{
		if (name_length == target_->name_length &&
			0 == memcmp(&target_->name, name, name_length))
		{
			if (NULL != target)
			{
				(*target) = target_;
			}

			return 1;
		}
	}

	return 0;
}

void target_clear(struct buffer* targets)
{
	ptrdiff_t i = 0;
	struct target* target = NULL;

	if (NULL == targets)
	{
		return;
	}

	while (NULL != (target = buffer_target_data(targets, i++)))
	{
		buffer_release(&target->depends);
		buffer_release(&target->content);
	}

	buffer_release(targets);
}

const uint8_t* target_attributes[] =
{
	(const uint8_t*)"name",
	(const uint8_t*)"depends"
};

const uint8_t target_attributes_lengths[] =
{
	4,
	7
};

#define NAME_POSITION			0
#define DEPENDS_POSITION		1

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

uint8_t target_evaluate_task(void* project, const struct buffer* task_arguments,
							 const uint8_t* attributes_finish, const uint8_t* element_finish,
							 uint8_t verbose)
{
	(void)verbose;

	if (NULL == project || NULL == task_arguments)
	{
		return 0;
	}

	const struct buffer* name = buffer_buffer_data(task_arguments, NAME_POSITION);

	if (!buffer_size(name))
	{
		return 0;
	}

	struct range name_in_range;

	name_in_range.start = buffer_data(name, 0);

	name_in_range.finish = name_in_range.start + buffer_size(name);

	const struct buffer* depends = buffer_buffer_data(task_arguments, DEPENDS_POSITION);

	struct range depends_in_range;

	depends_in_range.start = depends_in_range.finish = NULL;

	if (buffer_size(depends))
	{
		depends_in_range.start = buffer_data(depends, 0);
		depends_in_range.finish = depends_in_range.start + buffer_size(depends);
	}

	struct range content;

	content.start = content.finish = NULL;

	if (!range_in_parts_is_null_or_empty(attributes_finish, element_finish))
	{
		content.start = attributes_finish;
		content.finish = element_finish;
	}

	return project_target_new(project, &name_in_range, &depends_in_range, &content);
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

uint8_t target_exec_function(const void* project,
							 const void* target,
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

	argument.start = argument.finish = NULL;

	if (arguments_count && !common_get_one_argument(arguments, &argument, 0))
	{
		return 0;
	}

	switch (function)
	{
		case target_exists_:
			return bool_to_string(project_target_exists(project, argument.start, (uint8_t)range_size(&argument)), output);

		case get_current_target_:
			return !arguments_count &&
				   target_get_current_target(target, &argument.start, &function) &&
				   buffer_append(output, argument.start, function);

		case has_executed_:
			return bool_to_string(
					   project_target_has_executed(project, argument.start, (uint8_t)range_size(&argument)), output);

		case UNKNOWN_TARGET_FUNCTION:
		default:
			break;
	}

	return 0;
}
