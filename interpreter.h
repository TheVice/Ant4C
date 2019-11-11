/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stdint.h>

struct buffer;
struct range;

uint8_t interpreter_get_value_from_quote(const struct range* quote, struct range* value);
uint8_t interpreter_disassemble_function(const struct range* function,
		struct range* name_space, struct range* name, struct range* arguments_area);
uint8_t interpreter_get_values_for_arguments(const void* project, const void* target,
		const struct range* arguments_area, struct buffer* values);
uint8_t interpreter_evaluate_function(const void* project, const void* target,
									  const struct range* function, struct buffer* return_of_function);
uint8_t interpreter_evaluate_code(const void* project, const void* target,
								  const struct range* code, struct buffer* output);
uint8_t interpreter_is_xml_tag_should_be_skip_by_if_or_unless(const void* project, const void* target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes, uint8_t* skip);
uint8_t interpreter_get_arguments_from_xml_tag_record(const void* project, const void* target,
		const uint8_t* start_of_attributes, const uint8_t* finish_of_attributes,
		const uint8_t** attributes, const uint8_t* attributes_lengths,
		uint8_t index, uint8_t attributes_count, struct buffer* output);
uint8_t interpreter_get_task(const uint8_t* task_name_start, const uint8_t* task_name_finish);
uint8_t interpreter_evaluate_task(void* project, const void* target, uint8_t command,
								  const uint8_t* attributes_start, const uint8_t* element_finish);
uint8_t interpreter_evaluate_tasks(void* project, const void* target,
								   const struct buffer* elements, uint8_t verbose);

#endif
