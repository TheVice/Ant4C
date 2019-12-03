/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "property.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "project.h"
#include "range.h"
#include "xml.h"

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
	uint8_t readonly;
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

	for (uint8_t i = 0; i < name_length; ++i)
	{
		const uint8_t is_letter = ('A' <= name[i] && name[i] <= 'Z') || ('a' <= name[i] && name[i] <= 'z');
		const uint8_t is_digit = ('0' <= name[i] && name[i] <= '9');
		const uint8_t is_underscore = ('_' == name[i]);
		const uint8_t is_dash = ('-' == name[i]);
		const uint8_t is_dot = ('.' == name[i]);

		if (!is_letter && !is_digit && !is_underscore && !is_dash && !is_dot)
		{
			return 0;
		}

		if ((0 == i || name_length - 1 == i) && (is_dash || is_dot))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t property_new(struct buffer* properties,
					 const uint8_t* property_name,
					 uint8_t property_name_length,
					 const void* value, ptrdiff_t value_length,
					 enum data_type type_of_value,
					 uint8_t dynamic, uint8_t readonly, uint8_t verbose)
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
	the_property.dynamic = the_property.readonly = 0;

	if (!property_set_by_pointer(&the_property, value, value_length, type_of_value, dynamic,
								 readonly, verbose))
	{
		buffer_release(&the_property.value);
		return 0;
	}

	return buffer_append_property(properties, &the_property, 1);
}

uint8_t property_get_by_name(const void* project, const uint8_t* property_name, uint8_t property_name_length,
							 struct buffer* output)
{
	if (NULL == output)
	{
		return 0;
	}

	void* the_property = NULL;

	if (!project_property_exists(project, property_name, property_name_length, &the_property))
	{
		return 0;
	}

	return property_get_by_pointer(the_property, output);
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

uint8_t property_is_dynamic(const void* the_property, uint8_t* is_dynamic)
{
	if (NULL == the_property ||
		NULL == is_dynamic)
	{
		return 0;
	}

	const struct property* prop = (const struct property*)the_property;
	*is_dynamic = prop->dynamic;
	return 1;
}

uint8_t property_is_readonly(const void* the_property, uint8_t* is_readonly)
{
	if (NULL == the_property ||
		NULL == is_readonly)
	{
		return 0;
	}

	const struct property* prop = (const struct property*)the_property;
	*is_readonly = prop->readonly;
	return 1;
}

uint8_t property_set_by_name(struct buffer* properties,
							 const uint8_t* name, uint8_t name_length,
							 const uint8_t* value, ptrdiff_t value_length,
							 enum data_type type_of_value,
							 uint8_t dynamic, uint8_t overwrite,
							 uint8_t readonly, uint8_t verbose)
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
		if (!overwrite)
		{
			return 1;
		}

		return property_set_by_pointer(prop,
									   value, value_length, type_of_value,
									   dynamic, readonly, verbose);
	}

	return property_new(properties, name, name_length,
						value, value_length, type_of_value,
						dynamic, readonly, verbose);
}

uint8_t property_set_by_pointer(void* the_property,
								const void* value, ptrdiff_t value_length,
								enum data_type type_of_value,
								uint8_t dynamic, uint8_t readonly, uint8_t verbose)
{
	if (NULL == the_property ||
		NULL == value)
	{
		return 0;
	}

	(void)verbose;/*TODO: */
	struct property* prop = (struct property*)the_property;

	if (prop->readonly)
	{
		return 0;
	}

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
	prop->readonly = 0 < readonly;
	return 1;
}

static const uint8_t* property_attributes[] =
{
	(const uint8_t*)"dynamic",
	(const uint8_t*)"name",
	(const uint8_t*)"overwrite",
	(const uint8_t*)"readonly",
	(const uint8_t*)"failonerror",
	(const uint8_t*)"verbose",
	(const uint8_t*)"value"
};

static const uint8_t property_attributes_lengths[] = { 7, 4, 9, 8, 11, 7, 5 };

#define DYNAMIC_POSITION		0
#define NAME_POSITION			1
#define OVERWRITE_POSITION		2
#define READONLY_POSITION		3
#define FAIL_ON_ERROR_POSITION	4
#define VERBOSE_POSITION		5
#define VALUE_POSITION			6

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

uint8_t property_evaluate_task(void* project, const struct buffer* task_arguments)
{
	if (NULL == project || NULL == task_arguments)
	{
		return 0;
	}

	const struct buffer* name = buffer_buffer_data(task_arguments, NAME_POSITION);

	if (!buffer_size(name))
	{
		return 0;
	}

	const struct buffer* value = buffer_buffer_data(task_arguments, VALUE_POSITION);
	const struct buffer* dynamic_in_buffer = buffer_buffer_data(task_arguments, DYNAMIC_POSITION);
	const struct buffer* overwrite_in_buffer = buffer_buffer_data(task_arguments, OVERWRITE_POSITION);
	const struct buffer* readonly_in_buffer = buffer_buffer_data(task_arguments, READONLY_POSITION);
	/*const struct buffer* fail_on_error = buffer_buffer_data(task_arguments, FAIL_ON_ERROR_POSITION);*/
	const struct buffer* verbose_in_buffer = buffer_buffer_data(task_arguments, VERBOSE_POSITION);
	/**/
	uint8_t dynamic = 0;

	if (buffer_size(dynamic_in_buffer) &&
		!bool_parse(buffer_data(dynamic_in_buffer, 0), buffer_size(dynamic_in_buffer), &dynamic))
	{
		return 0;
	}

	uint8_t overwrite = 1;

	if (buffer_size(overwrite_in_buffer) &&
		!bool_parse(buffer_data(overwrite_in_buffer, 0), buffer_size(overwrite_in_buffer), &overwrite))
	{
		return 0;
	}

	uint8_t readonly = 0;

	if (buffer_size(readonly_in_buffer) &&
		!bool_parse(buffer_data(readonly_in_buffer, 0), buffer_size(readonly_in_buffer), &readonly))
	{
		return 0;
	}

	uint8_t verbose = 0;

	if (buffer_size(verbose_in_buffer) &&
		!bool_parse(buffer_data(verbose_in_buffer, 0), buffer_size(verbose_in_buffer), &verbose))
	{
		return 0;
	}

	return project_property_set_value(
			   project,
			   buffer_data(name, 0), (uint8_t)buffer_size(name),
			   buffer_data(value, 0), buffer_size(value),
			   dynamic, overwrite, readonly, verbose);
	/*TODO: explain fail_on_error factor, if verbose set, and return true.
	return fail_on_error ? returned : 1;*/
}

uint8_t property_add_at_project(void* project, const struct buffer* properties, uint8_t verbose)
{
	if (NULL == project || NULL == properties)
	{
		return 0;
	}

	static const uint8_t overwrite = 1;
	ptrdiff_t i = 0;
	struct property* prop = NULL;

	while (NULL != (prop = buffer_property_data(properties, i++)))
	{
		const void* value = buffer_size(&prop->value) ? buffer_data(&prop->value, 0) : (void*)prop;

		if (!project_property_set_value(project, prop->name, prop->name_length,
										value, buffer_size(&prop->value), prop->dynamic,
										overwrite, prop->readonly, verbose))
		{
			return 0;
		}
	}

	return 1;
}

void property_clear(struct buffer* properties)
{
	ptrdiff_t i = 0;
	struct property* prop = NULL;

	if (NULL == properties)
	{
		return;
	}

	while (NULL != (prop = buffer_property_data(properties, i++)))
	{
		buffer_release(&prop->value);
	}

	buffer_release(properties);
}

static const uint8_t* property_str[] =
{
	(const uint8_t*)"exists",
	(const uint8_t*)"get-value",
	(const uint8_t*)"is-dynamic",
	(const uint8_t*)"is-readonly"
};

uint8_t property_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, property_str, PROPERTY_UNKNOWN_FUNCTION);
}

uint8_t property_exec_function(const void* project, uint8_t function, const struct buffer* arguments,
							   uint8_t arguments_count, const void** the_property, struct buffer* output)
{
	if (PROPERTY_UNKNOWN_FUNCTION <= function ||
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
	const uint8_t is_exists = project_property_exists(project,
							  argument.start, (uint8_t)range_size(&argument), &non_const_prop);
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
			return NULL != prop && bool_to_string(prop->readonly, output);

		case PROPERTY_UNKNOWN_FUNCTION:
		default:
			break;
	}

	return 0;
}
