/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "math_unit.h"

#include <math.h>
#include <float.h>

static const uint8_t D180 = 180;

double math_abs(double value)
{
	return value < 0 ? -value : value;
}

double math_ceiling(double value)
{
	const double rest = modf(value, &value);
	return (rest < 2 * DBL_EPSILON ? value : 1 + value);
}

double math_floor(double value)
{
	const double rest = modf(value, &value);
	return (rest < 0 ? value - 1 : value);
}

double math_round(double value)
{
	const double rest = modf(value, &value);

	if (math_abs(value) < 1)
	{
		return (0.5 + 2 * DBL_EPSILON < math_abs(rest) ? (rest < 0 ? value - 1 : value + 1) : value);
	}

	return (0.5 - 2 * DBL_EPSILON < math_abs(rest) ? (rest < 0 ? value - 1 : value + 1) : value);
}

double math_acos(double value)
{
	return acos(value);
}

double math_asin(double value)
{
	return asin(value);
}
double math_atan(double value)
{
	return atan(value);
}

double math_atan2(double x_value, double y_value)
{
	return atan2(y_value, x_value);
}

double math_cos(double value)
{
	return cos(value);
}

double math_cosh(double value)
{
	return cosh(value);
}

double math_exp(double value)
{
	return exp(value);
}

double math_log(double value)
{
	return log(value);
}

double math_log10(double value)
{
	return log10(value);
}

double math_max(double value1, double value2)
{
	return value1 < value2 ? value2 : value1;
}

double math_min(double value1, double value2)
{
	return value1 < value2 ? value1 : value2;
}

double math_pow(double base_value, double exponent_value)
{
	return pow(base_value, exponent_value);
}

int8_t math_sign(double value)
{
	return value < 0 ? -1 : 1;
}

double math_sin(double value)
{
	return sin(value);
}

double math_sinh(double value)
{
	return sinh(value);
}

double math_sqrt(double value)
{
	return sqrt(value);
}

double math_tan(double value)
{
	return tan(value);
}

double math_tanh(double value)
{
	return tanh(value);
}

double math_cot(double value)
{
	return 1 / tan(value);
}

double math_coth(double value)
{
	return 1 / tanh(value);
}

int64_t math_truncate(double value)
{
	const double rest = modf(value, &value);
	(void)rest;
	return (int64_t)value;
}

double math_degrees(double r)
{
	return r * D180 / math_PI;
}

double math_radians(double d)
{
	return d * math_PI / D180;
}

uint8_t math_double_near(double value1, double value2, double epsilon)
{
	return (value2 - epsilon < value1 && value1 < value2 + epsilon);
}
