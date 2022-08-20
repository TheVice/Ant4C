/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "date_time.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "math_unit.h"
#include "range.h"
#include "string_unit.h"

#include <time.h>
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
	if (!tm_)
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

uint8_t datetime_format_to_string(
	int64_t input, const uint8_t* format, void* output)
{
	struct tm tm_;

	if (!format || !output || !datetime_decode_to_tm(input, &tm_))
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, INT8_MAX))
	{
		return 0;
	}

	char* output_ = buffer_char_data(output, size);
	tm_.tm_year -= 1900;
	tm_.tm_mon -= 1;
	/**/
	const ptrdiff_t new_size = strftime(output_, INT8_MAX, (const char*)format, &tm_);

	if (!new_size)
	{
		return 0;
	}

	return buffer_resize(output, size + new_size);
}

uint8_t datetime_parse(
	const uint8_t* input_start, const uint8_t* input_finish,
	uint32_t* year, uint8_t* month, uint8_t* day,
	uint8_t* hour, uint8_t* minute, uint8_t* second)
{
	static const uint8_t* digits = (const uint8_t*)"0123456789";
	static const uint8_t count_of_digits = 10;

	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		!year || !month || !day ||
		!hour || !minute || !second)
	{
		return 0;
	}

	uint8_t step = 0;
	uint8_t* input[6];
	input[0] = day;
	input[1] = month;
	input[2] = NULL;
	input[3] = hour;
	input[4] = minute;
	input[5] = second;

	while (input_start != input_finish && step < 6)
	{
		input_start = string_find_any_symbol_like_or_not_like_that(
						  input_start, input_finish, digits, digits + count_of_digits, 1, 1);

		if (input_start == input_finish)
		{
			break;
		}

		const int32_t value = int_parse(input_start, input_finish);

		if (2 == step)
		{
			*year = value;
		}
		else
		{
			*input[step] = (uint8_t)value;
		}

		++step;
		input_start = string_enumerate(input_start, input_finish, NULL);
		input_start = string_find_any_symbol_like_or_not_like_that(
						  input_start, input_finish,
						  digits, digits + count_of_digits, 0, 1);
	}

	return (0 < *day && *day < 32 && 1 < *month && *month < 31 && *hour < 24 && *minute < 60 && *second < 60);
}

uint8_t datetime_parse_range(const struct range* input, int64_t* output)
{
	if (range_is_null_or_empty(input) ||
		!output)
	{
		return 0;
	}

	uint32_t year = 0;
	uint8_t month = 0;
	uint8_t day = 0;
	uint8_t hour = 0;
	uint8_t minute = 0;
	uint8_t second = 0;

	if (!datetime_parse(input->start, input->finish, &year, &month, &day, &hour, &minute, &second))
	{
		return 0;
	}

	*output = datetime_encode(year, month, day, hour, minute, second);
	return 1;
}

uint8_t datetime_to_char_array(const int* inputs, uint8_t* output)
{
	if (!inputs || !output)
	{
		return 0;
	}

	uint8_t* output_finish = output;

	for (uint8_t i = 0; i < 6; ++i)
	{
		static const uint8_t zero = '0';

		if (i < 2 || 3 < i)
		{
			if (inputs[i] < 10)
			{
				*output_finish = zero;
				++output_finish;
			}
		}

#define MAXIMUM_STR_LENGTH 21
		uint8_t* a = output_finish;
		uint8_t* b = output_finish + MAXIMUM_STR_LENGTH;
		/**/
		const uint8_t* c = uint64_to_string_to_byte_array(inputs[i], a, b, MAXIMUM_STR_LENGTH);
		const uint8_t* d = c + MAXIMUM_STR_LENGTH;
		/**/
		c = string_find_any_symbol_like_or_not_like_that(
				c, d, &zero, &zero + 1, 0, 1);
		/**/
		const uint8_t length = (uint8_t)(d - c);

		if (length)
		{
			MEM_CPY(a, c, length);
		}
		else
		{
			*a = zero;
			++a;
		}

		if (i < 2)
		{
			*a = '.';
			++a;
		}
		else if (2 == i)
		{
			*a = ' ';
			++a;
		}
		else if (5 != i)
		{
			*a = ':';
			++a;
		}

		output_finish = a;
	}

	return (uint8_t)(output_finish - output);
}

uint8_t datetime_to_string(
	uint32_t year, uint8_t month, uint8_t day,
	uint8_t hour, uint8_t minute, uint8_t second,
	void* output)
{
	if (year < 1970 || INT32_MAX < year ||
		month < 1 || 12 < month ||
		day < 1 || 31 < day ||
		23 < hour || 59 < minute ||
		60 < second ||
		!output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, INT8_MAX))
	{
		return 0;
	}

#if defined(_MSC_VER) && (_MSC_VER < 1920)
	int inputs[6];
	inputs[0] = day;
	inputs[1] = month;
	inputs[2] = (int)year;
	inputs[3] = hour;
	inputs[4] = minute;
	inputs[5] = second;
#else
	int inputs[] = { day, month, (int)year, hour, minute, second };
#endif
	uint8_t* output_ = buffer_uint8_t_data(output, size);
	return buffer_resize(output, size + datetime_to_char_array(inputs, output_));
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

int64_t datetime_encode(
	uint32_t year, uint8_t month, uint8_t day,
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

uint8_t datetime_decode(
	int64_t time, uint32_t* year, uint8_t* month, uint8_t* day,
	uint8_t* hour, uint8_t* minute, uint8_t* second, uint16_t* year_day)
{
	if (!year && !month && !day &&
		!hour && !minute && !second &&
		!year_day)
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
		*year = tm_.tm_year;
	}

	if (month)
	{
		*month = (uint8_t)tm_.tm_mon;
	}

	if (day)
	{
		*day = (uint8_t)tm_.tm_mday;
	}

	if (hour)
	{
		*hour = (uint8_t)tm_.tm_hour;
	}

	if (minute)
	{
		*minute = (uint8_t)tm_.tm_min;
	}

	if (second)
	{
		*second = (uint8_t)tm_.tm_sec;
	}

	if (year_day)
	{
		*year_day = (uint16_t)tm_.tm_yday;
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
int64_t date_time_millisecond_to_second(uint64_t millisecond)
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
	if (ticks < (int64_t)CLOCKS_PER_SEC)
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
