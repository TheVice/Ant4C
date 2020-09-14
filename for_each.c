/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
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
#include "string_unit.h"
#include "xml.h"

#include <stdint.h>

static const uint8_t* items_str[] =
{
	(const uint8_t*)"File",
	(const uint8_t*)"Folder",
	(const uint8_t*)"Line",
	(const uint8_t*)"String"
};

enum items
{
	File,
	Folder,
	Line,
	String,
	UNKNOWN_ITEM
};

static const uint8_t* trims_str[] =
{
	(const uint8_t*)"Both",
	(const uint8_t*)"End",
	(const uint8_t*)"None",
	(const uint8_t*)"Start"
};

enum trims
{
	Both,
	End,
	None,
	Start,
	UNKNOWN_TRIM
};

uint8_t for_each_apply_trim(struct range* item, uint8_t trim_value)
{
	switch (trim_value)
	{
		case Both:
			return string_trim(item);

		case End:
			return string_trim_end(item);

		case None:
			return 1;

		case Start:
			return string_trim_start(item);

		default:
			break;
	}

	return 0;
}

uint8_t for_each_substring(void* the_project, const void* the_target,
						   const uint8_t* property_name, uint8_t property_name_length,
						   const uint8_t* start, const uint8_t* finish,
						   const uint8_t* substing_start, const uint8_t* substing_finish,
						   const struct buffer* elements, uint8_t trim_value, uint8_t verbose)
{
	while (start < finish)
	{
		ptrdiff_t index = string_index_of_any(start, finish, substing_start, substing_finish);
		const uint8_t* pos = (-1 == index) ? finish : start;

		while (0 < index && NULL != pos)
		{
			pos = string_enumerate(pos, finish, NULL);
			--index;
		}

		struct range item;

		item.start = start;

		item.finish = pos;

		if (!for_each_apply_trim(&item, trim_value))
		{
			return 0;
		}

		if (!project_property_set_value(the_project,
										property_name, property_name_length,
										item.start, range_size(&item),
										0, 1, 0, verbose))
		{
			return 0;
		}

		if (!interpreter_evaluate_tasks(the_project, the_target, elements, NULL, 0, verbose))
		{
			return 0;
		}

		pos = (-1 == index) ? finish : string_enumerate(pos, finish, NULL);
		start = NULL == pos ? finish : pos;
	}

	return 1;
}

uint8_t for_each_with_trim(void* the_project, const void* the_target,
						   const uint8_t* property_name, uint8_t property_name_length,
						   const uint8_t* start, const uint8_t* finish,
						   const struct buffer* delim, struct buffer* tmp,
						   const uint8_t* attributes_finish, const uint8_t* element_finish,
						   const uint8_t* delimiter, uint8_t trim_value, uint8_t verbose)
{
	if (!buffer_resize(tmp, 0))
	{
		return 0;
	}

	if (!xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, tmp))
	{
		return 1;
	}

	struct range substing;

	BUFFER_TO_RANGE(substing, delim);

	if (!range_is_null_or_empty(&substing))
	{
		const uint8_t* pos = find_any_symbol_like_or_not_like_that(start, finish, delimiter, 1, 1, 1);
		/**/
		struct range item;
		item.start = start;
		item.finish = pos;

		if (!for_each_apply_trim(&item, trim_value))
		{
			return 0;
		}

		if (!for_each_substring(the_project, the_target,
								property_name, property_name_length,
								item.start, item.finish,
								substing.start, substing.finish,
								tmp, trim_value, verbose))
		{
			return 0;
		}

		return 1;
	}

	return for_each_substring(the_project, the_target,
							  property_name, property_name_length,
							  start, finish,
							  delimiter, delimiter + 1,
							  tmp, trim_value, verbose);
}

uint8_t for_each_file_system_entries(void* the_project, const void* the_target,
									 const uint8_t* property_name, uint8_t property_name_length,
									 struct buffer* input, struct buffer* tmp,
									 const uint8_t* attributes_finish, const uint8_t* element_finish,
									 uint8_t item_value, uint8_t fail_on_error, uint8_t verbose)
{
	if (NULL == the_project ||
		NULL == property_name ||
		0 == property_name_length ||
		NULL == input ||
		NULL == tmp ||
		(File != item_value && Folder != item_value))
	{
		return 0;
	}

	if (!buffer_size(input))
	{
		if (!project_get_current_directory(the_project, the_target, input, 0, verbose))
		{
			return 0;
		}
	}

#ifdef _WIN32
	static const uint8_t* wild_card = (const uint8_t*)"*\0";

	if (!path_combine_in_place(input, 0, wild_card, wild_card + 2))
	{
		return 0;
	}

#else

	if (!buffer_push_back(input, 0))
	{
		return 0;
	}

#endif

	if (!buffer_resize(tmp, 0))
	{
		return 0;
	}

	fail_on_error = directory_enumerate_file_system_entries(input, File == item_value, 0, tmp, fail_on_error);

	if (!fail_on_error)
	{
		return 0;
	}

	const uint8_t* start = buffer_data(tmp, 0);
	const uint8_t* finish = start + buffer_size(tmp);
	static const uint8_t zero = 0;

	if (!for_each_with_trim(the_project, the_target,
							property_name, property_name_length,
							start, finish, NULL, input,
							attributes_finish, element_finish,
							&zero, None, verbose))
	{
		return 0;
	}

	return fail_on_error;
}

uint8_t for_each_line(void* the_project, const void* the_target,
					  const uint8_t* property_name, uint8_t property_name_length,
					  struct buffer* input, struct buffer* delim, struct buffer* tmp,
					  const uint8_t* attributes_finish, const uint8_t* element_finish,
					  uint8_t trim_value, uint8_t verbose)
{
	if (!buffer_size(input))
	{
		return 1;
	}

	if (!buffer_push_back(input, 0))
	{
		return 0;
	}

	const uint8_t* start = buffer_data(input, 0);

	if (!buffer_resize(input, 0) ||
		!file_read_all(start, input))
	{
		return 0;
	}

	if (!buffer_size(input))
	{
		return 1;
	}

	start = buffer_data(input, 0);
	const uint8_t* finish = start + buffer_size(input);
	static const uint8_t n = '\n';
	/**/
	return for_each_with_trim(the_project, the_target,
							  property_name, property_name_length,
							  start, finish, delim, tmp,
							  attributes_finish, element_finish,
							  &n, trim_value, verbose);
}

uint8_t for_each_string(void* the_project, const void* the_target,
						const uint8_t* property_name, uint8_t property_name_length,
						const struct buffer* input, const struct buffer* delim, struct buffer* tmp,
						const uint8_t* attributes_finish, const uint8_t* element_finish,
						uint8_t trim_value, uint8_t verbose)
{
	if (!buffer_size(input))
	{
		return 1;
	}

	const uint8_t* start = buffer_data(input, 0);
	const uint8_t* finish = start + buffer_size(input);
	static const uint8_t zero = 0;
	/**/
	return for_each_with_trim(the_project, the_target,
							  property_name, property_name_length,
							  start, finish, delim, tmp,
							  attributes_finish, element_finish,
							  &zero, trim_value, verbose);
}

#define FOR_EACH_ITEM_POSITION		0
#define FOR_EACH_PROPERTY_POSITION	1
#define FOR_EACH_DELIM_POSITION		2
#define FOR_EACH_IN_POSITION		3
#define FOR_EACH_TRIM_POSITION		4

uint8_t for_each_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	static const uint8_t* for_each_attributes[] =
	{
		(const uint8_t*)"item",
		(const uint8_t*)"property",
		(const uint8_t*)"delim",
		(const uint8_t*)"in",
		(const uint8_t*)"trim"
	};
	/**/
	static const uint8_t for_each_attributes_lengths[] =
	{
		4, 8, 5, 2, 4
	};
	/**/
	return common_get_attributes_and_arguments_for_task(
			   for_each_attributes, for_each_attributes_lengths,
			   COUNT_OF(for_each_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t for_each_evaluate_task(void* the_project, const void* the_target,
							   const uint8_t* attributes_finish, const uint8_t* element_finish,
							   struct buffer* task_arguments, uint8_t fail_on_error, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* item_value_in_a_buffer = buffer_buffer_data(task_arguments, FOR_EACH_ITEM_POSITION);

	if (!buffer_size(item_value_in_a_buffer))
	{
		return 0;
	}

	const uint8_t item_value = common_string_to_enum(buffer_data(item_value_in_a_buffer, 0),
							   buffer_data(item_value_in_a_buffer, 0) + buffer_size(item_value_in_a_buffer), items_str, UNKNOWN_ITEM);

	if (UNKNOWN_ITEM == item_value ||
		!buffer_resize(item_value_in_a_buffer, 0))
	{
		return 0;
	}

	const struct buffer* property_name_in_a_buffer = buffer_buffer_data(task_arguments,
			FOR_EACH_PROPERTY_POSITION);
	const uint8_t property_name_length = (uint8_t)buffer_size(property_name_in_a_buffer);

	if (!property_name_length)
	{
		return 0;
	}

	const uint8_t* property_name = buffer_data(property_name_in_a_buffer, 0);
	/**/
	uint8_t dynamic = 0;
	void* the_property = NULL;

	if (project_property_exists(the_project, property_name, property_name_length, &the_property, verbose))
	{
		uint8_t read_only = 0;

		if (!property_is_readonly(the_property, &read_only) || read_only)
		{
			return 0;
		}

		if (!property_is_dynamic(the_property, &dynamic))
		{
			return 0;
		}

		if (!property_get_by_pointer(the_property, item_value_in_a_buffer))
		{
			return 0;
		}

		the_property = NULL;
	}
	else
	{
		if (!project_property_set_value(
				the_project, property_name, property_name_length,
				(const uint8_t*)&verbose, 0, 0, 1, 0, verbose))
		{
			return 0;
		}
	}

	struct buffer* delim_value_in_a_buffer = buffer_buffer_data(task_arguments, FOR_EACH_DELIM_POSITION);

	struct buffer* in_value_in_a_buffer = buffer_buffer_data(task_arguments, FOR_EACH_IN_POSITION);

	uint8_t trim_value = None;

	struct buffer* trim_value_in_a_buffer = buffer_buffer_data(task_arguments, FOR_EACH_TRIM_POSITION);

	if (buffer_size(trim_value_in_a_buffer))
	{
		trim_value = common_string_to_enum(buffer_data(trim_value_in_a_buffer, 0),
										   buffer_data(trim_value_in_a_buffer, 0) + buffer_size(trim_value_in_a_buffer),
										   trims_str, UNKNOWN_TRIM);

		if (UNKNOWN_TRIM == trim_value)
		{
			return 0;
		}
	}

	switch (item_value)
	{
		case File:
		case Folder:
			fail_on_error = for_each_file_system_entries(
								the_project, the_target, property_name, property_name_length,
								in_value_in_a_buffer, trim_value_in_a_buffer,
								attributes_finish, element_finish, item_value, fail_on_error, verbose);
			break;

		case Line:
			fail_on_error = for_each_line(
								the_project, the_target, property_name, property_name_length,
								in_value_in_a_buffer, delim_value_in_a_buffer, trim_value_in_a_buffer,
								attributes_finish, element_finish, trim_value, verbose);
			break;

		case String:
			fail_on_error = for_each_string(
								the_project, the_target, property_name, property_name_length,
								in_value_in_a_buffer, delim_value_in_a_buffer, trim_value_in_a_buffer,
								attributes_finish, element_finish, trim_value, verbose);
			break;

		default:
			return 0;
	}

	if (!fail_on_error)
	{
		return 0;
	}

	if (buffer_size(item_value_in_a_buffer))
	{
		if (!project_property_set_value(the_project, property_name, property_name_length,
										buffer_data(item_value_in_a_buffer, 0), buffer_size(item_value_in_a_buffer),
										dynamic, 1, 0, verbose))
		{
			return 0;
		}

		return fail_on_error;
	}

	if (!project_property_set_value(the_project, property_name, property_name_length,
									(const uint8_t*)&verbose, 0, 0, 1, 0, verbose))
	{
		return 0;
	}

	return fail_on_error;
}

uint8_t do_evaluate_task(void* the_project, const void* the_target,
						 const uint8_t* attributes_finish, const uint8_t* element_finish,
						 struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	if (!common_get_attributes_and_arguments_for_task(NULL, NULL, 1, NULL, NULL, NULL, task_arguments))
	{
		return 0;
	}

	struct buffer* elements = buffer_buffer_data(task_arguments, 0);

	if (!elements)
	{
		return 0;
	}

	if (xml_get_sub_nodes_elements(attributes_finish, element_finish, NULL, elements))
	{
		return interpreter_evaluate_tasks(the_project, the_target, elements, NULL, 0, verbose);
	}

	return 1;
}
