/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "conversion.h"
#include "buffer.h"
#include "common.h"
#include "range.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

static const char* False = "False";
static const char* True = "True";

#define FALSE_LENGTH 5
#define TRUE_LENGTH  4

uint8_t bool_parse(const char* input_string_start, const char* input_string_finish, uint8_t* bool_value)
{
	if (range_in_parts_is_null_or_empty(input_string_start, input_string_finish) ||
		NULL == bool_value)
	{
		return 0;
	}

	const ptrdiff_t length = input_string_finish - input_string_start;

	if (FALSE_LENGTH == length &&
		(0 == memcmp(False, input_string_start, length) ||
		 0 == memcmp("false", input_string_start, length)))
	{
		*bool_value = 0;
		return 1;
	}
	else if (TRUE_LENGTH == length &&
			 (0 == memcmp(True, input_string_start, length) ||
			  0 == memcmp("true", input_string_start, length)))
	{
		*bool_value = 1;
		return 1;
	}

	return 0;
}

uint8_t bool_to_string(uint8_t bool_value, struct buffer* output_string)
{
	if (0 == bool_value)
	{
		return buffer_append_char(output_string, False, FALSE_LENGTH);
	}
	else if (1 == bool_value)
	{
		return buffer_append_char(output_string, True, TRUE_LENGTH);
	}

	return 0;
}

#define DIGIT_TO_STRING_COMMON(EXPECTED_SIZE, OUTPUT)					\
	if (NULL == (OUTPUT))												\
	{																	\
		return 0;														\
	}																	\
	\
	const ptrdiff_t size = buffer_size(OUTPUT);							\
	\
	if (!buffer_append_char((OUTPUT), NULL, (EXPECTED_SIZE)))			\
	{																	\
		return 0;														\
	}																	\
	\
	char* ptr = (char*)buffer_data((OUTPUT), size);

#define DIGIT_TO_STRING(VALUE, EXPECTED_SIZE, FORMAT, OUTPUT)				\
	DIGIT_TO_STRING_COMMON((EXPECTED_SIZE), (OUTPUT))						\
	return buffer_resize((OUTPUT), size + sprintf(ptr, (FORMAT), (VALUE)));

#define DIGIT_TO_STRING_STDC_SEC_API(VALUE, EXPECTED_SIZE, FORMAT, OUTPUT)	\
	DIGIT_TO_STRING_COMMON((EXPECTED_SIZE), (OUTPUT))						\
	return buffer_resize((OUTPUT), size + sprintf_s(ptr, (EXPECTED_SIZE), (FORMAT), (VALUE)));

double double_parse(const char* value)
{
	return atof(value);
}

uint8_t double_to_string(double double_value, struct buffer* output_string)
{
#if __STDC_SEC_API__
	DIGIT_TO_STRING_STDC_SEC_API(double_value, 386, "%.16lf", output_string);
#else
	DIGIT_TO_STRING(double_value, 386, "%.16lf", output_string);
#endif
}

int32_t int_parse(const char* value)
{
	return atoi(value);
}

uint8_t int_to_string(int32_t int_value, struct buffer* output_string)
{
#if __STDC_SEC_API__
	DIGIT_TO_STRING_STDC_SEC_API(int_value, 12, "%i", output_string);
#else
	DIGIT_TO_STRING(int_value, 12, "%i", output_string);
#endif
}

long long_parse(const char* value)
{
	return atol(value);
}

long long_parse_wchar_t(const wchar_t* value)
{
	wchar_t* ch = NULL;
	return wcstol(value, &ch, 10);
}

uint8_t long_to_string(long long_value, struct buffer* output_string)
{
#if __STDC_SEC_API__
	DIGIT_TO_STRING_STDC_SEC_API(long_value, 24, "%ld", output_string);
#else
	DIGIT_TO_STRING(long_value, 24, "%ld", output_string);
#endif
}

int64_t int64_parse(const char* value)
{
	return atoll(value);
}

int64_t int64_parse_wchar_t(const wchar_t* value)
{
	wchar_t* ch = NULL;
	return (int64_t)wcstoimax(value, &ch, 10);
}

uint8_t int64_to_string(int64_t int_value, struct buffer* output_string)
{
#if __STDC_SEC_API__
	DIGIT_TO_STRING_STDC_SEC_API(int_value, 24, "%"PRId64, output_string);
#else
	DIGIT_TO_STRING(int_value, 24, "%"PRId64, output_string);
#endif
}

static const char* conversion_str[] =
{
	"parse", "to-string"
};

enum conversion_function
{
	parse, to_string,
	UNKNOWN_CONVERSION
};

uint8_t conversion_get_function(const char* name_start, const char* name_finish)
{
	return common_string_to_enum(name_start, name_finish, conversion_str, UNKNOWN_CONVERSION);
}

uint8_t bool_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						   struct buffer* output)
{
	if (UNKNOWN_CONVERSION <= function || NULL == arguments || 1 != arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_one_argument(arguments, &argument, 0))
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

uint8_t conversion_exec_function(uint8_t name_space, uint8_t function,
								 const struct buffer* arguments, uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_CONVERSION <= function || NULL == arguments || 1 != arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_one_argument(arguments, &argument, 1))
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
				return int_to_string(int_parse(argument.start), output);
			}

#if !defined(_WIN32)
			else if (3 == name_space)
			{
				return long_to_string(long_parse(argument.start), output);
			}

#endif
			return int64_to_string(int64_parse(argument.start), output);

		case UNKNOWN_CONVERSION:
		default:
			break;
	}

	return 0;
}

uint8_t double_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							 struct buffer* output)
{
	return conversion_exec_function(1, function, arguments, arguments_count, output);
}

uint8_t int_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						  struct buffer* output)
{
	return conversion_exec_function(2, function, arguments, arguments_count, output);
}

uint8_t long_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						   struct buffer* output)
{
	return conversion_exec_function(3, function, arguments, arguments_count, output);
}

uint8_t int64_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							struct buffer* output)
{
	return conversion_exec_function(4, function, arguments, arguments_count, output);
}
