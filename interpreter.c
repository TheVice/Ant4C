/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
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
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "target.h"
#include "version.h"
#include "xml.h"

static const uint8_t start_of_function_arguments_area = '(';
static const uint8_t finish_of_function_arguments_area = ')';
static const uint8_t function_call_finish = '}';
static const uint8_t quote_single_symbol = '\'';

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
	(const uint8_t*)"dns",
	(const uint8_t*)"double",
	(const uint8_t*)"environment",
	(const uint8_t*)"file",
	(const uint8_t*)"fileversioninfo",
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
	dns_unit,
	double_unit,
	environment_unit,
	file_unit,
	fileversioninfo_unit,
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

uint8_t interpreter_get_value_from_quote(const struct range* quote, struct range* value)
{
	if (range_is_null_or_empty(quote) || NULL == value)
	{
		return 0;
	}

	if (quote->finish == (value->start = find_any_symbol_like_or_not_like_that(
			quote->start, quote->finish, &quote_single_symbol, 1, 1, 1)))
	{
		return 0;
	}

	++value->start;
	value->finish = find_any_symbol_like_or_not_like_that(value->start, quote->finish, &quote_single_symbol, 1, 1,
					1);
	return quote_single_symbol == *value->finish;
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
	const struct range* argument_area, struct buffer* output)
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
			!interpreter_evaluate_function(project, target, &function, output))
		{
			return 0;
		}

		pos = function.finish;
	}

	return (pos != argument_area->start) ? buffer_append(output, pos, argument_area->finish - pos) : 1;
}

uint8_t interpreter_get_value_for_argument(
	const void* project, const void* target,
	struct range* argument_area, struct buffer* values)
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

	if (!interpreter_evaluate_argument_area(project, target, argument_area, &value))
	{
		buffer_release(&value);
		return 0;
	}

	if (!buffer_size(&value))
	{
		if (property_get_by_name(project, argument_area->start, (uint8_t)range_size(argument_area), &value))
		{
			void* the_property = NULL;

			if (!project_property_exists(project, argument_area->start, (uint8_t)range_size(argument_area),
										 &the_property))
			{
				buffer_release(&value);
				return 0;
			}

			if (!property_actualize_value(project, target, 1, the_property, 0, &value))
			{
				buffer_release(&value);
				return 0;
			}
		}
		else
		{
			if (!buffer_resize(&value, 0) ||
				!interpreter_get_value_from_quote(argument_area, argument_area) ||
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
	const struct range* arguments_area, struct buffer* values)
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

			if (!interpreter_get_value_for_argument(project, target, &argument_area, values))
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

	if (!interpreter_get_value_for_argument(project, target, &argument_area, values))
	{
		return 0;
	}

	++count;
	return count;
}

uint8_t interpreter_evaluate_function(const void* project, const void* target, const struct range* function,
									  struct buffer* return_of_function)
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
							   project, target, &arguments_area, &values);

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
#if 0

		case directory_:
			break;

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

			if (path_get_full_path_function == path_function_id)
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

				if (!property_actualize_value(project, target, property_get_value_function, &the_property, size,
											  return_of_function))
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

			if (!property_actualize_value(project, target,
										  property_get_value_function,
										  the_property, size, return_of_function))
			{
				values_count = 0;
				break;
			}

			values_count = 1;
		}
		break;

		case property_unit:
		{
			const void* the_property = NULL;
			const uint8_t property_function_id = property_get_function(name.start, name.finish);
			ptrdiff_t size = buffer_size(return_of_function);

			if (!property_exec_function(project, property_function_id, &values, values_count, &the_property,
										return_of_function))
			{
				values_count = 0;
				break;
			}

			if (!property_actualize_value(project, target, property_function_id, the_property, size,
										  return_of_function))
			{
				values_count = 0;
				break;
			}

			values_count = 1;
		}
		break;

		case string_unit:
			values_count = string_exec_function(
							   string_get_function(name.start, name.finish),
							   &values, values_count, return_of_function);
			break;
#if 0

		case target_:
			break;

		case task_:
			break;
#endif

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
								  const struct range* code, struct buffer* output)
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

		if (!interpreter_evaluate_function(project, target, &function, &return_of_function))
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

uint8_t interpreter_is_xml_tag_should_be_skip_by_if_or_unless(const void* project, const void* target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes, uint8_t* skip)
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

	if (range_in_parts_is_null_or_empty(start_of_attributes, finish_of_attributes) || NULL == skip)
	{
		return 0;
	}

	struct buffer attributes;

	SET_NULL_TO_BUFFER(attributes);

	if (!buffer_append_buffer(&attributes, NULL, COUNT_OF(bool_values_to_pass)))
	{
		buffer_release_with_inner_buffers(&attributes);
		return 0;
	}

	for (uint8_t i = 0, count = COUNT_OF(bool_values_to_pass); i < count; ++i)
	{
		struct buffer* attribute = buffer_buffer_data(&attributes, i);
		SET_NULL_TO_BUFFER(*attribute);
	}

	for (uint8_t i = 0, count = COUNT_OF(bool_values_to_pass); i < count; ++i)
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
			if_and_unless, if_and_unless_lengths, 0, COUNT_OF(if_and_unless_lengths), &attributes))
	{
		buffer_release_with_inner_buffers(&attributes);
		return 0;
	}

	for (uint8_t i = 0, count = COUNT_OF(bool_values_to_pass); i < count; ++i)
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

uint8_t interpreter_get_arguments_from_xml_tag_record(const void* project, const void* target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
		const uint8_t** attributes, const uint8_t* attributes_lengths,
		uint8_t index, uint8_t attributes_count, struct buffer* output)
{
	for (; index < attributes_count; ++index)
	{
		struct buffer* argument = buffer_buffer_data(output, index);

		if (NULL == argument)
		{
			return 0;
		}

		struct range attribute_value;

		if (!xml_get_attribute_value(start_of_attributes, finish_of_attributes,
									 attributes[index], attributes_lengths[index],
									 &attribute_value))
		{
			continue;
		}

		if (!buffer_resize(argument, 0) ||
			(!range_is_null_or_empty(&attribute_value) &&
			 !interpreter_evaluate_code(project, target, &attribute_value, argument)))
		{
			return 0;
		}
	}

	return 1;
}

static const uint8_t* interpreter_task_str[] =
{
	(const uint8_t*)"al",
	(const uint8_t*)"asminfo",
	(const uint8_t*)"attrib",
	(const uint8_t*)"aximp",
	(const uint8_t*)"call",
	(const uint8_t*)"choose",
	(const uint8_t*)"cl",
	(const uint8_t*)"copy",
	(const uint8_t*)"csc",
	(const uint8_t*)"delay-sign",
	(const uint8_t*)"delete",
	(const uint8_t*)"description",
	(const uint8_t*)"echo",
	(const uint8_t*)"exec",
	(const uint8_t*)"fail",
	(const uint8_t*)"foreach",
	(const uint8_t*)"get",
	(const uint8_t*)"gunzip",
	(const uint8_t*)"if",
	(const uint8_t*)"ilasm",
	(const uint8_t*)"ildasm",
	(const uint8_t*)"include",
	(const uint8_t*)"jsc",
	(const uint8_t*)"lib",
	(const uint8_t*)"license",
	(const uint8_t*)"link",
	(const uint8_t*)"loadfile",
	(const uint8_t*)"loadtasks",
	(const uint8_t*)"mail",
	(const uint8_t*)"mc",
	(const uint8_t*)"midl",
	(const uint8_t*)"mkdir",
	(const uint8_t*)"move",
	(const uint8_t*)"ndoc",
	(const uint8_t*)"nunit2",
	(const uint8_t*)"project",
	(const uint8_t*)"property",
	(const uint8_t*)"rc",
	(const uint8_t*)"readregistry",
	(const uint8_t*)"regasm",
	(const uint8_t*)"regex",
	(const uint8_t*)"regsvcs",
	(const uint8_t*)"resgen",
	(const uint8_t*)"script",
	(const uint8_t*)"servicecontroller",
	(const uint8_t*)"setenv",
	(const uint8_t*)"sleep",
	(const uint8_t*)"solution",
	(const uint8_t*)"style",
	(const uint8_t*)"sysinfo",
	(const uint8_t*)"tar",
	(const uint8_t*)"target",
	(const uint8_t*)"tlbexp",
	(const uint8_t*)"tlbimp",
	(const uint8_t*)"touch",
	(const uint8_t*)"trycatch",
	(const uint8_t*)"tstamp",
	(const uint8_t*)"untar",
	(const uint8_t*)"unzip",
	(const uint8_t*)"uptodate",
	(const uint8_t*)"vbc",
	(const uint8_t*)"vjc",
	(const uint8_t*)"xmlpeek",
	(const uint8_t*)"xmlpoke",
	(const uint8_t*)"zip"
};

enum interpreter_task
{
	al_task,
	asminfo_task,
	attrib_task,
	aximp_task,
	call_task,
	choose_task,
	cl_task,
	copy_task,
	csc_task,
	delay_sign_task,
	delete_task,
	description_task,
	echo_task,
	exec_task,
	fail_task,
	foreach_task,
	get_task,
	gunzip_task,
	if_task,
	ilasm_task,
	ildasm_task,
	include_task,
	jsc_task,
	lib_task,
	license_task,
	link_task,
	loadfile_task,
	loadtasks_task,
	mail_task,
	mc_task,
	midl_task,
	mkdir_task,
	move_task,
	ndoc_task,
	nunit2_task,
	project_task,
	property_task,
	rc_task,
	readregistry_task,
	regasm_task,
	regex_task,
	regsvcs_task,
	resgen_task,
	script_task,
	servicecontroller_task,
	setenv_task,
	sleep_task,
	solution_task,
	style_task,
	sysinfo_task,
	tar_task,
	target_task,
	tlbexp_task,
	tlbimp_task,
	touch_task,
	trycatch_task,
	tstamp_task,
	untar_task,
	unzip_task,
	uptodate_task,
	vbc_task,
	vjc_task,
	xmlpeek_task,
	xmlpoke_task,
	zip_task,
	UNKNOWN_TASK
};

uint8_t interpreter_get_task(const uint8_t* task_name_start, const uint8_t* task_name_finish)
{
	return common_string_to_enum(task_name_start, task_name_finish,
								 interpreter_task_str, UNKNOWN_TASK);
}

uint8_t interpreter_evaluate_task(void* project, const void* target, uint8_t command,
								  const uint8_t* attributes_start, const uint8_t* element_finish)
{
	if (UNKNOWN_TASK < command || range_in_parts_is_null_or_empty(attributes_start, element_finish))
	{
		return 0;
	}

	uint8_t task_attributes_count = 0;
	const uint8_t* attributes_finish = xml_get_tag_finish_pos(attributes_start, element_finish);

	if (!range_in_parts_is_null_or_empty(attributes_start, attributes_finish) &&
		!interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
			project, target, attributes_start, attributes_finish, &task_attributes_count))
	{
		return 0;
	}

	if (task_attributes_count)
	{
		return 1;
	}

	struct buffer task_arguments;

	SET_NULL_TO_BUFFER(task_arguments);

	const uint8_t** task_attributes = NULL;

	const uint8_t* task_attributes_lengths = NULL;

	switch (command)
	{
#if 0

		case al_:
			break;

		case asminfo_:
			break;

		case attrib_:
			break;

		case aximp_:
			break;

		case call_:
			break;

		case choose_:
			break;

		case cl_:
			break;

		case copy_:
			break;

		case csc_:
			break;

		case delay_sign_:
			break;

		case delete_:
			break;
#endif

		case description_task:
			/*TODO:*/
			task_attributes_count = 1;
			break;

		case echo_task:
			return echo_evaluate_task(project, target, attributes_start, attributes_finish, element_finish);

		case exec_task:
			return exec_evaluate_task(project, target, attributes_start, attributes_finish, element_finish);
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

		case ilasm_:
			break;

		case ildasm_:
			break;

		case include_:
			break;

		case jsc_:
			break;

		case lib_:
			break;

		case license_:
			break;

		case link_:
			break;

		case loadfile_:
			break;

		case loadtasks_:
			break;

		case mail_:
			break;

		case mc_:
			break;

		case midl_:
			break;

		case mkdir_:
			break;

		case move_:
			break;

		case ndoc_:
			break;

		case nunit2_:
			break;
#endif

		case project_task:
			if (!project_get_attributes_and_arguments_for_task(&task_attributes, &task_attributes_lengths,
					&task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			if (!interpreter_get_arguments_from_xml_tag_record(
					project, target, attributes_start, attributes_finish,
					task_attributes, task_attributes_lengths, 0, task_attributes_count, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			task_attributes_count = project_evaluate_task(project, &task_arguments);
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
					task_attributes, task_attributes_lengths, 0, 1, &task_arguments))
			{
				task_attributes_count = 0;
				break;
			}

			{
				struct buffer* argument = buffer_buffer_data(&task_arguments, 0);
				uint8_t dynamic = 0;

				if (!bool_parse(buffer_data(argument, 0), buffer_size(argument), &dynamic))
				{
					task_attributes_count = 0;
					break;
				}

				if (dynamic)
				{
					if (!interpreter_get_arguments_from_xml_tag_record(
							project, target, attributes_start, attributes_finish,
							task_attributes, task_attributes_lengths,
							1, task_attributes_count - 1, &task_arguments))
					{
						task_attributes_count = 0;
						break;
					}

					struct range value;

					if (!xml_get_attribute_value(attributes_start, attributes_finish,
												 task_attributes[task_attributes_count - 1],
												 task_attributes_lengths[task_attributes_count - 1],
												 &value))
					{
						task_attributes_count = 0;
						break;
					}

					argument = buffer_buffer_data(&task_arguments, task_attributes_count - 1);

					if (!buffer_resize(argument, 0) || !buffer_append_data_from_range(argument, &value))
					{
						task_attributes_count = 0;
						break;
					}
				}
				else if (!interpreter_get_arguments_from_xml_tag_record(
							 project, target, attributes_start, attributes_finish,
							 task_attributes, task_attributes_lengths,
							 1, task_attributes_count, &task_arguments))
				{
					task_attributes_count = 0;
					break;
				}
			}

			task_attributes_count = property_evaluate_task(project, &task_arguments);
			break;
#if 0

		case rc_:
			break;

		case readregistry_:
			break;

		case regasm_:
			break;

		case regex_:
			break;

		case regsvcs_:
			break;

		case resgen_:
			break;

		case script_:
			break;

		case servicecontroller_:
			break;

		case setenv_:
			break;

		case sleep_:
			break;

		case solution_:
			break;

		case style_:
			break;

		case sysinfo_:
			break;

		case tar_:
			break;
#endif

		case target_task:
			/*TODO:*/
			task_attributes_count = 1;
			break;
#if 0

		case tlbexp_:
			break;

		case tlbimp_:
			break;

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

		case vbc_:
			break;

		case vjc_:
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
	return task_attributes_count;
}

uint8_t interpreter_evaluate_tasks(void* project, const void* target,
								   const struct buffer* elements, uint8_t verbose)
{
	ptrdiff_t i = 0;
	struct range* element = NULL;
	(void)verbose;/*TODO:*/

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
									   element->finish))
		{
			i = 0;
			break;
		}
	}

	return 0 < i;
}
