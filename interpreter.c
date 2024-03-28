/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "interpreter.h"

#include "buffer.h"
#include "choose_task.h"
#include "common.h"
#include "conversion.h"
#include "copy_move.h"
#include "echo.h"
#include "fail_task.h"
#include "for_each.h"
#include "if_task.h"
#include "interpreter.exec.h"
#include "interpreter.file_system.h"
#include "interpreter.load_file.h"
#include "interpreter.property.h"
#include "interpreter.string_unit.h"
#include "listener.h"
#include "load_tasks.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "task.h"
#include "text_encoding.h"
#include "try_catch.h"
#include "xml.h"

#include <string.h>

static const uint8_t start_of_function_arguments_area = '(';
static const uint8_t finish_of_function_arguments_area = ')';
static const uint8_t call_of_expression_finish = '}';
static const uint8_t apos = '\'';
static const uint8_t arguments_delimiter = ',';

static const uint8_t* space_and_tab = (const uint8_t*)" \t";
#define SPACE_AND_TAB_LENGTH 2

static const uint8_t* name_space_border = (const uint8_t*)"::";
#define NAME_SPACE_BORDER_LENGTH 2

static const uint8_t* call_of_expression_start = (const uint8_t*)"${";
#define CALL_OF_EXPRESSION_START_LENGTH 2

enum interpreter_enumeration_unit
{
	bool_unit,
	cygpath_unit,
	datetime_unit,
	directory_unit,
	double_unit,
	environment_unit,
	file_unit,
	hash_unit,
	int_unit,
	int64_unit,
	long_unit,
	math_unit,
	operating_system_unit,
	path_unit,
	platform_unit,
	program_unit,
	project_unit,
	property_unit,
	string_unit,
	target_unit,
	task_unit,
	timespan_unit,
	version_unit,
	UNKNOWN_UNIT
};

uint8_t interpreter_get_unit(const uint8_t* name_space_start, const uint8_t* name_space_finish)
{
	static const uint8_t* interpreter_string_enumeration_unit[] =
	{
		(const uint8_t*)"bool",
		(const uint8_t*)"cygpath",
		(const uint8_t*)"datetime",
		(const uint8_t*)"directory",
		(const uint8_t*)"double",
		(const uint8_t*)"environment",
		(const uint8_t*)"file",
		(const uint8_t*)"hash",
		(const uint8_t*)"int",
		(const uint8_t*)"int64",
		(const uint8_t*)"long",
		(const uint8_t*)"math",
		(const uint8_t*)"operating-system",
		(const uint8_t*)"path",
		(const uint8_t*)"platform",
		(const uint8_t*)"program",
		(const uint8_t*)"project",
		(const uint8_t*)"property",
		(const uint8_t*)"string",
		(const uint8_t*)"target",
		(const uint8_t*)"task",
		(const uint8_t*)"timespan",
		(const uint8_t*)"version"
	};
	/**/
	return common_string_to_enum(
			   name_space_start, name_space_finish,
			   interpreter_string_enumeration_unit, UNKNOWN_UNIT);
}

uint8_t interpreter_disassemble_function(
	const struct range* function,
	struct range* name_space,
	struct range* name,
	struct range* arguments_area)
{
	if (range_is_null_or_empty(function) ||
		NULL == name_space ||
		NULL == name ||
		NULL == arguments_area)
	{
		return 0;
	}

	name_space->start = function->start;
	name_space->finish = function->start;
	uint8_t found = 0;

	while (name_space->finish + NAME_SPACE_BORDER_LENGTH <= function->finish)
	{
		if (0 != memcmp(name_space->finish, name_space_border, NAME_SPACE_BORDER_LENGTH))
		{
			name_space->finish = string_enumerate(name_space->finish, function->finish, NULL);
			continue;
		}

		found = 1;
		break;
	}

	if (!found)
	{
		return 0;
	}

	name->start = name_space->finish + NAME_SPACE_BORDER_LENGTH;
	name->finish = string_find_any_symbol_like_or_not_like_that(
					   name->start, function->finish,
					   &start_of_function_arguments_area,
					   &start_of_function_arguments_area + 1, 1, 1);

	if (function->finish == name->finish)
	{
		return 0;
	}

	arguments_area->start = string_enumerate(name->finish, function->finish, NULL);

	if (NULL == arguments_area->start)
	{
		return 0;
	}

	arguments_area->finish = string_find_any_symbol_like_or_not_like_that(
								 function->finish, arguments_area->start,
								 &finish_of_function_arguments_area,
								 &finish_of_function_arguments_area + 1, 1, -1);
	uint32_t char_set;

	if (!string_enumerate(arguments_area->finish, function->finish, &char_set))
	{
		return 0;
	}

	return finish_of_function_arguments_area == char_set;
}

uint8_t interpreter_get_function_from_argument(
	struct range* argument_area)
{
	if (range_is_null_or_empty(argument_area))
	{
		return 0;
	}

	const uint8_t* finish = argument_area->finish;
	argument_area->finish = string_find_any_symbol_like_or_not_like_that(
								argument_area->start, argument_area->finish,
								&start_of_function_arguments_area,
								&start_of_function_arguments_area + 1, 1, 1);

	if (finish == argument_area->finish)
	{
		return 0;
	}

	uint32_t char_set = 0;
	uint8_t depth = 0;

	while (argument_area->finish < finish)
	{
		argument_area->finish = string_enumerate(argument_area->finish, finish, &char_set);

		if (!string_enumerate(argument_area->finish, finish, &char_set))
		{
			return 0;
		}

		if (apos == char_set)
		{
			argument_area->finish = string_enumerate(argument_area->finish, finish, NULL);
			argument_area->finish =
				string_find_any_symbol_like_or_not_like_that(
					argument_area->finish, finish, &apos, &apos + 1, 1, 1);
			continue;
		}

		if (finish_of_function_arguments_area == char_set)
		{
			if (0 == depth)
			{
				argument_area->finish = string_enumerate(argument_area->finish, finish, NULL);
				break;
			}

			--depth;
		}

		if (start_of_function_arguments_area == char_set)
		{
			++depth;
		}
	}

	if (finish_of_function_arguments_area != char_set || 0 != depth)
	{
		return 0;
	}

	return 1;
}

uint8_t interpreter_evaluate_argument_area(
	const void* the_project, const void* the_target,
	const struct range* argument_area, void* output, uint8_t verbose)
{
	const uint8_t* pos = argument_area->start;

	if (apos != *(argument_area->start) || apos != *(argument_area->finish - 1))
	{
		const uint8_t* pos_with_index = pos;

		while (pos_with_index + NAME_SPACE_BORDER_LENGTH <= argument_area->finish)
		{
			if (0 != memcmp(pos_with_index, name_space_border, NAME_SPACE_BORDER_LENGTH))
			{
				pos_with_index = string_enumerate(pos_with_index, argument_area->finish, NULL);
				continue;
			}

			struct range function;

			function.start = string_find_any_symbol_like_or_not_like_that(
								 pos_with_index, pos,
								 space_and_tab, space_and_tab + SPACE_AND_TAB_LENGTH, 1, -1);

			function.finish = argument_area->finish;

			if (!string_trim(&function) ||
				!interpreter_get_function_from_argument(&function) ||
				!buffer_append(output, pos, function.start - pos) ||
				!interpreter_evaluate_function(the_project, the_target, &function, output, verbose))
			{
				return 0;
			}

			pos = function.finish;
			pos_with_index = pos;
		}
	}

	return (pos != argument_area->start) ? buffer_append(output, pos, argument_area->finish - pos) : 1;
}

uint8_t interpreter_actualize_property_value(
	const void* the_project, const void* the_target,
	uint8_t property_function_id, const void* the_property,
	ptrdiff_t size, void* output, uint8_t verbose)
{
	if (property_get_id_of_get_value_function() != property_function_id ||
		NULL == the_property)
	{
		return 1;
	}

	uint8_t dynamic = 0;

	if (!property_is_dynamic(the_property, &dynamic))
	{
		return 0;
	}

	if (!dynamic)
	{
		return 1;
	}

	uint8_t code_in_a_buffer_[BUFFER_SIZE_OF];
	void* code_in_a_buffer = (void*)code_in_a_buffer_;

	if (!buffer_init(code_in_a_buffer, BUFFER_SIZE_OF))
	{
		return 0;
	}

	if (!buffer_append(code_in_a_buffer, buffer_data(output, size),
					   buffer_size(output) - size) ||
		!buffer_resize(output, size))
	{
		buffer_release(code_in_a_buffer);
		return 0;
	}

	struct range code;

	BUFFER_TO_RANGE(code, code_in_a_buffer);

	if (code.start < code.finish &&
		!interpreter_evaluate_code(the_project, the_target,
								   the_property, &code, output, verbose))
	{
		buffer_release(code_in_a_buffer);
		return 0;
	}

	buffer_release(code_in_a_buffer);
	return 1;
}

uint8_t interpreter_get_value_for_argument(
	const void* the_project, const void* the_target,
	struct range* argument_area, void* values, uint8_t verbose)
{
	uint8_t value_buffer[BUFFER_SIZE_OF];
	void* value = (void*)value_buffer;

	if (!buffer_init(value, BUFFER_SIZE_OF))
	{
		return 0;
	}

	if (range_is_null_or_empty(argument_area) ||
		NULL == values)
	{
		buffer_release(value);
		return 0;
	}

	if (!string_trim(argument_area))
	{
		buffer_release(value);
		return 0;
	}

	if (!interpreter_evaluate_argument_area(
			the_project, the_target, argument_area, value, verbose))
	{
		buffer_release(value);
		return 0;
	}

	if (!buffer_size(value))
	{
		void* the_property = NULL;

		if (project_property_exists(
				the_project,
				argument_area->start, (uint8_t)range_size(argument_area),
				&the_property, verbose))
		{
			if (!property_get_by_pointer(the_property, value))
			{
				buffer_release(value);
				return 0;
			}

			if (!interpreter_actualize_property_value(
					the_project, the_target,
					property_get_id_of_get_value_function(),
					the_property, 0, value, verbose))
			{
				buffer_release(value);
				return 0;
			}
		}
		else
		{
			const ptrdiff_t size = range_size(argument_area);

			if (!string_un_quote(argument_area))
			{
				buffer_release(value);
				return 0;
			}

			if (range_size(argument_area) < size)
			{
				if (!buffer_append_data_from_range(value, argument_area))
				{
					buffer_release(value);
					return 0;
				}
			}
			else
			{
				if (!string_contains(
						argument_area->start, argument_area->finish,
						name_space_border, name_space_border + NAME_SPACE_BORDER_LENGTH))
				{
					buffer_release(value);
					return 0;
				}
			}
		}
	}

	/*TODO: value can be represent the quote or property name, recursion in this case should be used at else.*/
	return buffer_append_buffer(values, value, 1);
}

uint8_t interpreter_get_values_for_arguments(
	const void* the_project, const void* the_target,
	const struct range* arguments_area,
	void* values, uint8_t* values_count, uint8_t verbose)
{
	if (NULL == values ||
		NULL == values_count)
	{
		return 0;
	}

	if (range_is_null_or_empty(arguments_area))
	{
		*values_count = 0;
		return 1;
	}

	uint8_t count = 0;
	uint8_t depth = 0;
	/**/
	struct range argument_area;
	argument_area.start = arguments_area->start;
	argument_area.finish = arguments_area->start;

	while (argument_area.finish < arguments_area->finish)
	{
		uint32_t char_set;

		if (!string_enumerate(argument_area.finish, arguments_area->finish, &char_set))
		{
			return 0;
		}

		if (apos == char_set)
		{
			argument_area.finish = string_enumerate(argument_area.finish, arguments_area->finish, NULL);
			argument_area.finish = string_find_any_symbol_like_or_not_like_that(
									   argument_area.finish, arguments_area->finish,
									   &apos, &apos + 1, 1, 1);
			argument_area.finish = string_enumerate(argument_area.finish, arguments_area->finish, NULL);
			argument_area.finish = string_find_any_symbol_like_or_not_like_that(
									   argument_area.finish, arguments_area->finish,
									   &apos, &apos + 1, 0, 1);
			continue;
		}
		else if (arguments_delimiter == char_set && 0 == depth)
		{
			const uint8_t* pos = string_enumerate(argument_area.finish, arguments_area->finish, NULL);

			if (NULL == pos ||
				!interpreter_get_value_for_argument(the_project, the_target, &argument_area, values, verbose))
			{
				return 0;
			}

			++count;
			/**/
			argument_area.start = pos;
			argument_area.finish = pos;
		}
		else if (start_of_function_arguments_area == char_set)
		{
			if (UINT8_MAX == depth)
			{
				/*TODO:*/
				break;
			}

			++depth;
		}
		else if (finish_of_function_arguments_area == char_set)
		{
			if (0 == depth)
			{
				/*TODO:*/
				break;
			}

			--depth;
		}

		argument_area.finish = string_enumerate(argument_area.finish, arguments_area->finish, NULL);
	}

	if (range_is_null_or_empty(&argument_area))
	{
		*values_count = count;
		return 1;
	}

	if (!interpreter_get_value_for_argument(the_project, the_target, &argument_area, values, verbose))
	{
		return 0;
	}

	++count;
	*values_count = count;
	return 1;
}

uint8_t interpreter_evaluate_function(const void* the_project, const void* the_target,
									  const struct range* function,
									  void* return_of_function, uint8_t verbose)
{
	uint8_t values_buffer[BUFFER_SIZE_OF];
	void* values = (void*)values_buffer;

	if (!buffer_init(values, BUFFER_SIZE_OF))
	{
		return 0;
	}

	struct range name_space;

	struct range name;

	struct range arguments_area;

	if (!interpreter_disassemble_function(function, &name_space, &name, &arguments_area))
	{
		return 0;
	}

	uint8_t values_count;

	if (!interpreter_get_values_for_arguments(
			the_project, the_target, &arguments_area,
			values, &values_count, verbose))
	{
		buffer_release_with_inner_buffers(values);
		return 0;
	}

	void* the_module = NULL;
	const uint8_t* func = NULL;

	if (common_get_module_priority())
	{
		func = project_get_function_from_module(the_project, &name_space, &name, &the_module, NULL);

		if (NULL != func && NULL != the_module)
		{
			values_count = load_tasks_evaluate_loaded_function(the_module, func, values, values_count,
						   return_of_function, verbose);
			/**/
			buffer_release_with_inner_buffers(values);
			return values_count;
		}
	}

	switch (interpreter_get_unit(name_space.start, name_space.finish))
	{
		case bool_unit:
			values_count = bool_exec_function(
							   conversion_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case cygpath_unit:
			values_count = cygpath_exec_function(
							   path_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case datetime_unit:
			values_count = datetime_exec_function(
							   datetime_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case directory_unit:
		{
			const uint8_t dir_function_id = dir_get_function(name.start, name.finish);

			if (0 == values_count &&
				dir_get_id_of_get_current_directory_function() == dir_function_id)
			{
				const ptrdiff_t size = buffer_size(return_of_function);
				values_count = project_get_current_directory(
								   the_project, the_target,
								   return_of_function, size, verbose);
			}
			else
			{
				values_count = dir_exec_function(dir_function_id, values, values_count, return_of_function);
			}
		}
		break;

		case double_unit:
			values_count = double_exec_function(
							   conversion_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case environment_unit:
			values_count = environment_exec_function(
							   environment_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case file_unit:
			values_count = file_exec_function(
							   file_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case hash_unit:
			values_count = hash_algorithm_exec_function(
							   hash_algorithm_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case int_unit:
			values_count = int_exec_function(
							   conversion_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case int64_unit:
			values_count = int64_exec_function(
							   conversion_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case long_unit:
			values_count = long_exec_function(
							   conversion_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case math_unit:
			values_count = math_exec_function(
							   math_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case operating_system_unit:
			values_count = os_exec_function(os_get_function(name.start, name.finish), values, values_count,
											return_of_function);
			break;

		case path_unit:
		{
			const uint8_t path_function_id = path_get_function(name.start, name.finish);

			if (path_get_id_of_get_full_path_function() == path_function_id)
			{
				if (1 != values_count)
				{
					values_count = 0;
					break;
				}

				struct range path;

				if (!common_get_arguments(values, 1, &path, 0))
				{
					path.start = path.finish = (const uint8_t*)&path;
				}

				const ptrdiff_t size = buffer_size(return_of_function);
				values_count = project_get_current_directory(
								   the_project, the_target,
								   return_of_function, size, verbose);

				if (!values_count)
				{
					break;
				}

				uint8_t full_path_buffer[BUFFER_SIZE_OF];
				void* full_path = (void*)full_path_buffer;

				if (!buffer_init(full_path, BUFFER_SIZE_OF))
				{
					values_count = 0;
					break;
				}

				if (!path_get_full_path(buffer_uint8_t_data(return_of_function, size),
										buffer_uint8_t_data(return_of_function, 0) + buffer_size(return_of_function),
										path.start, path.finish, full_path))
				{
					buffer_release(full_path);
					values_count = 0;
					break;
				}

				values_count = buffer_resize(return_of_function, size) &&
							   buffer_append_data_from_buffer(return_of_function, full_path);
				buffer_release(full_path);
			}
			else
			{
				values_count = path_exec_function(the_project, path_function_id, values, values_count, return_of_function);
			}
		}
		break;

		case platform_unit:
			values_count = platform_exec_function(os_get_function(name.start, name.finish), values, values_count,
												  return_of_function);
			break;

		case program_unit:
			values_count = program_exec_function(project_get_function(name.start, name.finish), values, values_count,
												 return_of_function);
			break;

		case project_unit:
		{
			const void* the_property = NULL;
			const ptrdiff_t size = buffer_size(return_of_function);

			if (!project_exec_function(
					the_project,
					project_get_function(name.start, name.finish),
					values, values_count, &the_property, return_of_function, verbose))
			{
				values_count = 0;
				break;
			}

			values_count = interpreter_actualize_property_value(
							   the_project, the_target, property_get_id_of_get_value_function(),
							   the_property, size, return_of_function, verbose);
		}
		break;

		case property_unit:
		{
			const void* the_property = NULL;
			const uint8_t property_function_id = property_get_function(name.start, name.finish);
			const ptrdiff_t size = buffer_size(return_of_function);

			if (!property_exec_function(the_project, property_function_id, values, values_count, &the_property,
										return_of_function, verbose))
			{
				values_count = 0;
				break;
			}

			values_count = interpreter_actualize_property_value(
							   the_project, the_target, property_function_id,
							   the_property, size, return_of_function, verbose);
		}
		break;

		case string_unit:
			values_count = string_exec_function(
							   string_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case target_unit:
			values_count = target_exec_function(
							   the_project, the_target,
							   target_get_function(name.start, name.finish),
							   values, values_count, return_of_function, verbose);
			break;

		case task_unit:
			values_count = task_exec_function(
							   the_project,
							   task_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case timespan_unit:
			values_count = timespan_exec_function(
							   timespan_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case version_unit:
			values_count = version_exec_function(
							   version_get_function(name.start, name.finish),
							   values, values_count, return_of_function);
			break;

		case UNKNOWN_UNIT:
			if (common_get_module_priority())
			{
				values_count = 0;
				break;
			}

			func = project_get_function_from_module(the_project, &name_space, &name, &the_module, NULL);
			values_count = load_tasks_evaluate_loaded_function(the_module, func, values, values_count,
						   return_of_function, verbose);
			break;

		default:
			values_count = 0;
			break;
	}

	buffer_release_with_inner_buffers(values);
	return values_count;
}

uint8_t interpreter_evaluate_code(
	const void* the_project, const void* the_target,
	const void* the_current_property, const struct range* code,
	void* output, uint8_t verbose)
{
	if (range_is_null_or_empty(code) || NULL == output)
	{
		return 0;
	}

	uint8_t return_of_function_buffer[BUFFER_SIZE_OF];
	void* return_of_function = (void*)return_of_function_buffer;

	if (!buffer_init(return_of_function, BUFFER_SIZE_OF))
	{
		return 0;
	}

	const uint8_t* previous_pos = code->start;
	struct range function;

	for (function.start = code->start; function.start + CALL_OF_EXPRESSION_START_LENGTH <= code->finish;)
	{
		if (0 != memcmp(function.start, call_of_expression_start, CALL_OF_EXPRESSION_START_LENGTH))
		{
			function.start = string_enumerate(function.start, code->finish, NULL);
			continue;
		}

		if (!buffer_resize(return_of_function, 0))
		{
			buffer_release(return_of_function);
			return 0;
		}

		function.finish = function.start;
		uint32_t char_set;

		while (NULL != function.finish && function.finish < code->finish)
		{
			const uint8_t* pos = function.finish;
			function.finish = string_enumerate(function.finish, code->finish, &char_set);

			if (apos == char_set)
			{
				function.finish = string_find_any_symbol_like_or_not_like_that(
									  function.finish, code->finish,
									  &apos, &apos + 1, 1, 1);
				function.finish = string_enumerate(function.finish, code->finish, NULL);
				function.finish = string_find_any_symbol_like_or_not_like_that(
									  function.finish, code->finish,
									  &apos, &apos + 1, 0, 1);
			}
			else if (call_of_expression_finish == char_set)
			{
				function.finish = pos;
				break;
			}
		}

		if (NULL == function.finish)
		{
			buffer_release(return_of_function);
			return 0;
		}

		if (previous_pos < function.start && !buffer_append(output, previous_pos, function.start - previous_pos))
		{
			buffer_release(return_of_function);
			return 0;
		}

		function.start += CALL_OF_EXPRESSION_START_LENGTH;
		void* the_property = NULL;

		if (project_property_exists(
				the_project, function.start,
				(uint8_t)range_size(&function), &the_property, verbose))
		{
			if (!property_get_by_pointer(the_property, return_of_function))
			{
				buffer_release(return_of_function);
				return 0;
			}

			if (NULL != the_current_property &&
				the_current_property == the_property)
			{
				buffer_release(return_of_function);
				return 0;
			}

			if (!interpreter_actualize_property_value(
					the_project, the_target, property_get_id_of_get_value_function(),
					the_property, 0, return_of_function, verbose))
			{
				buffer_release(return_of_function);
				return 0;
			}
		}
		else
		{
			if (!interpreter_evaluate_function(the_project, the_target, &function, return_of_function, verbose))
			{
				buffer_release(return_of_function);
				return 0;
			}
		}

		if (!buffer_append_data_from_buffer(output, return_of_function))
		{
			buffer_release(return_of_function);
			return 0;
		}

		previous_pos = string_enumerate(function.finish, code->finish, NULL);
		function.start = previous_pos;
	}

	buffer_release(return_of_function);
	return buffer_append(output, previous_pos, range_size(code) - (previous_pos - code->start));
}

uint8_t interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
	const void* the_project,
	const void* the_target,
	const uint8_t* start_of_attributes,
	const uint8_t* finish_of_attributes,
	uint8_t* skip,
	void* attributes,
	uint8_t verbose)
{
	static const uint8_t* if_and_unless[] =
	{
		(const uint8_t*)"if",
		(const uint8_t*)"unless"
	};
	/**/
	static const uint8_t if_and_unless_lengths[] = { 2, 6 };
	static const uint8_t bool_values_to_pass[] = { 1, 0 };
	static const uint8_t count_of_attributes = 2;

	if (NULL == skip)
	{
		return 0;
	}

	if (range_in_parts_is_null_or_empty(start_of_attributes, finish_of_attributes))
	{
		*skip = 0;
		return 1;
	}

	if (NULL == attributes)
	{
		return 0;
	}

	if (!common_get_attributes_and_arguments_for_task(
			NULL, NULL, count_of_attributes,
			NULL, NULL, NULL, attributes))
	{
		return 0;
	}

	for (uint8_t i = 0; i < count_of_attributes; ++i)
	{
		void* attribute = buffer_buffer_data(attributes, i);

		if (NULL == attribute || !bool_to_string(bool_values_to_pass[i], attribute))
		{
			return 0;
		}
	}

	if (!interpreter_get_arguments_from_xml_tag_record(
			the_project, the_target, start_of_attributes, finish_of_attributes,
			if_and_unless, if_and_unless_lengths, 0, count_of_attributes, attributes, verbose))
	{
		return 0;
	}

	for (uint8_t i = 0; i < count_of_attributes; ++i)
	{
		const void* attribute = buffer_buffer_data(attributes, i);
		const uint8_t* value = buffer_uint8_t_data(attribute, 0);
		uint8_t bool_value = (uint8_t)buffer_size(attribute);

		if (!bool_parse(value, value + bool_value, &bool_value))
		{
			return 0;
		}

		if (bool_values_to_pass[i] == bool_value)
		{
			continue;
		}

		*skip = 1;
		return 1;
	}

	*skip = 0;
	return 1;
}

uint8_t interpreter_get_xml_tag_attribute_values(
	const void* the_project,
	const void* the_target,
	const uint8_t* start_of_attributes,
	const uint8_t* finish_of_attributes,
	uint8_t* fail_on_error,
	uint8_t* task_verbose,
	void* attributes,
	uint8_t verbose)
{
	static const uint8_t* task_attributes[] =
	{
		(const uint8_t*)"failonerror",
		(const uint8_t*)"verbose"
	};
	static const uint8_t task_attributes_lengths[] = { 11, 7 };

	if (NULL == fail_on_error ||
		NULL == task_verbose ||
		NULL == attributes)
	{
		return 0;
	}

	const uint8_t count_of_attributes = (0 == *task_verbose ? 2 : 1);

	if (!common_get_attributes_and_arguments_for_task(
			NULL, NULL, count_of_attributes,
			NULL, NULL, NULL, attributes))
	{
		return 0;
	}

	if (!interpreter_get_arguments_from_xml_tag_record(the_project, the_target, start_of_attributes,
			finish_of_attributes,
			task_attributes, task_attributes_lengths, 0, count_of_attributes, attributes, verbose))
	{
		return 0;
	}

	const void* attribute;
	uint8_t* outputs[2];
	outputs[0] = fail_on_error;
	outputs[1] = task_verbose;
	uint8_t i = 0;

	while (NULL != (attribute = buffer_buffer_data(attributes, i++)))
	{
		const uint8_t size = (uint8_t)buffer_size(attribute);

		if (!size)
		{
			continue;
		}

		const uint8_t* value = buffer_uint8_t_data(attribute, 0);

		if (!bool_parse(value, value + size, outputs[i - 1]))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t interpreter_get_arguments_from_xml_tag_record(
	const void* the_project, const void* the_target,
	const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
	const uint8_t** attributes, const uint8_t* attributes_lengths,
	uint8_t index, uint8_t attributes_count, void* output, uint8_t verbose)
{
	uint8_t attribute_value_buffer[BUFFER_SIZE_OF];
	void* attribute_value = (void*)attribute_value_buffer;

	if (!buffer_init(attribute_value, BUFFER_SIZE_OF))
	{
		return 0;
	}

	for (; index < attributes_count; ++index)
	{
		void* argument = buffer_buffer_data(output, index);

		if (NULL == argument ||
			!buffer_resize(attribute_value, 0))
		{
			buffer_release(attribute_value);
			return 0;
		}

		if (!xml_get_attribute_value(start_of_attributes, finish_of_attributes,
									 attributes[index], attributes_lengths[index],
									 attribute_value))
		{
			continue;
		}

		struct range code;

		BUFFER_TO_RANGE(code, attribute_value);

		if (!buffer_resize(argument, 0) ||
			((code.start < code.finish) &&
			 !interpreter_evaluate_code(the_project, the_target, NULL, &code, argument, verbose)))
		{
			buffer_release(attribute_value);
			return 0;
		}
	}

	buffer_release(attribute_value);
	return 1;
}

uint8_t interpreter_get_xml_element_value(
	const void* the_project,
	const void* the_target,
	const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	void* output,
	uint8_t verbose)
{
	uint8_t value_buffer[BUFFER_SIZE_OF];
	void* value = (void*)value_buffer;

	if (!buffer_init(value, BUFFER_SIZE_OF))
	{
		return 0;
	}

	if (!xml_get_element_value(attributes_finish, element_finish, value))
	{
		buffer_release(value);
		return 0;
	}

	struct range code;

	BUFFER_TO_RANGE(code, value);

	if (!range_is_null_or_empty(&code) &&
		!interpreter_evaluate_code(the_project, the_target, NULL, &code, output, verbose))
	{
		buffer_release(value);
		return 0;
	}

	buffer_release(value);
	return 1;
}

enum interpreter_task
{
	attrib_task,
	call_task,
	choose_task,
	copy_task,
	delete_task,
	description_task,
	do_task,
	echo_task,
	exec_task,
	fail_task,
	foreach_task,
	if_task,
#if 0
	include_task,
#endif
	loadfile_task,
	loadtasks_task,
	mkdir_task,
	move_task,
	program_task,
	project_task,
	property_task,
#if 0
	setenv_task,
#endif
	sleep_task,
	target_task,
	touch_task,
	trycatch_task,
#if 0
	tstamp_task,
	uptodate_task,
	xmlpeek_task,
	xmlpoke_task,
#endif
	UNKNOWN_TASK
};

uint8_t interpreter_get_task(const uint8_t* task_name_start, const uint8_t* task_name_finish)
{
	static const uint8_t* interpreter_task_names[] =
	{
		(const uint8_t*)"attrib",
		(const uint8_t*)"call",
		(const uint8_t*)"choose",
		(const uint8_t*)"copy",
		(const uint8_t*)"delete",
		(const uint8_t*)"description",
		(const uint8_t*)"do",
		(const uint8_t*)"echo",
		(const uint8_t*)"exec",
		(const uint8_t*)"fail",
		(const uint8_t*)"foreach",
		(const uint8_t*)"if",
#if 0
		(const uint8_t*)"include",
#endif
		(const uint8_t*)"loadfile",
		(const uint8_t*)"loadtasks",
		(const uint8_t*)"mkdir",
		(const uint8_t*)"move",
		(const uint8_t*)"program",
		(const uint8_t*)"project",
		(const uint8_t*)"property",
#if 0
		(const uint8_t*)"setenv",
#endif
		(const uint8_t*)"sleep",
		(const uint8_t*)"target",
		(const uint8_t*)"touch",
		(const uint8_t*)"trycatch",
#if 0
		(const uint8_t*)"tstamp",
		(const uint8_t*)"uptodate",
		(const uint8_t*)"xmlpeek",
		(const uint8_t*)"xmlpoke"
#endif
	};
	/**/
	return common_string_to_enum(
			   task_name_start, task_name_finish,
			   interpreter_task_names, UNKNOWN_TASK);
}

uint8_t interpreter_prepare_attributes_and_arguments_for_property_task(
	const void* the_project, const void* the_target,
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	uint8_t verbose)
{
	if (NULL == the_project ||
		NULL == task_attributes ||
		NULL == task_attributes_lengths ||
		NULL == task_attributes_count ||
		NULL == task_arguments)
	{
		return 0;
	}

	if (!property_get_attributes_and_arguments_for_task(
			task_attributes, task_attributes_lengths,
			task_attributes_count, task_arguments))
	{
		return 0;
	}

	if (!interpreter_get_arguments_from_xml_tag_record(
			the_project, the_target, attributes_start, attributes_finish,
			*task_attributes, *task_attributes_lengths, 0, 1, task_arguments, verbose))
	{
		return 0;
	}

	void* argument = buffer_buffer_data(task_arguments, 0);
	const uint8_t* value = buffer_uint8_t_data(argument, 0);
	uint8_t dynamic = (uint8_t)buffer_size(argument);

	if (dynamic && !bool_parse(value, value + dynamic, &dynamic))
	{
		return 0;
	}

	if (dynamic)
	{
		dynamic = *task_attributes_count - 1;

		if (!interpreter_get_arguments_from_xml_tag_record(
				the_project, the_target, attributes_start, attributes_finish,
				*task_attributes, *task_attributes_lengths,
				1, dynamic, task_arguments, verbose))
		{
			return 0;
		}

		argument = buffer_buffer_data(task_arguments, dynamic);

		if (!buffer_resize(argument, 0) ||
			!xml_get_attribute_value(attributes_start, attributes_finish,
									 (*task_attributes)[dynamic], (*task_attributes_lengths)[dynamic],
									 argument))
		{
			return 0;
		}
	}
	else
	{
		if (!interpreter_get_arguments_from_xml_tag_record(
				the_project, the_target, attributes_start, attributes_finish,
				*task_attributes, *task_attributes_lengths,
				1, *task_attributes_count, task_arguments, verbose))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t interpreter_evaluate_task(
	void* the_project, const void* the_target, const struct range* task_name,
	const uint8_t* element_finish, const struct range* sub_nodes_names,
	uint8_t target_help, uint8_t verbose)
{
	uint8_t task_arguments_buffer[BUFFER_SIZE_OF];
	void* task_arguments = (void*)&task_arguments_buffer;

	if (!buffer_init(task_arguments, BUFFER_SIZE_OF))
	{
		return 0;
	}

	void* the_module = NULL;

	if (range_is_null_or_empty(task_name))
	{
		listener_task_started(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name,
							  UNKNOWN_TASK, (const uint8_t*)the_module, verbose);
		project_on_failure(the_project, the_target, task_arguments, verbose);
		buffer_release(task_arguments);
		listener_task_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name,
							   UNKNOWN_TASK, (const uint8_t*)the_module, 0, verbose);
		/**/
		return 0;
	}

	ptrdiff_t task_id = interpreter_get_task(task_name->start, task_name->finish);
	listener_task_started(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id,
						  (const uint8_t*)the_module, verbose);
	const uint8_t* attributes_start = task_name->finish;

	if (range_in_parts_is_null_or_empty(attributes_start, element_finish))
	{
		project_on_failure(the_project, the_target, task_arguments, verbose);
		buffer_release(task_arguments);
		listener_task_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id,
							   (const uint8_t*)the_module, 0, verbose);
		/**/
		return 0;
	}

	const uint8_t** task_attributes = NULL;
	const uint8_t* task_attributes_lengths = NULL;
	uint8_t task_attributes_count = 0;
	/**/
	const uint8_t* attributes_finish = xml_get_tag_finish_pos(attributes_start, element_finish);

	if (target_task == task_id)
	{
		if (!target_get_attributes_and_arguments_for_task(
				&task_attributes, &task_attributes_lengths, &task_attributes_count, task_arguments) ||
			task_attributes_count < 1)
		{
			buffer_release_inner_buffers(task_arguments);
			project_on_failure(the_project, the_target, task_arguments, verbose);
			buffer_release(task_arguments);
			listener_task_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id,
								   (const uint8_t*)the_module, 0, verbose);
			/**/
			return 0;
		}

		if (!target_help)
		{
			task_attributes_count -= 1;
		}

		if (!interpreter_get_arguments_from_xml_tag_record(
				the_project, the_target, attributes_start, attributes_finish,
				task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
		{
			buffer_release_inner_buffers(task_arguments);
			project_on_failure(the_project, the_target, task_arguments, verbose);
			buffer_release(task_arguments);
			listener_task_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id,
								   (const uint8_t*)the_module, 0, verbose);
			/**/
			return 0;
		}

		task_attributes_count = target_evaluate_task(
									the_project, task_arguments, target_help,
									attributes_start, attributes_finish, element_finish, sub_nodes_names, verbose);

		if (task_attributes_count)
		{
			buffer_release_with_inner_buffers(task_arguments);
		}
		else
		{
			buffer_release_inner_buffers(task_arguments);
			project_on_failure(the_project, the_target, task_arguments, verbose);
			buffer_release(task_arguments);
		}

		listener_task_finished(
			NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id, NULL,
			task_attributes_count, verbose);
		return task_attributes_count;
	}

	if (!interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
			the_project, the_target, attributes_start, attributes_finish,
			&task_attributes_count, task_arguments, verbose))
	{
		buffer_release_inner_buffers(task_arguments);
		project_on_failure(the_project, the_target, task_arguments, verbose);
		buffer_release(task_arguments);
		listener_task_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id,
							   (const uint8_t*)the_module, 0, verbose);
		/**/
		return 0;
	}

	if (task_attributes_count)
	{
		buffer_release_with_inner_buffers(task_arguments);
		listener_task_finished(
			NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id, NULL,
			task_attributes_count, verbose);
		/**/
		return task_attributes_count;
	}

	buffer_release_inner_buffers(task_arguments);
	uint8_t fail_on_error = 1;

	if (!interpreter_get_xml_tag_attribute_values(
			the_project, the_target, attributes_start, attributes_finish,
			&fail_on_error, &verbose, task_arguments, verbose))
	{
		buffer_release_inner_buffers(task_arguments);
		project_on_failure(the_project, the_target, task_arguments, verbose);
		buffer_release(task_arguments);
		listener_task_finished(NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id,
							   (const uint8_t*)the_module, 0, verbose);
		/**/
		return 0;
	}

	buffer_release_inner_buffers(task_arguments);
	const uint8_t* pointer_to_the_task = NULL;

	if (common_get_module_priority())
	{
		pointer_to_the_task = project_get_task_from_module(the_project, task_name, &the_module, &task_id);

		if (NULL != pointer_to_the_task &&
			NULL != the_module)
		{
			task_attributes_count = load_tasks_evaluate_loaded_task(
										the_project, the_target, task_name->finish,
										attributes_finish, element_finish, task_arguments,
										the_module, pointer_to_the_task, verbose);

			if (task_attributes_count)
			{
				buffer_release_with_inner_buffers(task_arguments);
			}
			else
			{
				buffer_release_inner_buffers(task_arguments);
				project_on_failure(the_project, the_target, task_arguments, verbose);
				buffer_release(task_arguments);
			}

			listener_task_finished(
				NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id,
				(const uint8_t*)the_module, task_attributes_count, verbose);
			return task_attributes_count;
		}
	}

	switch (task_id)
	{
		case attrib_task:
			if (!attrib_get_attributes_and_arguments_for_task(
					&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = attrib_evaluate_task(task_arguments, verbose);
			break;

		case call_task:
			if (!call_get_attributes_and_arguments_for_task(
					&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = call_evaluate_task(the_project, task_arguments, verbose);
			break;

		case choose_task:
			task_attributes_count = choose_evaluate_task(the_project, the_target, attributes_finish, element_finish,
									task_arguments, verbose);
			break;

		case copy_task:
			if (!copy_move_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = copy_evaluate_task(the_project, the_target, task_arguments, verbose);
			break;

		case delete_task:
			if (!delete_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = delete_evaluate_task(task_arguments, verbose);
			break;

		case description_task:
			if (target_help)
			{
				if (!buffer_resize(task_arguments, 0) ||
					!xml_get_element_value(attributes_finish, element_finish, task_arguments))
				{
					task_attributes_count = 0;
					break;
				}

				task_attributes_count =
					echo(0, UTF8, NULL, Info, buffer_uint8_t_data(task_arguments, 0), buffer_size(task_arguments), 1, verbose);
				buffer_release(task_arguments);
			}
			else
			{
				task_attributes_count = 1;
			}

			break;

		case do_task:
			task_attributes_count = do_evaluate_task(
										the_project, the_target, attributes_finish, element_finish, task_arguments, verbose);
			break;
#ifdef ENABLED

		case echo_task:
			if (!echo_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			{
				void* message = buffer_buffer_data(task_arguments, task_attributes_count - 1);

				if (!buffer_size(message) && attributes_finish < element_finish)
				{
					if (!interpreter_get_xml_element_value(the_project, the_target, attributes_finish, element_finish, message,
														   verbose))
					{
						task_attributes_count = 0;
						break;
					}
				}
			}

			task_attributes_count = echo_evaluate_task(task_arguments, verbose);
			break;
#endif

		case exec_task:
			if (!exec_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			if (attributes_finish < element_finish)
			{
				void* environments = buffer_buffer_data(task_arguments, task_attributes_count);

				if (!interpreter_get_environments(the_project, the_target, attributes_finish, element_finish, environments,
												  verbose))
				{
					task_attributes_count = 0;
					break;
				}
			}

			task_attributes_count = exec_evaluate_task(the_project, the_target, task_arguments, verbose);
			break;

		case fail_task:
			if (!fail_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = fail_evaluate_task(the_project, the_target, attributes_finish, element_finish,
									task_arguments, verbose);
			break;

		case foreach_task:
			if (!for_each_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = for_each_evaluate_task(the_project, the_target, attributes_finish, element_finish,
									task_arguments, fail_on_error, verbose);
			break;

		case if_task:
			if (!if_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = if_evaluate_task(
										the_project, the_target, task_arguments, attributes_finish, element_finish, verbose);
			break;
#if 0

		case include_:
			break;
#endif

		case loadfile_task:
			if (!load_file_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = load_file_evaluate_task(the_project, task_arguments, verbose);
			break;

		case loadtasks_task:
			if (!load_tasks_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = load_tasks_evaluate_task(the_project, the_target, task_arguments, verbose);
			break;

		case mkdir_task:
			if (!mkdir_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = mkdir_evaluate_task(task_arguments, verbose);
			break;

		case move_task:
			if (!copy_move_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = move_evaluate_task(the_project, the_target, task_arguments, verbose);
			break;

		case program_task:
			if (!program_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = program_evaluate_task(
										the_project, the_target, task_arguments, attributes_finish, element_finish, verbose);
			break;

		case project_task:
			if (!project_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = project_evaluate_task(the_project,
									attributes_finish, element_finish,
									sub_nodes_names, target_help,
									task_arguments, verbose);
			break;

		case property_task:
			task_attributes_count = interpreter_prepare_attributes_and_arguments_for_property_task(
										the_project, the_target,
										&task_attributes, &task_attributes_lengths, &task_attributes_count, task_arguments,
										attributes_start, attributes_finish, verbose);

			if (task_attributes_count)
			{
				task_attributes_count = property_evaluate_task(the_project, NULL, task_arguments, verbose);
			}

			break;
#if 0

		case setenv_:
			break;
#endif

		case sleep_task:
			if (!sleep_unit_get_attributes_and_arguments_for_task(
					&task_attributes, &task_attributes_lengths, &task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = sleep_unit_evaluate_task(task_arguments, verbose);
			break;

		case touch_task:
			if (!touch_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = touch_evaluate_task(task_arguments, verbose);
			break;

		case trycatch_task:
			task_attributes_count = try_catch_evaluate_task(
										the_project, the_target, attributes_finish, element_finish,
										task_arguments, verbose);
			break;
#if 0

		case tstamp_:
			break;

		case uptodate_:
			break;

		case xmlpeek_:
			break;

		case xmlpoke_:
			break;
#endif

		case UNKNOWN_TASK:
			if (common_get_module_priority())
			{
				task_attributes_count = 0;
				break;
			}

			pointer_to_the_task = project_get_task_from_module(the_project, task_name, &the_module, &task_id);
			task_attributes_count = load_tasks_evaluate_loaded_task(
										the_project, the_target, task_name->finish,
										attributes_finish, element_finish,
										task_arguments, the_module, pointer_to_the_task, verbose);
			break;

		default:
			break;
	}

	if (!task_attributes_count && !fail_on_error)
	{
		task_attributes_count = FAIL_WITH_OUT_ERROR;
	}

	if (task_attributes_count)
	{
		buffer_release_with_inner_buffers(task_arguments);
	}
	else
	{
		buffer_release_inner_buffers(task_arguments);
		project_on_failure(the_project, the_target, task_arguments, verbose);
		buffer_release(task_arguments);
	}

	listener_task_finished(
		NULL, 0, (const uint8_t*)the_project, (const uint8_t*)the_target, task_name, task_id,
		(const uint8_t*)the_module, task_attributes_count, verbose);
	return task_attributes_count;
}

uint8_t interpreter_evaluate_tasks(
	void* the_project, const void* the_target,
	const void* elements, const struct range* sub_nodes_names,
	uint8_t target_help, uint8_t verbose)
{
	ptrdiff_t i = 0;
	uint8_t returned = 1;
	struct range* element;

	while (NULL != (element = buffer_range_data(elements, i++)))
	{
		struct range tag_name;
		tag_name.start = element->start;
		tag_name.finish = xml_get_tag_name(element->start, element->finish);
		/**/
		returned = interpreter_evaluate_task(
					   the_project, the_target, &tag_name, element->finish,
					   sub_nodes_names, target_help, verbose);

		if (!returned)
		{
			break;
		}
	}

	return returned;
}

uint8_t interpreter_get_unknown_task_id()
{
	return UNKNOWN_TASK;
}
