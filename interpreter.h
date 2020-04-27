/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

uint8_t interpreter_disassemble_function(const struct range* function,
		struct range* name_space, struct range* name, struct range* arguments_area);
uint8_t interpreter_actualize_property_value(const void* the_project, const void* the_target,
		uint8_t property_function_id, const void* the_property,
		ptrdiff_t size, struct buffer* output, uint8_t verbose);
uint8_t interpreter_get_values_for_arguments(const void* the_project, const void* the_target,
		const struct range* arguments_area, struct buffer* values, uint8_t verbose);
uint8_t interpreter_evaluate_function(const void* the_project, const void* the_target,
									  const struct range* function, struct buffer* output, uint8_t verbose);
uint8_t interpreter_evaluate_code(const void* the_project, const void* the_target,
								  const struct range* code, struct buffer* output, uint8_t verbose);
uint8_t interpreter_is_xml_tag_should_be_skip_by_if_or_unless(const void* the_project, const void* the_target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
		uint8_t* skip, struct buffer* attributes, uint8_t verbose);
uint8_t interpreter_get_xml_tag_attribute_values(const void* the_project, const void* the_target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
		uint8_t* fail_on_error, uint8_t* task_verbose, struct buffer* attributes, uint8_t verbose);
uint8_t interpreter_get_arguments_from_xml_tag_record(const void* the_project, const void* the_target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
		const uint8_t** attributes, const uint8_t* attributes_lengths,
		uint8_t index, uint8_t attributes_count, struct buffer* output, uint8_t verbose);
uint8_t interpreter_get_task(const uint8_t* task_name_start, const uint8_t* task_name_finish);
uint8_t interpreter_prepare_attributes_and_arguments_for_property_task(
	const void* the_project, const void* the_target,
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments,
	const uint8_t* attributes_start, const uint8_t* attributes_finish, uint8_t verbose);
uint8_t interpreter_evaluate_task(void* the_project, const void* the_target, uint8_t command,
								  const uint8_t* attributes_start, const uint8_t* element_finish,
								  uint8_t target_help, uint8_t verbose);
uint8_t interpreter_evaluate_tasks(void* the_project, const void* the_target,
								   const struct buffer* elements,
								   uint8_t target_help, uint8_t verbose);

uint8_t task_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t task_exec_function(uint8_t function, const struct buffer* arguments,
						   uint8_t arguments_count, struct buffer* output);

#endif
