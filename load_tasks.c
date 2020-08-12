/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "load_tasks.h"

#include "buffer.h"
#include "common.h"
#include "interpreter.h"
#include "path.h"
#include "project.h"
#include "range.h"
#include "shared_object.h"
#include "xml.h"

#include <stddef.h>
#include <string.h>

#define ASSEMBLY_POSITION	0
#define PATH_POSITION		1
#define MODULE_POSITION		2

static const uint8_t* attributes[] =
{
	(const uint8_t*)"assembly",
	(const uint8_t*)"path",
	(const uint8_t*)"module"
};

static const uint8_t attributes_lengths[] = { 8, 4, 6 };

uint8_t load_tasks_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   attributes, attributes_lengths,
			   COUNT_OF(attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

typedef const uint8_t* (*enumerate_tasks)(uint8_t index);
typedef const uint8_t* (*enumerate_name_spaces)(uint8_t index);
typedef const uint8_t* (*enumerate_functions)(const uint8_t* name_space, uint8_t index);

typedef uint8_t(*get_attributes_and_arguments_for_task)(
	const uint8_t* task, const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count);

typedef uint8_t(*evaluate_task)(const uint8_t* task,
								const uint8_t** arguments, const uint16_t* arguments_lengths, uint8_t arguments_count,
								const uint8_t** output, uint16_t* output_length,
								uint8_t verbose);

typedef uint8_t(*evaluate_function)(const uint8_t* function,
									const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
									const uint8_t** output, uint16_t* output_length);
typedef void (*module_release)();

/*struct name_space_in_module
{
	const uint8_t* name_space;
	struct buffer functions;
};*/

struct module_
{
	void* object;
	/**/
	enumerate_tasks enum_tasks;
	enumerate_name_spaces enum_name_spaces;
	enumerate_functions enum_functions;
	/**/
	get_attributes_and_arguments_for_task _get_attribute_and_arguments_for_task;
	evaluate_task _evaluate_task;
	evaluate_function _evaluate_function;
	module_release _module_release;
	/**/
	struct buffer tasks;
	/*struct buffer name_spaces;*/
};

struct module_* buffer_module_data(const struct buffer* modules, ptrdiff_t data_position)
{
	return (struct module_*)buffer_data(modules, sizeof(struct module_) * data_position);
}

uint8_t load_tasks_evaluate_task(void* the_project, const void* the_target,
								 struct buffer* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* path_to_assembly_in_a_buffer = buffer_buffer_data(task_arguments, ASSEMBLY_POSITION);

	struct buffer* path_in_a_buffer = buffer_buffer_data(task_arguments, PATH_POSITION);

	if (buffer_size(path_to_assembly_in_a_buffer) ||
		buffer_size(path_in_a_buffer))
	{
		return 0;
	}

	struct buffer* path_to_module_in_a_buffer = buffer_buffer_data(task_arguments, MODULE_POSITION);

	if (!path_to_module_in_a_buffer)
	{
		return 1;
	}

	const uint8_t* path = path_try_to_get_absolute_path(
							  the_project, the_target,
							  path_to_module_in_a_buffer, path_to_assembly_in_a_buffer, verbose);
	struct module_ the_module;
	the_module.object = shared_object_load(path);

	if (NULL == the_module.object)
	{
		return 0;
	}

	the_module.enum_tasks = (enumerate_tasks)shared_object_get_procedure_address(
								the_module.object, (const uint8_t*)"enumerate_tasks");
	the_module.enum_name_spaces = (enumerate_name_spaces)shared_object_get_procedure_address(
									  the_module.object, (const uint8_t*)"enumerate_name_spaces");
	the_module.enum_functions = (enumerate_functions)shared_object_get_procedure_address(
									the_module.object, (const uint8_t*)"enumerate_functions");
	the_module._get_attribute_and_arguments_for_task = (get_attributes_and_arguments_for_task)
			shared_object_get_procedure_address(the_module.object,
					(const uint8_t*)"get_attributes_and_arguments_for_task");
	the_module._evaluate_task = (evaluate_task)shared_object_get_procedure_address(
									the_module.object, (const uint8_t*)"evaluate_task");
	the_module._evaluate_function = (evaluate_function)shared_object_get_procedure_address(
										the_module.object, (const uint8_t*)"evaluate_function");
	the_module._module_release = (module_release)shared_object_get_procedure_address(
									 the_module.object, (const uint8_t*)"module_release");

	if (the_module.enum_tasks)
	{
		uint8_t i = 0;
		const uint8_t* ptr = NULL;
		/**/
		SET_NULL_TO_BUFFER(the_module.tasks);

		while (NULL != (ptr = the_module.enum_tasks(i++)))
		{
			if (!buffer_append(&the_module.tasks, (const uint8_t*)&ptr, sizeof(const uint8_t**)))
			{
				return 0;
			}
		}
	}

	return project_add_module(the_project, &the_module, sizeof(struct module_));
}

uint8_t load_tasks_evaluate_loaded_task(
	void* the_project, const void* the_target,
	const struct range* task_name,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == the_project ||
		range_is_null_or_empty(task_name) ||
		element_finish < attributes_finish ||
		NULL == task_arguments)
	{
		return 0;
	}

	struct module_* the_module = NULL;

	const uint8_t* pointer_to_the_task = project_get_task_from_module(the_project, task_name,
										 (void**)&the_module);

	if (NULL == pointer_to_the_task ||
		NULL == the_module)
	{
		return 0;
	}

	const uint8_t** task_attributes = NULL;
	const uint8_t* task_attributes_lengths = NULL;
	uint8_t task_attributes_count = 0;

	if (!the_module->_get_attribute_and_arguments_for_task(pointer_to_the_task, &task_attributes,
			&task_attributes_lengths,
			&task_attributes_count))
	{
		return 0;
	}

	const uint8_t** arguments = (const uint8_t**)&task_attributes_count;
	uint16_t* arguments_lengths = (uint16_t*)&task_attributes_count;

	if (0 < task_attributes_count)
	{
		if (!common_get_attributes_and_arguments_for_task(NULL, NULL, task_attributes_count, NULL, NULL, NULL,
				task_arguments))
		{
			return 0;
		}

		if (!interpreter_get_arguments_from_xml_tag_record(
				the_project, the_target,
				task_name->finish, attributes_finish,
				task_attributes, task_attributes_lengths,
				0, task_attributes_count, task_arguments, verbose))
		{
			return 0;
		}

		task_attributes_count += 2;
		const ptrdiff_t size = buffer_size(task_arguments);

		if (!buffer_append(task_arguments, NULL, task_attributes_count * sizeof(uint8_t**)))
		{
			return 0;
		}

		const ptrdiff_t new_size = buffer_size(task_arguments);

		if (!buffer_append(task_arguments, NULL, task_attributes_count * sizeof(uint16_t)))
		{
			return 0;
		}

		arguments = (const uint8_t**)buffer_data(task_arguments, size);
		arguments_lengths = (uint16_t*)buffer_data(task_arguments, new_size);

		if (NULL == arguments ||
			NULL == arguments_lengths)
		{
			return 0;
		}

		uint8_t* ptr = buffer_data(task_arguments, size);

		if (!buffer_resize(task_arguments, size))
		{
			return 0;
		}

		memset(ptr, 0,
			   task_attributes_count * sizeof(uint8_t**) + task_attributes_count * sizeof(uint16_t));
		task_attributes_count -= 2;
		/**/
		ptrdiff_t i = 0;
		const struct buffer* argument = NULL;

		while (NULL != (argument = buffer_buffer_data(task_arguments, i++)))
		{
			if (task_attributes_count < i)
			{
				return 0;
			}

			arguments[i - 1] = buffer_data(argument, 0);
			arguments_lengths[i - 1] = (uint16_t)buffer_size(argument);
		}

		arguments[task_attributes_count] = attributes_finish;
		arguments[task_attributes_count + 1] = element_finish;
	}
	else
	{
		if (!buffer_resize(task_arguments, 2 * sizeof(uint8_t**)))
		{
			return 0;
		}

		arguments = (const uint8_t**)buffer_data(task_arguments, 0);
		/**/
		arguments[task_attributes_count] = attributes_finish;
		arguments[task_attributes_count + 1] = element_finish;

		if (!buffer_resize(task_arguments, 0))
		{
			return 0;
		}
	}

	const uint8_t* output = NULL;
	uint16_t output_length = 0;

	if (!the_module->_evaluate_task(pointer_to_the_task, arguments,
									arguments_lengths, task_attributes_count,
									&output, &output_length, verbose))
	{
		return 0;
	}

	uint8_t result = 1;

	if (output && output_length)
	{
		if (!task_attributes_count)
		{
			if (!common_get_attributes_and_arguments_for_task(NULL, NULL, 1, NULL, NULL, NULL,
					task_arguments))
			{
				return 0;
			}
		}

		struct buffer* elements = buffer_buffer_data(task_arguments, 0);

		if (!buffer_resize(elements, 0))
		{
			return 0;
		}

		if (xml_get_sub_nodes_elements(output, output + output_length, NULL, elements))
		{
			result = interpreter_evaluate_tasks(the_project, the_target, elements, 0, verbose);
		}
	}

	return result;
}

const uint8_t* load_tasks_get_task(const struct buffer* modules, const struct range* task_name,
								   void** the_module_of_task)
{
	if (range_is_null_or_empty(task_name))
	{
		return NULL;
	}

	ptrdiff_t i = 0;
	struct module_* the_module = NULL;
	const uint8_t** pointer_to_the_task = NULL;
	const ptrdiff_t size = range_size(task_name);

	while (NULL != (the_module = buffer_module_data(modules, i++)))
	{
		if (NULL == the_module->enum_tasks ||
			NULL == the_module->_get_attribute_and_arguments_for_task ||
			NULL == the_module->_evaluate_task ||
			0 == buffer_size(&the_module->tasks))
		{
			continue;
		}

		ptrdiff_t j = 0;

		while (NULL != (pointer_to_the_task = (const uint8_t**)buffer_data(&the_module->tasks,
											  sizeof(const uint8_t*) * j++)))
		{
			if (size != common_count_bytes_until(*pointer_to_the_task, 0))
			{
				continue;
			}

			if (0 == memcmp(task_name->start, *pointer_to_the_task, size))
			{
				if (NULL != the_module_of_task)
				{
					*the_module_of_task = the_module;
				}

				break;
			}
		}
	}

	return NULL != pointer_to_the_task ? *pointer_to_the_task : NULL;
}

void load_tasks_unload(struct buffer* modules)
{
	ptrdiff_t i = 0;
	struct module_* the_module = NULL;

	while (NULL != (the_module = buffer_module_data(modules, i++)))
	{
		if (the_module->enum_tasks)
		{
			the_module->enum_tasks = NULL;
			buffer_release(&the_module->tasks);
		}

		the_module->enum_name_spaces = NULL;
		the_module->enum_functions = NULL;
		the_module->_get_attribute_and_arguments_for_task = NULL;
		the_module->_evaluate_task = NULL;
		the_module->_evaluate_function = NULL;

		if (the_module->_module_release)
		{
			the_module->_module_release();
			the_module->_module_release = NULL;
		}

		if (the_module->object)
		{
			shared_object_unload(the_module->object);
			the_module->object = NULL;
		}
	}
}
