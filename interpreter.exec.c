/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include "interpreter.exec.h"

#include "buffer.h"
#include "common.h"
#include "interpreter.h"
#include "range.h"
#include "string_unit.h"
#include "xml.h"

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
	static const uint8_t count_of_attributes = 2;

	if (range_in_parts_is_null_or_empty(start_of_attributes, finish_of_attributes))
	{
		return 1;
	}

	if (NULL == attributes)
	{
		return 0;
	}

	if (!common_get_attributes_and_arguments_for_task(
			NULL, NULL, count_of_attributes,
			NULL, NULL, NULL, attributes))
	{
		return 0;
	}

	if (!interpreter_get_arguments_from_xml_tag_record(
			the_project, the_target, start_of_attributes, finish_of_attributes,
			name_and_value, name_and_value_lengths, 0, count_of_attributes, attributes, verbose))
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
		const uint8_t* tag_name_finish = xml_get_tag_name(env_ptr->start, env_ptr->finish);

		if (!string_equal(env_ptr->start, tag_name_finish, env_name, env_name + 11))
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

			for (uint8_t i = 0; i < 2; ++i)
			{
				const struct buffer* name_value = buffer_buffer_data(&attribute_value, i);

				if (!name_value)
				{
					buffer_release(&sub_elements);
					buffer_release_with_inner_buffers(&attribute_value);
					buffer_release(&elements);
					return 0;
				}

				if (i < 1 && !buffer_size(name_value))
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

				if (i < 1)
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
