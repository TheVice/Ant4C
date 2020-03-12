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
#include "date_time.h"
#include "echo.h"
#include "environment.h"
#include "exec.h"
#include "file_system.h"
#include "math_unit.h"
#include "operating_system.h"
#include "hash.h"
#include "load_file.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "target.h"
#include "text_encoding.h"
#include "version.h"
#include "xml.h"

static const uint8_t start_of_function_arguments_area = '(';
static const uint8_t finish_of_function_arguments_area = ')';
static const uint8_t function_call_finish = '}';

static const uint8_t arguments_delimiter = ',';

static const uint8_t space_and_tab[] = { ' ', '\t' };
#define SPACE_AND_TAB_LENGTH COUNT_OF(space_and_tab)

static const uint8_t namespace_border[] = { ':', ':' };
#define NAMESPACE_BORDER_LENGTH COUNT_OF(namespace_border)

static const uint8_t function_call_start[] = { '$', '{' };
#define FUNCTION_CALL_START_LENGTH COUNT_OF(function_call_start)

static const uint8_t* interpreter_string_enumeration_unit[] =
{
	(const uint8_t*)"bool",
	(const uint8_t*)"cygpath",
	(const uint8_t*)"datetime",
	(const uint8_t*)"directory",
#if 0
	(const uint8_t*)"dns",
#endif
	(const uint8_t*)"double",
	(const uint8_t*)"environment",
#if 0
	(const uint8_t*)"file",
	(const uint8_t*)"fileversioninfo",
#endif
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
#if 0
	dns_unit,
#endif
	double_unit,
	environment_unit,
#if 0
	file_unit,
	fileversioninfo_unit,
#endif
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
									   namespace_border,
									   namespace_border + NAMESPACE_BORDER_LENGTH)))
	{
		return 0;
	}

	name_space->start = function->start;
	name_space->finish = function->start + index;
	/**/
	name->start = name_space->finish + NAMESPACE_BORDER_LENGTH;
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
	const void* project, const void* target,
	const struct range* argument_area, struct buffer* output, uint8_t verbose)
{
	ptrdiff_t index = 0;
	const uint8_t* pos = argument_area->start;

	while (-1 != (index = string_index_of(pos, argument_area->finish,
										  namespace_border,
										  namespace_border + NAMESPACE_BORDER_LENGTH)))
	{
		struct range function;
		function.start = find_any_symbol_like_or_not_like_that(pos + index, pos, space_and_tab, SPACE_AND_TAB_LENGTH,
						 1, -1);
		function.finish = argument_area->finish;

		if (!string_trim(&function) ||
			!interpreter_get_function_from_argument(&function) ||
			!buffer_append(output, pos, function.start - pos) ||
			!interpreter_evaluate_function(project, target, &function, output, verbose))
		{
			return 0;
		}

		pos = function.finish;
	}

	return (pos != argument_area->start) ? buffer_append(output, pos, argument_area->finish - pos) : 1;
}

uint8_t interpreter_actualize_property_value(const void* project, const void* target,
		uint8_t property_function_id, const void* the_property,
		ptrdiff_t size, struct buffer* return_of_function, uint8_t verbose)
{
	if (property_get_id_of_get_value_function() != property_function_id)
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

	if (!buffer_append(&code_in_buffer, buffer_data(return_of_function, size),
					   buffer_size(return_of_function) - size) ||
		!buffer_resize(return_of_function, size))
	{
		buffer_release(&code_in_buffer);
		return 0;
	}

	struct range code;

	code.start = buffer_data(&code_in_buffer, 0);

	code.finish = code.start + buffer_size(&code_in_buffer);

	if (!interpreter_evaluate_code(project, target, &code, return_of_function, verbose))
	{
		buffer_release(&code_in_buffer);
		return 0;
	}

	buffer_release(&code_in_buffer);
	return 1;
}

uint8_t interpreter_get_value_for_argument(
	const void* project, const void* target,
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

	if (!interpreter_evaluate_argument_area(project, target, argument_area, &value, verbose))
	{
		buffer_release(&value);
		return 0;
	}

	if (!buffer_size(&value))
	{
		if (project_property_get_by_name(project, argument_area->start, (uint8_t)range_size(argument_area), &value,
										 verbose))
		{
			void* the_property = NULL;

			if (!project_property_exists(project, argument_area->start, (uint8_t)range_size(argument_area),
										 &the_property, verbose))
			{
				buffer_release(&value);
				return 0;
			}

			if (!interpreter_actualize_property_value(project, target, 1, the_property, 0, &value, verbose))
			{
				buffer_release(&value);
				return 0;
			}
		}
		else
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

	/*TODO: value can be represent the quot or property name, recursion in this case should be used at else.*/
	return buffer_append_buffer(values, &value, 1);
}

uint8_t interpreter_get_values_for_arguments(
	const void* project, const void* target,
	const struct range* arguments_area, struct buffer* values, uint8_t verbose)
{
	if (range_is_null_or_empty(arguments_area) || NULL == values)
	{
		return 0;
	}

	uint8_t count = 0;
	uint8_t depth = 0;
	struct range argument_area;
	argument_area.start = arguments_area->start;
	argument_area.finish = arguments_area->start;

	while (argument_area.finish < arguments_area->finish)
	{
		const uint8_t ch = *argument_area.finish;

		if (arguments_delimiter == ch && 0 == depth)
		{
			const uint8_t* pos = argument_area.finish + 1;/*TODO: MIN(pos, arguments_area->finish)*/

			if (!interpreter_get_value_for_argument(project, target, &argument_area, values, verbose))
			{
				return 0;
			}

			++count;
			argument_area.start = pos;
			argument_area.finish = pos + 1;
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

	if (!interpreter_get_value_for_argument(project, target, &argument_area, values, verbose))
	{
		return 0;
	}

	++count;
	return count;
}

uint8_t interpreter_evaluate_function(const void* project, const void* target, const struct range* function,
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
							   project, target, &arguments_area, &values, verbose);

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
				const void* the_property = NULL;

				if (!directory_get_current_directory(project, &the_property, return_of_function))
				{
					values_count = 0;
					break;
				}

				values_count = interpreter_actualize_property_value(
								   project, target,
								   property_get_id_of_get_value_function(),
								   &the_property, size, return_of_function, verbose);
			}
			else
			{
				values_count = dir_exec_function(dir_function_id, &values, values_count, return_of_function);
			}
		}
		break;
#if 0

		case dns_:
			break;
#endif

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
#if 0

		case file_:
			break;

		case fileversioninfo_:
			break;
#endif

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

				ptrdiff_t size = buffer_size(return_of_function);
				const void* the_property = NULL;

				if (!directory_get_current_directory(project, &the_property, return_of_function))
				{
					values_count = 0;
					break;
				}

				if (!interpreter_actualize_property_value(project, target, property_get_id_of_get_value_function(),
						&the_property, size,
						return_of_function, verbose))
				{
					values_count = 0;
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
				values_count = path_exec_function(project, path_function_id, &values, values_count, return_of_function);
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
			ptrdiff_t size = buffer_size(return_of_function);

			if (!project_exec_function(
					project,
					project_get_function(name.start, name.finish),
					&values, values_count, &the_property, return_of_function))
			{
				values_count = 0;
				break;
			}

			values_count = interpreter_actualize_property_value(
							   project, target,
							   property_get_id_of_get_value_function(),
							   the_property, size, return_of_function, verbose);
		}
		break;

		case property_unit:
		{
			const void* the_property = NULL;
			const uint8_t property_function_id = property_get_function(name.start, name.finish);
			ptrdiff_t size = buffer_size(return_of_function);

			if (!property_exec_function(project, property_function_id, &values, values_count, &the_property,
										return_of_function, verbose))
			{
				values_count = 0;
				break;
			}

			values_count = interpreter_actualize_property_value(
							   project, target,
							   property_function_id,
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
							   project, target,
							   target_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;

		case task_unit:
			values_count = task_exec_function(
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
		default:
			values_count = 0;
			break;
	}

	buffer_release_with_inner_buffers(&values);
	return values_count;
}

uint8_t interpreter_evaluate_code(const void* project, const void* target,
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
		function.finish = find_any_symbol_like_or_not_like_that(
							  function.start, code->finish,
							  &function_call_finish, 1, 1, 1);

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

		if (!interpreter_evaluate_function(project, target, &function, &return_of_function, verbose))
		{
			buffer_release(&return_of_function);
			return 0;
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
	const void* project,
	const void* target,
	const uint8_t* start_of_attributes,
	const uint8_t* finish_of_attributes,
	uint8_t* skip,
	uint8_t verbose)
{
	static const uint8_t* if_and_unless[] =
	{
		(const uint8_t*)"if",
		(const uint8_t*)"unless"
	};
	/**/
	static const uint8_t if_and_unless_lengths[] =
	{
		2, 6
	};
	/**/
	static const uint8_t bool_values_to_pass[] = { 1, 0 };
	/**/
	static const uint8_t count_of_attributes = 2;

	if (NULL == skip)
	{
		return 0;
	}

	/*TODO: can be from outside.*/
	struct buffer attributes;
	SET_NULL_TO_BUFFER(attributes);

	if (!buffer_append_buffer(&attributes, NULL, count_of_attributes))
	{
		buffer_release_with_inner_buffers(&attributes);
		return 0;
	}

	for (uint8_t i = 0; i < count_of_attributes; ++i)
	{
		struct buffer* attribute = buffer_buffer_data(&attributes, i);
		SET_NULL_TO_BUFFER(*attribute);
	}

	for (uint8_t i = 0; i < count_of_attributes; ++i)
	{
		struct buffer* attribute = buffer_buffer_data(&attributes, i);

		if (NULL == attribute || !bool_to_string(bool_values_to_pass[i], attribute))
		{
			buffer_release_with_inner_buffers(&attributes);
			return 0;
		}
	}

	if (!interpreter_get_arguments_from_xml_tag_record(
			project, target, start_of_attributes, finish_of_attributes,
			if_and_unless, if_and_unless_lengths, 0, count_of_attributes, &attributes, verbose))
	{
		buffer_release_with_inner_buffers(&attributes);
		return 0;
	}

	for (uint8_t i = 0; i < count_of_attributes; ++i)
	{
		const struct buffer* attribute = buffer_buffer_data(&attributes, i);
		uint8_t bool_value = 0;

		if (!bool_parse(buffer_data(attribute, 0), buffer_size(attribute), &bool_value))
		{
			buffer_release_with_inner_buffers(&attributes);
			return 0;
		}

		if (bool_values_to_pass[i] == bool_value)
		{
			continue;
		}

		buffer_release_with_inner_buffers(&attributes);
		*skip = 1;
		return 1;
	}

	buffer_release_with_inner_buffers(&attributes);
	*skip = 0;
	return 1;
}

uint8_t interpreter_get_xml_tag_attribute_values(
	const void* project,
	const void* target,
	const uint8_t* start_of_attributes,
	const uint8_t* finish_of_attributes,
	uint8_t* fail_on_error,
	uint8_t* verbose_value,
	uint8_t verbose)
{
	static const uint8_t* attributes[] =
	{
		(const uint8_t*)"failonerror",
		(const uint8_t*)"verbose"
	};
	/**/
	static const uint8_t attributes_lengths[] =
	{
		11, 7
	};

	if (NULL == fail_on_error || NULL == verbose_value)
	{
		return 0;
	}

	const uint8_t count_of_attributes = (0 == (*verbose_value) ? 2 : 1);
	/*TODO: can be from outside.*/
	struct buffer values;
	SET_NULL_TO_BUFFER(values);

	if (!buffer_append_buffer(&values, NULL, count_of_attributes))
	{
		buffer_release(&values);
		return 0;
	}

	ptrdiff_t i = 0;
	struct buffer* ptr = NULL;

	while (NULL != (ptr = buffer_buffer_data(&values, i++)))
	{
		SET_NULL_TO_BUFFER(*ptr);
	}

	if (count_of_attributes != i - 1)
	{
		buffer_release(&values);
		return 0;
	}

	if (!interpreter_get_arguments_from_xml_tag_record(project, target, start_of_attributes, finish_of_attributes,
			attributes, attributes_lengths, 0, count_of_attributes, &values, verbose))
	{
		buffer_release_with_inner_buffers(&values);
		return 0;
	}

	uint8_t* outputs[2];
	outputs[0] = fail_on_error;
	outputs[1] = verbose_value;
	i = 0;

	while (NULL != (ptr = buffer_buffer_data(&values, i++)))
	{
		if (!buffer_size(ptr))
		{
			continue;
		}

		if (!bool_parse(buffer_data(ptr, 0), buffer_size(ptr), outputs[i - 1]))
		{
			buffer_release_with_inner_buffers(&values);
			return 0;
		}
	}

	buffer_release_with_inner_buffers(&values);
	return 1;
}

uint8_t interpreter_get_arguments_from_xml_tag_record(const void* project, const void* target,
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
			 !interpreter_evaluate_code(project, target, &code, argument, verbose)))
		{
			buffer_release(&attribute_value);
			return 0;
		}
	}

	buffer_release(&attribute_value);
	return 1;
}

uint8_t interpreter_get_xml_element_value(
	const void* project,
	const void* target,
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
		!interpreter_evaluate_code(project, target, &code, output, verbose))
	{
		buffer_release(&value);
		return 0;
	}

	buffer_release(&value);
	return 1;
}

uint8_t interpreter_get_environments(
	const void* project,
	const void* target,
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

	uint16_t count = xml_get_sub_nodes_elements(attributes_finish, element_finish, &elements);

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

	count = xml_get_sub_nodes_elements(name.start, name.finish, &elements);

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

			if (!interpreter_evaluate_code(project, target, &name, environments, verbose))
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

static const uint8_t* interpreter_task_str[] =
{
#if 0
	(const uint8_t*)"attrib",
	(const uint8_t*)"call",
	(const uint8_t*)"choose",
#endif
	(const uint8_t*)"copy",
	(const uint8_t*)"delete",
	(const uint8_t*)"description",
	(const uint8_t*)"echo",
	(const uint8_t*)"exec",
#if 0
	(const uint8_t*)"fail",
	(const uint8_t*)"foreach",
	(const uint8_t*)"get",
	(const uint8_t*)"gunzip",
	(const uint8_t*)"if",
	(const uint8_t*)"include",
#endif
	(const uint8_t*)"loadfile",
#if 0
	(const uint8_t*)"loadtasks",
	(const uint8_t*)"mail",
#endif
	(const uint8_t*)"mkdir",
	(const uint8_t*)"move",
	(const uint8_t*)"project",
	(const uint8_t*)"property",
#if 0
	(const uint8_t*)"readregistry",
	(const uint8_t*)"regex",
	(const uint8_t*)"script",
	(const uint8_t*)"setenv",
	(const uint8_t*)"sleep",
	(const uint8_t*)"style",
	(const uint8_t*)"tar",
#endif
	(const uint8_t*)"target",
#if 0
	(const uint8_t*)"touch",
	(const uint8_t*)"trycatch",
	(const uint8_t*)"tstamp",
	(const uint8_t*)"untar",
	(const uint8_t*)"unzip",
	(const uint8_t*)"uptodate",
	(const uint8_t*)"xmlpeek",
	(const uint8_t*)"xmlpoke",
	(const uint8_t*)"zip"
#endif
};

enum interpreter_task
{
#if 0
	attrib_task,
	call_task,
	choose_task,
#endif
	copy_task,
	delete_task,
	description_task,
	echo_task,
	exec_task,
#if 0
	fail_task,
	foreach_task,
	get_task,
	gunzip_task,
	if_task,
	include_task,
#endif
	loadfile_task,
#if 0
	loadtasks_task,
	mail_task,
#endif
	mkdir_task,
	move_task,
	project_task,
	property_task,
#if 0
	readregistry_task,
	regex_task,
	script_task,
	setenv_task,
	sleep_task,
	style_task,
	tar_task,
#endif
	target_task,
#if 0
	touch_task,
	trycatch_task,
	tstamp_task,
	untar_task,
	unzip_task,
	uptodate_task,
	xmlpeek_task,
	xmlpoke_task,
	zip_task,
#endif
	UNKNOWN_TASK
};

uint8_t interpreter_get_task(const uint8_t* task_name_start, const uint8_t* task_name_finish)
{
	return common_string_to_enum(task_name_start, task_name_finish,
								 interpreter_task_str, UNKNOWN_TASK);
}

uint8_t interpreter_evaluate_task(void* project, const void* target, uint8_t command,
								  const uint8_t* attributes_start, const uint8_t* element_finish, uint8_t verbose)
{
	if (UNKNOWN_TASK < command || range_in_parts_is_null_or_empty(attributes_start, element_finish))
	{
		return 0;
	}

	uint8_t fail_on_error = 1;
	uint8_t task_attributes_count = 0;
	const uint8_t* attributes_finish = xml_get_tag_finish_pos(attributes_start, element_finish);

	if (!range_in_parts_is_null_or_empty(attributes_start, attributes_finish))
	{
		uint8_t task_verbose = verbose;

		if (!interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
				project, target, attributes_start, attributes_finish, &task_attributes_count, verbose))
		{
			return 0;
		}

		if (task_attributes_count)
		{
			return 1;
		}

		if (!interpreter_get_xml_tag_attribute_values(
				project, target, attributes_start, attributes_finish, &fail_on_error, &task_verbose, verbose))
		{
			return 0;
		}

		verbose = verbose < task_verbose ? task_verbose : verbose;
	}

	struct buffer task_arguments;

	SET_NULL_TO_BUFFER(task_arguments);

	const uint8_t** task_attributes = NULL;

	const uint8_t* task_attributes_lengths = NULL;

	switch (command)
	{
#if 0

		case attrib_:
			break;

		case call_:
			break;

		case choose_:
			break;
#endif

		case copy_task:
			if (!copy_move_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = copy_evaluate_task(&task_arguments, verbose);
			break;

		case delete_task:
			if (!delete_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = delete_evaluate_task(&task_arguments, verbose);
			break;

		case description_task:
			/*TODO:*/
			task_attributes_count = 1;
			break;

		case echo_task:
			if (!echo_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			{
				struct buffer* message = buffer_buffer_data(&task_arguments, task_attributes_count - 1);

				if (!buffer_size(message))
				{
					if (!interpreter_get_xml_element_value(project, target, attributes_finish, element_finish, message, verbose))
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
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			if (attributes_finish < element_finish)
			{
				struct buffer* environments = buffer_buffer_data(&task_arguments, task_attributes_count);

				if (!interpreter_get_environments(project, target, attributes_finish, element_finish, environments, verbose))
				{
					task_attributes_count = 0;
					break;
				}
			}

			task_attributes_count = exec_evaluate_task(project, &task_arguments, verbose);
			break;
#if 0

		case fail_:
			break;

		case foreach_:
			break;

		case get_:
			break;

		case gunzip_:
			break;

		case if_:
			break;

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
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = load_file_evaluate_task(project, &task_arguments, verbose);
			break;
#if 0

		case loadtasks_:
			break;

		case mail_:
			break;
#endif

		case mkdir_task:
			if (!mkdir_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					project, target, attributes_start, attributes_finish,
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
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = move_evaluate_task(&task_arguments, verbose);
			break;

		case project_task:
			if (!project_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = project_evaluate_task(project, &task_arguments, verbose);
			break;

		case property_task:
			if (!property_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, 1, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			{
				struct buffer* argument = buffer_buffer_data(&task_arguments, 0);
				uint8_t dynamic = 0;

				if (buffer_size(argument) &&
					!bool_parse(buffer_data(argument, 0), buffer_size(argument), &dynamic))
				{
					task_attributes_count = 0;
					break;
				}

				if (dynamic)
				{
					if (!interpreter_get_arguments_from_xml_tag_record(
							project, target, attributes_start, attributes_finish,
							task_attributes, task_attributes_lengths,
							1, task_attributes_count - 1, &task_arguments, verbose))
					{
						task_attributes_count = 0;
						break;
					}

					argument = buffer_buffer_data(&task_arguments, task_attributes_count - 1);

					if (!buffer_resize(argument, 0) ||
						!xml_get_attribute_value(attributes_start, attributes_finish,
												 task_attributes[task_attributes_count - 1],
												 task_attributes_lengths[task_attributes_count - 1],
												 argument))
					{
						task_attributes_count = 0;
						break;
					}
				}
				else if (!interpreter_get_arguments_from_xml_tag_record(
							 project, target, attributes_start, attributes_finish,
							 task_attributes, task_attributes_lengths,
							 1, task_attributes_count, &task_arguments, verbose))
				{
					task_attributes_count = 0;
					break;
				}
			}

			task_attributes_count = property_evaluate_task(project, &task_arguments, verbose);
			break;
#if 0

		case readregistry_:
			break;

		case regex_:
			break;

		case script_:
			break;

		case setenv_:
			break;

		case sleep_:
			break;

		case style_:
			break;

		case tar_:
			break;
#endif

		case target_task:
			if (!target_get_attributes_and_arguments_for_task(
					&task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments, verbose))
			{
				task_attributes_count = 0;
				break;
			}

			attributes_finish = find_any_symbol_like_or_not_like_that(
									attributes_finish + 1, element_finish, (const uint8_t*)"<", 1, 1, 1);
			element_finish = find_any_symbol_like_or_not_like_that(element_finish - 1, attributes_finish,
							 (const uint8_t*)"<", 1, 1, -1);
			element_finish = find_any_symbol_like_or_not_like_that(element_finish - 1, attributes_finish,
							 (const uint8_t*)">", 1, 1, -1);
			task_attributes_count = target_evaluate_task(project, &task_arguments, attributes_finish, element_finish,
									verbose);
			break;
#if 0

		case touch_:
			break;

		case trycatch_:
			break;

		case tstamp_:
			break;

		case untar_:
			break;

		case unzip_:
			break;

		case uptodate_:
			break;

		case xmlpeek_:
			break;

		case xmlpoke_:
			break;

		case zip_:
			break;
#endif

		case UNKNOWN_TASK:
		default:
			break;
	}

	buffer_release_with_inner_buffers(&task_arguments);

	if (!task_attributes_count && !fail_on_error)
	{
		if (!echo(0, Default, NULL, Warning,
				  (const uint8_t*)
				  "[interpreter]: task was failed. However evaluation of script continue as requested at 'fail on error' attribute.",
				  112,
				  1, verbose))
		{
			return 0;
		}

		task_attributes_count = 1;
	}

	return task_attributes_count;
}

uint8_t interpreter_evaluate_tasks(void* project, const void* target,
								   const struct buffer* elements, uint8_t verbose)
{
	ptrdiff_t i = 0;
	struct range* element = NULL;

	while (NULL != (element = buffer_range_data(elements, i++)))
	{
		struct range tag_name_or_content;

		if (!xml_get_tag_name(element->start, element->finish, &tag_name_or_content))
		{
			i = 0;
			break;
		}

		if (!interpreter_evaluate_task(project, target,
									   interpreter_get_task(tag_name_or_content.start, tag_name_or_content.finish),
									   tag_name_or_content.finish,
									   element->finish, verbose))
		{
			i = 0;
			break;
		}
	}

	return 0 < i;
}

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

uint8_t task_exec_function(uint8_t function, const struct buffer* arguments,
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

	argument.start = argument.finish = NULL;

	if (!common_get_one_argument(arguments, &argument, 0))
	{
		return 0;
	}

	arguments_count = (UNKNOWN_TASK != interpreter_get_task(argument.start, argument.finish));
	return bool_to_string(arguments_count, output);
}
