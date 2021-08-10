/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "sleep_unit.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"

#define HOURS_POSITION			0
#define MILLISECONDS_POSITION	1
#define MINUTES_POSITION		2
#define SECONDS_POSITION		3

#define MAX_POSITION			(SECONDS_POSITION + 2)

uint8_t sleep_unit_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	static const uint8_t* attributes[] =
	{
		(const uint8_t*)"hours",
		(const uint8_t*)"milliseconds",
		(const uint8_t*)"minutes",
		(const uint8_t*)"seconds"
	};
	/**/
	static const uint8_t attributes_lengths[] = { 5, 12, 7, 7 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   attributes, attributes_lengths,
			   COUNT_OF(attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t sleep_unit_evaluate_task(struct buffer* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == task_arguments)
	{
		return 0;
	}

	uint8_t counter = 0;
	uint64_t milliseconds = 0;
	struct buffer* argument = NULL;

	while (NULL != (argument = buffer_buffer_data(task_arguments, counter++)))
	{
		if (!buffer_size(argument))
		{
			continue;
		}

		const uint8_t* start = buffer_data(argument, 0);
		const uint8_t* finish = start + buffer_size(argument);
		const uint64_t argument_value = int64_parse(start, finish);

		switch (counter - 1)
		{
			case HOURS_POSITION:
				milliseconds += 1000 * timespan_from_hours((double)argument_value);
				break;

			case MILLISECONDS_POSITION:
				milliseconds += argument_value;
				break;

			case MINUTES_POSITION:
				milliseconds += 1000 * timespan_from_minutes((double)argument_value);
				break;

			case SECONDS_POSITION:
				milliseconds += argument_value;
				break;

			default:
				break;
		}
	}

	if (MAX_POSITION != counter)
	{
		return 0;
	}

	return sleep_for((uint32_t)milliseconds);
}
