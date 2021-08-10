/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "date_time.h"

#include "common.h"
#include "conversion.h"
#include "range.h"

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
	format_to_string, parse, to_string, get_day, get_day_of_week,
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
	if (UNKNOWN_DATETIME_FUNCTION <= function || !arguments || 2 < arguments_count || !output)
	{
		return 0;
	}

	struct range values[2];

	if (arguments_count && !common_get_arguments(arguments, arguments_count, values, 0))
	{
		return 0;
	}

	switch (function)
	{
		case format_to_string:
			return 2 == arguments_count &&
				   datetime_format_to_string(int64_parse(values[0].start, values[0].finish), values[1].start, output);

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
			return 1 == arguments_count &&
				   int_to_string(datetime_get_day(int64_parse(values[0].start, values[0].finish)), output);

		case get_day_of_week:
			return 1 == arguments_count &&
				   int_to_string(datetime_get_day_of_week(int64_parse(values[0].start, values[0].finish)), output);

		case get_day_of_year:
			return 1 == arguments_count &&
				   int_to_string(datetime_get_day_of_year(int64_parse(values[0].start, values[0].finish)), output);

		case get_days_in_month:
			return 2 == arguments_count &&
				   int_to_string(datetime_get_days_in_month(
									 int_parse(values[0].start, values[0].finish),
									 (uint8_t)int_parse(values[1].start, values[1].finish)),
								 output);

		case get_hour:
			return 1 == arguments_count &&
				   int_to_string(datetime_get_hour(int64_parse(values[0].start, values[0].finish)), output);

		case get_millisecond:
			/*TODO:*/
			break;

		case get_minute:
			return 1 == arguments_count &&
				   int_to_string(datetime_get_minute(int64_parse(values[0].start, values[0].finish)), output);

		case get_month:
			return 1 == arguments_count &&
				   int_to_string(datetime_get_month(int64_parse(values[0].start, values[0].finish)), output);

		case get_second:
			return 1 == arguments_count &&
				   int_to_string(datetime_get_second(int64_parse(values[0].start, values[0].finish)), output);

		case get_ticks:
			/*TODO:*/
			break;

		case get_year:
			return 1 == arguments_count &&
				   int_to_string(datetime_get_year(int64_parse(values[0].start, values[0].finish)), output);

		case is_leap_year:
			return 1 == arguments_count &&
				   bool_to_string(datetime_is_leap_year(int_parse(values[0].start, values[0].finish)), output);

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
	if (UNKNOWN_TIMESPAN_FUNCTION <= function || !arguments || 1 != arguments_count || !output)
	{
		return 0;
	}

	struct range argument;

	const uint8_t is_double_parse_will_use =
		(ts_from_days_ == function ||
		 ts_from_hours_ == function ||
		 ts_from_milliseconds_ == function ||
		 ts_from_minutes_ == function);

	if (!common_get_arguments(arguments, arguments_count, &argument, is_double_parse_will_use))
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
			return int64_to_string(int64_parse(argument.start, argument.finish), output);

		case ts_from_ticks_:
			return int64_to_string(timespan_from_ticks(int64_parse(argument.start, argument.finish)), output);

		case ts_get_days_:
			return int_to_string(timespan_get_days(int64_parse(argument.start, argument.finish)), output);

		case ts_get_hours_:
			return int_to_string(timespan_get_hours(int64_parse(argument.start, argument.finish)), output);

		case ts_get_milliseconds_:
			/*TODO:*/
			break;

		case ts_get_minutes_:
			return int_to_string(timespan_get_minutes(int64_parse(argument.start, argument.finish)), output);

		case ts_parse_:
		case ts_to_string_:
		case ts_get_seconds_:
		case ts_get_total_seconds_:
			return int64_to_string(int64_parse(argument.start, argument.finish), output);

		case ts_get_ticks_:
			return int64_to_string(timespan_get_ticks(int64_parse(argument.start, argument.finish)), output);

		case ts_get_total_days_:
			return double_to_string(timespan_get_total_days(int64_parse(argument.start, argument.finish)), output);

		case ts_get_total_hours_:
			return double_to_string(timespan_get_total_hours(int64_parse(argument.start, argument.finish)), output);

		case ts_get_total_milliseconds_:
			return int64_to_string(timespan_get_total_milliseconds(int64_parse(argument.start, argument.finish)), output);

		case ts_get_total_minutes_:
			return double_to_string(timespan_get_total_minutes(int64_parse(argument.start, argument.finish)), output);

		case UNKNOWN_TIMESPAN_FUNCTION:
		default:
			break;
	}

	return 0;
}

