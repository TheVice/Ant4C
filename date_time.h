/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _DATE_TIME_H_
#define _DATE_TIME_H_

#include <stdint.h>

struct buffer;

uint8_t datetime_format_to_string(int64_t input, const uint8_t* format, struct buffer* output);
uint8_t datetime_parse(const uint8_t* input_start, const uint8_t* input_finish,
					   uint32_t* year, uint8_t* month, uint8_t* day,
					   uint8_t* hour, uint8_t* minute, uint8_t* second);
uint8_t datetime_parse_buffer(struct buffer* input_output);
uint8_t datetime_to_string(uint32_t year, uint8_t month, uint8_t day,
						   uint8_t hour, uint8_t minute, uint8_t second,
						   struct buffer* output);

uint8_t datetime_get_day(int64_t input);
uint8_t datetime_get_day_of_week(int64_t input);
uint16_t datetime_get_day_of_year(int64_t input);

uint8_t datetime_get_days_in_month(uint32_t year, uint8_t month);

uint8_t datetime_get_hour(int64_t input);
/*uint16_t datetime_get_millisecond(int64_t input);*/
uint8_t datetime_get_minute(int64_t input);
uint8_t datetime_get_month(int64_t input);
uint8_t datetime_get_second(int64_t input);
/*int64_t datetime_get_ticks(int64_t input);*/
uint32_t datetime_get_year(int64_t input);

uint8_t datetime_is_leap_year(uint32_t year);

int64_t datetime_ticks();
int64_t datetime_now_utc();
int64_t datetime_now();

int64_t datetime_encode(uint32_t year, uint8_t month, uint8_t day,
						uint8_t hour, uint8_t minute, uint8_t second);
uint8_t datetime_decode(int64_t time, uint32_t* year, uint8_t* month, uint8_t* day,
						uint8_t* hour, uint8_t* minute, uint8_t* second, uint16_t* year_day);

#if !defined(_WIN32)
long datetime_get_bias();
#endif
int64_t date_time_millisecond_to_second(uint64_t millisecond);

int64_t timespan_from_days(double input);
int64_t timespan_from_hours(double input);
int64_t timespan_from_milliseconds(double input);
int64_t timespan_from_minutes(double input);
int64_t timespan_from_seconds(double input);
int64_t timespan_from_ticks(int64_t ticks);
int32_t timespan_get_days(int64_t input);
int32_t timespan_get_hours(int64_t input);
int32_t timespan_get_minutes(int64_t input);
int64_t timespan_get_ticks(int64_t input);
double timespan_get_total_days(int64_t input);
double timespan_get_total_hours(int64_t input);
int64_t timespan_get_total_milliseconds(int64_t input);
double timespan_get_total_minutes(int64_t input);

uint8_t datetime_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t datetime_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							   struct buffer* output);

uint8_t timespan_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t timespan_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							   struct buffer* output);

#endif
