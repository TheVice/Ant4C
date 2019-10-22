/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "date_time.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"

#include <time.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

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

static const int32_t seconds_per_day = 86400;
static const int16_t seconds_per_hour = 3600;
static const int8_t seconds_per_minute = 60;

uint8_t datetime_decode_to_tm(int64_t time, struct tm* tm_)
{
	if (NULL == tm_)
	{
		return 0;
	}

	memset(tm_, 0, sizeof(struct tm));
	tm_->tm_year = 1970;
	tm_->tm_mday = 1;
	tm_->tm_yday = 1;
	/**/
	int64_t this_time = 0;

	while (1)
	{
		int64_t year = (int64_t)seconds_per_day * (datetime_is_leap_year(tm_->tm_year) ? 366 : 365);

		if (time < this_time + year)
		{
			break;
		}

		this_time += year;
		++tm_->tm_year;
	}

	while (1)
	{
		uint8_t days = datetime_get_days_in_month(tm_->tm_year, 1 + (uint8_t)tm_->tm_mon);
		int64_t month = (int64_t)seconds_per_day * days;

		if (time < this_time + month)
		{
			break;
		}

		this_time += month;
		++tm_->tm_mon;
		tm_->tm_yday += days;
	}

	while (1)
	{
		if (time < this_time + seconds_per_day)
		{
			break;
		}

		this_time += seconds_per_day;
		++tm_->tm_mday;
		++tm_->tm_yday;
	}

	while (1)
	{
		if (time < this_time + seconds_per_hour)
		{
			break;
		}

		this_time += seconds_per_hour;
		++tm_->tm_hour;
	}

	while (1)
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

uint8_t datetime_format_to_string(int64_t input, const char* format, struct buffer* output)
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
	const ptrdiff_t new_size = strftime((char*)buffer_data(output, size), INT8_MAX, format, &tm_);

	if (!new_size)
	{
		return 0;
	}

	return buffer_resize(output, size + new_size);
}

uint8_t datetime_parse(const char* input_start, const char* input_finish,
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
		input_start = find_any_symbol_like_or_not_like_that(input_start, input_finish, "0123456789", 10, 1, 1);

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
		input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, "0123456789", 10, 0, 1);
	}

	return (0 < *day && *day < 32 && 1 < *month && *month < 31 && *hour < 24 && *minute < 60 && *second < 60);
}

uint8_t datetime_to_char_array(const struct tm* tm_, char* output)
{
	if (NULL == tm_ || NULL == output)
	{
		return 0;
	}

	uint8_t length = 0;
#if __STDC_SEC_API__
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
	(void)input;
	return UINT8_MAX;
	/*TODO:
	struct tm tm_;

	if (!datetime_decode_to_tm(input, &tm_))
	{
		return 0;
	}

	return (uint8_t)tm_.tm_wday;*/
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

int64_t datetime_now_utc(time_t* now)
{
	if (NULL == now || ((time_t) -1) == time(now))
	{
		return 0;
	}

	/*TODO: gmtime();
	gmtime_s();*/
	return (*now);
}

int64_t datetime_now()
{
	time_t now = 0;

	if (!datetime_now_utc(&now))
	{
		return 0;
	}

#if __STDC_SEC_API__
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

int64_t timespan_from_days(double input)
{
	return (int64_t)((double)seconds_per_day * input);
}

int64_t timespan_from_hours(double input)
{
	return (int64_t)((double)seconds_per_hour * input);
}
/*timespan_from_milliseconds*/
int64_t timespan_from_minutes(double input)
{
	return (int64_t)((double)seconds_per_minute * input);
}

int64_t timespan_from_seconds(double input)
{
	return (int64_t)input;
}
/*timespan_from_ticks*/
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
/*timespan_get_seconds*/
/*timespan_get_ticks*/
double timespan_get_total_days(int64_t input)
{
	return (double)input / seconds_per_day;
}

double timespan_get_total_hours(int64_t input)
{
	return (double)input / seconds_per_hour;
}
/*timespan_get_total_milliseconds*/
double timespan_get_total_minutes(int64_t input)
{
	return (double)input / seconds_per_minute;
}

static const char* datetime_function_str[] =
{
	"format-to-string", "parse", "to-string", "get-day", "get-day-of-week",
	"get-day-of-year", "get-days-in-month",	"get-hour", "get-millisecond",
	"get-minute", "get-month", "get-second", "get-ticks", "get-year",
	"is-leap-year", "now", "from-input"
};

enum datetime_function
{
	format_to_string, parse, to_string,	get_day, get_day_of_week,
	get_day_of_year, get_days_in_month, get_hour, get_millisecond,
	get_minute, get_month, get_second, get_ticks, get_year,
	is_leap_year, now, encode,
	UNKNOWN_DATETIME_FUNCTION
};

uint8_t datetime_get_function(const char* name_start, const char* name_finish)
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

	struct range argument1;

	struct range argument2;

	argument1.start = argument2.start = argument1.finish = argument2.finish = NULL;

	if (1 == arguments_count)
	{
		if (!common_get_one_argument(arguments, &argument1, 1))
		{
			return 0;
		}
	}
	else if (2 == arguments_count)
	{
		if (!common_get_two_arguments(arguments, &argument1, &argument2, 1))
		{
			return 0;
		}
	}

	switch (function)
	{
		case format_to_string:
			return 2 == arguments_count &&
				   datetime_format_to_string(int64_parse(argument1.start), argument2.start, output);

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
				!datetime_parse(argument1.start, argument1.finish, &year, &month, &day, &hour, &minute, &second))
			{
				break;
			}

			return datetime_to_string(year, month, day, hour, minute, second, output);
		}

		case get_day:
			return 1 == arguments_count && int_to_string(datetime_get_day(int64_parse(argument1.start)), output);

		case get_day_of_week:
			/*TODO:
			return 1 == arguments_count && int_to_string(datetime_get_day_of_week(int64_parse(argument1.start)), output);*/
			break;

		case get_day_of_year:
			return 1 == arguments_count && int_to_string(datetime_get_day_of_year(int64_parse(argument1.start)), output);

		case get_days_in_month:
			return 2 == arguments_count &&
				   int_to_string(datetime_get_days_in_month(int_parse(argument1.start), (uint8_t)int_parse(argument2.start)),
								 output);

		case get_hour:
			return 1 == arguments_count && int_to_string(datetime_get_hour(int64_parse(argument1.start)), output);

		case get_millisecond:
			break;

		case get_minute:
			return 1 == arguments_count && int_to_string(datetime_get_minute(int64_parse(argument1.start)), output);

		case get_month:
			return 1 == arguments_count && int_to_string(datetime_get_month(int64_parse(argument1.start)), output);

		case get_second:
			return 1 == arguments_count && int_to_string(datetime_get_second(int64_parse(argument1.start)), output);

		case get_ticks:
			break;

		case get_year:
			return 1 == arguments_count && int_to_string(datetime_get_year(int64_parse(argument1.start)), output);

		case is_leap_year:
			return 1 == arguments_count && bool_to_string(datetime_is_leap_year(int_parse(argument1.start)), output);

		case now:
			if (!arguments_count)
			{
				int64_t time_is_now = datetime_now();
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
				!datetime_parse(argument1.start, argument1.finish, &year, &month, &day, &hour, &minute, &second))
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

static const char* timespan_function_str[] =
{
	"parse", "to-string", "from-days", "from-hours",
	"from-milliseconds", "from-minutes", "from-seconds",
	"from-ticks", "get-days", "get-hours",
	"get-milliseconds", "get-minutes", "get-seconds",
	"get-ticks", "get-total-days", "get-total-hours",
	"get-total-milliseconds", "get-total-minutes",
	"get-total-seconds"
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

uint8_t timespan_get_function(const char* name_start, const char* name_finish)
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

	argument.start = argument.finish = NULL;

	if (!common_get_one_argument(arguments, &argument, 1))
	{
		return 0;
	}

	/*double double_argument = 0;
	int64_t int64_argument = 0;*/

	switch (function)
	{
		case ts_from_days_:
			return int64_to_string(timespan_from_days(double_parse(argument.start)), output);

		case ts_from_hours_:
			return int64_to_string(timespan_from_hours(double_parse(argument.start)), output);

		case ts_from_milliseconds_:
			break;

		case ts_from_minutes_:
			return int64_to_string(timespan_from_minutes(double_parse(argument.start)), output);

		case ts_from_seconds_:
			return int64_to_string((int64_t)double_parse(argument.start), output);

		case ts_from_ticks_:
			break;

		case ts_get_days_:
			return int_to_string(timespan_get_days(int64_parse(argument.start)), output);

		case ts_get_hours_:
			return int_to_string(timespan_get_hours(int64_parse(argument.start)), output);

		case ts_get_milliseconds_:
			break;

		case ts_get_minutes_:
			return int_to_string(timespan_get_minutes(int64_parse(argument.start)), output);

		case ts_parse_:
		case ts_to_string_:
		case ts_get_seconds_:
		case ts_get_total_seconds_:
			return int64_to_string(int64_parse(argument.start), output);

		case ts_get_ticks_:
			break;

		case ts_get_total_days_:
			return double_to_string(timespan_get_total_days(int64_parse(argument.start)), output);

		case ts_get_total_hours_:
			return double_to_string(timespan_get_total_hours(int64_parse(argument.start)), output);

		case ts_get_total_milliseconds_:
			break;

		case ts_get_total_minutes_:
			return double_to_string(timespan_get_total_minutes(int64_parse(argument.start)), output);

		case UNKNOWN_TIMESPAN_FUNCTION:
		default:
			break;
	}

	return 0;
}
