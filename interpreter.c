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

static const char* interpreter_string_enumeration_unit[] =
{
	"bool", "cygpath", "datetime", "directory", "dns",
	"double", "environment", "file", "fileversioninfo",
	"int", "long", "int64", "math", "operating-system",
	"path", "platform", "program", "project", "property",
	"string", "target", "task", "timespan", "version"
};

enum interpreter_enumeration_unit
{
	bool_, cygpath_, datetime_, directory_, dns_,
	double_, environment_, file_, fileversioninfo_,
	int_, long_, int64_, math_, operating_system_,
	path_, platform_, program_, project_, property_,
	string_, target_, task_, timespan_, version_, UNKNOWN_UNIT
};

uint8_t interpreter_get_unit(const char* name_space_start, const char* name_space_finish)
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
			quote->start, quote->finish, "'", 1, 1, 1)))
	{
		return 0;
	}

	++value->start;
	value->finish = find_any_symbol_like_or_not_like_that(value->start, quote->finish, "'", 1, 1, 1);
	return '\'' == *value->finish;
}

static const char* namespace_border = "::";
static const ptrdiff_t namespace_border_length = 2;

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
									   namespace_border + namespace_border_length)))
	{
		return 0;
	}

	name_space->start = function->start;
	name_space->finish = function->start + index;
	/**/
	name->start = name_space->finish + namespace_border_length;
	name->finish = find_any_symbol_like_or_not_like_that(
					   name->start, function->finish, "(", 1, 1, 1);

	if (function->finish == name->finish)
	{
		return 0;
	}

	arguments_area->start = name->finish + 1;
	arguments_area->finish = find_any_symbol_like_or_not_like_that(
								 function->finish, arguments_area->start, ")", 1, 1, -1);
	/**/
	return ')' == *arguments_area->finish;
}

uint8_t interpreter_get_function_from_argument(
	struct range* argument_area)
{
	if (range_is_null_or_empty(argument_area))
	{
		return 0;
	}

	const char* finish = argument_area->finish;
	argument_area->finish = find_any_symbol_like_or_not_like_that(argument_area->start, argument_area->finish,
							"(", 1, 1, 1);

	if (finish == argument_area->finish)
	{
		return 0;
	}

	char ch = '\0';
	uint8_t depth = 0;

	while (argument_area->finish < finish)
	{
		++argument_area->finish;
		ch = *argument_area->finish;

		if (')' == ch)
		{
			if (0 == depth)
			{
				++argument_area->finish;
				break;
			}

			--depth;
		}

		if ('(' == ch)
		{
			++depth;
		}
	}

	if (')' != ch || 0 != depth)
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
	const char* pos = argument_area->start;

	while (-1 != (index = string_index_of(pos, argument_area->finish,
										  namespace_border,
										  namespace_border + namespace_border_length)))
	{
		struct range function;
		function.start = find_any_symbol_like_or_not_like_that(pos + index, pos, " \t", 2, 1, -1);
		function.finish = argument_area->finish;

		if (!string_trim(&function) ||
			!interpreter_get_function_from_argument(&function) ||
			!buffer_append_char(output, pos, function.start - pos) ||
			!interpreter_evaluate_function(project, target, &function, output))
		{
			return 0;
		}

		pos = function.finish;
	}

	return (pos != argument_area->start) ? buffer_append_char(output, pos, argument_area->finish - pos) : 1;
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
		const char ch = *argument_area.finish;

		if (',' == ch && 0 == depth)
		{
			const char* pos = argument_area.finish + 1;/*TODO: MIN(pos, arguments_area->finish)*/

			if (!interpreter_get_value_for_argument(project, target, &argument_area, values))
			{
				return 0;
			}

			++count;
			argument_area.start = pos;
			argument_area.finish = pos + 1;
		}
		else if ('(' == ch)
		{
			if (UINT8_MAX == depth)
			{
				/*TODO:*/
				break;
			}

			++depth;
		}
		else if (')' == ch)
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
	static const char* function_call_start = "${";
	static const char* function_call_finish = "}";

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
	const char* previous_pos = code->start;

	while (-1 != (index = string_index_of(function.start, code->finish,
										  function_call_start, function_call_start + 2)))
	{
		if (!buffer_resize(&return_of_function, 0))
		{
			buffer_release(&return_of_function);
			return 0;
		}

		function.start += index;
		function.finish = find_any_symbol_like_or_not_like_that(
							  function.start, code->finish,
							  function_call_finish, 1, 1, 1);

		if (function.finish == code->finish && function_call_finish[0] != *function.finish)
		{
			buffer_release(&return_of_function);
			return 0;
		}

		if (previous_pos < function.start && !buffer_append_char(output, previous_pos, function.start - previous_pos))
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
	return buffer_append_char(output, previous_pos, code_length - (previous_pos - code->start));
}

uint8_t interpreter_xml_tag_should_be_skip_by_if_or_unless(const void* project, const void* target,
		const char* tag_start, const char* tag_finish, uint8_t* skip)
{
	static const char* if_str = "if";
	static const char* unless_str = "unless";
	/**/
	static const uint8_t if_length = 2;
	static const uint8_t unless_length = 6;

	if (range_in_parts_is_null_or_empty(tag_start, tag_finish) || NULL == skip)
	{
		return 0;
	}

	struct range if_and_unless;

	/**/
	struct buffer output_of_code;

	SET_NULL_TO_BUFFER(output_of_code);

	/**/
	const char* if_and_unless_strings[2];

	if_and_unless_strings[0] = if_str;

	if_and_unless_strings[1] = unless_str;

	/**/
	uint8_t if_and_unless_lengths[2];

	if_and_unless_lengths[0] = if_length;

	if_and_unless_lengths[1] = unless_length;

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

			const char* start = buffer_char_data(&output_of_code, 0);

			if (NULL == start)
			{
				buffer_release(&output_of_code);
				return 0;
			}

			const char* finish = buffer_char_data(&output_of_code, buffer_size(&output_of_code) - 1);

			if (NULL == finish)
			{
				buffer_release(&output_of_code);
				return 0;
			}

			if (!bool_parse(start, 1 + finish, &bool_value))
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
		const char* start_of_attributes, const char* finish_of_attributes,
		const char** attributes, const uint8_t* attributes_lengths,
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

static const char* interpreter_task_str[] =
{
	"al", "asminfo", "attrib", "aximp", "call", "choose", "cl", "copy", "csc",
	"delay-sign", "delete", "description", "echo", "exec", "fail", "foreach", "get", "gunzip", "if",
	"ilasm", "ildasm", "include", "jsc", "lib", "license", "link", "loadfile", "loadtasks", "mail", "mc", "midl",
	"mkdir", "move", "ndoc", "nunit2", "property", "rc", "readregistry", "regasm",
	"regex", "regsvcs", "resgen", "script", "servicecontroller", "setenv", "sleep", "solution", "style",
	"sysinfo", "tar", "tlbexp", "tlbimp", "touch", "trycatch", "tstamp", "untar", "unzip", "uptodate", "vbc",
	"vjc", "xmlpeek", "xmlpoke", "zip"
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

uint8_t interpreter_get_task(const char* task_name_start, const char* task_name_finish)
{
	return common_string_to_enum(task_name_start, task_name_finish,
								 interpreter_task_str, UNKNOWN_TASK);
}

uint8_t interpreter_evaluate_task(void* project, const void* target, uint8_t command,
								  const char* attributes_start, const char* element_finish)
{
	if (UNKNOWN_TASK < command || range_in_parts_is_null_or_empty(attributes_start, element_finish))
	{
		return 0;
	}

	const char* attributes_finish = xml_get_tag_finish_pos(attributes_start, element_finish);

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
