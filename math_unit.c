/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "math_unit.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"
#include "string_unit.h"

#include <math.h>
#include <float.h>

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

static const double PI = 3.1415926535897931;
static const uint8_t D180 = 180;

double math_degrees(double r)
{
	return r * D180 / PI;
}

double math_radians(double d)
{
	return d * PI / D180;
}

static const char* math_function_str[] =
{
	"abs", "ceiling", "floor", "round", "acos", "asin", "atan", "atan2",
	"cos", "cosh", "exp", "log", "log10", "max", "min", "pow", "sign",
	"sin", "sinh", "sqrt", "tan", "tanh", "cot", "coth",
	"truncate", "PI", "E",	"degrees", "radians",
	"addition", "subtraction", "multiplication", "division",
	"near", "less", "greater"
};

enum math_function
{
	abs_, ceiling_, floor_, round_, acos_, asin_, atan_, atan2_,
	cos_, cosh_, exp_, log_, log10_, max_, min_, pow_, sign_,
	sin_, sinh_, sqrt_, tan_, tanh_, cot_, coth_,
	truncate_, PI_, E_,	degrees_, radians_,
	addition_, subtraction_, multiplication_, division_,
	near_, less_, greater_,
	UNKNOWN_MATH_FUNCTION
};

uint8_t math_get_function(const char* name_start, const char* name_finish)
{
	return common_string_to_enum(name_start, name_finish, math_function_str, UNKNOWN_MATH_FUNCTION);
}

uint8_t math_double_near(double value1, double value2)
{
	static const double epsilon = 2 * DBL_EPSILON;
	return (value2 - epsilon < value1 && value1 < value2 + epsilon);
}

uint8_t math_double_near_to_zero(double value)
{
	return math_double_near(value, 0.0);
}

uint8_t math_exec_function(uint8_t function, const struct buffer* arguments,
						   uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_MATH_FUNCTION <= function || NULL == arguments || 2 < arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument1;

	struct range argument2;

	argument1.start = argument2.start = argument1.finish = argument2.finish = NULL;

	double double_argument_1 = 0;

	double double_argument_2 = 0;

	if (1 == arguments_count)
	{
		if (!common_get_one_argument(arguments, &argument1, 1))
		{
			return 0;
		}

		double_argument_1 = double_parse(argument1.start);
	}
	else if (2 == arguments_count)
	{
		if (!common_get_two_arguments(arguments, &argument1, &argument2, 1))
		{
			return 0;
		}

		double_argument_1 = double_parse(argument1.start);
		double_argument_2 = double_parse(argument2.start);
	}

	switch (function)
	{
		case abs_:
			return 1 == arguments_count && double_to_string(math_abs(double_argument_1), output);

		case ceiling_:
			return 1 == arguments_count && double_to_string(math_ceiling(double_argument_1), output);

		case floor_:
			return 1 == arguments_count && double_to_string(math_floor(double_argument_1), output);

		case round_:
			return 1 == arguments_count && double_to_string(math_round(double_argument_1), output);

		case acos_:
			if (1 != arguments_count || double_argument_1 < -1 || 1 < double_argument_1)
			{
				break;
			}

			return double_to_string(math_acos(double_argument_1), output);

		case asin_:
			if (1 != arguments_count || double_argument_1 < -1 || 1 < double_argument_1)
			{
				break;
			}

			return double_to_string(math_asin(double_argument_1), output);

		case atan_:
			return 1 == arguments_count && double_to_string(math_atan(double_argument_1), output);

		case atan2_:
			return 2 == arguments_count && double_to_string(math_atan2(double_argument_1, double_argument_2), output);

		case cos_:
			return 1 == arguments_count && double_to_string(math_cos(double_argument_1), output);

		case cosh_:
			return 1 == arguments_count && double_to_string(math_cosh(double_argument_1), output);

		case exp_:
			return 1 == arguments_count && double_to_string(math_exp(double_argument_1), output);

		case log_:
			return 1 == arguments_count && double_to_string(math_log(double_argument_1), output);

		case log10_:
			return 1 == arguments_count && double_to_string(math_log10(double_argument_1), output);

		case max_:
			return 2 == arguments_count && double_to_string(math_max(double_argument_1, double_argument_2), output);

		case min_:
			return 2 == arguments_count && double_to_string(math_min(double_argument_1, double_argument_2), output);

		case pow_:
			return 2 == arguments_count && double_to_string(math_pow(double_argument_1, double_argument_2), output);

		case sign_:
			return 1 == arguments_count && int_to_string(math_sign(double_argument_1), output);

		case sin_:
			return 1 == arguments_count && double_to_string(math_sin(double_argument_1), output);

		case sinh_:
			return 1 == arguments_count && double_to_string(math_sinh(double_argument_1), output);

		case sqrt_:
			if (1 != arguments_count || double_argument_1 < 0)
			{
				return 0;
			}

			return double_to_string(math_sqrt(double_argument_1), output);

		case tan_:
			return 1 == arguments_count && double_to_string(math_tan(double_argument_1), output);

		case tanh_:
			return 1 == arguments_count && double_to_string(math_tanh(double_argument_1), output);

		case cot_:
			return 1 == arguments_count && double_to_string(math_cot(double_argument_1), output);

		case coth_:
			return 1 == arguments_count && double_to_string(math_coth(double_argument_1), output);

		case truncate_:
			return 1 == arguments_count && int64_to_string(math_truncate(double_argument_1), output);

		case PI_:
			return !arguments_count && common_append_string_to_buffer("3.1415926535897931", output);

		case E_:
			return !arguments_count && common_append_string_to_buffer("2.7182818284590451", output);

		case degrees_:
			return 1 == arguments_count && double_to_string(math_degrees(double_argument_1), output);

		case radians_:
			return 1 == arguments_count && double_to_string(math_radians(double_argument_1), output);

		case addition_:
			return 2 == arguments_count && double_to_string(double_argument_1 + double_argument_2, output);

		case subtraction_:
			return 2 == arguments_count && double_to_string(double_argument_1 - double_argument_2, output);

		case multiplication_:
			return 2 == arguments_count && double_to_string(double_argument_1 * double_argument_2, output);

		case division_:
			if (2 != arguments_count || math_double_near_to_zero(double_argument_2))
			{
				break;
			}

			return double_to_string(double_argument_1 / double_argument_2, output);

		case near_:
			return 2 == arguments_count && bool_to_string(math_double_near(double_argument_1, double_argument_2), output);

		case less_:
			return 2 == arguments_count && bool_to_string(double_argument_1 < double_argument_2, output);

		case greater_:
			return 2 == arguments_count && bool_to_string(double_argument_1 > double_argument_2, output);

		case UNKNOWN_MATH_FUNCTION:
		default:
			break;
	}

	return 0;
}
