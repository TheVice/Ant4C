/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "interpreter.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "copy_move.h"
#include "date_time.h"
#include "echo.h"
#include "environment.h"
#include "exec.h"
#include "file_system.h"
#include "for_each.h"
#include "hash.h"
#include "listener.h"
#include "load_file.h"
#include "load_tasks.h"
#include "math_unit.h"
#include "operating_system.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "sleep_unit.h"
#include "string_unit.h"
#include "target.h"
#include "task.h"
#include "text_encoding.h"
#include "try_catch.h"
#include "version.h"
#include "xml.h"

static const uint8_t start_of_function_arguments_area = '(';
static const uint8_t finish_of_function_arguments_area = ')';
static const uint8_t function_call_finish = '}';
static const uint8_t apos = '\'';

static const uint8_t arguments_delimiter = ',';

static const uint8_t space_and_tab[] = { ' ', '\t' };
#define SPACE_AND_TAB_LENGTH COUNT_OF(space_and_tab)

static const uint8_t name_space_border[] = { ':', ':' };
#define NAME_SPACE_BORDER_LENGTH COUNT_OF(name_space_border)

static const uint8_t function_call_start[] = { '$', '{' };
#define FUNCTION_CALL_START_LENGTH COUNT_OF(function_call_start)

static const uint8_t* test = (const uint8_t*)"test";
static const uint8_t test_length = 4;

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
	return common_string_to_enum(name_space_start, name_space_finish, interpreter_string_enumeration_unit,
								 UNKNOWN_UNIT);
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

	ptrdiff_t index = 0;

	if (-1 == (index = string_index_of(function->start,
									   function->finish,
									   name_space_border,
									   name_space_border + NAME_SPACE_BORDER_LENGTH)))
	{
		return 0;
	}

	name_space->start = function->start;
	name_space->finish = function->start + index;
	/**/
	name->start = name_space->finish + NAME_SPACE_BORDER_LENGTH;
	name->finish = find_any_symbol_like_or_not_like_that(
					   name->start, function->finish, &start_of_function_arguments_area, 1, 1, 1);

	if (function->finish == name->finish)
	{
		return 0;
	}

	arguments_area->start = name->finish + 1;
	arguments_area->finish = find_any_symbol_like_or_not_like_that(
								 function->finish, arguments_area->start, &finish_of_function_arguments_area, 1, 1, -1);
	/**/
	return finish_of_function_arguments_area == *arguments_area->finish;
}

uint8_t interpreter_get_function_from_argument(
	struct range* argument_area)
{
	if (range_is_null_or_empty(argument_area))
	{
		return 0;
	}

	const uint8_t* finish = argument_area->finish;
	argument_area->finish = find_any_symbol_like_or_not_like_that(argument_area->start, argument_area->finish,
							&start_of_function_arguments_area, 1, 1, 1);

	if (finish == argument_area->finish)
	{
		return 0;
	}

	uint8_t ch = 0;
	uint8_t depth = 0;

	while (argument_area->finish < finish)
	{
		++argument_area->finish;
		ch = *argument_area->finish;

		if (finish_of_function_arguments_area == ch)
		{
			if (0 == depth)
			{
				++argument_area->finish;
				break;
			}

			--depth;
		}

		if (start_of_function_arguments_area == ch)
		{
			++depth;
		}
	}

	if (finish_of_function_arguments_area != ch || 0 != depth)
	{
		return 0;
	}

	return 1;
}

uint8_t interpreter_evaluate_argument_area(
	const void* the_project, const void* the_target,
	const struct range* argument_area, struct buffer* output, uint8_t verbose)
{
	ptrdiff_t index = (apos == *(argument_area->start) && apos == *(argument_area->finish - 1));
	const uint8_t* pos = argument_area->start;

	if (!index)
	{
		while (-1 != (index = string_index_of(
								  pos, argument_area->finish,
								  name_space_border, name_space_border + NAME_SPACE_BORDER_LENGTH)))
		{
			struct range function;
			function.start = find_any_symbol_like_or_not_like_that(pos + index, pos,
							 space_and_tab, SPACE_AND_TAB_LENGTH, 1, -1);
			function.finish = argument_area->finish;

			if (!string_trim(&function) ||
				!interpreter_get_function_from_argument(&function) ||
				!buffer_append(output, pos, function.start - pos) ||
				!interpreter_evaluate_function(the_project, the_target, &function, output, verbose))
			{
				return 0;
			}

			pos = function.finish;
		}
	}

	return (pos != argument_area->start) ? buffer_append(output, pos, argument_area->finish - pos) : 1;
}

uint8_t interpreter_actualize_property_value(
	const void* the_project, const void* the_target,
	uint8_t property_function_id, const void* the_property,
	ptrdiff_t size, struct buffer* output, uint8_t verbose)
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

	struct buffer code_in_buffer;

	SET_NULL_TO_BUFFER(code_in_buffer);

	if (!buffer_append(&code_in_buffer, buffer_data(output, size),
					   buffer_size(output) - size) ||
		!buffer_resize(output, size))
	{
		buffer_release(&code_in_buffer);
		return 0;
	}

	struct range code;

	code.start = buffer_data(&code_in_buffer, 0);

	code.finish = code.start + buffer_size(&code_in_buffer);

	if (code.start < code.finish &&
		!interpreter_evaluate_code(the_project, the_target, &code, output, verbose))
	{
		buffer_release(&code_in_buffer);
		return 0;
	}

	buffer_release(&code_in_buffer);
	return 1;
}

uint8_t interpreter_get_value_for_argument(
	const void* the_project, const void* the_target,
	struct range* argument_area, struct buffer* values, uint8_t verbose)
{
	struct buffer value;
	SET_NULL_TO_BUFFER(value);

	if (range_is_null_or_empty(argument_area) ||
		NULL == values)
	{
		buffer_release(&value);
		return 0;
	}

	if (!string_trim(argument_area))
	{
		buffer_release(&value);
		return 0;
	}

	if (!interpreter_evaluate_argument_area(the_project, the_target, argument_area, &value, verbose))
	{
		buffer_release(&value);
		return 0;
	}

	if (!buffer_size(&value))
	{
		void* the_property = NULL;

		if (project_property_exists(the_project, argument_area->start, (uint8_t)range_size(argument_area),
									&the_property, verbose))
		{
			if (!property_get_by_pointer(the_property, &value))
			{
				buffer_release(&value);
				return 0;
			}

			if (!interpreter_actualize_property_value(
					the_project, the_target, property_get_id_of_get_value_function(),
					the_property, 0, &value, verbose))
			{
				buffer_release(&value);
				return 0;
			}
		}
		else
		{
			const ptrdiff_t index_1 = string_index_of(
										  argument_area->start, argument_area->finish, name_space_border,
										  name_space_border + NAME_SPACE_BORDER_LENGTH);
			const ptrdiff_t index_2 = -1 == index_1 ? index_1 :
									  string_last_index_of(
										  argument_area->start, argument_area->finish, name_space_border,
										  name_space_border + NAME_SPACE_BORDER_LENGTH);

			if (-1 == index_1 || index_1 != index_2)
			{
				if (!buffer_resize(&value, 0) ||
					!string_un_quote(argument_area) ||
					!buffer_append_data_from_range(&value, argument_area))
				{
					buffer_release(&value);
					return 0;
				}
			}
		}
	}

	/*TODO: value can be represent the quote or property name, recursion in this case should be used at else.*/
	return buffer_append_buffer(values, &value, 1);
}

uint8_t interpreter_get_values_for_arguments(
	const void* the_project, const void* the_target,
	const struct range* arguments_area, struct buffer* values, uint8_t verbose)
{
	if (range_is_null_or_empty(arguments_area) || NULL == values)
	{
		return 0;
	}

	uint8_t count = 0;
	uint8_t depth = 0;
	/**/
	struct range argument_area;
	argument_area.start = arguments_area->start;
	argument_area.finish = arguments_area->start;

	while (argument_area.finish < arguments_area->finish)
	{
		const uint8_t ch = *argument_area.finish;

		if (apos == ch)
		{
			argument_area.finish = find_any_symbol_like_or_not_like_that(
									   argument_area.finish + 1, arguments_area->finish, &apos, 1, 1, 1);
			argument_area.finish = find_any_symbol_like_or_not_like_that(
									   argument_area.finish + 1, arguments_area->finish, &apos, 1, 0, 1);
			continue;
		}
		else if (arguments_delimiter == ch && 0 == depth)
		{
			const uint8_t* pos = argument_area.finish + 1;/*TODO: MIN(pos, arguments_area->finish)*/

			if (!interpreter_get_value_for_argument(the_project, the_target, &argument_area, values, verbose))
			{
				return 0;
			}

			++count;
			/**/
			argument_area.start = pos;
			argument_area.finish = pos;
		}
		else if (start_of_function_arguments_area == ch)
		{
			if (UINT8_MAX == depth)
			{
				/*TODO:*/
				break;
			}

			++depth;
		}
		else if (finish_of_function_arguments_area == ch)
		{
			if (0 == depth)
			{
				/*TODO:*/
				break;
			}

			--depth;
		}

		++argument_area.finish;
	}

	if (!interpreter_get_value_for_argument(the_project, the_target, &argument_area, values, verbose))
	{
		return 0;
	}

	++count;
	return count;
}

uint8_t interpreter_evaluate_function(const void* the_project, const void* the_target,
									  const struct range* function,
									  struct buffer* return_of_function, uint8_t verbose)
{
	struct buffer values;
	SET_NULL_TO_BUFFER(values);
	/**/
	struct range name_space;
	struct range name;
	struct range arguments_area;

	if (!interpreter_disassemble_function(function, &name_space, &name, &arguments_area))
	{
		return 0;
	}

	uint8_t values_count = interpreter_get_values_for_arguments(
							   the_project, the_target, &arguments_area, &values, verbose);
	/**/
	void* the_module = NULL;
	const uint8_t* func = NULL;
	/**/
	uint8_t module_priority = 0;/*TODO*/

	if (module_priority)
	{
		func = project_get_function_from_module(the_project, &name_space, &name, &the_module, NULL);

		if (NULL != func && NULL != the_module)
		{
			values_count = load_tasks_evaluate_loaded_function(the_module, func, &values, values_count,
						   return_of_function,
						   verbose);
			/**/
			buffer_release_with_inner_buffers(&values);
			return values_count;
		}
	}

	switch (interpreter_get_unit(name_space.start, name_space.finish))
	{
		case bool_unit:
			values_count = bool_exec_function(
							   conversion_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case cygpath_unit:
			values_count = cygpath_exec_function(
							   path_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case datetime_unit:
			values_count = datetime_exec_function(
							   datetime_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
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
				values_count = dir_exec_function(dir_function_id, &values, values_count, return_of_function);
			}
		}
		break;

		case double_unit:
			values_count = double_exec_function(
							   conversion_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case environment_unit:
			values_count = environment_exec_function(
							   environment_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case file_unit:
			values_count = file_exec_function(
							   file_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case hash_unit:
			values_count = hash_algorithm_exec_function(
							   hash_algorithm_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case int_unit:
			values_count = int_exec_function(
							   conversion_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case int64_unit:
			values_count = int64_exec_function(
							   conversion_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case long_unit:
			values_count = long_exec_function(
							   conversion_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case math_unit:
			values_count = math_exec_function(
							   math_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case operating_system_unit:
			values_count = os_exec_function(os_get_function(name.start, name.finish), &values, values_count,
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

				if (!common_get_one_argument(&values, &path, 0))
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

				struct buffer full_path;

				SET_NULL_TO_BUFFER(full_path);

				if (!path_get_full_path(buffer_data(return_of_function, size),
										buffer_data(return_of_function, 0) + buffer_size(return_of_function),
										path.start, path.finish, &full_path))
				{
					buffer_release(&full_path);
					values_count = 0;
					break;
				}

				values_count = buffer_resize(return_of_function, size) &&
							   buffer_append_data_from_buffer(return_of_function, &full_path);
				buffer_release(&full_path);
			}
			else
			{
				values_count = path_exec_function(the_project, path_function_id, &values, values_count, return_of_function);
			}
		}
		break;

		case platform_unit:
			values_count = platform_exec_function(os_get_function(name.start, name.finish), &values, values_count,
												  return_of_function);
			break;

		case program_unit:
			values_count = program_exec_function(project_get_function(name.start, name.finish), &values, values_count,
												 return_of_function);
			break;

		case project_unit:
		{
			const void* the_property = NULL;
			const ptrdiff_t size = buffer_size(return_of_function);

			if (!project_exec_function(
					the_project,
					project_get_function(name.start, name.finish),
					&values, values_count, &the_property, return_of_function, verbose))
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

			if (!property_exec_function(the_project, property_function_id, &values, values_count, &the_property,
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
							   &values, values_count, return_of_function);
			break;

		case target_unit:
			values_count = target_exec_function(
							   the_project, the_target,
							   target_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case task_unit:
			values_count = task_exec_function(
							   the_project,
							   task_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case timespan_unit:
			values_count = timespan_exec_function(
							   timespan_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case version_unit:
			values_count = version_exec_function(
							   version_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case UNKNOWN_UNIT:
			if (module_priority)
			{
				values_count = 0;
				break;
			}

			func = project_get_function_from_module(the_project, &name_space, &name, &the_module, NULL);
			values_count = load_tasks_evaluate_loaded_function(the_module, func, &values, values_count,
						   return_of_function,
						   verbose);
			break;

		default:
			values_count = 0;
			break;
	}

	buffer_release_with_inner_buffers(&values);
	return values_count;
}

uint8_t interpreter_evaluate_code(const void* the_project, const void* the_target,
								  const struct range* code, struct buffer* output, uint8_t verbose)
{
	if (range_is_null_or_empty(code) || NULL == output)
	{
		return 0;
	}

	const ptrdiff_t code_length = range_size(code);
	struct buffer return_of_function;
	SET_NULL_TO_BUFFER(return_of_function);
	ptrdiff_t index = 0;
	struct range function;
	function.start = code->start;
	function.finish = code->start;
	const uint8_t* previous_pos = code->start;

	while (-1 != (index = string_index_of(function.start,
										  code->finish,
										  function_call_start,
										  function_call_start + FUNCTION_CALL_START_LENGTH)))
	{
		if (!buffer_resize(&return_of_function, 0))
		{
			buffer_release(&return_of_function);
			return 0;
		}

		function.start += index;
		function.finish = function.start;

		while (function.finish < code->finish)
		{
			if (apos == (*function.finish))
			{
				function.finish = find_any_symbol_like_or_not_like_that(
									  function.finish + 1, code->finish,
									  &apos, 1, 1, 1);
				function.finish = find_any_symbol_like_or_not_like_that(
									  function.finish + 1, code->finish,
									  &apos, 1, 0, 1);
				continue;
			}

			if (function_call_finish == (*function.finish))
			{
				break;
			}

			++function.finish;
		}

		if (function.finish == code->finish && function_call_finish != *function.finish)
		{
			buffer_release(&return_of_function);
			return 0;
		}

		if (previous_pos < function.start && !buffer_append(output, previous_pos, function.start - previous_pos))
		{
			buffer_release(&return_of_function);
			return 0;
		}

		function.start += 2;
		void* the_property = NULL;

		if (project_property_exists(the_project, function.start, (uint8_t)range_size(&function), &the_property,
									verbose))
		{
			if (!property_get_by_pointer(the_property, &return_of_function))
			{
				buffer_release(&return_of_function);
				return 0;
			}

			if (!interpreter_actualize_property_value(
					the_project, the_target, property_get_id_of_get_value_function(),
					the_property, 0, &return_of_function, verbose))
			{
				buffer_release(&return_of_function);
				return 0;
			}
		}
		else
		{
			if (!interpreter_evaluate_function(the_project, the_target, &function, &return_of_function, verbose))
			{
				buffer_release(&return_of_function);
				return 0;
			}
		}

		if (!buffer_append(output, return_of_function.data, buffer_size(&return_of_function)))
		{
			buffer_release(&return_of_function);
			return 0;
		}

		previous_pos = 1 + function.finish;
		function.start = previous_pos;
	}

	buffer_release(&return_of_function);
	return buffer_append(output, previous_pos, code_length - (previous_pos - code->start));
}

uint8_t interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
	const void* the_project,
	const void* the_target,
	const uint8_t* start_of_attributes,
	const uint8_t* finish_of_attributes,
	uint8_t* skip,
	struct buffer* attributes,
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
		struct buffer* attribute = buffer_buffer_data(attributes, i);

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
		const struct buffer* attribute = buffer_buffer_data(attributes, i);
		uint8_t bool_value = 0;

		if (!bool_parse(buffer_data(attribute, 0), buffer_size(attribute), &bool_value))
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
	struct buffer* attributes,
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

	const uint8_t count_of_attributes = (0 == (*task_verbose) ? 2 : 1);

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

	const struct buffer* ptr = NULL;
	uint8_t* outputs[2];
	outputs[0] = fail_on_error;
	outputs[1] = task_verbose;
	uint8_t i = 0;

	while (NULL != (ptr = buffer_buffer_data(attributes, i++)))
	{
		if (!buffer_size(ptr))
		{
			continue;
		}

		if (!bool_parse(buffer_data(ptr, 0), buffer_size(ptr), outputs[i - 1]))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t interpreter_get_arguments_from_xml_tag_record(const void* the_project, const void* the_target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
		const uint8_t** attributes, const uint8_t* attributes_lengths,
		uint8_t index, uint8_t attributes_count, struct buffer* output, uint8_t verbose)
{
	struct buffer attribute_value;
	SET_NULL_TO_BUFFER(attribute_value);

	for (; index < attributes_count; ++index)
	{
		struct buffer* argument = buffer_buffer_data(output, index);

		if (NULL == argument ||
			!buffer_resize(&attribute_value, 0))
		{
			buffer_release(&attribute_value);
			return 0;
		}

		if (!xml_get_attribute_value(start_of_attributes, finish_of_attributes,
									 attributes[index], attributes_lengths[index],
									 &attribute_value))
		{
			continue;
		}

		struct range code;

		code.start = buffer_data(&attribute_value, 0);

		code.finish = code.start + buffer_size(&attribute_value);

		if (!buffer_resize(argument, 0) ||
			((code.start < code.finish) &&
			 !interpreter_evaluate_code(the_project, the_target, &code, argument, verbose)))
		{
			buffer_release(&attribute_value);
			return 0;
		}
	}

	buffer_release(&attribute_value);
	return 1;
}

uint8_t interpreter_get_xml_element_value(
	const void* the_project,
	const void* the_target,
	const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	struct buffer* output,
	uint8_t verbose)
{
	struct buffer value;
	SET_NULL_TO_BUFFER(value);

	if (!xml_get_element_value(attributes_finish, element_finish, &value))
	{
		buffer_release(&value);
		return 0;
	}

	struct range code;

	code.start = buffer_data(&value, 0);

	code.finish = code.start + buffer_size(&value);

	if (!range_is_null_or_empty(&code) &&
		!interpreter_evaluate_code(the_project, the_target, &code, output, verbose))
	{
		buffer_release(&value);
		return 0;
	}

	buffer_release(&value);
	return 1;
}

uint8_t interpreter_get_environments(
	const void* the_project,
	const void* the_target,
	const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	struct buffer* environments,
	uint8_t verbose)
{
	if (range_in_parts_is_null_or_empty(attributes_finish, element_finish) ||
		NULL == environments)
	{
		return 0;
	}

	struct buffer elements;

	SET_NULL_TO_BUFFER(elements);

	if (!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		return 0;
	}

	uint16_t count = xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, &elements);

	if (!count)
	{
		buffer_release(&elements);
		return 1;
	}

	count = 0;
	struct range name;
	struct range* env_ptr = NULL;

	while (NULL != (env_ptr = buffer_range_data(&elements, count++)))
	{
		static const uint8_t* env_name = (const uint8_t*)"environment";

		if (!xml_get_tag_name(env_ptr->start, env_ptr->finish, &name))
		{
			buffer_release(&elements);
			return 0;
		}

		if (string_equal(name.start, name.finish, env_name, env_name + 11))
		{
			break;
		}
	}

	if (NULL == env_ptr)
	{
		buffer_release(&elements);
		return 1;
	}

	name = *env_ptr;

	if (!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		return 0;
	}

	count = xml_get_sub_nodes_elements(name.start, name.finish, NULL, &elements);

	if (!count)
	{
		buffer_release(&elements);
		return 1;
	}

	count = 0;
	/**/
	struct buffer attribute_value;
	SET_NULL_TO_BUFFER(attribute_value);

	while (NULL != (env_ptr = buffer_range_data(&elements, count++)))
	{
		static const uint8_t zero_symbol = '\0';
		static const uint8_t equal_symbol = '=';
		static const uint8_t* var_name = (const uint8_t*)"variable";
		static const uint8_t* name_str = (const uint8_t*)"name";
		static const uint8_t* value_str = (const uint8_t*)"value";

		if (!xml_get_tag_name(env_ptr->start, env_ptr->finish, &name))
		{
			buffer_release(&attribute_value);
			buffer_release(&elements);
			return 0;
		}

		if (!string_equal(name.start, name.finish, var_name, var_name + 8))
		{
			continue;
		}

		if (!buffer_resize(&attribute_value, 0) ||
			!xml_get_attribute_value(env_ptr->start, env_ptr->finish, name_str, 4, &attribute_value) ||
			!buffer_size(&attribute_value))
		{
			buffer_release(&attribute_value);
			buffer_release(&elements);
			return 0;
		}

		name.start = buffer_data(&attribute_value, 0);
		name.finish = name.start + buffer_size(&attribute_value);
		uint8_t contains = string_contains(name.start, name.finish, &(space_and_tab[0]), &(space_and_tab[0]) + 1);

		if (contains)
		{
			if (!string_quote(name.start, name.finish, environments))
			{
				buffer_release(&attribute_value);
				buffer_release(&elements);
				return 0;
			}
		}
		else
		{
			if (!buffer_append_data_from_range(environments, &name))
			{
				buffer_release(&attribute_value);
				buffer_release(&elements);
				return 0;
			}
		}

		if (!buffer_push_back(environments, equal_symbol) ||
			!buffer_resize(&attribute_value, 0))
		{
			buffer_release(&attribute_value);
			buffer_release(&elements);
			return 0;
		}

		if (xml_get_attribute_value(env_ptr->start, env_ptr->finish, value_str, 5, &attribute_value))
		{
			name.start = buffer_data(&attribute_value, 0);
			name.finish = name.start + buffer_size(&attribute_value);

			if (!interpreter_evaluate_code(the_project, the_target, &name, environments, verbose))
			{
				buffer_release(&attribute_value);
				buffer_release(&elements);
			}

#if 0
			contains = string_contains(name.start, name.finish, &(space_and_tab[0]), &(space_and_tab[0]) + 1);

			if (contains)
			{
				if (!string_quote(name.start, name.finish, environments))
				{
					buffer_release(&attribute_value);
					buffer_release(&elements);
					return 0;
				}
			}
			else
			{
				if (!buffer_append_data_from_range(environments, &name))
				{
					buffer_release(&attribute_value);
					buffer_release(&elements);
					return 0;
				}
			}

#endif
		}

		if (!buffer_push_back(environments, zero_symbol))
		{
			buffer_release(&attribute_value);
			buffer_release(&elements);
			return 0;
		}
	}

	buffer_release(&attribute_value);
	buffer_release(&elements);
	/**/
	return 1;
}

uint8_t choose_evaluate_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	struct buffer* task_arguments, uint8_t verbose)
{
	if (!common_get_attributes_and_arguments_for_task(NULL, NULL, 3, NULL, NULL, NULL, task_arguments))
	{
		return 0;
	}

	struct buffer* sub_nodes_names = buffer_buffer_data(task_arguments, 0);

	SET_NULL_TO_BUFFER(*sub_nodes_names);

	struct buffer* elements = buffer_buffer_data(task_arguments, 1);

	SET_NULL_TO_BUFFER(*elements);

	struct buffer* attribute_value = buffer_buffer_data(task_arguments, 2);

	SET_NULL_TO_BUFFER(*attribute_value);

	if (!range_from_string((const uint8_t*)"when\0otherwise\0", 15, 2, sub_nodes_names))
	{
		return 0;
	}

	const uint16_t count = xml_get_sub_nodes_elements(
							   attributes_finish, element_finish, sub_nodes_names, elements);

	if (!count)
	{
		return 1;
	}

	uint16_t otherwise_index = count + 1;

	if (!buffer_append_buffer(attribute_value, NULL, 1))
	{
		return 0;
	}

	struct buffer* test_value_in_buffer = buffer_buffer_data(attribute_value, 0);

	SET_NULL_TO_BUFFER(*buffer_buffer_data(attribute_value, 0));

	struct range* tag_name = buffer_range_data(sub_nodes_names, 0);

	for (uint16_t i = 0; i < count; ++i)
	{
		struct range current_tag_name;
		struct range* element = buffer_range_data(elements, i);

		if (!xml_get_tag_name(element->start, element->finish, &current_tag_name))
		{
			buffer_release_inner_buffers(attribute_value);
			return 0;
		}

		if (!string_equal(tag_name->start, tag_name->finish, current_tag_name.start, current_tag_name.finish))
		{
			otherwise_index = count < otherwise_index ? i : otherwise_index;
			continue;
		}

		if (!interpreter_get_arguments_from_xml_tag_record(the_project, the_target, current_tag_name.start,
				element->finish,
				&test, &test_length, 0, 1, attribute_value, verbose))
		{
			buffer_release_inner_buffers(attribute_value);
			return 0;
		}

		uint8_t test_value = 0;

		if (!bool_parse(buffer_data(test_value_in_buffer, 0), buffer_size(test_value_in_buffer),
						&test_value))
		{
			buffer_release_inner_buffers(attribute_value);
			return 0;
		}

		if (!test_value)
		{
			continue;
		}

		buffer_release_inner_buffers(attribute_value);
		current_tag_name.finish = element->finish;

		if (!buffer_resize(elements, 0))
		{
			return 0;
		}

		return xml_get_sub_nodes_elements(current_tag_name.start, current_tag_name.finish, NULL, elements) ?
			   interpreter_evaluate_tasks(the_project, the_target, elements, 0, verbose) : 1;
	}

	buffer_release_inner_buffers(attribute_value);

	if (otherwise_index < count)
	{
		struct range* element = buffer_range_data(elements, otherwise_index);
		struct range tag;
		tag.start = element->start;
		tag.finish = element->finish;

		if (!buffer_resize(elements, 0))
		{
			return 0;
		}

		return xml_get_sub_nodes_elements(tag.start, tag.finish, NULL, elements) ?
			   interpreter_evaluate_tasks(the_project, the_target, elements, 0, verbose) : 1;
	}

	return 1;
}

#define FAIL_MESSAGE_POSITION		0

uint8_t fail_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	static const uint8_t* fail_attributes[] = { (const uint8_t*)"message" };
	static const uint8_t fail_attributes_lengths[] = { 7 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   fail_attributes, fail_attributes_lengths,
			   COUNT_OF(fail_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t fail_evaluate_task(
	const void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	struct buffer* message, uint8_t verbose)
{
	if (!buffer_size(message))
	{
		if (!interpreter_get_xml_element_value(the_project, the_target, attributes_finish, element_finish, message,
											   verbose))
		{
			return 0;
		}
	}

	echo(0, UTF8, NULL, Error, buffer_data(message, 0), buffer_size(message), 1, verbose);
	return 0;
}

#define IF_TEST_POSITION		0

uint8_t if_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   &test, &test_length, 1,
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t if_evaluate_task(
	void* the_project, const void* the_target, struct buffer* task_arguments,
	const uint8_t* attributes_finish, const uint8_t* element_finish, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* test_in_a_buffer = buffer_buffer_data(task_arguments, IF_TEST_POSITION);

	if (!buffer_size(test_in_a_buffer))
	{
		return 1;
	}

	uint8_t test_value = 0;

	if (!bool_parse(buffer_data(test_in_a_buffer, 0), buffer_size(test_in_a_buffer), &test_value))
	{
		return 0;
	}

	if (!test_value)
	{
		return 1;
	}

	if (!buffer_resize(test_in_a_buffer, 0))
	{
		return 0;
	}

	if (!xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, test_in_a_buffer))
	{
		return 1;
	}

	return interpreter_evaluate_tasks(the_project, the_target, test_in_a_buffer, 0, verbose);
}

static const uint8_t* interpreter_task_str[] =
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
	return common_string_to_enum(task_name_start, task_name_finish,
								 interpreter_task_str, UNKNOWN_TASK);
}

uint8_t interpreter_prepare_attributes_and_arguments_for_property_task(
	const void* the_project, const void* the_target,
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments,
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

	struct buffer* argument = buffer_buffer_data(task_arguments, 0);

	uint8_t dynamic = (uint8_t)buffer_size(argument);

	if (dynamic && !bool_parse(buffer_data(argument, 0), dynamic, &dynamic))
	{
		return 0;
	}

	if (dynamic)
	{
		dynamic = (*task_attributes_count) - 1;

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

uint8_t interpreter_evaluate_task(void* the_project, const void* the_target,
								  uint8_t command, const struct range* command_in_range,
								  const uint8_t* element_finish,
								  uint8_t target_help,
								  uint8_t verbose)
{
	if (range_is_null_or_empty(command_in_range))
	{
		return 0;
	}

	const uint8_t* attributes_start = command_in_range->finish;

	if (range_in_parts_is_null_or_empty(attributes_start, element_finish))
	{
		return 0;
	}

	const uint8_t** task_attributes = NULL;
	const uint8_t* task_attributes_lengths = NULL;
	uint8_t task_attributes_count = 0;
	/**/
	const uint8_t* attributes_finish = xml_get_tag_finish_pos(attributes_start, element_finish);
	/**/
	struct buffer task_arguments;
	SET_NULL_TO_BUFFER(task_arguments);

	if (target_task == command)
	{
		if (!target_get_attributes_and_arguments_for_task(
				&task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments) ||
			task_attributes_count < 1)
		{
			buffer_release_with_inner_buffers(&task_arguments);
			return 0;
		}

		if (!target_help)
		{
			task_attributes_count -= 1;
		}

		if (!interpreter_get_arguments_from_xml_tag_record(
				the_project, the_target, attributes_start, attributes_finish,
				task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
		{
			buffer_release_with_inner_buffers(&task_arguments);
			return 0;
		}

		task_attributes_count = target_evaluate_task(
									the_project, &task_arguments, target_help,
									attributes_start, attributes_finish, element_finish, verbose);
		buffer_release_with_inner_buffers(&task_arguments);
		/**/
		return task_attributes_count;
	}

	if (!interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
			the_project, the_target, attributes_start, attributes_finish,
			&task_attributes_count, &task_arguments, verbose))
	{
		buffer_release_with_inner_buffers(&task_arguments);
		return 0;
	}

	if (task_attributes_count)
	{
		buffer_release_with_inner_buffers(&task_arguments);
		return 1;
	}

	buffer_release_inner_buffers(&task_arguments);
	uint8_t fail_on_error = 1;

	if (!interpreter_get_xml_tag_attribute_values(
			the_project, the_target, attributes_start, attributes_finish, &fail_on_error, &verbose, &task_arguments,
			verbose))
	{
		buffer_release_with_inner_buffers(&task_arguments);
		return 0;
	}

	buffer_release_inner_buffers(&task_arguments);
	void* the_module = NULL;
	const uint8_t* pointer_to_the_task = NULL;
	/**/
	uint8_t module_priority = 0;/*TODO*/

	if (module_priority)
	{
		pointer_to_the_task = project_get_task_from_module(the_project, command_in_range, &the_module);

		if (NULL != pointer_to_the_task &&
			NULL != the_module)
		{
			task_attributes_count = load_tasks_evaluate_loaded_task(
										the_project, the_target, command_in_range,
										attributes_finish, element_finish, &task_arguments,
										the_module, pointer_to_the_task, verbose);
			/**/
			buffer_release_with_inner_buffers(&task_arguments);
			return task_attributes_count;
		}
	}

	switch (command)
	{
		case attrib_task:
			if (!attrib_get_attributes_and_arguments_for_task(
					&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = attrib_evaluate_task(&task_arguments, verbose);
			break;

		case call_task:
			if (!call_get_attributes_and_arguments_for_task(
					&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = call_evaluate_task(the_project, &task_arguments, verbose);
			break;

		case choose_task:
			task_attributes_count = choose_evaluate_task(the_project, the_target, attributes_finish, element_finish,
									&task_arguments, verbose);
			break;

		case copy_task:
			if (!copy_move_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = copy_evaluate_task(the_project, the_target, &task_arguments, verbose);
			break;

		case delete_task:
			if (!delete_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = delete_evaluate_task(&task_arguments, verbose);
			break;

		case description_task:
			if (target_help)
			{
				if (!buffer_resize(&task_arguments, 0) ||
					!xml_get_element_value(attributes_finish, element_finish, &task_arguments))
				{
					task_attributes_count = 0;
					break;
				}

				task_attributes_count =
					echo(0, UTF8, NULL, Info, buffer_data(&task_arguments, 0), buffer_size(&task_arguments), 1, verbose);
				buffer_release(&task_arguments);
			}
			else
			{
				task_attributes_count = 1;
			}

			break;

		case do_task:
			task_attributes_count = do_evaluate_task(
										the_project, the_target, attributes_finish, element_finish, &task_arguments, verbose);
			break;

		case echo_task:
			if (!echo_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			{
				struct buffer* message = buffer_buffer_data(&task_arguments, task_attributes_count - 1);

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

			task_attributes_count = echo_evaluate_task(&task_arguments, verbose);
			break;

		case exec_task:
			if (!exec_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			if (attributes_finish < element_finish)
			{
				struct buffer* environments = buffer_buffer_data(&task_arguments, task_attributes_count);

				if (!interpreter_get_environments(the_project, the_target, attributes_finish, element_finish, environments,
												  verbose))
				{
					task_attributes_count = 0;
					break;
				}
			}

			task_attributes_count = exec_evaluate_task(the_project, &task_arguments, verbose);
			break;

		case fail_task:
			if (!fail_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = fail_evaluate_task(the_project, the_target, attributes_finish, element_finish,
									buffer_buffer_data(&task_arguments, FAIL_MESSAGE_POSITION), verbose);
			break;

		case foreach_task:
			if (!for_each_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = for_each_evaluate_task(the_project, the_target, attributes_finish, element_finish,
									&task_arguments, fail_on_error, verbose);
			break;

		case if_task:
			if (!if_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = if_evaluate_task(
										the_project, the_target, &task_arguments, attributes_finish, element_finish, verbose);
			break;
#if 0

		case include_:
			break;
#endif

		case loadfile_task:
			if (!load_file_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = load_file_evaluate_task(the_project, &task_arguments, verbose);
			break;

		case loadtasks_task:
			if (!load_tasks_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = load_tasks_evaluate_task(the_project, the_target, &task_arguments, verbose);
			break;

		case mkdir_task:
			if (!mkdir_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = mkdir_evaluate_task(&task_arguments, verbose);
			break;

		case move_task:
			if (!copy_move_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = move_evaluate_task(the_project, the_target, &task_arguments, verbose);
			break;

		case program_task:
			if (!program_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = program_evaluate_task(
										the_project, the_target, &task_arguments, attributes_finish, element_finish, verbose);
			break;

		case project_task:
			if (!project_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = project_evaluate_task(the_project, &task_arguments, verbose);
			break;

		case property_task:
			task_attributes_count = interpreter_prepare_attributes_and_arguments_for_property_task(
										the_project, the_target,
										&task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments,
										attributes_start, attributes_finish, verbose);

			if (task_attributes_count)
			{
				task_attributes_count = property_evaluate_task(the_project, NULL, &task_arguments, verbose);
			}

			break;
#if 0

		case setenv_:
			break;
#endif

		case sleep_task:
			if (!sleep_unit_get_attributes_and_arguments_for_task(
					&task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = sleep_unit_evaluate_task(&task_arguments, verbose);
			break;

		case touch_task:
			if (!touch_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					the_project, the_target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = touch_evaluate_task(&task_arguments, verbose);
			break;

		case trycatch_task:
			task_attributes_count = try_catch_evaluate_task(
										the_project, the_target, attributes_finish, element_finish,
										&task_arguments, verbose);
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
			if (module_priority)
			{
				task_attributes_count = 0;
				break;
			}

			pointer_to_the_task = project_get_task_from_module(the_project, command_in_range, &the_module);
			task_attributes_count = load_tasks_evaluate_loaded_task(
										the_project, the_target, command_in_range,
										attributes_finish, element_finish,
										&task_arguments, the_module, pointer_to_the_task, verbose);
			break;

		default:
			break;
	}

	buffer_release_with_inner_buffers(&task_arguments);

	if (!task_attributes_count && !fail_on_error)
	{
		task_attributes_count = FAIL_WITH_OUT_ERROR;
	}

	return task_attributes_count;
}

uint8_t interpreter_evaluate_tasks(void* the_project, const void* the_target,
								   const struct buffer* elements,
								   uint8_t target_help, uint8_t verbose)
{
	ptrdiff_t i = 0;
	uint8_t returned = 1;
	struct range* element = NULL;

	while (NULL != (element = buffer_range_data(elements, i++)))
	{
		struct range tag_name_or_content;
		returned = xml_get_tag_name(element->start, element->finish, &tag_name_or_content);

		if (!returned)
		{
			break;
		}

		const ptrdiff_t offset = project_get_source_offset(the_project, tag_name_or_content.finish);
		const uint8_t task = interpreter_get_task(tag_name_or_content.start, tag_name_or_content.finish);
		/**/
		listener_task_started(NULL, offset, the_project, the_target, task);
		returned = interpreter_evaluate_task(the_project, the_target,
											 task, &tag_name_or_content,
											 element->finish, target_help, verbose);
		listener_task_finished(NULL, offset, the_project, the_target, task, returned);

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
