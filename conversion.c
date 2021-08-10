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
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static const uint8_t* digits = (const uint8_t*)"0123456789";
static const uint8_t count_of_digits = 10;

static const uint8_t minus = '-';
static const uint8_t zero = '0';

static const char* False = "False";
static const char* True = "True";

#define FALSE_LENGTH 5
#define TRUE_LENGTH  4

uint8_t bool_parse(const uint8_t* input, ptrdiff_t length, uint8_t* output)
{
	if (!input ||
		length < TRUE_LENGTH ||
		FALSE_LENGTH < length ||
		!output)
	{
		return 0;
	}

	if (FALSE_LENGTH == length &&
		(0 == memcmp(False, input, length) ||
		 0 == memcmp("false", input, length)))
	{
		*output = 0;
		return 1;
	}
	else if (TRUE_LENGTH == length &&
			 (0 == memcmp(True, input, length) ||
			  0 == memcmp("true", input, length)))
	{
		*output = 1;
		return 1;
	}

	return 0;
}

uint8_t bool_to_string(uint8_t input, struct buffer* output)
{
	if (!input)
	{
		return buffer_append_char(output, False, FALSE_LENGTH);
	}
	else if (1 == input)
	{
		return buffer_append_char(output, True, TRUE_LENGTH);
	}

	return 0;
}

#define DIGIT_TO_STRING_COMMON(EXPECTED_SIZE, OUTPUT)			\
	if (NULL == (OUTPUT))										\
	{															\
		return 0;												\
	}															\
	\
	const ptrdiff_t size = buffer_size(OUTPUT);					\
	\
	if (!buffer_append_char((OUTPUT), NULL, (EXPECTED_SIZE)))	\
	{															\
		return 0;												\
	}															\
	\
	char* ptr = (char*)buffer_data((OUTPUT), size);

#define DIGIT_TO_STRING(VALUE, EXPECTED_SIZE, FORMAT, OUTPUT)	\
	DIGIT_TO_STRING_COMMON((EXPECTED_SIZE), (OUTPUT))			\
	return buffer_resize((OUTPUT), size + sprintf(ptr, (FORMAT), (VALUE)));

#define DIGIT_TO_STRING_STDC_SEC_API(VALUE, EXPECTED_SIZE, FORMAT, OUTPUT)	\
	DIGIT_TO_STRING_COMMON((EXPECTED_SIZE), (OUTPUT))						\
	return buffer_resize((OUTPUT), size + sprintf_s(ptr, (EXPECTED_SIZE), (FORMAT), (VALUE)));

#define PARSE(START, FINISH, MAX_VALUE, MIN_VALUE, TYPE)			\
	const uint64_t output = uint64_parse((START), (FINISH));		\
	\
	if (!output)													\
	{																\
		return 0;													\
	}																\
	\
	(FINISH) = find_any_symbol_like_or_not_like_that(				\
			   (START), (FINISH), digits, count_of_digits, 1, 1);	\
	(START) = find_any_symbol_like_or_not_like_that(				\
			  (START), (FINISH), &minus, 1, 1, 1);					\
	const uint8_t is_minus = minus == *(START);						\
	\
	if ((MAX_VALUE) < output)										\
	{																\
		return is_minus ? (MIN_VALUE) : (MAX_VALUE);				\
	}																\
	else if (is_minus && ((MAX_VALUE) - 1) < output)				\
	{																\
		return (MIN_VALUE);											\
	}																\
	\
	return (TYPE)(is_minus ? ((TYPE)-1) * output : output);

#define TO_STRING(INPUT, OUTPUT)				\
	uint64_t input_;							\
	\
	if ((INPUT) < 0)							\
	{											\
		if (!buffer_push_back((OUTPUT), minus))	\
		{										\
			return 0;							\
		}										\
		\
		(INPUT) += 1;							\
		(INPUT) *= -1;							\
		input_ = (INPUT);						\
		++input_;								\
	}											\
	else										\
	{											\
		input_ = (INPUT);						\
	}											\
	\
	return uint64_to_string(input_, (OUTPUT));

double double_parse(const uint8_t* value)
{
	return atof((const char*)value);
}

uint8_t double_to_string(double input, struct buffer* output)
{
#if __STDC_LIB_EXT1__
	DIGIT_TO_STRING_STDC_SEC_API(input, 386, "%.16lf", output);
#else
	DIGIT_TO_STRING(input, 386, "%.16lf", output);
#endif
}

int32_t int_parse(const uint8_t* input_start, const uint8_t* input_finish)
{
	PARSE(input_start, input_finish, INT32_MAX, INT32_MIN, int32_t);
}

uint8_t int_to_string(int32_t input, struct buffer* output)
{
	TO_STRING(input, output);
}

long long_parse(const uint8_t* input_start, const uint8_t* input_finish)
{
	PARSE(input_start, input_finish, LONG_MAX, LONG_MIN, long);
}

uint8_t long_to_string(long input, struct buffer* output)
{
	TO_STRING(input, output);
}

int64_t int64_parse(const uint8_t* input_start, const uint8_t* input_finish)
{
	PARSE(input_start, input_finish, INT64_MAX, INT64_MIN, int64_t);
}

uint8_t int64_to_string(int64_t input, struct buffer* output)
{
	TO_STRING(input, output);
}

uint64_t uint64_parse(const uint8_t* input_start, const uint8_t* input_finish)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish))
	{
		return 0;
	}

	input_start = find_any_symbol_like_or_not_like_that(
					  input_start, input_finish,
					  digits, count_of_digits, 1, 1);
	const uint8_t* start = input_start;
	input_start = find_any_symbol_like_or_not_like_that(
					  input_start, input_finish,
					  digits + 1, count_of_digits - 1, 1, 1);
	start = find_any_symbol_like_or_not_like_that(
				start, input_start,
				digits, count_of_digits, 0, 1);

	if (start != input_start)
	{
		return 0;
	}

	input_finish = find_any_symbol_like_or_not_like_that(
					   input_start, input_finish,
					   digits, count_of_digits, 0, 1);

	if (input_finish == input_start)
	{
		return 0;
	}
	else if (20 < input_finish - input_start)
	{
		return UINT64_MAX;
	}

	uint64_t multi = 1;
	uint64_t result = 0;

	do
	{
		--input_finish;
		const uint64_t previous_result = result;

		for (uint8_t i = 0; i < count_of_digits; ++i)
		{
			if (digits[i] == *input_finish)
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
	while (input_start != input_finish);

	return result;
}

void plus_(uint8_t a, uint8_t b, uint8_t* c)
{
	a -= zero;
	b -= zero;
	c[1] -= zero;
	c[1] += a + b;

	if (9 < c[1])
	{
		c[1] -= 10;
		c[0] = '1';
	}

	c[1] += zero;
}

const uint8_t* plus(
	const uint8_t* a_start, const uint8_t* a_finish,
	const uint8_t* b_start, const uint8_t* b_finish)
{
	static uint8_t result[21];
	memset(result, zero, sizeof(result));
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
			a = zero;
		}

		if (b_start < b_finish)
		{
			b_finish--;
			b = *b_finish;
		}
		else
		{
			b = zero;
		}

		if (a_start == a_finish &&
			b_start == b_finish &&
			zero == a && zero == b)
		{
			break;
		}

		plus_(a, b, c);
		--c;
	}

	return result;
}

uint8_t uint64_to_string(uint64_t input, struct buffer* output)
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
	const uint8_t* ptr_to_result;
	memset(result, zero, sizeof(result));
	static uint8_t* result_finish = result + 21;

	while (0 < input)
	{
		const uint8_t the_byte = input & 0x1;
		input = input >> 1;

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

	ptr_to_result = find_any_symbol_like_or_not_like_that(result, result_finish, &zero, 1, 0, 1);
	i = (uint8_t)(result_finish - ptr_to_result);
	/**/
	return i ? buffer_append(output, ptr_to_result, i) : buffer_push_back(output, zero);
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

uint8_t pointer_to_string(const void* input, struct buffer* output)
{
#if __STDC_LIB_EXT1__
	DIGIT_TO_STRING_STDC_SEC_API(input, 32, "%p", output);
#else
	DIGIT_TO_STRING(input, 32, "%p", output);
#endif
}
