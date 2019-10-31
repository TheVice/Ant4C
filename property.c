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
#include "interpreter.h"
#include "project.h"
#include "range.h"
#include "target.h"
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

uint8_t property_new(const void* project, const void* target,
					 struct buffer* properties,
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

	if (!property_set_by_pointer(project, target, &the_property, value, value_length, type_of_value, dynamic,
								 readonly, verbose))
	{
		buffer_release(&the_property.value);
		return 0;
	}

	return buffer_append_property(properties, &the_property, 1);
}

uint8_t property_get_pointer(const struct buffer* properties,
							 const uint8_t* property_name, uint8_t property_name_length,
							 void** the_property)
{
	ptrdiff_t i = 0;
	struct property* prop = NULL;

	if (NULL == properties ||
		!property_is_name_valid(property_name, property_name_length))
	{
		return 0;
	}

	while (NULL != (prop = buffer_property_data(properties, i++)))
	{
		if (property_name_length == prop->name_length &&
			0 == memcmp(&prop->name, property_name, property_name_length))
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

uint8_t property_exists(const struct buffer* properties, const uint8_t* name, uint8_t name_length)
{
	if (NULL == properties ||
		NULL == name ||
		0 == name_length)
	{
		return 0;
	}

	return property_get_pointer(properties, name, name_length, NULL);
}

uint8_t property_get_by_name(const void* project, const void* target,
							 const uint8_t* property_name,
							 uint8_t property_name_length,
							 struct buffer* output)
{
	if (NULL == output)
	{
		return 0;
	}

	void* the_property = NULL;

	if (!project_property_get_pointer(project, property_name, property_name_length, &the_property))
	{
		return 0;
	}

	return property_get_by_pointer(project, target, the_property, output);
}

uint8_t property_get_by_pointer(const void* project, const void* target,
								const void* the_property, struct buffer* output)
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

	if (prop->dynamic)
	{
		struct range code;
		code.start = buffer_data(&prop->value, 0);
		code.finish = code.start + buffer_size(&prop->value);

		if (!interpreter_evaluate_code(project, target, &code, output))
		{
			return 0;
		}
	}
	else
	{
		if (!buffer_append_data_from_buffer(output, &prop->value))
		{
			return 0;
		}
	}

	return 1;
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

uint8_t property_set_by_name(const void* project, const void* target,
							 struct buffer* properties,
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

	if (property_get_pointer(properties, name, name_length, (void**)&prop))
	{
		if (!overwrite)
		{
			return 1;
		}

		return property_set_by_pointer(project, target, prop,
									   value, value_length, type_of_value,
									   dynamic, readonly, verbose);
	}

	return property_new(project, target, properties, name, name_length,
						value, value_length, type_of_value,
						dynamic, readonly, verbose);
}

uint8_t property_set_by_pointer(const void* project, const void* target,
								void* the_property,
								const void* value, ptrdiff_t value_length,
								enum data_type type_of_value,
								uint8_t dynamic, uint8_t readonly, uint8_t verbose)
{
	if (NULL == the_property ||
		NULL == value ||
		(0 < value_length &&
		 property_value_is_byte_array != type_of_value &&
		 0 < dynamic))
	{
		return 0;
	}

	(void)verbose;/*TODO: */
	struct property* prop = (struct property*)the_property;

	if (prop->readonly)
	{
		return 0;
	}

	if (property_value_is_byte_array != type_of_value &&
		!buffer_resize(&prop->value, 0))
	{
		return 0;
	}

	if (0 < value_length)
	{
		switch (type_of_value)
		{
			case property_value_is_byte_array:
				if (dynamic)
				{
					if (!buffer_resize(&prop->value, 0) ||
						!buffer_append(&prop->value, value, value_length))
					{
						return 0;
					}
				}
				else if (value)
				{
					struct range code;
					code.start = value;
					code.finish = code.start + value_length;
					value_length = buffer_size(&prop->value);

					if (!interpreter_evaluate_code(project, target, &code, &prop->value))
					{
						return 0;
					}

					if (!value_length)
					{
						break;
					}

					uint8_t* dst = buffer_data(&prop->value, 0);
					uint8_t* src = buffer_data(&prop->value, value_length);
					value_length = buffer_size(&prop->value) - value_length;

					for (ptrdiff_t i = 0; i < value_length; ++i)
					{
						(*dst) = (*src);
						++dst;
						++src;
					}

#if 0
#if __STDC_SEC_API__

					if (0 != memcpy_s(buffer_data(&prop->value, 0),
									  buffer_size(&prop->value) - value_length,
									  buffer_data(&prop->value, value_length),
									  buffer_size(&prop->value) - value_length))
					{
						return 0;
					}

#else
					memcpy(buffer_data(&prop->value, 0),
						   buffer_data(&prop->value, value_length),
						   buffer_size(&prop->value) - value_length);
#endif
					/*NOTE: buffer_size(&prop->value) - value_length)*/
#endif

					if (!buffer_resize(&prop->value, value_length))
					{
						return 0;
					}
				}

				break;

			case property_value_is_integer:
				if (!int64_to_string(*((int64_t*)value), &prop->value))
				{
					return 0;
				}

				break;

			case property_value_is_double:
				if (!double_to_string(*((double*)value), &prop->value))
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

uint8_t property_set_from_xml_tag_record(
	const void* project, const void* target,
	struct buffer* properties,
	const uint8_t* record_start, const uint8_t* record_finish,
	uint8_t verbose)
{
	if (NULL == properties ||
		NULL == record_start || NULL == record_finish ||
		record_finish <= record_start)
	{
		return 0;
	}

	static const uint8_t* name_str = (const uint8_t*)"name";
	static const uint8_t name_length = 4;
	static const uint8_t* value_str = (const uint8_t*)"value";
	static const uint8_t value_length = 5;
	/**/
	static const uint8_t* property_attributes[] =
	{
		(const uint8_t*)"dynamic",
		(const uint8_t*)"overwrite",
		(const uint8_t*)"readonly",
		(const uint8_t*)"failonerror",
		(const uint8_t*)"verbose"
	};
	/**/
	static const uint8_t property_attributes_lengths[] = { 7, 9, 8, 11, 7 };
	/**/
	struct range property_name;
	struct range property_value;

	if (!xml_get_attribute_value(record_start, record_finish, name_str, name_length, &property_name))
	{
		return 0;
	}

	if (!xml_get_attribute_value(record_start, record_finish, value_str, value_length, &property_value))
	{
		return 0;
	}

	uint8_t dynamic = 0;
	uint8_t overwrite = 1;
	uint8_t readonly = 0;
	uint8_t fail_on_error = 1;
	uint8_t verbose_local = 0;
	/**/
	uint8_t* value_pointers[5];
	value_pointers[0] = &dynamic;
	value_pointers[1] = &overwrite;
	value_pointers[2] = &readonly;
	value_pointers[3] = &fail_on_error;
	value_pointers[4] = &verbose_local;

	for (uint8_t i = 0; i < 5; ++i)
	{
		struct range attribute_value;

		if (!xml_get_attribute_value(record_start, record_finish, property_attributes[i],
									 property_attributes_lengths[i], &attribute_value))
		{
			continue;
		}

		if (!bool_parse(attribute_value.start, range_size(&attribute_value), value_pointers[i]))
		{
			return 0;
		}
	}

	if (!verbose && verbose_local)
	{
		verbose = verbose_local;
	}

	return property_set_by_name(project, target, properties,
								property_name.start, (uint8_t)(range_size(&property_name)),
								property_value.start, range_size(&property_value),
								property_value_is_byte_array,
								dynamic, overwrite, readonly, verbose);
	/*TODO: explain fail_on_error factor, if verbose set, and return true.
	return fail_on_error ? returned : 1;*/
}

uint8_t property_append(const void* project, const void* target,
						struct buffer* target_properties,
						const struct buffer* properties, uint8_t verbose)
{
	if (NULL == target_properties || NULL == properties)
	{
		return 0;
	}

	ptrdiff_t i = 0;
	struct property* prop = NULL;

	while (NULL != (prop = buffer_property_data(properties, i++)))
	{
		const void* value = buffer_size(&prop->value) ? buffer_data(&prop->value, 0) : (void*)prop;

		if (!property_new(project, target, target_properties, prop->name, prop->name_length,
						  value, buffer_size(&prop->value),
						  property_value_is_byte_array, prop->dynamic, prop->readonly, verbose))
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

enum property_function
{
	property_exists_, property_get_value_,
	property_is_dynamic_, property_is_readonly_,
	UNKNOWN_PROPERTY
};

uint8_t property_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, property_str, UNKNOWN_PROPERTY);
}

uint8_t property_exec_function(const void* project, const void* target,
							   uint8_t function, const struct buffer* arguments,
							   uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_PROPERTY <= function ||
		NULL == arguments ||
		1 != arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_one_argument(arguments, &argument, 0))
	{
		return 0;
	}

	const ptrdiff_t length = range_size(&argument);

	if (length < 1)
	{
		return 0;
	}

	struct property* prop = NULL;

	const uint8_t is_exists = project_property_get_pointer(project, argument.start, (uint8_t)length,
							  (void**)&prop);

	if (function != property_exists_ && !is_exists)
	{
		return 0;
	}

	switch (function)
	{
		case property_exists_:
			return bool_to_string(is_exists, output);

		case property_get_value_:
			return is_exists && property_get_by_pointer(project, target, prop, output);

		case property_is_dynamic_:
			return NULL != prop && bool_to_string(prop->dynamic, output);

		case property_is_readonly_:
			return NULL != prop && bool_to_string(prop->readonly, output);

		case UNKNOWN_PROPERTY:
		default:
			break;
	}

	return 0;
}
