/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2022 TheVice
 *
 */

#include "choose_task.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "interpreter.h"
#include "range.h"
#include "string_unit.h"
#include "xml.h"

#include <stddef.h>

#define ELEMENTS_POSITION			0
#define ATTRIBUTE_POSITION			1

#define COUNT						(ATTRIBUTE_POSITION + 1)

uint8_t choose_evaluate_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	void* task_arguments, uint8_t verbose)
{
	if (!common_get_attributes_and_arguments_for_task(NULL, NULL, COUNT, NULL, NULL, NULL, task_arguments))
	{
		return 0;
	}

	void* elements = buffer_buffer_data(task_arguments, ELEMENTS_POSITION);

	if (!buffer_init(elements, buffer_size_of()))
	{
		return 0;
	}

	void* attribute_value = buffer_buffer_data(task_arguments, ATTRIBUTE_POSITION);

	if (!buffer_init(attribute_value, buffer_size_of()))
	{
		return 0;
	}

	static const uint8_t* tags = (const uint8_t*)"when\0otherwise\0";
	struct range sub_nodes_names;
	sub_nodes_names.start = tags;
	sub_nodes_names.finish = sub_nodes_names.start + 15;
	const uint16_t count = xml_get_sub_nodes_elements(
							   attributes_finish, element_finish,
							   &sub_nodes_names, elements);

	if (!count)
	{
		return 1;
	}

	uint16_t otherwise_index = count + 1;

	if (!buffer_append_buffer(attribute_value, NULL, 1))
	{
		return 0;
	}

	void* test_value_in_buffer = buffer_buffer_data(attribute_value, 0);

	if (!buffer_init(test_value_in_buffer, buffer_size_of()))
	{
		return 0;
	}

	sub_nodes_names.finish = sub_nodes_names.start + 4;

	for (uint16_t i = 0; i < count; ++i)
	{
		static const uint8_t* test = (const uint8_t*)"test";
		static const uint8_t test_length = 4;
		/**/
		struct range* element = buffer_range_data(elements, i);
		element_finish = xml_get_tag_name(element->start, element->finish);

		if (!string_equal(sub_nodes_names.start, sub_nodes_names.finish,
						  element->start, element_finish))
		{
			otherwise_index = count < otherwise_index ? i : otherwise_index;
			continue;
		}

		if (!interpreter_get_arguments_from_xml_tag_record(the_project, the_target,
				element->start, element->finish,
				&test, &test_length, 0, 1, attribute_value, verbose))
		{
			buffer_release_inner_buffers(attribute_value);
			return 0;
		}

		const uint8_t* value = buffer_uint8_t_data(test_value_in_buffer, 0);
		uint8_t test_value;

		if (!bool_parse(value, value + buffer_size(test_value_in_buffer),
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
		attributes_finish = element->start;
		element_finish = element->finish;

		if (!buffer_resize(elements, 0))
		{
			return 0;
		}

		return xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, elements) ?
			   interpreter_evaluate_tasks(the_project, the_target, elements, NULL, 0, verbose) : 1;
	}

	buffer_release_inner_buffers(attribute_value);

	if (otherwise_index < count)
	{
		struct range* element = buffer_range_data(elements, otherwise_index);
		attributes_finish = element->start;
		element_finish = element->finish;

		if (!buffer_resize(elements, 0))
		{
			return 0;
		}

		return xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, elements) ?
			   interpreter_evaluate_tasks(the_project, the_target, elements, NULL, 0, verbose) : 1;
	}

	return 1;
}
