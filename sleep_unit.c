/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "sleep_unit.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

uint8_t sleep_for(uint32_t milliseconds)
{
#if defined(_WIN32)
	/*Sleep(milliseconds);
	return 1;*/
	return 0 == SleepEx(milliseconds, FALSE);
#else
	return 0 == sleep(date_time_millisecond_to_second(milliseconds));
#endif
}

#define HOURS_POSITION			0
#define MILLISECONDS_POSITION	1
#define MINUTES_POSITION		2
#define SECONDS_POSITION		3

#define MAX_POSITION			(SECONDS_POSITION + 2)

static const uint8_t* attributes[] =
{
	(const uint8_t*)"hours",
	(const uint8_t*)"milliseconds",
	(const uint8_t*)"minutes",
	(const uint8_t*)"seconds"
};

static const uint8_t attributes_lengths[] = { 5, 12, 7, 7 };

uint8_t sleep_unit_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
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
	uint32_t milliseconds = 0;
	struct buffer* ptr = NULL;

	while (NULL != (ptr = buffer_buffer_data(task_arguments, counter++)))
	{
		if (buffer_size(ptr))
		{
			if (!buffer_push_back(ptr, 0))
			{
				return 0;
			}

			switch (counter - 1)
			{
				case HOURS_POSITION	:
					milliseconds += 1000 * (uint32_t)timespan_from_hours(int_parse(buffer_data(ptr, 0)));
					break;

				case MILLISECONDS_POSITION:
					milliseconds += int_parse(buffer_data(ptr, 0));
					break;

				case MINUTES_POSITION:
					milliseconds += 1000 * (uint32_t)timespan_from_minutes(int_parse(buffer_data(ptr, 0)));
					break;

				case SECONDS_POSITION:
					milliseconds += int_parse(buffer_data(ptr, 0));
					break;

				default:
					break;
			}
		}
	}

	if (MAX_POSITION != counter)
	{
		return 0;
	}

	return sleep_for(milliseconds);
}
