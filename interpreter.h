/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

/*conversion*/
uint8_t conversion_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t bool_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);
uint8_t double_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);
uint8_t int_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);
uint8_t long_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);
uint8_t int64_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);

/*date_time*/
uint8_t datetime_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t datetime_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);

uint8_t timespan_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t timespan_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);

/*environment*/
uint8_t environment_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t environment_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);

/*operating_system*/
uint8_t os_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t os_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);
uint8_t platform_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);

/*path*/
uint8_t path_get_id_of_get_full_path_function();
uint8_t path_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t path_exec_function(
	const void* project,
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);
uint8_t cygpath_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);

/*sleep_unit*/
uint8_t sleep_unit_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t sleep_unit_evaluate_task(
	struct buffer* task_arguments, uint8_t verbose);

/*version*/
uint8_t version_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t version_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output);

uint8_t interpreter_disassemble_function(
	const struct range* function, struct range* name_space,
	struct range* name, struct range* arguments_area);
uint8_t interpreter_actualize_property_value(
	const void* the_project, const void* the_target,
	uint8_t property_function_id, const void* the_property,
	ptrdiff_t size, struct buffer* output, uint8_t verbose);
uint8_t interpreter_get_values_for_arguments(
	const void* the_project, const void* the_target,
	const struct range* arguments_area, struct buffer* values,
	uint8_t* values_count, uint8_t verbose);
uint8_t interpreter_evaluate_function(
	const void* the_project, const void* the_target,
	const struct range* function, struct buffer* output, uint8_t verbose);
uint8_t interpreter_evaluate_code(
	const void* the_project, const void* the_target,
	const struct range* code, struct buffer* output, uint8_t verbose);
uint8_t interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
	const void* the_project, const void* the_target,
	const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
	uint8_t* skip, struct buffer* attributes, uint8_t verbose);
uint8_t interpreter_get_xml_tag_attribute_values(
	const void* the_project, const void* the_target,
	const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
	uint8_t* fail_on_error, uint8_t* task_verbose,
	struct buffer* attributes, uint8_t verbose);
uint8_t interpreter_get_arguments_from_xml_tag_record(
	const void* the_project, const void* the_target,
	const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
	const uint8_t** attributes, const uint8_t* attributes_lengths,
	uint8_t index, uint8_t attributes_count,
	struct buffer* output, uint8_t verbose);
uint8_t interpreter_get_xml_element_value(
	const void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	struct buffer* output, uint8_t verbose);
uint8_t interpreter_get_task(
	const uint8_t* task_name_start, const uint8_t* task_name_finish);
uint8_t interpreter_prepare_attributes_and_arguments_for_property_task(
	const void* the_project, const void* the_target,
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	uint8_t verbose);
uint8_t interpreter_evaluate_task(
	void* the_project, const void* the_target, const struct range* task_name,
	const uint8_t* element_finish, const struct buffer* sub_nodes_names,
	uint8_t target_help, uint8_t verbose);
uint8_t interpreter_evaluate_tasks(
	void* the_project, const void* the_target,
	const struct buffer* elements, const struct buffer* sub_nodes_names,
	uint8_t target_help, uint8_t verbose);
uint8_t interpreter_get_unknown_task_id();

#endif
