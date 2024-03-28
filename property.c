/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022, 2024 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "property.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "interpreter.string_unit.h"
#include "load_file.h"
#include "project.h"
#include "string_unit.h"

#include <string.h>

struct property
{
	uint8_t name[UINT8_MAX + 1];
	uint8_t value[BUFFER_SIZE_OF];
	uint8_t name_length;
	uint8_t dynamic;
	uint8_t read_only;
};

uint8_t buffer_append_property(void* properties, const struct property* data, ptrdiff_t data_count)
{
	return buffer_append(properties, (const void*)data, sizeof(struct property) * data_count);
}

struct property* buffer_property_data(const void* properties, ptrdiff_t data_position)
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

	while (NULL != (pos = string_enumerate(name, name_finish, &out)))
	{
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

uint8_t property_new(
	void* properties,
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

#if defined(__STDC_LIB_EXT1__)
	if (0 != memcpy_s(the_property.name, UINT8_MAX, property_name, property_name_length))
	{
		return 0;
	}

#else
	memcpy(the_property.name, property_name, property_name_length);
#endif
	the_property.name[property_name_length] = '\0';
	the_property.name_length = property_name_length;

	if (!buffer_init((void*)the_property.value, BUFFER_SIZE_OF))
	{
		return 0;
	}

	the_property.dynamic = the_property.read_only = 0;

	if (!property_set_by_pointer(&the_property, value, value_length, type_of_value, dynamic,
								 read_only, verbose))
	{
		buffer_release(&the_property.value);
		return 0;
	}

	return buffer_append_property(properties, &the_property, 1);
}

uint8_t property_get_by_pointer(const void* the_property, void* output)
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

uint8_t property_exists(
	const void* properties, const uint8_t* name, uint8_t name_length, void** the_property)
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
				*the_property = prop;
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

uint8_t property_set_by_name(
	void* properties,
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

uint8_t property_set_by_pointer(
	void* the_property, const void* value, ptrdiff_t value_length,
	enum data_type type_of_value, uint8_t dynamic, uint8_t read_only, uint8_t verbose)
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
		return ATTEMPT_TO_WRITE_READ_ONLY_PROPERTY;
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
				if (!buffer_append(&prop->value, (const uint8_t*)value, value_length))
				{
					return 0;
				}

				break;

			case property_value_is_integer:
				if ((ptrdiff_t)sizeof(int64_t) <= value_length)
				{
					if (!int64_to_string(*((const int64_t*)value), &prop->value))
					{
						return 0;
					}
				}
				else if ((ptrdiff_t)sizeof(int32_t) <= value_length)
				{
					if (!int_to_string(*((const int32_t*)value), &prop->value))
					{
						return 0;
					}
				}
				else if ((ptrdiff_t)sizeof(int16_t) <= value_length)
				{
					if (!int_to_string(*((const int16_t*)value), &prop->value))
					{
						return 0;
					}
				}
				else
				{
					if (!int_to_string(*((const int8_t*)value), &prop->value))
					{
						return 0;
					}
				}

				break;

			case property_value_is_double:
				if ((ptrdiff_t)sizeof(double) <= value_length)
				{
					if (!double_to_string(*((const double*)value), &prop->value))
					{
						return 0;
					}
				}
				else
				{
					if (!double_to_string(*((const float*)value), &prop->value))
					{
						return 0;
					}
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
	void* the_property, const uint8_t* file_name, uint16_t encoding,
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

	if (!load_file(file_name, encoding, &prop->value, verbose))
	{
		return 0;
	}

	prop->dynamic = 0 < dynamic;
	prop->read_only = 0 < read_only;
	return 1;
}

uint8_t property_set_from_stream(
	void* the_property, void* stream, uint8_t dynamic, uint8_t read_only, uint8_t verbose)
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

uint8_t property_add_at_project(
	void* the_project, const void* properties, uint8_t verbose)
{
	if (NULL == the_project || NULL == properties)
	{
		return 0;
	}

	ptrdiff_t i = 0;
	const struct property* prop = NULL;
	static const uint8_t over_write = 1;

	while (NULL != (prop = buffer_property_data(properties, i++)))
	{
		const ptrdiff_t size = buffer_size(&prop->value);
		const void* value = size ? buffer_data(&prop->value, 0) : (const void*)prop;

		if (!project_property_set_value(the_project, prop->name, prop->name_length,
										(const uint8_t*)value, size, prop->dynamic, over_write,
										prop->read_only, verbose))
		{
			return 0;
		}
	}

	return 1;
}

void property_release_inner(void* properties)
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

void property_release(void* properties)
{
	if (NULL == properties)
	{
		return;
	}

	property_release_inner(properties);
	buffer_release(properties);
}
