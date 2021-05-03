/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#include "example_of_the_module.h"
#include "buffer.h"

static struct buffer output_data;
static uint8_t is_buffer_initialized = 0;

static const uint8_t* tasks[] =
{
	(const uint8_t*)"task_without_attributes",
	(const uint8_t*)"this_task_with_attributes"
};

static const uint8_t* name_spaces[] =
{
	(const uint8_t*)"name_space_number_one",
	(const uint8_t*)"name_space_number_two"
};

static const uint8_t* all_functions[][3] =
{
	{ (const uint8_t*)"function", NULL },
	{ (const uint8_t*)"this_is_the_function_number_one", (const uint8_t*)"and_this_is_the_function_number_two", NULL }
};

static const uint8_t* tasks_attributes[][3] =
{
	{ NULL },
	{ (const uint8_t*)"task_first_attribute", (const uint8_t*)"task_second_attribute", NULL }
};

const uint8_t tasks_attributes_lengths[][3] =
{
	{ 0 },
	{ 20, 21, 0 }
};

static const uint8_t tasks_attributes_count[] =
{
	0, 2
};

const uint8_t* enumerate_tasks(ptrdiff_t index)
{
	static const ptrdiff_t count = sizeof(tasks) / sizeof(*tasks);

	if (index < 0 || count <= index)
	{
		return NULL;
	}

	return tasks[index];
}

const uint8_t* enumerate_name_spaces(ptrdiff_t index)
{
	static const ptrdiff_t count = sizeof(name_spaces) / sizeof(*name_spaces);

	if (index < 0 || count <= index)
	{
		return NULL;
	}

	return name_spaces[index];
}

const uint8_t* enumerate_functions(const uint8_t* name_space, ptrdiff_t index)
{
	static const ptrdiff_t count = sizeof(all_functions) / sizeof(*all_functions);

	if (NULL == name_space || index < 0)
	{
		return NULL;
	}

	ptrdiff_t i = 0;
	const uint8_t* ptr = NULL;

	while (NULL != (ptr = enumerate_name_spaces(i++)))
	{
		if (ptr == name_space)
		{
			break;
		}
	}

	if (NULL == ptr || (count <= (i - 1)))
	{
		return NULL;
	}

	const uint8_t** function_from_name_space = all_functions[i - 1];
	i = 0;

	while (NULL != (ptr = function_from_name_space[i++]))
	{
		if (index == (i - 1))
		{
			return ptr;
		}
	}

	return NULL;
}

uint8_t get_attributes_and_arguments_for_task(
	const uint8_t* task, const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count)
{
	if (NULL == task ||
		NULL == task_attributes ||
		NULL == task_attributes_lengths ||
		NULL == task_attributes_count)
	{
		return 0;
	}

	static const ptrdiff_t count = sizeof(tasks) / sizeof(*tasks);
	ptrdiff_t index = 0;

	for (; index < count; ++index)
	{
		if (task == tasks[index])
		{
			break;
		}
	}

	if (count == index)
	{
		return 0;
	}

	*task_attributes = tasks_attributes[index];
	*task_attributes_lengths = tasks_attributes_lengths[index];
	*task_attributes_count = tasks_attributes_count[index];
	/**/
	return 1;
}

uint8_t task_number_one(const uint8_t** arguments, const uint8_t** output, uint16_t* output_length)
{
	const uint8_t* start = arguments[tasks_attributes_count[0]];
	const uint8_t* finish = arguments[tasks_attributes_count[0] + 1];
	const ptrdiff_t length = finish - start;

	if (length < 0)
	{
		return 0;
	}

	if (!is_buffer_initialized)
	{
		module_release();
	}

	if (!buffer_resize(&output_data, 0))
	{
		return 0;
	}

	if (!buffer_append_char(
			&output_data, "<property name=\"content_of_the_task_from_script\" value=\"<![CDATA[", 65))
	{
		return 0;
	}

	if (!buffer_append(&output_data, start, length))
	{
		return 0;
	}

	if (!buffer_append_char(&output_data, "]]>\" />", 7))
	{
		return 0;
	}

	*output = buffer_data(&output_data, 0);
	*output_length = (uint16_t)buffer_size(&output_data);
	/**/
	return 1;
}

uint8_t task_number_two(const uint8_t** arguments, const uint16_t* arguments_lengths,
						const uint8_t** output, uint16_t* output_length)
{
	if (!is_buffer_initialized)
	{
		module_release();
	}

	if (!buffer_resize(&output_data, 0))
	{
		return 0;
	}

	if (!buffer_append_char(
			&output_data, "<property name=\"property_from_the_task_number_two\" value=\"", 58))
	{
		return 0;
	}

	for (uint8_t i = 0, count = tasks_attributes_count[1]; i < count; ++i)
	{
		if (!buffer_append(&output_data, arguments[i], arguments_lengths[i]))
		{
			return 0;
		}
	}

	if (!buffer_append_char(&output_data, "\" />", 4))
	{
		return 0;
	}

	*output = buffer_data(&output_data, 0);
	*output_length = (uint16_t)buffer_size(&output_data);
	/**/
	return 1;
}

uint8_t evaluate_task(const uint8_t* task,
					  const uint8_t** arguments, const uint16_t* arguments_lengths, uint8_t arguments_count,
					  const uint8_t** output, uint16_t* output_length,
					  uint8_t verbose)
{
	(void)verbose;

	if (NULL == task ||
		NULL == arguments ||
		NULL == arguments_lengths)
	{
		return 0;
	}

	static const ptrdiff_t count = sizeof(tasks) / sizeof(*tasks);
	ptrdiff_t index = 0;

	for (; index < count; ++index)
	{
		if (task == tasks[index])
		{
			break;
		}
	}

	if (count == index ||
		arguments_count != tasks_attributes_count[index])
	{
		return 0;
	}

	switch (index)
	{
		case 0:
			return task_number_one(arguments, output, output_length);
			break;

		case 1:
			return task_number_two(arguments, arguments_lengths, output, output_length);

		default:
			break;
	}

	return 0;
}

uint8_t the_function(const uint8_t** output, uint16_t* output_length)
{
	if (!is_buffer_initialized)
	{
		module_release();
	}

	if (!buffer_resize(&output_data, 0))
	{
		return 0;
	}

	if (!buffer_append_char(&output_data, "You call function with out arguments.", 37))
	{
		return 0;
	}

	*output = buffer_data(&output_data, 0);
	*output_length = (uint16_t)buffer_size(&output_data);
	/**/
	return 1;
}

uint8_t this_is_the_function_number_one(
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count, const uint8_t** output, uint16_t* output_length)
{
	if (!is_buffer_initialized)
	{
		module_release();
	}

	if (!buffer_resize(&output_data, 0))
	{
		return 0;
	}

	if (!buffer_append_char(&output_data, "You call function with variable count of the arguments. ", 56))
	{
		return 0;
	}

	for (uint8_t i = 0; i < values_count; ++i)
	{
		if (!buffer_append(&output_data, values[i], values_lengths[i]))
		{
			return 0;
		}
	}

	*output = buffer_data(&output_data, 0);
	*output_length = (uint16_t)buffer_size(&output_data);
	/**/
	return 1;
}

uint8_t and_this_is_the_function_number_two(const uint8_t** values, const uint16_t* values_lengths,
		uint8_t values_count, const uint8_t** output, uint16_t* output_length)
{
	if (2 != values_count)
	{
		return 0;
	}

	if (!is_buffer_initialized)
	{
		module_release();
	}

	if (!buffer_resize(&output_data, 0))
	{
		return 0;
	}

	if (!buffer_append_char(&output_data, "You call function with two arguments. ", 38))
	{
		return 0;
	}

	for (uint8_t i = 0; i < values_count; ++i)
	{
		if (!buffer_append(&output_data, values[i], values_lengths[i]))
		{
			return 0;
		}
	}

	*output = buffer_data(&output_data, 0);
	*output_length = (uint16_t)buffer_size(&output_data);
	/**/
	return 1;
}

uint8_t evaluate_function(const uint8_t* function,
						  const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
						  const uint8_t** output, uint16_t* output_length)
{
	if (NULL == function ||
		NULL == values ||
		NULL == values_lengths ||
		NULL == output ||
		NULL == output_length)
	{
		return 0;
	}

	const uint8_t* ptr = NULL;
	ptrdiff_t function_number = 0;

	for (ptrdiff_t i = 0, count = sizeof(all_functions) / sizeof(*all_functions); i < count; ++i)
	{
		ptrdiff_t j = 0;
		const uint8_t** functions_from_name_space = all_functions[i];

		while (NULL != (ptr = functions_from_name_space[j++]))
		{
			if (function == ptr)
			{
				i = count;
				break;
			}

			++function_number;
		}
	}

	if (NULL == ptr)
	{
		return 0;
	}

	switch (function_number)
	{
		case 0:
			return the_function(output, output_length);

		case 1:
			return this_is_the_function_number_one(values, values_lengths, values_count, output, output_length);

		case 2:
			return and_this_is_the_function_number_two(values, values_lengths, values_count, output, output_length);

		default:
			break;
	}

	return 0;
}

void module_release()
{
	if (is_buffer_initialized)
	{
		buffer_release(&output_data);
	}
	else
	{
		SET_NULL_TO_BUFFER(output_data);
		is_buffer_initialized = 1;
	}
}
