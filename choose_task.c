/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
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
		static const uint8_t* test = (const uint8_t*)"test";
		static const uint8_t test_length = 4;
		/**/
		struct range current_tag_name;
		current_tag_name.start = current_tag_name.finish = NULL;
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
			   interpreter_evaluate_tasks(the_project, the_target, elements, NULL, 0, verbose) : 1;
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
			   interpreter_evaluate_tasks(the_project, the_target, elements, NULL, 0, verbose) : 1;
	}

	return 1;
}
