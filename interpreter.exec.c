/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include "interpreter.exec.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "exec.h"
#include "interpreter.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"
#include "xml.h"

/*#define NAME_POSITION			0*/
#define VALUE_POSITION			1

#define COUNT					(VALUE_POSITION + 1)

uint8_t interpreter_get_environments_value(
	const void* the_project,
	const void* the_target,
	const uint8_t* start_of_attributes,
	const uint8_t* finish_of_attributes,
	struct buffer* attributes,
	uint8_t verbose)
{
	static const uint8_t* name_and_value[] =
	{
		(const uint8_t*)"name",
		(const uint8_t*)"value"
	};
	/**/
	static const uint8_t name_and_value_lengths[] = { 4, 5 };

	if (range_in_parts_is_null_or_empty(start_of_attributes, finish_of_attributes))
	{
		return 1;
	}

	if (NULL == attributes)
	{
		return 0;
	}

	if (!common_get_attributes_and_arguments_for_task(
			NULL, NULL, COUNT,
			NULL, NULL, NULL, attributes))
	{
		return 0;
	}

	if (!interpreter_get_arguments_from_xml_tag_record(
			the_project, the_target, start_of_attributes, finish_of_attributes,
			name_and_value, name_and_value_lengths, 0, COUNT, attributes, verbose))
	{
		return 0;
	}

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

	uint16_t elements_count = xml_get_sub_nodes_elements(
								  attributes_finish, element_finish, NULL, &elements);

	if (!elements_count)
	{
		buffer_release(&elements);
		return 1;
	}

	elements_count = 0;
	struct range* env_ptr;
	struct buffer attribute_value;
	SET_NULL_TO_BUFFER(attribute_value);

	while (NULL != (env_ptr = buffer_range_data(&elements, elements_count++)))
	{
		static const uint8_t* env_name = (const uint8_t*)"environment";
		static const uint8_t env_name_length = 11;
		const uint8_t* tag_name_finish = xml_get_tag_name(env_ptr->start, env_ptr->finish);

		if (!string_equal(env_ptr->start, tag_name_finish, env_name, env_name + env_name_length))
		{
			continue;
		}

		struct buffer sub_elements;

		SET_NULL_TO_BUFFER(sub_elements);

		uint16_t sub_elements_count = xml_get_sub_nodes_elements(
										  env_ptr->start, env_ptr->finish, NULL, &sub_elements);

		if (!sub_elements_count)
		{
			buffer_release(&sub_elements);
			continue;
		}

		sub_elements_count = 0;

		while (NULL != (env_ptr = buffer_range_data(&sub_elements, sub_elements_count++)))
		{
			static const uint8_t* var_name = (const uint8_t*)"variable";
			tag_name_finish = xml_get_tag_name(env_ptr->start, env_ptr->finish);

			if (!string_equal(env_ptr->start, tag_name_finish, var_name, var_name + 8))
			{
				continue;
			}

			uint8_t skip;

			if (!interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
					the_project, the_target,
					tag_name_finish, env_ptr->finish,
					&skip, &attribute_value, verbose))
			{
				buffer_release(&sub_elements);
				buffer_release_with_inner_buffers(&attribute_value);
				buffer_release(&elements);
				return 0;
			}

			if (skip)
			{
				continue;
			}

			if (!interpreter_get_environments_value(
					the_project, the_target,
					tag_name_finish, env_ptr->finish,
					&attribute_value, verbose))
			{
				buffer_release(&sub_elements);
				buffer_release_with_inner_buffers(&attribute_value);
				buffer_release(&elements);
				return 0;
			}

			for (uint8_t i = 0; i < COUNT; ++i)
			{
				const struct buffer* name_value = buffer_buffer_data(&attribute_value, i);

				if (!name_value)
				{
					buffer_release(&sub_elements);
					buffer_release_with_inner_buffers(&attribute_value);
					buffer_release(&elements);
					return 0;
				}

				if (i < VALUE_POSITION && !buffer_size(name_value))
				{
					buffer_release(&sub_elements);
					buffer_release_with_inner_buffers(&attribute_value);
					buffer_release(&elements);
					return 0;
				}

				struct range name;

				BUFFER_TO_RANGE(name, name_value);

				static const uint8_t space = ' ';

				const uint8_t contains = string_contains(name.start, name.finish, &space, &space + 1);

				if (contains ||
					range_is_null_or_empty(&name))
				{
					if (!string_quote(name.start, name.finish, environments))
					{
						buffer_release(&sub_elements);
						buffer_release_with_inner_buffers(&attribute_value);
						buffer_release(&elements);
						return 0;
					}
				}
				else
				{
					if (!buffer_append_data_from_range(environments, &name))
					{
						buffer_release(&sub_elements);
						buffer_release_with_inner_buffers(&attribute_value);
						buffer_release(&elements);
						return 0;
					}
				}

				if (i < VALUE_POSITION)
				{
					static const uint8_t equal_symbol = '=';

					if (!buffer_push_back(environments, equal_symbol))
					{
						buffer_release(&sub_elements);
						buffer_release_with_inner_buffers(&attribute_value);
						buffer_release(&elements);
						return 0;
					}
				}
				else
				{
					static const uint8_t zero_symbol = '\0';

					if (!buffer_push_back(environments, zero_symbol))
					{
						buffer_release(&sub_elements);
						buffer_release_with_inner_buffers(&attribute_value);
						buffer_release(&elements);
						return 0;
					}
				}
			}
		}

		buffer_release(&sub_elements);
	}

	buffer_release_with_inner_buffers(&attribute_value);
	buffer_release(&elements);
	/**/
	return 1;
}

static const uint8_t* exec_attributes[] =
{
	(const uint8_t*)"program",
	(const uint8_t*)"append",
	(const uint8_t*)"basedir",
	(const uint8_t*)"commandline",
	(const uint8_t*)"output",
	(const uint8_t*)"pidproperty",
	(const uint8_t*)"resultproperty",
	(const uint8_t*)"spawn",
	(const uint8_t*)"workingdir",
	(const uint8_t*)"timeout"
};

static const uint8_t exec_attributes_lengths[] =
{
	7, 6, 7, 11, 6, 11, 14, 5, 10, 7
};

#define PROGRAM_POSITION			0
#define APPEND_POSITION				1
#define BASE_DIR_POSITION			2
#define COMMAND_LINE_POSITION		3
#define OUTPUT_POSITION				4
#define PID_PROPERTY_POSITION		5
#define RESULT_PROPERTY_POSITION	6
#define SPAWN_POSITION				7
#define WORKING_DIR_POSITION		8
#define TIME_OUT_POSITION			9
#define ENVIRONMENT_POSITION		10

#define ATTRIBUTES_COUNT	(ENVIRONMENT_POSITION + 1)

uint8_t exec_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	if (!common_get_attributes_and_arguments_for_task(
			exec_attributes, exec_attributes_lengths,
			COUNT_OF(exec_attributes_lengths),
			task_attributes, task_attributes_lengths,
			task_attributes_count, task_arguments))
	{
		return 0;
	}

	buffer_release_inner_buffers(task_arguments);

	if (!buffer_resize(task_arguments, 0) ||
		!buffer_append_buffer(task_arguments, NULL, ATTRIBUTES_COUNT))
	{
		return 0;
	}

	for (uint8_t i = 0, attributes_count = ATTRIBUTES_COUNT; i < attributes_count; ++i)
	{
		struct buffer* attribute = buffer_buffer_data(task_arguments, i);
		SET_NULL_TO_BUFFER(*attribute);
	}

	return 1;
}

uint8_t exec_evaluate_task(
	void* the_project, const void* the_target,
	const struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* path_to_the_program = buffer_buffer_data(task_arguments, PROGRAM_POSITION);

	if (!buffer_size(path_to_the_program))
	{
		return 0;
	}

	const struct buffer* append_in_a_buffer = buffer_buffer_data(task_arguments, APPEND_POSITION);
	uint8_t append = (uint8_t)buffer_size(append_in_a_buffer);
	const uint8_t* value = buffer_data(append_in_a_buffer, 0);

	if (append && !bool_parse(value, value + append, &append))
	{
		return 0;
	}

	const struct buffer* base_dir_in_a_buffer = buffer_buffer_data(task_arguments, BASE_DIR_POSITION);
	struct range base_directory;
	BUFFER_TO_RANGE(base_directory, base_dir_in_a_buffer);
	/**/
	const struct buffer* command_line_in_a_buffer = buffer_buffer_data(task_arguments, COMMAND_LINE_POSITION);
	struct range command_line;
	BUFFER_TO_RANGE(command_line, command_line_in_a_buffer);
	/**/
	struct buffer* output_path_in_a_buffer = buffer_buffer_data(task_arguments, OUTPUT_POSITION);
	struct range output_file;
	ptrdiff_t size = buffer_size(output_path_in_a_buffer);

	if (size)
	{
		if (!buffer_push_back(output_path_in_a_buffer, 0))
		{
			return 0;
		}

		++size;
		output_file.start = buffer_data(output_path_in_a_buffer, 0);
		output_file.finish = output_file.start + size;
	}
	else
	{
		output_file.start = output_file.finish = NULL;
	}

	void* pid_property = NULL;
	void* result_property = NULL;

	for (uint8_t index = PID_PROPERTY_POSITION; ; index = RESULT_PROPERTY_POSITION)
	{
		const struct buffer* property_in_a_buffer = buffer_buffer_data(task_arguments, index);
		size = buffer_size(property_in_a_buffer);

		if (!size)
		{
			if (RESULT_PROPERTY_POSITION == index)
			{
				break;
			}

			continue;
		}

		if (NULL == the_project)
		{
			return 0;
		}

		void** the_property = (PID_PROPERTY_POSITION == index ? &pid_property : &result_property);
		value = buffer_data(property_in_a_buffer, 0);

		if (!project_property_set_value(the_project, value,
										(uint8_t)size, (const uint8_t*)the_property,
										0, 0, 1, 0, verbose) ||
			!project_property_exists(the_project, value,
									 (uint8_t)size, the_property, verbose))
		{
			return 0;
		}

		if (RESULT_PROPERTY_POSITION == index)
		{
			break;
		}
	}

	const struct buffer* spawn_in_a_buffer = buffer_buffer_data(task_arguments, SPAWN_POSITION);
	uint8_t spawn = (uint8_t)buffer_size(spawn_in_a_buffer);
	value = buffer_data(spawn_in_a_buffer, 0);

	if (spawn && !bool_parse(value, value + spawn, &spawn))
	{
		return 0;
	}

	struct buffer* working_dir_in_a_buffer = buffer_buffer_data(task_arguments, WORKING_DIR_POSITION);

	struct range working_directory;

	size = buffer_size(working_dir_in_a_buffer);

	if (size)
	{
		if (!buffer_push_back(working_dir_in_a_buffer, 0))
		{
			return 0;
		}

		++size;
		working_directory.start = buffer_data(working_dir_in_a_buffer, 0);
		working_directory.finish = working_directory.start + size;
	}
	else
	{
		working_directory.start = working_directory.finish = NULL;
	}

	struct buffer* time_out_in_a_buffer = buffer_buffer_data(task_arguments, TIME_OUT_POSITION);

	uint64_t time_out = 0;

	size = buffer_size(time_out_in_a_buffer);

	if (size)
	{
		value = buffer_data(time_out_in_a_buffer, 0);
		time_out = uint64_parse(value, value + size);

		if (1000 < time_out)
		{
			time_out = (uint64_t)date_time_millisecond_to_second(time_out);

			if (time_out < 5)
			{
				time_out = 5;
			}
		}
	}

	const struct buffer* environment_in_a_buffer = buffer_buffer_data(task_arguments, ENVIRONMENT_POSITION);
	struct range environment_variables;
	size = buffer_size(environment_in_a_buffer);

	if (size)
	{
		environment_variables.start = buffer_data(environment_in_a_buffer, 0);
		environment_variables.finish = environment_variables.start + size;
	}
	else
	{
		environment_variables.start = environment_variables.finish = NULL;
	}

	return exec(the_project, the_target,
				append, path_to_the_program, &base_directory, &command_line, &output_file,
				pid_property, result_property, &working_directory, &environment_variables,
				spawn, (uint32_t)time_out, verbose);
}
