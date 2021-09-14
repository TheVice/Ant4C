/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2021 TheVice
 *
 */

#include "sleep_unit.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include "date_time.h"

#include <unistd.h>
#endif

uint8_t sleep_for(uint32_t milliseconds)
{
#if defined(_WIN32)
	return 0 == SleepEx(milliseconds, FALSE);
#else
	return 0 == sleep(date_time_millisecond_to_second(milliseconds));
#endif
}
