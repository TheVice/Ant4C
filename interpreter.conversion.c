/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "conversion.h"

#include "common.h"
#include "range.h"

enum conversion_function
{
	parse, to_string,
	UNKNOWN_CONVERSION
};

uint8_t conversion_get_function(
	const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* conversion_str[] =
	{
		(const uint8_t*)"parse",
		(const uint8_t*)"to-string"
	};
	/**/
	return common_string_to_enum(name_start, name_finish, conversion_str, UNKNOWN_CONVERSION);
}


uint8_t bool_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output)
{
	if (UNKNOWN_CONVERSION <= function ||
		!arguments ||
		1 != arguments_count ||
		!output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_arguments(arguments, arguments_count, &argument, 0))
	{
		return 0;
	}

	switch (function)
	{
		case parse:
		case to_string:
		{
			uint8_t bool_value = 0;

			if (!bool_parse(argument.start, argument.finish, &bool_value))
			{
				break;
			}

			return bool_to_string(bool_value, output);
		}

		case UNKNOWN_CONVERSION:
		default:
			break;
	}

	return 0;
}

uint8_t conversion_exec_function(
	uint8_t name_space, uint8_t function,
	const struct buffer* arguments, uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_CONVERSION <= function ||
		!arguments ||
		1 != arguments_count ||
		!output)
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
		case parse:
		case to_string:
			if (1 == name_space)
			{
				return double_to_string(double_parse(argument.start), output);
			}
			else if (2 == name_space)
			{
				return int_to_string(int_parse(argument.start, argument.finish), output);
			}

#if !defined(_WIN32)
			else if (3 == name_space)
			{
				return long_to_string(long_parse(argument.start, argument.finish), output);
			}

#endif
			return int64_to_string(int64_parse(argument.start, argument.finish), output);

		case UNKNOWN_CONVERSION:
		default:
			break;
	}

	return 0;
}

uint8_t double_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output)
{
	return conversion_exec_function(1, function, arguments, arguments_count, output);
}

uint8_t int_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output)
{
	return conversion_exec_function(2, function, arguments, arguments_count, output);
}

uint8_t long_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output)
{
	return conversion_exec_function(3, function, arguments, arguments_count, output);
}

uint8_t int64_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
	struct buffer* output)
{
	return conversion_exec_function(4, function, arguments, arguments_count, output);
}
