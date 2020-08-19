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

#define ENUMERATE_TASKS_POSITION						3
#define ENUMERATE_NAME_SPACES_POSITION					4
#define ENUMERATE_FUNCTIONS_POSITION					5
#define GET_ATTRIBUTES_AND_ARGUMENTS_FOR_TASK_POSITION	6
#define EVALUATE_TASK_POSITION							7
#define EVALUATE_FUNCTION_POSITION						8
#define MODULE_RELEASE_POSITION							9

static const uint8_t* attributes[] =
{
	(const uint8_t*)"assembly",
	(const uint8_t*)"path",
	(const uint8_t*)"module",
	/**/
	(const uint8_t*)"enumerate_tasks",
	(const uint8_t*)"enumerate_name_spaces",
	(const uint8_t*)"enumerate_functions",
	(const uint8_t*)"get_attributes_and_arguments_for_task",
	(const uint8_t*)"evaluate_task",
	(const uint8_t*)"evaluate_function",
	(const uint8_t*)"module_release",
};

static const uint8_t attributes_lengths[] =
{
	8,
	4,
	6,
	/**/
	15,
	21,
	19,
	37,
	13,
	17,
	14
};

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

struct name_space_in_module
{
	const uint8_t* name_space;
	struct buffer functions;
};

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
	struct buffer name_spaces;
};

struct module_* buffer_module_data(const struct buffer* modules, ptrdiff_t data_position)
{
	return (struct module_*)buffer_data(modules, sizeof(struct module_) * data_position);
}

struct name_space_in_module* buffer_name_space_data(const struct buffer* name_spaces, ptrdiff_t data_position)
{
	return (struct name_space_in_module*)buffer_data(name_spaces,
			sizeof(struct name_space_in_module) * data_position);
}

#define GET_FUNCTION(TMP, BUFFERS, ATTRIBUTES, POSITION, NAME_OF_FUNCTION, FUNCTION, TYPE, OBJECT)	\
	(TMP) = buffer_buffer_data((BUFFERS), (POSITION));	\
	\
	if (buffer_size(TMP))\
	{\
		if (!buffer_push_back((TMP), 0))\
		{\
			return 0;\
		}\
		\
		(NAME_OF_FUNCTION) = buffer_data((TMP), 0);\
	}\
	else\
	{\
		(NAME_OF_FUNCTION) = (ATTRIBUTES)[(POSITION)];\
	}\
	\
	(FUNCTION) = (TYPE)shared_object_get_procedure_address((OBJECT), (NAME_OF_FUNCTION));

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
		return 0;/*TODO*/
	}

	struct buffer* path_to_module_in_a_buffer = buffer_buffer_data(task_arguments, MODULE_POSITION);

	if (!path_to_module_in_a_buffer)
	{
		return 1;
	}
	else
	{
		if (!buffer_push_back(path_to_module_in_a_buffer, 0))
		{
			return 0;
		}
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

	struct buffer* tmp = NULL;

	GET_FUNCTION(tmp, task_arguments, attributes, ENUMERATE_TASKS_POSITION, path,
				 the_module.enum_tasks, enumerate_tasks, the_module.object);

	GET_FUNCTION(tmp, task_arguments, attributes, ENUMERATE_NAME_SPACES_POSITION, path,
				 the_module.enum_name_spaces, enumerate_name_spaces, the_module.object);

	GET_FUNCTION(tmp, task_arguments, attributes, ENUMERATE_FUNCTIONS_POSITION, path,
				 the_module.enum_functions, enumerate_functions, the_module.object);

	GET_FUNCTION(tmp, task_arguments, attributes, GET_ATTRIBUTES_AND_ARGUMENTS_FOR_TASK_POSITION, path,
				 the_module._get_attribute_and_arguments_for_task, get_attributes_and_arguments_for_task, the_module.object);

	GET_FUNCTION(tmp, task_arguments, attributes, EVALUATE_TASK_POSITION, path,
				 the_module._evaluate_task, evaluate_task, the_module.object);

	GET_FUNCTION(tmp, task_arguments, attributes, EVALUATE_FUNCTION_POSITION, path,
				 the_module._evaluate_function, evaluate_function, the_module.object);

	GET_FUNCTION(tmp, task_arguments, attributes, MODULE_RELEASE_POSITION, path,
				 the_module._module_release, module_release, the_module.object);

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

	if (the_module.enum_name_spaces &&
		the_module.enum_functions)
	{
		uint8_t i = 0;
		const uint8_t* ptr = NULL;
		/**/
		SET_NULL_TO_BUFFER(the_module.name_spaces);

		while (NULL != (ptr = the_module.enum_name_spaces(i++)))
		{
			uint8_t j = 0;
			const uint8_t* fun = NULL;
			/**/
			struct name_space_in_module ns_in_module;
			ns_in_module.name_space = ptr;
			SET_NULL_TO_BUFFER(ns_in_module.functions);

			while (NULL != (fun = the_module.enum_functions(ptr, j++)))
			{
				if (!buffer_append(&ns_in_module.functions, (const uint8_t*)&fun, sizeof(const uint8_t**)))
				{
					return 0;
				}
			}

			if (!buffer_append(&the_module.name_spaces, (const uint8_t*)&ns_in_module,
							   sizeof(const struct name_space_in_module)))
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
	struct buffer* task_arguments,
	const void* the_module, const uint8_t* pointer_to_the_task,
	uint8_t verbose)
{
	if (NULL == the_project ||
		range_is_null_or_empty(task_name) ||
		element_finish < attributes_finish ||
		NULL == task_arguments ||
		NULL == pointer_to_the_task ||
		NULL == the_module)
	{
		return 0;
	}

	const uint8_t** task_attributes = NULL;
	const uint8_t* task_attributes_lengths = NULL;
	uint8_t task_attributes_count = 0;
	const struct module_* the_mod = (const struct module_*)the_module;

	if (!the_mod->_get_attribute_and_arguments_for_task(pointer_to_the_task, &task_attributes,
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

	if (!the_mod->_evaluate_task(pointer_to_the_task, arguments,
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

uint8_t load_tasks_evaluate_loaded_function(const void* the_module, const uint8_t* pointer_to_the_function,
		struct buffer* arguments, uint8_t arguments_count, struct buffer* return_of_function, uint8_t verbose)
{
	if (NULL == the_module ||
		NULL == pointer_to_the_function ||
		NULL == arguments ||
		NULL == return_of_function)
	{
		return 0;
	}

	const struct module_* the_mod = (const struct module_*)the_module;

	if (NULL == the_mod->enum_name_spaces ||
		NULL == the_mod->enum_functions ||
		NULL == the_mod->_evaluate_function)
	{
		return 0;
	}

	const uint8_t* output = NULL;
	uint16_t output_length = 0;
	/**/
	const uint8_t* values = (const uint8_t*)&arguments_count;
	const uint16_t* values_lengths = (const uint16_t*)&arguments_count;

	if (arguments_count)
	{
		/*TODO:*/
	}

	if (the_mod->_evaluate_function(pointer_to_the_function, &values, values_lengths,
									arguments_count, &output, &output_length))
	{
		return buffer_append(return_of_function, output, output_length);
	}

	(void)verbose;
	return 0;
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

const uint8_t* load_tasks_get_function(
	const struct buffer* modules, const struct range* name_space,
	const struct range* function_name, void** the_module_of_function, const uint8_t** name_space_at_module)
{
	if (NULL == modules)
	{
		return NULL;
	}

	ptrdiff_t i = 0;
	struct module_* the_module = NULL;
	const ptrdiff_t name_space_size = range_size(name_space);
	const ptrdiff_t function_name_size = range_size(function_name);

	while (NULL != (the_module = buffer_module_data(modules, i++)))
	{
		if (NULL == the_module->enum_name_spaces ||
			NULL == the_module->enum_functions)
		{
			continue;
		}

		ptrdiff_t j = 0;
		struct name_space_in_module* ns_in_module = NULL;

		while (NULL != (ns_in_module = buffer_name_space_data(&the_module->name_spaces, j++)))
		{
			if (name_space_size != common_count_bytes_until(ns_in_module->name_space, 0))
			{
				continue;
			}

			if (0 == memcmp(ns_in_module->name_space, name_space->start, name_space_size))
			{
				ptrdiff_t k = 0;
				const uint8_t** pointer_to_the_fun = NULL;

				while (NULL != (pointer_to_the_fun = (const uint8_t**)buffer_data(&ns_in_module->functions,
													 sizeof(const uint8_t*) * k++)))
				{
					if (function_name_size != common_count_bytes_until(*pointer_to_the_fun, 0))
					{
						continue;
					}

					if (0 == memcmp(*pointer_to_the_fun, function_name->start, function_name_size))
					{
						if (NULL != the_module_of_function)
						{
							*the_module_of_function = the_module;
						}

						if (NULL != name_space_at_module)
						{
							*name_space_at_module = ns_in_module->name_space;
						}

						return *pointer_to_the_fun;
					}
				}
			}
		}
	}

	return NULL;
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

		if (the_module->enum_name_spaces)
		{
			the_module->enum_name_spaces = NULL;

			if (the_module->enum_functions)
			{
				the_module->enum_functions = NULL;
				/**/
				ptrdiff_t j = 0;
				struct name_space_in_module* ns_in_module = NULL;

				while (NULL != (ns_in_module = buffer_name_space_data(&the_module->name_spaces, j++)))
				{
					ns_in_module->name_space = NULL;
					buffer_release(&ns_in_module->functions);
				}
			}

			buffer_release(&the_module->name_spaces);
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

uint8_t load_tasks_copy_modules_with_out_objects(const struct buffer* the_source,
		struct buffer* the_destination)
{
	if (NULL == the_source ||
		NULL == the_destination ||
		buffer_size(the_destination))
	{
		return 0;
	}

	if (!buffer_resize(the_destination, buffer_size(the_source)) ||
		!buffer_resize(the_destination, 0))
	{
		return 0;
	}

	ptrdiff_t i = 0;
	const struct module_* the_module = NULL;

	while (NULL != (the_module = buffer_module_data(the_source, i++)))
	{
		const ptrdiff_t size = buffer_size(the_destination);

		if (!buffer_append(the_destination, NULL, sizeof(struct module_)))
		{
			return 0;
		}

		struct module_* the_destination_module = (struct module_*)buffer_data(the_destination, size);

		if (NULL == the_destination_module)
		{
			return 0;
		}

		SET_NULL_TO_BUFFER(the_destination_module->tasks);

		if (the_module->enum_tasks)
		{
			the_destination_module->enum_tasks = the_module->enum_tasks;

			if (!buffer_append_data_from_buffer(&the_destination_module->tasks, &the_module->tasks))
			{
				return 0;
			}
		}
		else
		{
			the_destination_module->enum_tasks = NULL;
		}

		the_destination_module->enum_name_spaces = the_module->enum_name_spaces;
		the_destination_module->enum_functions = the_module->enum_functions;
		the_destination_module->_get_attribute_and_arguments_for_task =
			the_module->_get_attribute_and_arguments_for_task;
		the_destination_module->_evaluate_task = the_module->_evaluate_task;
		the_destination_module->_evaluate_function = the_module->_evaluate_function;
		the_destination_module->_module_release = NULL;
		the_destination_module->object = NULL;
	}

	return 1;
}
