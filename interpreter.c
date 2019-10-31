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

static const uint8_t if_str[] = { 'i', 'f' };
#define IF_LENGTH COUNT_OF(if_str)

static const uint8_t unless_str[] = { 'u', 'n', 'l', 'e', 's', 's' };
#define UNLESS_LENGTH COUNT_OF(unless_str)

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
	(const uint8_t*)"long",
	(const uint8_t*)"int64",
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
	bool_, cygpath_, datetime_, directory_, dns_,
	double_, environment_, file_, fileversioninfo_,
	int_, long_, int64_, math_, operating_system_,
	path_, platform_, program_, project_, property_,
	string_, target_, task_, timespan_, version_, UNKNOWN_UNIT
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
		return 0;
	}

	if (!string_trim(argument_area))
	{
		return 0;
	}

	if (!interpreter_evaluate_argument_area(project, target, argument_area, &value))
	{
		return 0;
	}

	if (!buffer_size(&value))
	{
		if (!property_get_by_name(project, target,
								  argument_area->start, (uint8_t)range_size(argument_area), &value))
		{
			if (!interpreter_get_value_from_quote(argument_area, argument_area) ||
				!buffer_append_data_from_range(&value, argument_area))
			{
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

	const uint8_t values_count = interpreter_get_values_for_arguments(
									 project, target, &arguments_area, &values);

	switch (interpreter_get_unit(name_space.start, name_space.finish))
	{
		case bool_:
			if (!bool_exec_function(
					conversion_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case cygpath_:
			if (!cygpath_exec_function(path_get_function(name.start, name.finish),
									   &values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case datetime_:
			if (!datetime_exec_function(
					datetime_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case directory_:
			break;

		case dns_:
			break;

		case double_:
			if (!double_exec_function(
					conversion_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case environment_:
			if (!environment_exec_function(
					environment_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case file_:
			break;

		case fileversioninfo_:
			break;

		case int_:
			if (!int_exec_function(
					conversion_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case long_:
			if (!long_exec_function(
					conversion_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case int64_:
			if (!int64_exec_function(
					conversion_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case math_:
			if (!math_exec_function(
					math_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case operating_system_:
			if (!os_exec_function(os_get_function(name.start, name.finish), &values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case path_:
			if (!path_exec_function(project,
									path_get_function(name.start, name.finish), &values, values_count,
									return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case platform_:
			if (!platform_exec_function(os_get_function(name.start, name.finish), &values, values_count,
										return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case program_:
			if (!program_exec_function(project_get_function(name.start, name.finish), &values, values_count,
									   return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case project_:
			if (!project_exec_function(
					project, target,
					project_get_function(name.start, name.finish), &values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case property_:
			if (!property_exec_function(
					project, target,
					property_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case string_:
			if (!string_exec_function(
					string_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case target_:
			break;

		case task_:
			break;

		case timespan_:
			if (!timespan_exec_function(
					timespan_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case version_:
			if (!version_exec_function(
					version_get_function(name.start, name.finish),
					&values, values_count, return_of_function))
			{
				buffer_release_with_inner_buffers(&values);
				return 0;
			}

			break;

		case UNKNOWN_UNIT:
		default:
			buffer_release_with_inner_buffers(&values);
			return 0;
	}

	buffer_release_with_inner_buffers(&values);
	return 1;
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

uint8_t interpreter_xml_tag_should_be_skip_by_if_or_unless(const void* project, const void* target,
		const uint8_t* tag_start, const uint8_t* tag_finish, uint8_t* skip)
{
	if (range_in_parts_is_null_or_empty(tag_start, tag_finish) || NULL == skip)
	{
		return 0;
	}

	struct range if_and_unless;

	/**/
	struct buffer output_of_code;

	SET_NULL_TO_BUFFER(output_of_code);

	/**/
	const uint8_t* if_and_unless_strings[2];

	if_and_unless_strings[0] = if_str;

	if_and_unless_strings[1] = unless_str;

	/**/
	uint8_t if_and_unless_lengths[2];

	if_and_unless_lengths[0] = IF_LENGTH;

	if_and_unless_lengths[1] = UNLESS_LENGTH;

	/**/
	uint8_t bool_value_to_pass[] = { 1, 0 };

	for (uint8_t j = 0; j < 2; ++j)
	{
		uint8_t bool_value = 0;

		if (xml_get_attribute_value(tag_start, tag_finish,
									if_and_unless_strings[j], if_and_unless_lengths[j], &if_and_unless))
		{
			if (!interpreter_evaluate_code(project, target, &if_and_unless, &output_of_code))
			{
				buffer_release(&output_of_code);
				return 0;
			}

			const uint8_t* start = buffer_data(&output_of_code, 0);

			if (NULL == start)
			{
				buffer_release(&output_of_code);
				return 0;
			}

			const uint8_t* finish = buffer_data(&output_of_code, buffer_size(&output_of_code) - 1);

			if (NULL == finish)
			{
				buffer_release(&output_of_code);
				return 0;
			}

			if (!bool_parse(start, (1 + finish) - start, &bool_value))
			{
				buffer_release(&output_of_code);
				return 0;
			}

			if (!buffer_resize(&output_of_code, 0))
			{
				buffer_release(&output_of_code);
				return 0;
			}

			if (bool_value_to_pass[j] == bool_value)
			{
				continue;
			}

			buffer_release(&output_of_code);
			*skip = 1;
			return 1;
		}
	}

	buffer_release(&output_of_code);
	*skip = 0;
	return 1;
}

uint8_t interpreter_get_arguments_from_xml_tag_record(const void* project, const void* target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
		const uint8_t** attributes, const uint8_t* attributes_lengths,
		uint8_t attributes_count, struct buffer* output)
{
	for (uint8_t i = 0; i < attributes_count; ++i)
	{
		struct buffer* argument = buffer_buffer_data(output, i);

		if (NULL == argument)
		{
			return 0;
		}

		struct range attribute_value;

		if (!xml_get_attribute_value(start_of_attributes, finish_of_attributes, attributes[i], attributes_lengths[i],
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
	al_, asminfo_, attrib_, aximp_, call_, choose_, cl_, copy_, csc_,
	delay_sign_, delete_, description_, echo_, exec_, fail_, foreach_, get_, gunzip_, if_,
	ilasm_, ildasm_, include_, jsc_, lib_, license_, link_, loadfile_, loadtasks_, mail_, mc_, midl_,
	mkdir_, move_, ndoc_, nunit2_, property_task_, rc_, readregistry_, regasm_,
	regex_, regsvcs_, resgen_, script_, servicecontroller_, setenv_, sleep_, solution_, style_,
	sysinfo_, tar_, tlbexp_, tlbimp_, touch_, trycatch_, tstamp_, untar_, unzip_, uptodate_, vbc_,
	vjc_, xmlpeek_, xmlpoke_, zip_, UNKNOWN_TASK
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

	const uint8_t* attributes_finish = xml_get_tag_finish_pos(attributes_start, element_finish);

	switch (command)
	{
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

		case description_:
			break;

		case echo_:
			return echo_evaluate_task(project, target, attributes_start, attributes_finish, element_finish);

		case exec_:
			return exec_evaluate_task(project, target, attributes_start, attributes_finish, element_finish);

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

		case property_task_:
			break;

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

		case UNKNOWN_TASK:
		default:
			break;
	}

	return 0;
}
