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
#include "string_unit.h"

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t* digits = (const uint8_t*)"0123456789";
static const uint8_t count_of_digits = 10;

static const uint8_t minus = '-';
static const uint8_t zero = '0';

static const uint8_t* False = (const uint8_t*)"False";
static const uint8_t* True = (const uint8_t*)"True";

#define FALSE_LENGTH 5
#define TRUE_LENGTH  4

uint8_t bool_parse(
	const uint8_t* input_start, const uint8_t* input_finish, uint8_t* output)
{
	static const uint8_t* false_ = (const uint8_t*)"false";
	static const uint8_t* true_ = (const uint8_t*)"true";

	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		!output)
	{
		return 0;
	}

	if (string_equal(input_start, input_finish, False, False + FALSE_LENGTH) ||
		string_equal(input_start, input_finish, false_, false_ + FALSE_LENGTH))
	{
		*output = 0;
	}
	else if (string_equal(
				 input_start, input_finish, True, True + TRUE_LENGTH) ||
			 string_equal(
				 input_start, input_finish, true_, true_ + TRUE_LENGTH))
	{
		*output = 1;
	}
	else
	{
		return 0;
	}

	return 1;
}

uint8_t bool_to_string(uint8_t input, struct buffer* output)
{
	if (!input)
	{
		return buffer_append(output, False, FALSE_LENGTH);
	}
	else if (1 == input)
	{
		return buffer_append(output, True, TRUE_LENGTH);
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
	char* out = (char*)buffer_data((OUTPUT), size);

#define DIGIT_TO_STRING(VALUE, EXPECTED_SIZE, FORMAT, OUTPUT)	\
	DIGIT_TO_STRING_COMMON((EXPECTED_SIZE), (OUTPUT))			\
	return buffer_resize((OUTPUT), size + sprintf(out, (FORMAT), (VALUE)));

#define DIGIT_TO_STRING_STDC_SEC_API(VALUE, EXPECTED_SIZE, FORMAT, OUTPUT)	\
	DIGIT_TO_STRING_COMMON((EXPECTED_SIZE), (OUTPUT))						\
	return buffer_resize((OUTPUT), size + sprintf_s(out, (EXPECTED_SIZE), (FORMAT), (VALUE)));

#define PARSE(START, FINISH, MAX_VALUE, MIN_VALUE, TYPE)			\
	const uint64_t output = uint64_parse((START), (FINISH));		\
	\
	if (!output)													\
	{																\
		return 0;													\
	}																\
	\
	(FINISH) = string_find_any_symbol_like_or_not_like_that(		\
			   (START), (FINISH), digits, digits + count_of_digits,	\
			   1, 1);												\
	(START) = string_find_any_symbol_like_or_not_like_that(			\
			  (START), (FINISH), &minus, &minus + 1, 1, 1);			\
	\
	uint32_t char_set;												\
	uint8_t is_minus;												\
	\
	if (!string_enumerate((START), (FINISH), &char_set))			\
	{																\
		is_minus = 0;												\
	}																\
	else															\
	{																\
		is_minus = minus == char_set;								\
	}																\
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
	return is_minus ? -1 * (TYPE)output : (TYPE)output;

#define TO_STRING(INPUT, OUTPUT)				\
	uint64_t digit;								\
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
		digit = (INPUT);						\
		++digit;								\
	}											\
	else										\
	{											\
		digit = (INPUT);						\
	}											\
	\
	return uint64_to_string(digit, (OUTPUT));

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

	input_start = string_find_any_symbol_like_or_not_like_that(
					  input_start, input_finish,
					  digits, digits + count_of_digits, 1, 1);
	const uint8_t* start = input_start;
	input_start = string_find_any_symbol_like_or_not_like_that(
					  input_start, input_finish,
					  digits + 1, digits + count_of_digits,
					  1, 1);
	start = string_find_any_symbol_like_or_not_like_that(
				start, input_start,
				digits, digits + count_of_digits, 0, 1);

	if (start != input_start)
	{
		return 0;
	}

	input_finish = string_find_any_symbol_like_or_not_like_that(
					   input_start, input_finish,
					   digits, digits + count_of_digits, 0, 1);
	/**/
	uint32_t output[21];
	uint32_t* out = output;
	const uint32_t* out_finish = out + 21;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, out)))
	{
		++out;

		if (out_finish == out)
		{
			return UINT64_MAX;
		}
	}

	uint64_t multi = 1;
	uint64_t result = 0;

	while (output < out)
	{
		--out;
		const uint64_t previous_result = result;

		for (uint8_t i = 0; i < count_of_digits; ++i)
		{
			if (digits[i] == *out)
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

	return result;
}

void conversion_addition_single_digit(uint8_t a, uint8_t b, uint8_t* c)
{
	if (a < '0' || '9' < a ||
		b < '0' || '9' < b ||
		!c)
	{
		return;
	}

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

void conversion_addition(
	const uint8_t* a_start, const uint8_t* a_finish,
	const uint8_t* b_start, const uint8_t* b_finish,
	uint8_t* result, uint8_t result_size)
{
	if (!result ||
		result_size < 2)
	{
		return;
	}

	memset(result, zero, result_size);
	uint8_t* c = result + (result_size - 2);

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

		conversion_addition_single_digit(a, b, c);
		--c;
	}
}

const uint8_t* uint64_to_string_to_byte_array(uint64_t input, uint8_t* a, uint8_t* b, uint8_t size)
{
	if (!a || !b || !size)
	{
		return NULL;
	}

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
	const uint8_t* result_start = a;
	const uint8_t* result_finish = b;
	uint8_t* out = b;
	memset(a, zero, size);

	while (0 < input)
	{
		const uint8_t the_byte = input & 0x1;
		input = input >> 1;

		if (!the_byte)
		{
			++i;
			continue;
		}

		conversion_addition(
			result_start, result_finish,
			str_bytes[i], str_bytes[i] + common_count_bytes_until(str_bytes[i], 0),
			out, size);

		if (a == result_start)
		{
			result_start = b;
			out = a;
		}
		else
		{
			result_start = a;
			out = b;
		}

		result_finish = result_start + size;
		++i;
	}

	return result_start;
}

uint8_t uint64_to_string(uint64_t input, struct buffer* output)
{
	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 42))
	{
		return 0;
	}

	uint8_t* a = buffer_data(output, size);
	uint8_t* b = a + 21;
	/**/
	const uint8_t* result_start = uint64_to_string_to_byte_array(input, a, b, 21);

	if (!result_start)
	{
		return 0;
	}

	const uint8_t* result_finish = result_start + 21;
	result_start = string_find_any_symbol_like_or_not_like_that(
					   result_start, result_finish, &zero, &zero + 1, 0, 1);
	/**/
	const uint8_t length = (uint8_t)(result_finish - result_start);

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	if (length)
	{
		MEM_CPY(a, result_start, length);
	}

	return length ? buffer_append(output, NULL, length) : buffer_push_back(output, zero);
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

uint8_t single_int_to_hex(uint8_t input)
{
	if (9 < input)
	{
		input += 'a' - 10;
	}
	else
	{
		input += '0';
	}

	return input;
}

uint8_t int_to_hex(uint8_t input, struct buffer* output)
{
	const uint8_t rest = input % 16;

	if (0 == input - rest)
	{
		return buffer_push_back(output, single_int_to_hex(rest));
	}

	return int_to_hex((input - rest) / 16, output) &&
		   buffer_push_back(output, single_int_to_hex(rest));
}
