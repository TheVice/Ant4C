/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "date_time.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "math_unit.h"
#include "range.h"

#include <time.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#define CLOCK_T	((clock_t)-1)
#define TIME_T	((time_t)-1)

#define LOCAL_TIME(T)												\
	struct tm* tm_ = localtime((const time_t* const)(T));

#define LOCAL_TIME_STDC_SEC_API(T)									\
	struct tm tm__;													\
	struct tm* tm_ = &tm__;											\
	\
	if (0 != localtime_s(tm_, (const time_t* const)(T)))			\
	{																\
		return 0;													\
	}

#define UTC_TIME(T)													\
	struct tm* tm_u = gmtime((const time_t* const)(T));

#define UTC_TIME_STDC_SEC_API(T)									\
	struct tm tm__u;												\
	struct tm* tm_u = &tm__u;										\
	\
	if (0 != gmtime_s(tm_u, (const time_t* const)(T)))				\
	{																\
		return 0;													\
	}

static const int32_t seconds_per_day = 86400;
static const int16_t seconds_per_hour = 3600;
static const int8_t seconds_per_minute = 60;

static const uint8_t digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
#define COUNT_OF_DIGITS COUNT_OF(digits)

#define WEEK_DAY(A)	\
	((A) < 6) ? ((A) + 1) : 0;

int datetime_add_days(int week_day, uint16_t days)
{
	for (; days != 0; --days)
	{
		week_day = WEEK_DAY(week_day);
	}

	return week_day;
}

uint8_t datetime_decode_to_tm(int64_t time, struct tm* tm_)
{
	if (NULL == tm_)
	{
		return 0;
	}

	memset(tm_, 0, sizeof(struct tm));
	tm_->tm_year = 1970;
	tm_->tm_mday = 1;
	tm_->tm_wday = 4;
	tm_->tm_yday = 1;
	/**/
	int64_t this_time = 0;

	for (;;)
	{
		const uint16_t days_in_year = datetime_is_leap_year(tm_->tm_year) ? 366 : 365;
		const int64_t year = (int64_t)seconds_per_day * days_in_year;

		if (time < this_time + year)
		{
			break;
		}

		this_time += year;
		++tm_->tm_year;
		tm_->tm_wday = datetime_add_days(tm_->tm_wday, days_in_year);
	}

	for (;;)
	{
		const uint8_t days = datetime_get_days_in_month(tm_->tm_year, 1 + (uint8_t)tm_->tm_mon);
		const int64_t month = (int64_t)seconds_per_day * days;

		if (time < this_time + month)
		{
			break;
		}

		this_time += month;
		++tm_->tm_mon;
		tm_->tm_yday += days;
		tm_->tm_wday = datetime_add_days(tm_->tm_wday, days);
	}

	for (;;)
	{
		if (time < this_time + seconds_per_day)
		{
			break;
		}

		this_time += seconds_per_day;
		++tm_->tm_mday;
		++tm_->tm_yday;
		tm_->tm_wday = WEEK_DAY(tm_->tm_wday);
	}

	for (;;)
	{
		if (time < this_time + seconds_per_hour)
		{
			break;
		}

		this_time += seconds_per_hour;
		++tm_->tm_hour;
	}

	for (;;)
	{
		if (time < this_time + seconds_per_minute)
		{
			break;
		}

		this_time += seconds_per_minute;
		++tm_->tm_min;
	}

	this_time = (time - this_time);

	if (60 < this_time)
	{
		return 0;
	}

	tm_->tm_sec = (int)this_time;
	tm_->tm_mon += 1;
	return 1;
}

uint8_t datetime_format_to_string(int64_t input, const uint8_t* format, struct buffer* output)
{
	struct tm tm_;

	if (NULL == format || NULL == output || !datetime_decode_to_tm(input, &tm_))
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, INT8_MAX))
	{
		return 0;
	}

	tm_.tm_year -= 1900;
	tm_.tm_mon -= 1;
	const ptrdiff_t new_size = strftime((char*)buffer_data(output, size), INT8_MAX, (const char*)format, &tm_);

	if (!new_size)
	{
		return 0;
	}

	return buffer_resize(output, size + new_size);
}

uint8_t datetime_parse(const uint8_t* input_start, const uint8_t* input_finish,
					   uint32_t* year, uint8_t* month, uint8_t* day,
					   uint8_t* hour, uint8_t* minute, uint8_t* second)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == year || NULL == month || NULL == day ||
		NULL == hour || NULL == minute || NULL == second)
	{
		return 0;
	}

	uint8_t step = 0;
	uint8_t* ptr[6];
	ptr[0] = day;
	ptr[1] = month;
	ptr[2] = NULL;
	ptr[3] = hour;
	ptr[4] = minute;
	ptr[5] = second;

	while (input_start != input_finish && step < 6)
	{
		input_start = find_any_symbol_like_or_not_like_that(input_start, input_finish, digits, COUNT_OF_DIGITS, 1, 1);

		if (input_start == input_finish)
		{
			break;
		}

		const int32_t value = int_parse(input_start);

		if (2 == step)
		{
			(*year) = value;
		}
		else
		{
			(*ptr[step]) = (uint8_t)value;
		}

		++step;
		input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, digits, COUNT_OF_DIGITS, 0,
					  1);
	}

	return (0 < *day && *day < 32 && 1 < *month && *month < 31 && *hour < 24 && *minute < 60 && *second < 60);
}

uint8_t datetime_parse_buffer(struct buffer* input_output)
{
	const ptrdiff_t size = buffer_size(input_output);

	if (!size)
	{
		return 0;
	}

	if (!buffer_append(input_output, NULL, sizeof(uint32_t) + 5 * sizeof(uint8_t)))
	{
		return 0;
	}

	const uint8_t* ptr = buffer_data(input_output, 0);
	uint32_t* year = (uint32_t*)buffer_data(input_output, size);
	uint8_t* month = (uint8_t*)buffer_data(input_output, size + sizeof(uint32_t));
	uint8_t* day = month + sizeof(uint8_t);
	uint8_t* hour = day + sizeof(uint8_t);
	uint8_t* minute = hour + sizeof(uint8_t);
	uint8_t* second = minute + sizeof(uint8_t);

	if (!datetime_parse(ptr, ptr + size, year, month, day, hour, minute, second))
	{
		return 0;
	}

	*((int64_t*)year) = datetime_encode(*year, *month, *day, *hour, *minute, *second);
	return buffer_resize(input_output, size + sizeof(int64_t));
}

uint8_t datetime_to_char_array(const struct tm* tm_, char* output)
{
	if (NULL == tm_ || NULL == output)
	{
		return 0;
	}

	uint8_t length = 0;
#if __STDC_LIB_EXT1__
	length += (uint8_t)sprintf_s(output + length, 75 - length, 9 < tm_->tm_mday ? "%i." : "0%i.", tm_->tm_mday);
	length += (uint8_t)sprintf_s(output + length, 75 - length, 9 < tm_->tm_mon ? "%i." : "0%i.", tm_->tm_mon);
	length += (uint8_t)sprintf_s(output + length, 75 - length, "%i ", tm_->tm_year);
	/**/
	length += (uint8_t)sprintf_s(output + length, 75 - length, "%i:", tm_->tm_hour);
	length += (uint8_t)sprintf_s(output + length, 75 - length, 9 < tm_->tm_min ? "%i:" : "0%i:", tm_->tm_min);
	length += (uint8_t)sprintf_s(output + length, 75 - length, 9 < tm_->tm_sec ? "%i" : "0%i", tm_->tm_sec);
#else
	length += (uint8_t)sprintf(output + length, 9 < tm_->tm_mday ? "%i." : "0%i.", tm_->tm_mday);
	length += (uint8_t)sprintf(output + length, 9 < tm_->tm_mon ? "%i." : "0%i.", tm_->tm_mon);
	length += (uint8_t)sprintf(output + length, "%i ", tm_->tm_year);
	/**/
	length += (uint8_t)sprintf(output + length, "%i:", tm_->tm_hour);
	length += (uint8_t)sprintf(output + length, 9 < tm_->tm_min ? "%i:" : "0%i:", tm_->tm_min);
	length += (uint8_t)sprintf(output + length, 9 < tm_->tm_sec ? "%i" : "0%i", tm_->tm_sec);
#endif
	return length;
}

uint8_t datetime_to_string(uint32_t year, uint8_t month, uint8_t day,
						   uint8_t hour, uint8_t minute, uint8_t second,
						   struct buffer* output)
{
	if (year < 1970 || INT32_MAX < year ||
		month < 1 || 12 < month ||
		day < 1 || 31 < day ||
		23 < hour || 59 < minute ||
		60 < second ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 75))
	{
		return 0;
	}

	struct tm tm_;

	tm_.tm_year = year;

	tm_.tm_mon = month;

	tm_.tm_mday = day;

	tm_.tm_hour = hour;

	tm_.tm_min = minute;

	tm_.tm_sec = second;

	char* ptr = (char*)buffer_data(output, size);

	return buffer_resize(output, size + datetime_to_char_array(&tm_, ptr));
}

uint8_t datetime_get_day(int64_t input)
{
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return 0;
	}

	return (uint8_t)tm_.tm_mday;
}

uint8_t datetime_get_day_of_week(int64_t input)
{
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return 0;
	}

	return (uint8_t)tm_.tm_wday;
}

uint16_t datetime_get_day_of_year(int64_t input)
{
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return UINT16_MAX;
	}

	return (uint16_t)tm_.tm_yday;
}

uint8_t datetime_get_days_in_month(uint32_t year, uint8_t month)
{
	if (month < 1 || 12 < month)
	{
		return 0;
	}

	if (2 == month)
	{
		return datetime_is_leap_year(year) ? 29 : 28;
	}

	return (4 == month || 6 == month || 9 == month || 11 == month) ? 30 : 31;
}

uint8_t datetime_get_hour(int64_t input)
{
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return UINT8_MAX;
	}

	return (uint8_t)tm_.tm_hour;
}
/*uint16_t datetime_get_millisecond(int64_t input)*/
uint8_t datetime_get_minute(int64_t input)
{
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return UINT8_MAX;
	}

	return (uint8_t)tm_.tm_min;
}

uint8_t datetime_get_month(int64_t input)
{
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return UINT8_MAX;
	}

	return (uint8_t)tm_.tm_mon;
}

uint8_t datetime_get_second(int64_t input)
{
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return UINT8_MAX;
	}

	return (uint8_t)tm_.tm_sec;
}
/*int64_t datetime_get_ticks(int64_t input);*/
uint32_t datetime_get_year(int64_t input)
{
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return UINT8_MAX;
	}

	return (uint32_t)tm_.tm_year;
}

uint8_t datetime_is_leap_year(uint32_t year)
{
	return ((!(year % 400)) || ((year % 100) && (!(year % 4))));
}

int64_t datetime_ticks()
{
	const clock_t ticks = clock();

	if (CLOCK_T == ticks)
	{
		return 0;
	}

	return ticks;
}

int64_t datetime_now_utc()
{
	time_t now = 0;

	if (TIME_T == time(&now))
	{
		return 0;
	}

#if __STDC_LIB_EXT1__
	UTC_TIME_STDC_SEC_API(&now);
#else
	UTC_TIME(&now);
#endif
	return datetime_encode(1900 + tm_u->tm_year, 1 + (uint8_t)tm_u->tm_mon, (uint8_t)tm_u->tm_mday,
						   (uint8_t)tm_u->tm_hour, (uint8_t)tm_u->tm_min, (uint8_t)tm_u->tm_sec);
}

int64_t datetime_now()
{
	time_t now = 0;

	if (TIME_T == time(&now))
	{
		return 0;
	}

#if __STDC_LIB_EXT1__
	LOCAL_TIME_STDC_SEC_API(&now);
#else
	LOCAL_TIME(&now);
#endif
	return datetime_encode(1900 + tm_->tm_year, 1 + (uint8_t)tm_->tm_mon, (uint8_t)tm_->tm_mday,
						   (uint8_t)tm_->tm_hour, (uint8_t)tm_->tm_min, (uint8_t)tm_->tm_sec);
}

int64_t datetime_encode(uint32_t year, uint8_t month, uint8_t day,
						uint8_t hour, uint8_t minute, uint8_t second)
{
	if (year < 1970 || INT32_MAX < year ||
		month < 1 || 12 < month ||
		day < 1 || 31 < day ||
		23 < hour || 59 < minute ||
		60 < second)
	{
		return 0;
	}

	int64_t this_time = 0;

	for (uint32_t y = 1970; y < year; ++y)
	{
		this_time += (int64_t)seconds_per_day * (datetime_is_leap_year(y) ? 366 : 365);
	}

	for (uint8_t m = 1; m < month; ++m)
	{
		uint8_t days = datetime_get_days_in_month(year, m);
		this_time += (int64_t)seconds_per_day * days;
	}

	for (uint8_t d = 1; d < day; ++d)
	{
		this_time += seconds_per_day;
	}

	for (uint8_t h = 0; h < hour; ++h)
	{
		this_time += seconds_per_hour;
	}

	for (uint8_t m = 0; m < minute; ++m)
	{
		this_time += seconds_per_minute;
	}

	this_time += second;
	return this_time;
}

uint8_t datetime_decode(int64_t time, uint32_t* year, uint8_t* month, uint8_t* day,
						uint8_t* hour, uint8_t* minute, uint8_t* second, uint16_t* year_day)
{
	if (NULL == year && NULL == month && NULL == day &&
		NULL == hour && NULL == minute && NULL == second &&
		NULL == year_day)
	{
		return 0;
	}

	struct tm tm_;

	if (!datetime_decode_to_tm(time, &tm_))
	{
		return 0;
	}

	if (year)
	{
		(*year) = tm_.tm_year;
	}

	if (month)
	{
		(*month) = (uint8_t)tm_.tm_mon;
	}

	if (day)
	{
		(*day) = (uint8_t)tm_.tm_mday;
	}

	if (hour)
	{
		(*hour) = (uint8_t)tm_.tm_hour;
	}

	if (minute)
	{
		(*minute) = (uint8_t)tm_.tm_min;
	}

	if (second)
	{
		(*second) = (uint8_t)tm_.tm_sec;
	}

	if (year_day)
	{
		(*year_day) = (uint16_t)tm_.tm_yday;
	}

	return 1;
}
#if !defined(_WIN32)
long datetime_get_bias()
{
	time_t now = 0;

	if (TIME_T == time(&now))
	{
		return 0;
	}

#if __STDC_LIB_EXT1__
	LOCAL_TIME_STDC_SEC_API(&now);
#else
	LOCAL_TIME(&now);
#endif
#if __STDC_LIB_EXT1__
	UTC_TIME_STDC_SEC_API(&now);
#else
	UTC_TIME(&now);
#endif
	return (long)(mktime(tm_u) - mktime(tm_)) / 60 - (tm_u->tm_isdst ? 0 : 60);
}
#endif
int64_t date_time_millisecond_to_second(int64_t millisecond)
{
	return math_truncate(math_ceiling((double)millisecond / 1000));
}

int64_t timespan_from_days(double input)
{
	return (int64_t)((double)seconds_per_day * input);
}

int64_t timespan_from_hours(double input)
{
	return (int64_t)((double)seconds_per_hour * input);
}

int64_t timespan_from_milliseconds(double input)
{
	return (int64_t)(input / 1000);
}

int64_t timespan_from_minutes(double input)
{
	return (int64_t)((double)seconds_per_minute * input);
}

int64_t timespan_from_seconds(double input)
{
	return (int64_t)input;
}

int64_t timespan_from_ticks(int64_t ticks)
{
	if (ticks < CLOCKS_PER_SEC)
	{
		return 0;
	}

	return (int64_t)(((double)ticks) / CLOCKS_PER_SEC);
}

int32_t timespan_get_days(int64_t input)
{
	int32_t days = 0;

	for (int64_t this_time = seconds_per_day; this_time <= input; ++days, this_time += seconds_per_day);

	return days;
}

int32_t timespan_get_hours(int64_t input)
{
	int32_t hours = 0;

	for (int64_t this_time = seconds_per_hour; this_time <= input; ++hours, this_time += seconds_per_hour);

	return hours;
}
/*timespan_get_milliseconds*/
int32_t timespan_get_minutes(int64_t input)
{
	int32_t minutes = 0;

	for (int64_t this_time = seconds_per_minute; this_time <= input; ++minutes, this_time += seconds_per_minute);

	return minutes;
}

int64_t timespan_get_ticks(int64_t input)
{
	return CLOCKS_PER_SEC * input;
}

double timespan_get_total_days(int64_t input)
{
	return (double)input / seconds_per_day;
}

double timespan_get_total_hours(int64_t input)
{
	return (double)input / seconds_per_hour;
}

int64_t timespan_get_total_milliseconds(int64_t input)
{
	return 1000 * input;
}

double timespan_get_total_minutes(int64_t input)
{
	return (double)input / seconds_per_minute;
}

static const uint8_t* datetime_function_str[] =
{
	(const uint8_t*)"format-to-string",
	(const uint8_t*)"parse",
	(const uint8_t*)"to-string",
	(const uint8_t*)"get-day",
	(const uint8_t*)"get-day-of-week",
	(const uint8_t*)"get-day-of-year",
	(const uint8_t*)"get-days-in-month",
	(const uint8_t*)"get-hour",
	(const uint8_t*)"get-millisecond",
	(const uint8_t*)"get-minute",
	(const uint8_t*)"get-month",
	(const uint8_t*)"get-second",
	(const uint8_t*)"get-ticks",
	(const uint8_t*)"get-year",
	(const uint8_t*)"is-leap-year",
	(const uint8_t*)"ticks",
	(const uint8_t*)"now-utc",
	(const uint8_t*)"now",
	(const uint8_t*)"from-input"
};

enum datetime_function
{
	format_to_string, parse, to_string,	get_day, get_day_of_week,
	get_day_of_year, get_days_in_month, get_hour, get_millisecond,
	get_minute, get_month, get_second, get_ticks, get_year,
	is_leap_year, ticks, now_utc, now, encode,
	UNKNOWN_DATETIME_FUNCTION
};

uint8_t datetime_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, datetime_function_str, UNKNOWN_DATETIME_FUNCTION);
}

uint8_t datetime_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							   struct buffer* output)
{
	if (UNKNOWN_DATETIME_FUNCTION <= function || NULL == arguments || 2 < arguments_count || NULL == output)
	{
		return 0;
	}

	struct range values[2];

	if (arguments_count && !common_get_arguments(arguments, arguments_count, values, 1))
	{
		return 0;
	}

	switch (function)
	{
		case format_to_string:
			return 2 == arguments_count &&
				   datetime_format_to_string(int64_parse(values[0].start), values[1].start, output);

		case parse:
		case to_string:
		{
			uint32_t year = 0;
			uint8_t month = 0;
			uint8_t day = 0;
			uint8_t hour = 0;
			uint8_t minute = 0;
			uint8_t second = 0;

			if (1 != arguments_count ||
				!datetime_parse(values[0].start, values[0].finish, &year, &month, &day, &hour, &minute, &second))
			{
				break;
			}

			return datetime_to_string(year, month, day, hour, minute, second, output);
		}

		case get_day:
			return 1 == arguments_count && int_to_string(datetime_get_day(int64_parse(values[0].start)), output);

		case get_day_of_week:
			return 1 == arguments_count && int_to_string(datetime_get_day_of_week(int64_parse(values[0].start)), output);

		case get_day_of_year:
			return 1 == arguments_count && int_to_string(datetime_get_day_of_year(int64_parse(values[0].start)), output);

		case get_days_in_month:
			return 2 == arguments_count &&
				   int_to_string(datetime_get_days_in_month(int_parse(values[0].start), (uint8_t)int_parse(values[1].start)),
								 output);

		case get_hour:
			return 1 == arguments_count && int_to_string(datetime_get_hour(int64_parse(values[0].start)), output);

		case get_millisecond:
			/*TODO:*/
			break;

		case get_minute:
			return 1 == arguments_count && int_to_string(datetime_get_minute(int64_parse(values[0].start)), output);

		case get_month:
			return 1 == arguments_count && int_to_string(datetime_get_month(int64_parse(values[0].start)), output);

		case get_second:
			return 1 == arguments_count && int_to_string(datetime_get_second(int64_parse(values[0].start)), output);

		case get_ticks:
			/*TODO:*/
			break;

		case get_year:
			return 1 == arguments_count && int_to_string(datetime_get_year(int64_parse(values[0].start)), output);

		case is_leap_year:
			return 1 == arguments_count && bool_to_string(datetime_is_leap_year(int_parse(values[0].start)), output);

		case ticks:
			if (!arguments_count)
			{
				const int64_t ticks_ = datetime_ticks();
				return /*0 < ticks_ && */int64_to_string(0 < ticks_ ? ticks_ : 0, output);
			}

			break;

		case now_utc:
			if (!arguments_count)
			{
				const int64_t time_is_now = datetime_now_utc();
				return 0 < time_is_now && int64_to_string(time_is_now, output);
			}

			break;

		case now:
			if (!arguments_count)
			{
				const int64_t time_is_now = datetime_now();
				return 0 < time_is_now && int64_to_string(time_is_now, output);
			}

			break;

		case encode:
		{
			uint32_t year = 0;
			uint8_t month = 0;
			uint8_t day = 0;
			uint8_t hour = 0;
			uint8_t minute = 0;
			uint8_t second = 0;

			if (1 != arguments_count ||
				!datetime_parse(values[0].start, values[0].finish, &year, &month, &day, &hour, &minute, &second))
			{
				break;
			}

			return int64_to_string(datetime_encode(year, month, day, hour, minute, second), output);
		}

		case UNKNOWN_DATETIME_FUNCTION:
		default:
			break;
	}

	return 0;
}

static const uint8_t* timespan_function_str[] =
{
	(const uint8_t*)"parse",
	(const uint8_t*)"to-string",
	(const uint8_t*)"from-days",
	(const uint8_t*)"from-hours",
	(const uint8_t*)"from-milliseconds",
	(const uint8_t*)"from-minutes",
	(const uint8_t*)"from-seconds",
	(const uint8_t*)"from-ticks",
	(const uint8_t*)"get-days",
	(const uint8_t*)"get-hours",
	(const uint8_t*)"get-milliseconds",
	(const uint8_t*)"get-minutes",
	(const uint8_t*)"get-seconds",
	(const uint8_t*)"get-ticks",
	(const uint8_t*)"get-total-days",
	(const uint8_t*)"get-total-hours",
	(const uint8_t*)"get-total-milliseconds",
	(const uint8_t*)"get-total-minutes",
	(const uint8_t*)"get-total-seconds"
};

enum timespan_function
{
	ts_parse_, ts_to_string_, ts_from_days_, ts_from_hours_,
	ts_from_milliseconds_, ts_from_minutes_, ts_from_seconds_,
	ts_from_ticks_, ts_get_days_, ts_get_hours_,
	ts_get_milliseconds_, ts_get_minutes_, ts_get_seconds_,
	ts_get_ticks_, ts_get_total_days_, ts_get_total_hours_,
	ts_get_total_milliseconds_, ts_get_total_minutes_,
	ts_get_total_seconds_, UNKNOWN_TIMESPAN_FUNCTION
};

uint8_t timespan_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, timespan_function_str, UNKNOWN_TIMESPAN_FUNCTION);
}

uint8_t timespan_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							   struct buffer* output)
{
	if (UNKNOWN_TIMESPAN_FUNCTION <= function || NULL == arguments || 1 != arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_arguments(arguments, arguments_count, &argument, 1))
	{
		return 0;
	}

	switch (function)
	{
		case ts_from_days_:
			return int64_to_string(timespan_from_days(double_parse(argument.start)), output);

		case ts_from_hours_:
			return int64_to_string(timespan_from_hours(double_parse(argument.start)), output);

		case ts_from_milliseconds_:
			return int64_to_string(timespan_from_milliseconds(double_parse(argument.start)), output);

		case ts_from_minutes_:
			return int64_to_string(timespan_from_minutes(double_parse(argument.start)), output);

		case ts_from_seconds_:
			return int64_to_string(int64_parse(argument.start), output);

		case ts_from_ticks_:
			return int64_to_string(timespan_from_ticks(int64_parse(argument.start)), output);

		case ts_get_days_:
			return int_to_string(timespan_get_days(int64_parse(argument.start)), output);

		case ts_get_hours_:
			return int_to_string(timespan_get_hours(int64_parse(argument.start)), output);

		case ts_get_milliseconds_:
			/*TODO:*/
			break;

		case ts_get_minutes_:
			return int_to_string(timespan_get_minutes(int64_parse(argument.start)), output);

		case ts_parse_:
		case ts_to_string_:
		case ts_get_seconds_:
		case ts_get_total_seconds_:
			return int64_to_string(int64_parse(argument.start), output);

		case ts_get_ticks_:
			return int64_to_string(timespan_get_ticks(int64_parse(argument.start)), output);

		case ts_get_total_days_:
			return double_to_string(timespan_get_total_days(int64_parse(argument.start)), output);

		case ts_get_total_hours_:
			return double_to_string(timespan_get_total_hours(int64_parse(argument.start)), output);

		case ts_get_total_milliseconds_:
			return int64_to_string(timespan_get_total_milliseconds(int64_parse(argument.start)), output);

		case ts_get_total_minutes_:
			return double_to_string(timespan_get_total_minutes(int64_parse(argument.start)), output);

		case UNKNOWN_TIMESPAN_FUNCTION:
		default:
			break;
	}

	return 0;
}
