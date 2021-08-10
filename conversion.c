/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "conversion.h"
#include "buffer.h"
#include "common.h"
#include "range.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static const char* False = "False";
static const char* True = "True";

#define FALSE_LENGTH 5
#define TRUE_LENGTH  4

uint8_t bool_parse(const uint8_t* input, ptrdiff_t input_length, uint8_t* bool_value)
{
	if (NULL == input ||
		0 == input_length ||
		NULL == bool_value)
	{
		return 0;
	}

	if (FALSE_LENGTH == input_length &&
		(0 == memcmp(False, input, input_length) ||
		 0 == memcmp("false", input, input_length)))
	{
		*bool_value = 0;
		return 1;
	}
	else if (TRUE_LENGTH == input_length &&
			 (0 == memcmp(True, input, input_length) ||
			  0 == memcmp("true", input, input_length)))
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

double double_parse(const uint8_t* value)
{
	return atof((const char*)value);
}

uint8_t double_to_string(double double_value, struct buffer* output_string)
{
#if __STDC_LIB_EXT1__
	DIGIT_TO_STRING_STDC_SEC_API(double_value, 386, "%.16lf", output_string);
#else
	DIGIT_TO_STRING(double_value, 386, "%.16lf", output_string);
#endif
}

int32_t int_parse(const uint8_t* value)
{
	return atoi((const char*)value);
}

uint8_t int_to_string(int32_t int_value, struct buffer* output_string)
{
#if __STDC_LIB_EXT1__
	DIGIT_TO_STRING_STDC_SEC_API(int_value, 12, "%i", output_string);
#else
	DIGIT_TO_STRING(int_value, 12, "%i", output_string);
#endif
}

long long_parse(const uint8_t* value)
{
	return atol((const char*)value);
}

uint8_t long_to_string(long long_value, struct buffer* output_string)
{
#if __STDC_LIB_EXT1__
	DIGIT_TO_STRING_STDC_SEC_API(long_value, 24, "%ld", output_string);
#else
	DIGIT_TO_STRING(long_value, 24, "%ld", output_string);
#endif
}

int64_t int64_parse(const uint8_t* value)
{
	return atoll((const char*)value);
}

uint8_t int64_to_string(int64_t int_value, struct buffer* output_string)
{
#if __STDC_LIB_EXT1__
	DIGIT_TO_STRING_STDC_SEC_API(int_value, 24, "%"PRId64, output_string);
#else
	DIGIT_TO_STRING(int_value, 24, "%"PRId64, output_string);
#endif
}

uint64_t uint64_parse(const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		return 0;
	}

	static const uint8_t* digits = (const uint8_t*)"0123456789";
	static const uint8_t count_of_digits = 10;
	/**/
	value_start = find_any_symbol_like_or_not_like_that(
					  value_start, value_finish,
					  digits + 1, count_of_digits - 1, 1, 1);
	value_finish = find_any_symbol_like_or_not_like_that(
					   value_start, value_finish,
					   digits, count_of_digits, 0, 1);

	if (value_finish == value_start)
	{
		return 0;
	}
	else if (20 < value_finish - value_start)
	{
		return UINT64_MAX;
	}

	uint64_t multi = 1;
	uint64_t result = 0;

	do
	{
		--value_finish;
		const uint64_t previous_result = result;

		for (uint8_t i = 0; i < count_of_digits; ++i)
		{
			if (digits[i] == *value_finish)
			{
				result += multi * i;
				break;
			}
		}

		if (result < previous_result)
		{
			return UINT64_MAX;
		}

		multi *= 10;
	}
	while (value_start != value_finish);

	return result;
}

void plus_(uint8_t a, uint8_t b, uint8_t* c)
{
	a -= '0';
	b -= '0';
	c[1] -= '0';
	c[1] += a + b;

	if (9 < c[1])
	{
		c[1] -= 10;
		c[0] = '1';
	}

	c[1] += '0';
}

const uint8_t* plus(
	const uint8_t* a_start, const uint8_t* a_finish,
	const uint8_t* b_start, const uint8_t* b_finish)
{
	static uint8_t result[21];
	memset(result, '0', sizeof(result));
	uint8_t* c = result + 19;

	while (result < c)
	{
		uint8_t a;
		uint8_t b;

		if (a_start < a_finish)
		{
			a_finish--;
			a = *a_finish;
		}
		else
		{
			a = '0';
		}

		if (b_start < b_finish)
		{
			b_finish--;
			b = *b_finish;
		}
		else
		{
			b = '0';
		}

		if (a_start == a_finish &&
			b_start == b_finish &&
			'0' == a && '0' == b)
		{
			break;
		}

		plus_(a, b, c);
		--c;
	}

	return result;
}

uint8_t uint64_to_string(uint64_t int_value, struct buffer* output_string)
{
	static const uint8_t* str_bytes[] =
	{
		(const uint8_t*)"1",
		(const uint8_t*)"2",
		(const uint8_t*)"4",
		(const uint8_t*)"8",
		(const uint8_t*)"16",
		(const uint8_t*)"32",
		(const uint8_t*)"64",
		(const uint8_t*)"128",
		(const uint8_t*)"256",
		(const uint8_t*)"512",
		(const uint8_t*)"1024",
		(const uint8_t*)"2048",
		(const uint8_t*)"4096",
		(const uint8_t*)"8192",
		(const uint8_t*)"16384",
		(const uint8_t*)"32768",
		(const uint8_t*)"65536",
		(const uint8_t*)"131072",
		(const uint8_t*)"262144",
		(const uint8_t*)"524288",
		(const uint8_t*)"1048576",
		(const uint8_t*)"2097152",
		(const uint8_t*)"4194304",
		(const uint8_t*)"8388608",
		(const uint8_t*)"16777216",
		(const uint8_t*)"33554432",
		(const uint8_t*)"67108864",
		(const uint8_t*)"134217728",
		(const uint8_t*)"268435456",
		(const uint8_t*)"536870912",
		(const uint8_t*)"1073741824",
		(const uint8_t*)"2147483648",
		(const uint8_t*)"4294967296",
		(const uint8_t*)"8589934592",
		(const uint8_t*)"17179869184",
		(const uint8_t*)"34359738368",
		(const uint8_t*)"68719476736",
		(const uint8_t*)"137438953472",
		(const uint8_t*)"274877906944",
		(const uint8_t*)"549755813888",
		(const uint8_t*)"1099511627776",
		(const uint8_t*)"2199023255552",
		(const uint8_t*)"4398046511104",
		(const uint8_t*)"8796093022208",
		(const uint8_t*)"17592186044416",
		(const uint8_t*)"35184372088832",
		(const uint8_t*)"70368744177664",
		(const uint8_t*)"140737488355328",
		(const uint8_t*)"281474976710656",
		(const uint8_t*)"562949953421312",
		(const uint8_t*)"1125899906842624",
		(const uint8_t*)"2251799813685248",
		(const uint8_t*)"4503599627370496",
		(const uint8_t*)"9007199254740992",
		(const uint8_t*)"18014398509481984",
		(const uint8_t*)"36028797018963968",
		(const uint8_t*)"72057594037927936",
		(const uint8_t*)"144115188075855872",
		(const uint8_t*)"288230376151711744",
		(const uint8_t*)"576460752303423488",
		(const uint8_t*)"1152921504606846976",
		(const uint8_t*)"2305843009213693952",
		(const uint8_t*)"4611686018427387904",
		(const uint8_t*)"9223372036854775808",
	};
	/**/
	uint8_t i = 0;
	static uint8_t result[21];
	const uint8_t* ptr_to_result = NULL;
	memset(result, '0', sizeof(result));
	static uint8_t* result_finish = result + 21;

	while (0 < int_value)
	{
		const uint8_t the_byte = int_value & 0x1;
		int_value = int_value >> 1;

		if (!the_byte)
		{
			++i;
			continue;
		}

		ptr_to_result = plus(result, result_finish, str_bytes[i],
							 str_bytes[i] + common_count_bytes_until(str_bytes[i], 0));
		memcpy(result, ptr_to_result, sizeof(result));
		/**/
		++i;
	}

	static const uint8_t zero = '0';
	ptr_to_result = find_any_symbol_like_or_not_like_that(result, result_finish, &zero, 1, 0, 1);
	i = (uint8_t)(result_finish - ptr_to_result);
	/**/
	return i ? buffer_append(output_string, ptr_to_result, i) : buffer_push_back(output_string, zero);
}

void* pointer_parse(const uint8_t* value)
{
	if (NULL == value)
	{
		return 0;
	}

	char* ch = NULL;
	return (void*)(ptrdiff_t)strtoll((const char*)value, &ch, 16);
}

uint8_t pointer_to_string(const void* pointer_value, struct buffer* output_string)
{
#if __STDC_LIB_EXT1__
	DIGIT_TO_STRING_STDC_SEC_API(pointer_value, 32, "%p", output_string);
#else
	DIGIT_TO_STRING(pointer_value, 32, "%p", output_string);
#endif
}

static const uint8_t* conversion_str[] =
{
	(const uint8_t*)"parse",
	(const uint8_t*)"to-string"
};

enum conversion_function
{
	parse, to_string,
	UNKNOWN_CONVERSION
};

uint8_t conversion_get_function(const uint8_t* name_start, const uint8_t* name_finish)
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

			if (!bool_parse(argument.start, range_size(&argument), &bool_value))
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
