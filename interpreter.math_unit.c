/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include "math_unit.h"

#include "common.h"
#include "conversion.h"
#include "range.h"

#include <float.h>

enum math_function
{
	abs_, ceiling_, floor_, round_, acos_, asin_, atan_, atan2_,
	cos_, cosh_, exp_, log_, log10_, max_, min_, pow_, sign_,
	sin_, sinh_, sqrt_, tan_, tanh_, cot_, coth_,
	truncate_, PI_, E_,	degrees_, radians_,
	addition_, subtraction_, multiplication_, division_,
	epsilon_, near_, less_, greater_,
	UNKNOWN_MATH_FUNCTION
};

uint8_t math_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* math_function_str[] =
	{
		(const uint8_t*)"abs",
		(const uint8_t*)"ceiling",
		(const uint8_t*)"floor",
		(const uint8_t*)"round",
		(const uint8_t*)"acos",
		(const uint8_t*)"asin",
		(const uint8_t*)"atan",
		(const uint8_t*)"atan2",
		(const uint8_t*)"cos",
		(const uint8_t*)"cosh",
		(const uint8_t*)"exp",
		(const uint8_t*)"log",
		(const uint8_t*)"log10",
		(const uint8_t*)"max",
		(const uint8_t*)"min",
		(const uint8_t*)"pow",
		(const uint8_t*)"sign",
		(const uint8_t*)"sin",
		(const uint8_t*)"sinh",
		(const uint8_t*)"sqrt",
		(const uint8_t*)"tan",
		(const uint8_t*)"tanh",
		(const uint8_t*)"cot",
		(const uint8_t*)"coth",
		(const uint8_t*)"truncate",
		(const uint8_t*)"PI",
		(const uint8_t*)"E",
		(const uint8_t*)"degrees",
		(const uint8_t*)"radians",
		(const uint8_t*)"addition",
		(const uint8_t*)"subtraction",
		(const uint8_t*)"multiplication",
		(const uint8_t*)"division",
		(const uint8_t*)"double_epsilon",
		(const uint8_t*)"near",
		(const uint8_t*)"less",
		(const uint8_t*)"greater"
	};
	/**/
	return common_string_to_enum(name_start, name_finish, math_function_str, UNKNOWN_MATH_FUNCTION);
}

uint8_t math_exec_function(uint8_t function, const void* arguments,
						   uint8_t arguments_count, void* output)
{
	if (UNKNOWN_MATH_FUNCTION <= function || NULL == arguments || 3 < arguments_count || NULL == output)
	{
		return 0;
	}

	struct range values[3];

	if (arguments_count && !common_get_arguments(arguments, arguments_count, values, 1))
	{
		return 0;
	}

	double double_values[COUNT_OF(values)];

	for (uint8_t i = 0, count = COUNT_OF(values); i < count; ++i)
	{
		double_values[i] =
			(arguments_count <= i || NULL == values[i].start) ? 0.0 : double_parse(values[i].start);
	}

	switch (function)
	{
		case abs_:
			return 1 == arguments_count && double_to_string(math_abs(double_values[0]), output);

		case ceiling_:
			return 1 == arguments_count && double_to_string(math_ceiling(double_values[0]), output);

		case floor_:
			return 1 == arguments_count && double_to_string(math_floor(double_values[0]), output);

		case round_:
			return 1 == arguments_count && double_to_string(math_round(double_values[0]), output);

		case acos_:
			if (1 != arguments_count || double_values[0] < -1 || 1 < double_values[0])
			{
				break;
			}

			return double_to_string(math_acos(double_values[0]), output);

		case asin_:
			if (1 != arguments_count || double_values[0] < -1 || 1 < double_values[0])
			{
				break;
			}

			return double_to_string(math_asin(double_values[0]), output);

		case atan_:
			return 1 == arguments_count && double_to_string(math_atan(double_values[0]), output);

		case atan2_:
			return 2 == arguments_count && double_to_string(math_atan2(double_values[0], double_values[1]), output);

		case cos_:
			return 1 == arguments_count && double_to_string(math_cos(double_values[0]), output);

		case cosh_:
			return 1 == arguments_count && double_to_string(math_cosh(double_values[0]), output);

		case exp_:
			return 1 == arguments_count && double_to_string(math_exp(double_values[0]), output);

		case log_:
			return 1 == arguments_count && double_to_string(math_log(double_values[0]), output);

		case log10_:
			return 1 == arguments_count && double_to_string(math_log10(double_values[0]), output);

		case max_:
			return 2 == arguments_count && double_to_string(math_max(double_values[0], double_values[1]), output);

		case min_:
			return 2 == arguments_count && double_to_string(math_min(double_values[0], double_values[1]), output);

		case pow_:
			return 2 == arguments_count && double_to_string(math_pow(double_values[0], double_values[1]), output);

		case sign_:
			return 1 == arguments_count && int_to_string(math_sign(double_values[0]), output);

		case sin_:
			return 1 == arguments_count && double_to_string(math_sin(double_values[0]), output);

		case sinh_:
			return 1 == arguments_count && double_to_string(math_sinh(double_values[0]), output);

		case sqrt_:
			if (1 != arguments_count || double_values[0] < 0)
			{
				return 0;
			}

			return double_to_string(math_sqrt(double_values[0]), output);

		case tan_:
			return 1 == arguments_count && double_to_string(math_tan(double_values[0]), output);

		case tanh_:
			return 1 == arguments_count && double_to_string(math_tanh(double_values[0]), output);

		case cot_:
			return 1 == arguments_count && double_to_string(math_cot(double_values[0]), output);

		case coth_:
			return 1 == arguments_count && double_to_string(math_coth(double_values[0]), output);

		case truncate_:
			return 1 == arguments_count && int64_to_string(math_truncate(double_values[0]), output);

		case PI_:
			return !arguments_count && double_to_string(math_PI, output);

		case E_:
			return !arguments_count && double_to_string(math_E, output);

		case degrees_:
			return 1 == arguments_count && double_to_string(math_degrees(double_values[0]), output);

		case radians_:
			return 1 == arguments_count && double_to_string(math_radians(double_values[0]), output);

		case addition_:
			return 2 == arguments_count && double_to_string(double_values[0] + double_values[1], output);

		case subtraction_:
			return 2 == arguments_count && double_to_string(double_values[0] - double_values[1], output);

		case multiplication_:
			return 2 == arguments_count && double_to_string(double_values[0] * double_values[1], output);

		case division_:
			if (2 != arguments_count || math_double_near(double_values[1], 0.0, 2 * DBL_EPSILON))
			{
				break;
			}

			return double_to_string(double_values[0] / double_values[1], output);

		case epsilon_:
			return !arguments_count && double_to_string(DBL_EPSILON, output);

		case near_:
			if (2 == arguments_count || 3 == arguments_count)
			{
				if (2 == arguments_count)
				{
					double_values[2] = 2 * DBL_EPSILON;
				}

				const uint8_t is_near = math_double_near(double_values[0], double_values[1], double_values[2]);
				return bool_to_string(is_near, output);
			}

			break;

		case less_:
			return 2 == arguments_count && bool_to_string(double_values[0] < double_values[1], output);

		case greater_:
			return 2 == arguments_count && bool_to_string(double_values[0] > double_values[1], output);

		case UNKNOWN_MATH_FUNCTION:
		default:
			break;
	}

	return 0;
}
