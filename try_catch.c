/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2021 TheVice
 *
 */

#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "interpreter.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "xml.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ELEMENTS_POSITION			0
#define SUB_ELEMENTS_POSITION		1

#define COUNT						(SUB_ELEMENTS_POSITION + 1)

void* try_catch_create_error_output_stream(struct buffer* tmp)
{
	if (NULL == tmp)
	{
		return NULL;
	}

	const ptrdiff_t size = buffer_size(tmp);

	if (!path_get_temp_file_name(tmp))
	{
		return NULL;
	}

	void* stream = NULL;

	if (!file_open(buffer_data(tmp, size), (const uint8_t*)"wb+", &stream))
	{
		return NULL;
	}

	return stream;
}

uint8_t try_catch_error_output_stream_read_to_property(void* stream, void* the_property, uint8_t verbose)
{
	if (NULL == stream ||
		NULL == the_property)
	{
		return 0;
	}

	if (!file_seek(stream, 0, SEEK_SET))
	{
		return 0;
	}

	return property_set_from_stream(the_property, stream, 0, 0, verbose);
}

uint8_t try_catch_copy_property_value(const void* the_source, void* the_destination,
									  struct buffer* tmp, uint8_t verbose)
{
	if (NULL == the_source ||
		NULL == the_destination ||
		NULL == tmp)
	{
		return 0;
	}

	if (the_source == the_destination)
	{
		return 1;
	}

	const ptrdiff_t size = buffer_size(tmp);

	if (!property_get_by_pointer(the_source, tmp))
	{
		return 0;
	}

	if (size == buffer_size(tmp))
	{
		return 1;
	}

	return property_set_by_pointer(the_destination,
								   buffer_data(tmp, size), buffer_size(tmp) - size,
								   property_value_is_byte_array, 0, 0, verbose);
}

uint8_t try_catch_evaluate_task(
	void* the_project, const void* the_target,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	struct buffer* task_arguments, uint8_t verbose)
{
	static const uint8_t* tags[] =
	{
		(const uint8_t*)"try\0",
		(const uint8_t*)"catch\0",
		(const uint8_t*)"finally\0"
	};
	/**/
	static const uint8_t tags_lengths[] =
	{
		4, 6, 8
	};
	/**/
	static const uint8_t count = 3;

	if (!common_get_attributes_and_arguments_for_task(NULL, NULL, COUNT, NULL, NULL, NULL, task_arguments))
	{
		return 0;
	}

	struct buffer tmp;

	SET_NULL_TO_BUFFER(tmp);

	struct buffer* elements = buffer_buffer_data(task_arguments, ELEMENTS_POSITION);

	struct buffer* sub_elements = buffer_buffer_data(task_arguments, SUB_ELEMENTS_POSITION);

	uint8_t result[3] = { 1, 1, 1 };

	void* error_output_stream = NULL;

	void* current_error_output_stream = common_is_error_output_stream_standard() ? NULL :
										common_get_error_output_stream();

	for (uint8_t i = 0; i < count;)
	{
		if (!buffer_resize(elements, 0))
		{
			memset(result, 0, sizeof(result));
			break;
		}

		struct range sub_nodes_name;

		sub_nodes_name.start = tags[i];

		sub_nodes_name.finish = sub_nodes_name.start + tags_lengths[i];

		if (!xml_get_sub_nodes_elements(attributes_finish, element_finish, &sub_nodes_name, elements))
		{
			if (!i)
			{
				i = 2;
			}
			else
			{
				++i;
			}

			continue;
		}

		if (!buffer_resize(sub_elements, 0))
		{
			memset(result, 0, sizeof(result));
			break;
		}

		ptrdiff_t index = 0;
		struct range* element;
		void* the_first_property = NULL;

		while (NULL != (element = buffer_range_data(elements, index++)))
		{
			if (xml_get_sub_nodes_elements(element->start, element->finish, NULL, sub_elements) && 1 == i)
			{
				if (!buffer_resize(&tmp, 0))
				{
					i = count;
					break;
				}

				static const uint8_t* property_str = (const uint8_t*)"property";
				static const uint8_t property_str_length = 8;

				if (xml_get_attribute_value(element->start, element->finish,
											property_str, property_str_length, &tmp))
				{
					void* the_property = NULL;
					const uint8_t* property_name = buffer_data(&tmp, 0);
					const uint8_t property_name_length = (uint8_t)buffer_size(&tmp);

					if (!project_property_set_value(
							the_project,
							property_name, property_name_length,
							(const uint8_t*)&index, 0, 0, 1, 0, verbose) ||
						!project_property_exists(
							the_project,
							property_name, property_name_length,
							&the_property, verbose))
					{
						i = count;
						break;
					}

					if (!the_first_property)
					{
						if (!try_catch_error_output_stream_read_to_property(
								error_output_stream, the_property, verbose))
						{
							i = count;
							break;
						}

						the_first_property = the_property;
					}
					else
					{
						if (!buffer_resize(&tmp, 0) ||
							!try_catch_copy_property_value(
								the_first_property, the_property, &tmp, verbose))
						{
							i = count;
							the_first_property = NULL;
							break;
						}
					}
				}
			}
		}

		if (count == i)
		{
			memset(result, 0, sizeof(result));
			break;
		}

		if (!i)
		{
			if (!buffer_resize(elements, 0) ||
				NULL == (error_output_stream = try_catch_create_error_output_stream(elements)))
			{
				memset(result, 0, sizeof(result));
				break;
			}

			common_set_error_output_stream(error_output_stream);
		}
		else if (error_output_stream)
		{
			if (!file_close(error_output_stream))
			{
				error_output_stream = NULL;
				memset(result, 0, sizeof(result));
				break;
			}

			error_output_stream = NULL;
		}

		result[i] = interpreter_evaluate_tasks(the_project, the_target, sub_elements, NULL, 0, verbose);

		if (!i)
		{
			common_set_error_output_stream(current_error_output_stream);

			if (!file_flush(error_output_stream))
			{
				memset(result, 0, sizeof(result));
				break;
			}
		}

		if (result[i] && !i)
		{
			i = 2;
			continue;
		}

		++i;
	}

	buffer_release(&tmp);

	if (error_output_stream)
	{
		common_set_error_output_stream(current_error_output_stream);
		result[0] = file_close(error_output_stream);
		error_output_stream = NULL;

		if (result[1])
		{
			result[1] = result[0];
		}
		else
		{
			result[2] = result[0];
		}
	}

	return result[1] && result[2];
}
