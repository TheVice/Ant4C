/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2022 TheVice
 *
 */

#include "load_tasks.h"

#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "interpreter.h"
#include "path.h"
#include "project.h"
#include "range.h"
#include "shared_object.h"
#include "string_unit.h"
#include "xml.h"

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

#define COUNT_OF_POSITIONS				(MODULE_RELEASE_POSITION + 1)

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
	(const uint8_t*)"module_release"
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
	uint8_t* task_attributes_count, void* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   attributes, attributes_lengths,
			   COUNT_OF(attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

typedef const uint8_t* (*enumerate_tasks)(ptrdiff_t index);
typedef const uint8_t* (*enumerate_name_spaces)(ptrdiff_t index);
typedef const uint8_t* (*enumerate_functions)(const uint8_t* name_space, ptrdiff_t index);

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
	uint8_t functions[BUFFER_SIZE_OF];
	const uint8_t* name_space;
};

struct module_
{
	uint8_t tasks[BUFFER_SIZE_OF];
	uint8_t name_spaces[BUFFER_SIZE_OF];
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
	void* object;
};

struct module_* buffer_module_data(const void* modules, ptrdiff_t data_position)
{
	return (struct module_*)buffer_data(modules, sizeof(struct module_) * data_position);
}

struct name_space_in_module* buffer_name_space_data(const void* name_spaces, ptrdiff_t data_position)
{
	return (struct name_space_in_module*)buffer_data(name_spaces,
			sizeof(struct name_space_in_module) * data_position);
}

void load_tasks_unload_module(struct module_* the_module)
{
	if (!the_module)
	{
		return;
	}

	if (the_module->enum_tasks)
	{
		the_module->enum_tasks = NULL;
		buffer_release((void*)the_module->tasks);
	}

	if (the_module->enum_name_spaces)
	{
		the_module->enum_name_spaces = NULL;

		if (the_module->enum_functions)
		{
			the_module->enum_functions = NULL;
			/**/
			ptrdiff_t i = 0;
			struct name_space_in_module* name_space_ = NULL;

			while (NULL != (name_space_ = buffer_name_space_data((void*)the_module->name_spaces, i++)))
			{
				name_space_->name_space = NULL;
				buffer_release((void*)&name_space_->functions);
			}
		}

		buffer_release((void*)the_module->name_spaces);
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

uint8_t load_tasks_load_module(void* the_project, const uint8_t* path, const uint8_t** functions_names)
{
	if (NULL == the_project ||
		NULL == path ||
		NULL == functions_names)
	{
		return 0;
	}

	struct module_ the_module;

	the_module.object = shared_object_load(path);

	if (NULL == the_module.object)
	{
		return 0;
	}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(disable: 4055)
#endif
	the_module.enum_tasks = (enumerate_tasks)shared_object_get_procedure_address(the_module.object,
							functions_names[0]);
	the_module.enum_name_spaces = (enumerate_name_spaces)shared_object_get_procedure_address(the_module.object,
								  functions_names[ENUMERATE_NAME_SPACES_POSITION - ENUMERATE_TASKS_POSITION]);
	the_module.enum_functions = (enumerate_functions)shared_object_get_procedure_address(the_module.object,
								functions_names[ENUMERATE_FUNCTIONS_POSITION - ENUMERATE_TASKS_POSITION]);
	the_module._get_attribute_and_arguments_for_task =
		(get_attributes_and_arguments_for_task)shared_object_get_procedure_address(the_module.object,
				functions_names[GET_ATTRIBUTES_AND_ARGUMENTS_FOR_TASK_POSITION - ENUMERATE_TASKS_POSITION]);
	the_module._evaluate_task = (evaluate_task)shared_object_get_procedure_address(the_module.object,
								functions_names[EVALUATE_TASK_POSITION - ENUMERATE_TASKS_POSITION]);
	the_module._evaluate_function = (evaluate_function)shared_object_get_procedure_address(the_module.object,
									functions_names[EVALUATE_FUNCTION_POSITION - ENUMERATE_TASKS_POSITION]);
	the_module._module_release = (module_release)shared_object_get_procedure_address(the_module.object,
								 functions_names[MODULE_RELEASE_POSITION - ENUMERATE_TASKS_POSITION]);
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(default: 4055)
#endif

	if (the_module.enum_tasks)
	{
		ptrdiff_t i = 0;
		const uint8_t* task_str = NULL;

		if (!buffer_init((void*)the_module.tasks, BUFFER_SIZE_OF))
		{
			return 0;
		}

		while (NULL != (task_str = the_module.enum_tasks(i++)))
		{
			if (!buffer_append(the_module.tasks, (const uint8_t*)&task_str, sizeof(const uint8_t**)))
			{
				return 0;
			}
		}
	}

	if (the_module.enum_name_spaces &&
		the_module.enum_functions)
	{
		ptrdiff_t i = 0;
		const uint8_t* name_space_str = NULL;

		if (!buffer_init((void*)the_module.name_spaces, BUFFER_SIZE_OF))
		{
			return 0;
		}

		while (NULL != (name_space_str = the_module.enum_name_spaces(i++)))
		{
			ptrdiff_t j = 0;
			const uint8_t* function_ = NULL;
			/**/
			struct name_space_in_module name_space_;
			name_space_.name_space = name_space_str;

			if (!buffer_init((void*)name_space_.functions, BUFFER_SIZE_OF))
			{
				return 0;
			}

			while (NULL != (function_ = the_module.enum_functions(name_space_str, j++)))
			{
				if (!buffer_append((void*)name_space_.functions, (const uint8_t*)&function_, sizeof(const uint8_t**)))
				{
					return 0;
				}
			}

			if (!buffer_append((void*)the_module.name_spaces, (const uint8_t*)&name_space_,
							   sizeof(const struct name_space_in_module)))
			{
				return 0;
			}
		}
	}

	if (project_add_module(the_project, &the_module, sizeof(struct module_)))
	{
		return 1;
	}

	load_tasks_unload_module(&the_module);
	return 0;
}

uint8_t load_tasks_evaluate_task(void* the_project, const void* the_target,
								 void* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == the_project ||
		NULL == task_arguments)
	{
		return 0;
	}

	void* path_to_assembly_in_a_buffer = buffer_buffer_data(task_arguments, ASSEMBLY_POSITION);

	if (buffer_size(path_to_assembly_in_a_buffer))
	{
		return 0;/*TODO*/
	}

	const uint8_t* functions_names[7];

	for (uint8_t i = ENUMERATE_TASKS_POSITION; i < COUNT_OF_POSITIONS; ++i)
	{
		void* tmp = buffer_buffer_data(task_arguments, i);

		if (buffer_size(tmp))
		{
			if (!buffer_push_back(tmp, 0))
			{
				return 0;
			}

			functions_names[i - ENUMERATE_TASKS_POSITION] = buffer_uint8_t_data(tmp, 0);
		}
		else
		{
			functions_names[i - ENUMERATE_TASKS_POSITION] = attributes[i];
		}
	}

	void* path_to_module_in_a_buffer = buffer_buffer_data(task_arguments, MODULE_POSITION);

	if (buffer_size(path_to_module_in_a_buffer))
	{
		if (!buffer_push_back(path_to_module_in_a_buffer, 0))
		{
			return 0;
		}

		const uint8_t* path = path_try_to_get_absolute_path(
								  the_project, the_target, path_to_module_in_a_buffer,
								  path_to_assembly_in_a_buffer, verbose);

		if (!load_tasks_load_module(the_project, path, functions_names))
		{
			return 0;
		}
	}

	void* path_in_a_buffer = buffer_buffer_data(task_arguments, PATH_POSITION);

	if (buffer_size(path_in_a_buffer))
	{
#ifdef _WIN32
		static const uint8_t* wild_card = (const uint8_t*)"*.dll\0";
#define WILD_CARD_LENGTH 6
#else
		static const uint8_t* wild_card = (const uint8_t*)"*.so\0";
#define WILD_CARD_LENGTH 5
#endif

		if (!path_combine_in_place(path_in_a_buffer, 0, wild_card, wild_card + WILD_CARD_LENGTH))
		{
			return 0;
		}

		if (!buffer_resize(path_to_module_in_a_buffer, 0) ||
			!directory_enumerate_file_system_entries(path_in_a_buffer, 1, 0, path_to_module_in_a_buffer, 1))
		{
			return 0;
		}

		if (buffer_size(path_to_module_in_a_buffer))
		{
			static const uint8_t zero = '\0';
			const uint8_t* start = buffer_uint8_t_data(path_to_module_in_a_buffer, 0);
			const uint8_t* finish = start + buffer_size(path_to_module_in_a_buffer);

			while (finish != (start = string_find_any_symbol_like_or_not_like_that(
										  start, finish, &zero, &zero + 1, 0, 1)))
			{
				const uint8_t* pos =
					string_find_any_symbol_like_or_not_like_that(
						start, finish, &zero, &zero + 1, 1, 1);

				if (!buffer_resize(path_in_a_buffer, 0) ||
					!buffer_append(path_in_a_buffer, start, pos - start) ||
					!buffer_push_back(path_in_a_buffer, 0))
				{
					return 0;
				}

				const uint8_t* path =
					path_try_to_get_absolute_path(
						the_project, the_target, path_in_a_buffer, path_to_assembly_in_a_buffer, verbose);

				if (!load_tasks_load_module(the_project, path, functions_names))
				{
					return 0;
				}

				start = pos;
			}

			return 1;
		}
	}

	if (!buffer_size(path_to_module_in_a_buffer) &&
		!buffer_size(path_in_a_buffer))
	{
		return 0;
	}

	return 1;
}

uint8_t load_tasks_buffer_to_arguments(
	void* task_arguments, uint8_t task_attributes_count,
	const uint8_t*** arguments, uint16_t** arguments_lengths)
{
	if (NULL == task_arguments ||
		0 == task_attributes_count ||
		NULL == arguments ||
		NULL == arguments_lengths)
	{
		return 0;
	}

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

	*arguments = (const uint8_t**)buffer_data(task_arguments, size);
	*arguments_lengths = (uint16_t*)buffer_data(task_arguments, new_size);

	if (NULL == (*arguments) ||
		NULL == (*arguments_lengths))
	{
		return 0;
	}

	if (!buffer_resize(task_arguments, size))
	{
		return 0;
	}

	ptrdiff_t i = 0;
	const void* argument = NULL;

	while (NULL != (argument = buffer_buffer_data(task_arguments, i++)))
	{
		if (task_attributes_count < i)
		{
			return 0;
		}

		(*arguments)[i - 1] = buffer_uint8_t_data(argument, 0);
		(*arguments_lengths)[i - 1] = (uint16_t)buffer_size(argument);
	}

	--i;

	while (i < task_attributes_count)
	{
		(*arguments)[i] = NULL;
		(*arguments_lengths)[i] = 0;
		/**/
		++i;
	}

	return 1;
}

uint8_t load_tasks_evaluate_loaded_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	const uint8_t* element_finish, void* task_arguments,
	const void* the_module, const uint8_t* pointer_to_the_task,
	uint8_t verbose)
{
	if (NULL == the_project ||
		attributes_finish < attributes_start ||
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
	/**/
	const struct module_* the_mod = (const struct module_*)the_module;

	if (!the_mod->_get_attribute_and_arguments_for_task(
			pointer_to_the_task, &task_attributes, &task_attributes_lengths, &task_attributes_count))
	{
		return 0;
	}

	const uint8_t** arguments = NULL;
	uint16_t* arguments_lengths = NULL;

	if (task_attributes_count)
	{
		if (!common_get_attributes_and_arguments_for_task(NULL, NULL, task_attributes_count, NULL, NULL, NULL,
				task_arguments))
		{
			return 0;
		}

		if (!interpreter_get_arguments_from_xml_tag_record(
				the_project, the_target,
				attributes_start, attributes_finish,
				task_attributes, task_attributes_lengths,
				0, task_attributes_count, task_arguments, verbose))
		{
			return 0;
		}

		if (!load_tasks_buffer_to_arguments(
				task_arguments, task_attributes_count + 2, &arguments, &arguments_lengths))
		{
			return 0;
		}
	}
	else
	{
		if (!buffer_resize(task_arguments, 2 * sizeof(uint8_t**)))
		{
			return 0;
		}

		arguments = (const uint8_t**)buffer_data(task_arguments, 0);
		arguments_lengths = (uint16_t*)&task_attributes_count;

		if (!buffer_resize(task_arguments, 0))
		{
			return 0;
		}
	}

	arguments[task_attributes_count] = attributes_finish;
	arguments[task_attributes_count + 1] = element_finish;
	/**/
	const uint8_t* output = NULL;
	uint16_t output_length = 0;

	if (!the_mod->_evaluate_task(
			pointer_to_the_task, arguments, arguments_lengths,
			task_attributes_count, &output, &output_length, verbose))
	{
		return 0;
	}

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

		void* elements = buffer_buffer_data(task_arguments, 0);

		if (!buffer_resize(elements, 0))
		{
			return 0;
		}

		if (xml_get_sub_nodes_elements(output, output + output_length, NULL, elements))
		{
			return interpreter_evaluate_tasks(the_project, the_target, elements, NULL, 0, verbose);
		}
	}

	return 1;
}

uint8_t load_tasks_evaluate_loaded_function(const void* the_module, const uint8_t* pointer_to_the_function,
		void* arguments, uint8_t arguments_count, void* return_of_function, uint8_t verbose)
{
	(void)verbose;

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

	const uint8_t** values = NULL;
	uint16_t* values_lengths = NULL;

	if (arguments_count)
	{
		if (!load_tasks_buffer_to_arguments(arguments, arguments_count, &values, &values_lengths))
		{
			return 0;
		}
	}
	else
	{
		values = (const uint8_t**)&arguments_count;
		values_lengths = (uint16_t*)&arguments_count;
	}

	const uint8_t* output = NULL;
	uint16_t output_length = 0;

	if (the_mod->_evaluate_function(pointer_to_the_function, values, values_lengths,
									arguments_count, &output, &output_length))
	{
		return buffer_append(return_of_function, output, output_length);
	}

	return 0;
}

const uint8_t* load_tasks_get_task(const void* modules, const struct range* task_name,
								   void** the_module_of_task, ptrdiff_t* task_id)
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
			0 == buffer_size((void*)the_module->tasks))
		{
			continue;
		}

		ptrdiff_t j = 0;

		while (NULL != (pointer_to_the_task = (const uint8_t**)buffer_data((void*)the_module->tasks,
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

				if (NULL != task_id)
				{
					*task_id = j - 1;
				}

				break;
			}
		}
	}

	return NULL != pointer_to_the_task ? *pointer_to_the_task : NULL;
}

const uint8_t* load_tasks_get_function(
	const void* modules, const struct range* name_space,
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

		while (NULL != (ns_in_module = buffer_name_space_data((void*)the_module->name_spaces, j++)))
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

void load_tasks_unload(void* modules)
{
	ptrdiff_t i = 0;
	struct module_* the_module = NULL;

	while (NULL != (the_module = buffer_module_data(modules, i++)))
	{
		load_tasks_unload_module(the_module);
	}
}

uint8_t load_tasks_copy_modules_with_out_objects(const void* the_source,
		void* the_destination)
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

		if (!buffer_init((void*)the_destination_module->tasks, BUFFER_SIZE_OF))
		{
			return 0;
		}

		if (the_module->enum_tasks)
		{
			the_destination_module->enum_tasks = the_module->enum_tasks;

			if (!buffer_append_data_from_buffer(&the_destination_module->tasks, (void*)the_module->tasks))
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
