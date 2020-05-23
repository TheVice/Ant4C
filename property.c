/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "property.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "load_file.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

struct property
{
	uint8_t name[UINT8_MAX + 1];
	uint8_t name_length;
	struct buffer value;
	uint8_t dynamic;
	uint8_t read_only;
};

uint8_t buffer_append_property(struct buffer* properties, const struct property* data, ptrdiff_t data_count)
{
	return buffer_append(properties, (const uint8_t*)data, sizeof(struct property) * data_count);
}

struct property* buffer_property_data(const struct buffer* properties, ptrdiff_t data_position)
{
	return (struct property*)buffer_data(properties, sizeof(struct property) * data_position);
}

uint8_t property_is_name_valid(const uint8_t* name, uint8_t name_length)
{
	if (NULL == name || 0 == name_length)
	{
		return 0;
	}

	uint32_t out = 0;
	const uint8_t* pos = NULL;
	const uint8_t* name_start = name;
	const uint8_t* name_finish = name + name_length;

	while (NULL != (pos = string_enumerate(name, name_finish)))
	{
		if (!text_encoding_decode_UTF8_single(name, pos, &out))
		{
			return 0;
		}

		if (INT8_MAX < out)
		{
			if (string_to_case(out, string_get_id_of_to_lower_function()) ==
				string_to_case(out, string_get_id_of_to_upper_function()))
			{
				return 0;
			}
		}
		else
		{
			const uint8_t is_letter = ('A' <= out && out <= 'Z') || ('a' <= out && out <= 'z');
			const uint8_t is_digit = ('0' <= out && out <= '9');
			const uint8_t is_underscore = ('_' == out);
			const uint8_t is_dash = ('-' == out);
			const uint8_t is_dot = ('.' == out);

			if (!is_letter && !is_digit && !is_underscore && !is_dash && !is_dot)
			{
				return 0;
			}

			if ((name_start == name || pos == name_finish) && (is_dash || is_dot))
			{
				return 0;
			}
		}

		name = pos;
	}

	return 1;
}

uint8_t property_new(struct buffer* properties,
					 const uint8_t* property_name,
					 uint8_t property_name_length,
					 const void* value, ptrdiff_t value_length,
					 enum data_type type_of_value,
					 uint8_t dynamic, uint8_t read_only, uint8_t verbose)
{
	if (NULL == properties ||
		!property_is_name_valid(property_name, property_name_length))
	{
		return 0;
	}

	struct property the_property;

#if __STDC_SEC_API__
	if (0 != memcpy_s(the_property.name, UINT8_MAX, property_name, property_name_length))
	{
		return 0;
	}

#else
	memcpy(the_property.name, property_name, property_name_length);
#endif
	the_property.name[property_name_length] = '\0';
	the_property.name_length = property_name_length;
	SET_NULL_TO_BUFFER(the_property.value);
	the_property.dynamic = the_property.read_only = 0;

	if (!property_set_by_pointer(&the_property, value, value_length, type_of_value, dynamic,
								 read_only, verbose))
	{
		buffer_release(&the_property.value);
		return 0;
	}

	return buffer_append_property(properties, &the_property, 1);
}

uint8_t property_get_by_pointer(const void* the_property, struct buffer* output)
{
	if (NULL == the_property ||
		NULL == output)
	{
		return 0;
	}

	const struct property* prop = (const struct property*)the_property;

	/*TODO: struct property* prop = NULL;

	while (NULL != (prop = buffer_property_data(properties, i++)))
	{
		if (the_property == prop)
		{
			break;
		}
	}*/
	if (!buffer_size(&prop->value))
	{
		return 1;
	}

	return buffer_append_data_from_buffer(output, &prop->value);
}

uint8_t property_exists(const struct buffer* properties, const uint8_t* name, uint8_t name_length,
						void** the_property)
{
	ptrdiff_t i = 0;
	struct property* prop = NULL;

	if (NULL == properties ||
		!property_is_name_valid(name, name_length))
	{
		return 0;
	}

	while (NULL != (prop = buffer_property_data(properties, i++)))
	{
		if (name_length == prop->name_length &&
			0 == memcmp(&prop->name, name, name_length))
		{
			if (the_property)
			{
				(*the_property) = prop;
			}

			return 1;
		}
	}

	return 0;
}

uint8_t property_is_dynamic(const void* the_property, uint8_t* dynamic)
{
	if (NULL == the_property ||
		NULL == dynamic)
	{
		return 0;
	}

	const struct property* prop = (const struct property*)the_property;
	*dynamic = prop->dynamic;
	return 1;
}

uint8_t property_is_readonly(const void* the_property, uint8_t* read_only)
{
	if (NULL == the_property ||
		NULL == read_only)
	{
		return 0;
	}

	const struct property* prop = (const struct property*)the_property;
	*read_only = prop->read_only;
	return 1;
}

uint8_t property_set_by_name(struct buffer* properties,
							 const uint8_t* name, uint8_t name_length,
							 const uint8_t* value, ptrdiff_t value_length,
							 enum data_type type_of_value,
							 uint8_t dynamic, uint8_t over_write,
							 uint8_t read_only, uint8_t verbose)
{
	struct property* prop = NULL;

	if (NULL == properties ||
		!property_is_name_valid(name, name_length) ||
		NULL == value || value_length < 0)
	{
		return 0;
	}

	if (property_exists(properties, name, name_length, (void**)&prop))
	{
		if (!over_write)
		{
			return 1;
		}

		return property_set_by_pointer(prop,
									   value, value_length, type_of_value,
									   dynamic, read_only, verbose);
	}

	return property_new(properties, name, name_length,
						value, value_length, type_of_value,
						dynamic, read_only, verbose);
}

uint8_t property_set_by_pointer(void* the_property,
								const void* value, ptrdiff_t value_length,
								enum data_type type_of_value,
								uint8_t dynamic, uint8_t read_only, uint8_t verbose)
{
	if (NULL == the_property ||
		NULL == value)
	{
		return 0;
	}

	(void)verbose;/*TODO: */
	struct property* prop = (struct property*)the_property;

	if (prop->read_only)
	{
		/*TODO: inform about attempt to set readonly property, but not fail process.*/
		return 1;
	}

	/*TODO: add append optimization:
	if property value match with begin of new value, just add new part.*/
	if (!buffer_resize(&prop->value, 0))
	{
		return 0;
	}

	if (0 < value_length)
	{
		switch (type_of_value)
		{
			case property_value_is_byte_array:
				if (!buffer_append(&prop->value, value, value_length))
				{
					return 0;
				}

				break;

			case property_value_is_integer:
				if (!int64_to_string(*((const int64_t*)value), &prop->value))
				{
					return 0;
				}

				break;

			case property_value_is_double:
				if (!double_to_string(*((const double*)value), &prop->value))
				{
					return 0;
				}

				break;

			default:
				return 0;
		}
	}

	prop->dynamic = 0 < dynamic;
	prop->read_only = 0 < read_only;
	return 1;
}

uint8_t property_set_from_file(
	void* the_property,
	const uint8_t* file_name, uint16_t encoding,
	uint8_t dynamic, uint8_t read_only, uint8_t verbose)
{
	if (NULL == the_property ||
		NULL == file_name)
	{
		return 0;
	}

	(void)verbose;/*TODO: */
	struct property* prop = (struct property*)the_property;

	if (prop->read_only)
	{
		return 0;
	}

	if (!buffer_resize(&prop->value, 0))
	{
		return 0;
	}

	if (!load_file_to_buffer(file_name, encoding, &prop->value, verbose))
	{
		return 0;
	}

	prop->dynamic = 0 < dynamic;
	prop->read_only = 0 < read_only;
	return 1;
}

uint8_t property_set_from_stream(void* the_property, void* stream,
								 uint8_t dynamic, uint8_t read_only, uint8_t verbose)
{
	if (NULL == the_property ||
		NULL == stream)
	{
		return 0;
	}

	(void)verbose;/*TODO: */
	struct property* prop = (struct property*)the_property;

	if (prop->read_only)
	{
		return 0;
	}

	if (!buffer_resize(&prop->value, 0))
	{
		return 0;
	}

	if (!file_read_with_several_steps(stream, &prop->value))
	{
		return 0;
	}

	prop->dynamic = 0 < dynamic;
	prop->read_only = 0 < read_only;
	return 1;
}

static const uint8_t* property_attributes[] =
{
	(const uint8_t*)"dynamic",
	(const uint8_t*)"name",
	(const uint8_t*)"overwrite",
	(const uint8_t*)"readonly",
	(const uint8_t*)"value"
};

static const uint8_t property_attributes_lengths[] = { 7, 4, 9, 8, 5 };

#define DYNAMIC_POSITION		0
#define NAME_POSITION			1
#define OVER_WRITE_POSITION		2
#define READ_ONLY_POSITION		3
#define VALUE_POSITION			4

uint8_t property_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   property_attributes, property_attributes_lengths,
			   COUNT_OF(property_attributes_lengths),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t property_evaluate_task(void* the_project, struct buffer* properties,
							   const struct buffer* task_arguments, uint8_t verbose)
{
	if ((NULL == the_project && NULL == properties) ||
		NULL == task_arguments)
	{
		return 0;
	}

	const struct buffer* name = buffer_buffer_data(task_arguments, NAME_POSITION);
	const uint8_t name_length = (uint8_t)buffer_size(name);

	if (!name_length)
	{
		return 0;
	}

	const struct buffer* value_in_a_buffer = buffer_buffer_data(task_arguments, VALUE_POSITION);
	const struct buffer* dynamic_in_a_buffer = buffer_buffer_data(task_arguments, DYNAMIC_POSITION);
	const struct buffer* over_write_in_a_buffer = buffer_buffer_data(task_arguments, OVER_WRITE_POSITION);
	const struct buffer* read_only_in_a_buffer = buffer_buffer_data(task_arguments, READ_ONLY_POSITION);
	/**/
	const uint8_t* value = buffer_data(value_in_a_buffer, 0);

	if (NULL == value)
	{
		value = (const uint8_t*)&value;
	}

	uint8_t dynamic = (uint8_t)buffer_size(dynamic_in_a_buffer);

	if (dynamic && !bool_parse(buffer_data(dynamic_in_a_buffer, 0), dynamic, &dynamic))
	{
		return 0;
	}

	uint8_t over_write = 1;

	if (buffer_size(over_write_in_a_buffer) &&
		!bool_parse(buffer_data(over_write_in_a_buffer, 0), buffer_size(over_write_in_a_buffer), &over_write))
	{
		return 0;
	}

	uint8_t read_only = (uint8_t)buffer_size(read_only_in_a_buffer);

	if (read_only && !bool_parse(buffer_data(read_only_in_a_buffer, 0), read_only, &read_only))
	{
		return 0;
	}

	if (the_project)
	{
		return project_property_set_value(
				   the_project,
				   buffer_data(name, 0), name_length,
				   value, buffer_size(value_in_a_buffer),
				   dynamic, over_write, read_only, verbose);
	}

	return property_set_by_name(
			   properties,
			   buffer_data(name, 0), name_length,
			   value, buffer_size(value_in_a_buffer),
			   property_value_is_byte_array,
			   dynamic, over_write, read_only, verbose);
}

uint8_t property_add_at_project(void* the_project, const struct buffer* properties,
								uint8_t(*except_filter)(const uint8_t*, uint8_t), uint8_t verbose)
{
	if (NULL == the_project || NULL == properties)
	{
		return 0;
	}

	ptrdiff_t i = 0;
	const struct property* prop = NULL;
	static const uint8_t over_write = 1;

	if (except_filter)
	{
		while (NULL != (prop = buffer_property_data(properties, i++)))
		{
			if (except_filter(prop->name, prop->name_length))
			{
				continue;
			}

			const ptrdiff_t size = buffer_size(&prop->value);
			const void* value = size ? buffer_data(&prop->value, 0) : (const void*)prop;

			if (!project_property_set_value(the_project, prop->name, prop->name_length,
											value, size, prop->dynamic, over_write,
											prop->read_only, verbose))
			{
				return 0;
			}
		}
	}
	else
	{
		while (NULL != (prop = buffer_property_data(properties, i++)))
		{
			const ptrdiff_t size = buffer_size(&prop->value);
			const void* value = size ? buffer_data(&prop->value, 0) : (const void*)prop;

			if (!project_property_set_value(the_project, prop->name, prop->name_length,
											value, size, prop->dynamic, over_write,
											prop->read_only, verbose))
			{
				return 0;
			}
		}
	}

	return 1;
}

void property_release_inner(struct buffer* properties)
{
	if (NULL == properties)
	{
		return;
	}

	ptrdiff_t i = 0;
	struct property* prop = NULL;

	while (NULL != (prop = buffer_property_data(properties, i++)))
	{
		buffer_release(&prop->value);
	}
}

void property_release(struct buffer* properties)
{
	if (NULL == properties)
	{
		return;
	}

	property_release_inner(properties);
	buffer_release(properties);
}

static const uint8_t* property_str[] =
{
	(const uint8_t*)"exists",
	(const uint8_t*)"get-value",
	(const uint8_t*)"is-dynamic",
	(const uint8_t*)"is-readonly"
};

enum property_function
{
	property_exists_function,
	property_get_value_function,
	property_is_dynamic_function,
	property_is_readonly_function,
	UNKNOWN_PROPERTY_FUNCTION
};

uint8_t property_get_id_of_get_value_function()
{
	return property_get_value_function;
}

uint8_t property_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, property_str, UNKNOWN_PROPERTY_FUNCTION);
}

uint8_t property_exec_function(const void* the_project, uint8_t function, const struct buffer* arguments,
							   uint8_t arguments_count, const void** the_property, struct buffer* output, uint8_t verbose)
{
	if (UNKNOWN_PROPERTY_FUNCTION <= function ||
		NULL == arguments ||
		1 != arguments_count ||
		NULL == the_property ||
		NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_one_argument(arguments, &argument, 0))
	{
		return 0;
	}

	void* non_const_prop = NULL;
	const uint8_t is_exists = project_property_exists(the_project,
							  argument.start, (uint8_t)range_size(&argument), &non_const_prop, verbose);
	(*the_property) = is_exists ? non_const_prop : NULL;
	const struct property* prop = (const struct property*)(*the_property);

	if (function != property_exists_function && !is_exists)
	{
		return 0;
	}

	switch (function)
	{
		case property_exists_function:
			return bool_to_string(is_exists, output);

		case property_get_value_function:
			return is_exists && property_get_by_pointer(prop, output);

		case property_is_dynamic_function:
			return NULL != prop && bool_to_string(prop->dynamic, output);

		case property_is_readonly_function:
			return NULL != prop && bool_to_string(prop->read_only, output);

		case UNKNOWN_PROPERTY_FUNCTION:
		default:
			break;
	}

	return 0;
}
