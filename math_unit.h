/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 TheVice
 *
 */

#ifndef _MATH_UNIT_H_
#define _MATH_UNIT_H_

#include <stdint.h>

struct buffer;

double math_abs(double value);
double math_ceiling(double value);
double math_floor(double value);
double math_round(double value);

double math_acos(double value);
double math_asin(double value);
double math_atan(double value);
double math_atan2(double x_value, double y_value);

double math_cos(double value);
double math_cosh(double value);

double math_exp(double value);
double math_log(double value);
double math_log10(double value);

double math_max(double value1, double value2);
double math_min(double value1, double value2);

double math_pow(double base_value, double exponent_value);

int8_t math_sign(double value);

double math_sin(double value);
double math_sinh(double value);

double math_sqrt(double value);

double math_tan(double value);
double math_tanh(double value);

double math_cot(double value);
double math_coth(double value);

int64_t math_truncate(double value);

double math_degrees(double r);
double math_radians(double d);

uint8_t math_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t math_exec_function(uint8_t function, const struct buffer* arguments,
						   uint8_t arguments_count, struct buffer* output);

#endif
