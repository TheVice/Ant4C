/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "string_unit.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"

#include <ctype.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

ptrdiff_t string_index_of_any(const char* input_start, const char* input_finish,
							  const char* value_start, const char* value_finish, int8_t step)
{
	if (NULL == input_start || NULL == input_finish ||
		input_finish < input_start ||
		NULL == value_start || NULL == value_finish ||
		value_finish < value_start ||
		(-1 != step && 1 != step))
	{
		return -1;
	}

	const ptrdiff_t input_length = input_finish - input_start;
	const ptrdiff_t value_length = value_finish - value_start;

	if (input_length < value_length)
	{
		return -1;
	}

	if (input_length == 0 && input_length == value_length)
	{
		return 0;
	}

	if (input_length == value_length)
	{
		if (input_start == value_start)
		{
			return 0;
		}

		return (0 == (memcmp(input_start, value_start, input_length)) ? 0 : -1);
	}

	if (0 < step)
	{
		for (ptrdiff_t i = 0, count = 1 + input_length - value_length; i < count; ++i)
		{
			if (0 == memcmp(&input_start[i], value_start, value_length))
			{
				return i;
			}
		}
	}
	else
	{
		for (ptrdiff_t i = input_length - value_length; i > -1; --i)
		{
			if (0 == memcmp(&input_start[i], value_start, value_length))
			{
				return i;
			}
		}
	}

	return -1;
}

uint8_t string_contains(const char* input_start, const char* input_finish,
						const char* value_start, const char* value_finish)
{
	return -1 != string_index_of_any(input_start, input_finish, value_start, value_finish, 1);
}

uint8_t string_ends_with(const char* input_start, const char* input_finish,
						 const char* value_start, const char* value_finish)
{
	ptrdiff_t index = 0;

	if (-1 == (index = string_index_of_any(input_start, input_finish, value_start, value_finish, 1)))
	{
		return 0;
	}

	const ptrdiff_t expected_index = (input_finish - input_start) - (value_finish - value_start);
	return expected_index == index;
}

ptrdiff_t string_get_length(const char* input_start, const char* input_finish)
{
	if (input_start == input_finish)
	{
		return 0;
	}

	if (NULL == input_start || NULL == input_finish || input_finish < input_start)
	{
		return -1;
	}

	return input_finish - input_start;
}

ptrdiff_t string_index_of(const char* input_start, const char* input_finish,
						  const char* value_start, const char* value_finish)
{
	return string_index_of_any(input_start, input_finish, value_start, value_finish, 1);
}

ptrdiff_t string_last_index_of(const char* input_start, const char* input_finish,
							   const char* value_start, const char* value_finish)
{
	return string_index_of_any(input_start, input_finish, value_start, value_finish, -1);
}
/*TODO:string_pad_left
string_pad_right*/
ptrdiff_t string_get_index_of_first_non_equal_symbol(const char* input_a, ptrdiff_t a_length,
		const char* input_b, ptrdiff_t b_length)
{
	if (NULL == input_a ||
		NULL == input_b ||
		a_length < 1 ||
		b_length < 1)
	{
		return 0;
	}

	ptrdiff_t i = 0;

	for (; i < a_length && i < b_length; ++i)
	{
		if (input_a[i] != input_b[i])
		{
			break;
		}
	}

	return i;
}
#if 0
uint8_t string_replace_in_buffer(struct buffer* input_output,
								 const char* to_be_replaced, ptrdiff_t to_be_replaced_length,
								 const char* by_replacement, ptrdiff_t by_replacement_length)
{
	if (NULL == input_output ||
		NULL == to_be_replaced ||
		to_be_replaced_length < 1 ||
		(0 < by_replacement_length && NULL == by_replacement) ||
		(NULL != by_replacement && by_replacement_length < 0))
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(input_output);

	if (!size)
	{
		return 1;
	}
	else if (to_be_replaced_length < by_replacement_length)
	{
		const ptrdiff_t delta = by_replacement_length - to_be_replaced_length;
		const ptrdiff_t offset = string_get_index_of_first_non_equal_symbol(
									 to_be_replaced, to_be_replaced_length,
									 by_replacement, by_replacement_length);
		/**/
		ptrdiff_t index = 0;
		ptrdiff_t previous_index = -1;
		char* start = buffer_char_data(input_output, 0);
		char* finish = 1 + buffer_char_data(input_output, size - 1);

		while (previous_index < (index += string_index_of_any(start + index, finish,
										  to_be_replaced, to_be_replaced + to_be_replaced_length, 1)))
		{
			if (!buffer_append_char(input_output, NULL, delta))
			{
				return 0;
			}

			size = buffer_size(input_output);
			start = buffer_char_data(input_output, 0);
			finish = 1 + buffer_char_data(input_output, size - 1);
#if 0
#if __STDC_SEC_API__

			if (0 != memcpy_s(&start[index + by_replacement_length], size - (index + by_replacement_length),
							  &start[index + to_be_replaced_length], size - delta - (index + to_be_replaced_length)))
			{
				return 0;
			}

#else
			memcpy(&start[index + by_replacement_length], &start[index + to_be_replaced_length],
				   size - delta - (index + to_be_replaced_length));
#endif
#else
			char* dst = finish - 1;
			const char* src = dst - delta;

			for (; start + index + to_be_replaced_length - 1 < src; --src, --dst)
			{
				dst[0] = src[0];
			}

#endif
#if __STDC_SEC_API__

			if (0 != memcpy_s(&start[index + offset], to_be_replaced_length + delta - offset,
							  by_replacement + offset, by_replacement_length - offset))
			{
				return 0;
			}

#else
			memcpy(&start[index + offset], by_replacement + offset, by_replacement_length - offset);
#endif
			index += by_replacement_length;
			previous_index = index - 1;
		}
	}
	else if (1 == to_be_replaced_length &&
			 to_be_replaced_length == by_replacement_length)
	{
		if (to_be_replaced[0] == by_replacement[0])
		{
			return 1;
		}

		char* ptr = buffer_char_data(input_output, 0);

		for (ptrdiff_t i = 0; i < size; ++i)
		{
			if (to_be_replaced[0] == ptr[i])
			{
				ptr[i] = by_replacement[0];
			}
		}
	}
	else if (to_be_replaced_length == by_replacement_length)
	{
		if (string_equal(to_be_replaced, to_be_replaced + to_be_replaced_length,
						 by_replacement, by_replacement + by_replacement_length))
		{
			return 1;
		}

		const ptrdiff_t offset = string_get_index_of_first_non_equal_symbol(
									 to_be_replaced, to_be_replaced_length,
									 by_replacement, by_replacement_length);
		ptrdiff_t index = 0;
		ptrdiff_t previous_index = -1;
		char* start = buffer_char_data(input_output, 0);
		char* finish = 1 + buffer_char_data(input_output, size - 1);

		while (previous_index < (index += string_index_of_any(start + index, finish,
										  to_be_replaced, to_be_replaced + to_be_replaced_length, 1)))
		{
#if __STDC_SEC_API__

			if (0 != memcpy_s(&start[index + offset], to_be_replaced_length - offset,
							  by_replacement + offset, by_replacement_length - offset))
			{
				return 0;
			}

#else
			memcpy(&start[index + offset], by_replacement + offset, by_replacement_length - offset);
#endif
			index += to_be_replaced_length;
			previous_index = index - 1;
		}

		start += to_be_replaced_length;
	}
	else /*if (to_be_replaced_length > by_replacement_length)*/
	{
		const ptrdiff_t delta = to_be_replaced_length - by_replacement_length;
		const ptrdiff_t offset = string_get_index_of_first_non_equal_symbol(
									 to_be_replaced, to_be_replaced_length,
									 by_replacement, by_replacement_length);
		ptrdiff_t index = 0;
		ptrdiff_t previous_index = -1;
		char* start = buffer_char_data(input_output, 0);
		char* finish = 1 + buffer_char_data(input_output, size - 1);

		while (previous_index < (index += string_index_of_any(start + index, finish,
										  to_be_replaced, to_be_replaced + to_be_replaced_length, 1)))
		{
#if __STDC_SEC_API__

			if (0 != memcpy_s(&start[index + offset], to_be_replaced_length - offset,
							  by_replacement, by_replacement_length - offset))
			{
				return 0;
			}

#else
			memcpy(&start[index + offset], by_replacement, by_replacement_length - offset);
#endif
#if __STDC_SEC_API__

			if (0 != memcpy_s(&start[index + by_replacement_length], size - (index + by_replacement_length),
							  &start[index + to_be_replaced_length], size - (index + to_be_replaced_length)))
			{
				return 0;
			}

#else
			memcpy(&start[index + by_replacement_length], &start[index + to_be_replaced_length],
				   size - (index + to_be_replaced_length));
#endif

			if (!buffer_resize(input_output, size - delta))
			{
				return 0;
			}

			size = buffer_size(input_output);
			start = buffer_char_data(input_output, 0);
			finish = 1 + buffer_char_data(input_output, size - 1);
			/**/
			index += to_be_replaced_length;
			previous_index = index - 1;
		}
	}

	return 1;
}
#endif
uint8_t string_replace(const char* input_start, const char* input_finish,
					   const char* to_be_replaced_start, const char* to_be_replaced_finish,
					   const char* by_replacement_start, const char* by_replacement_finish,
					   struct buffer* output)
{
	if (NULL == input_start || NULL == input_finish ||
		NULL == to_be_replaced_start || NULL == to_be_replaced_finish ||
		NULL == output || input_finish <= input_start ||
		to_be_replaced_finish <= to_be_replaced_start)
	{
		return 0;
	}

	const ptrdiff_t input_length = input_finish - input_start;
	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append_char(output, input_start, input_length))
	{
		return 0;
	}

	const ptrdiff_t to_be_replaced_length = to_be_replaced_finish - to_be_replaced_start;
	const ptrdiff_t by_replacement_length =	(NULL == by_replacement_start || NULL == by_replacement_finish ||
											by_replacement_finish < by_replacement_start) ? -1 : (by_replacement_finish - by_replacement_start);

	if (1 == to_be_replaced_length && to_be_replaced_length == by_replacement_length)
	{
		if (to_be_replaced_start[0] == by_replacement_start[0])
		{
			return 1;
		}

		for (ptrdiff_t i = 0; i < input_length; ++i)
		{
			if (to_be_replaced_start[0] == *((char*)buffer_data(output, size + i)))
			{
				*((char*)buffer_data(output, size + i)) = by_replacement_start[0];
			}
		}

		return 1;
	}

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	const char* previous_position = input_start;
	ptrdiff_t index = -1;

	while (-1 != (index = string_index_of_any(
							  previous_position, input_finish, to_be_replaced_start, to_be_replaced_finish, 1)))
	{
		if (!buffer_append_char(output, previous_position, index))
		{
			return 0;
		}

		if (-1 != by_replacement_length)
		{
			if (!buffer_append_char(output, by_replacement_start, by_replacement_length))
			{
				return 0;
			}
		}

		previous_position += index + to_be_replaced_length;
	}

	if (!buffer_append_char(output, previous_position, input_finish - previous_position))
	{
		return 0;
	}

	return 1;
}

uint8_t string_starts_with(const char* input_start, const char* input_finish,
						   const char* value_start, const char* value_finish)
{
	return 0 == string_index_of_any(input_start, input_finish, value_start, value_finish, 1);
}

uint8_t string_substring(const char* input, ptrdiff_t input_length,
						 ptrdiff_t index, ptrdiff_t length, struct buffer* output)
{
	if (NULL == input || NULL == output ||
		input_length < 0 ||
		index < 0 ||
		length < 0 ||
		input_length < index + length)
	{
		return 0;
	}

	return buffer_append_char(output, &input[index], length);
}

uint8_t string_to_lower(const char* input_start, const char* input_finish, char* output)
{
	if (NULL == input_start || NULL == input_finish || NULL == output ||
		input_finish <= input_start)
	{
		return 0;
	}

	const char* pos = input_start;

	while (input_finish != pos)
	{
		*output = (char)tolower(*pos);
		++pos;
		++output;
	}

	return 1;
}

uint8_t string_to_upper(const char* input_start, const char* input_finish, char* output)
{
	if (NULL == input_start || NULL == input_finish || NULL == output ||
		input_finish <= input_start)
	{
		return 0;
	}

	const char* pos = input_start;

	while (input_finish != pos)
	{
		*output = (char)toupper(*pos);
		++pos;
		++output;
	}

	return 1;
}

enum string_function
{
	contains, ends_with, get_length, index_of, last_index_of,
	pad_left, pad_right, replace, starts_with, substring,
	to_lower, to_upper, trim, trim_end, trim_start,
	quote, un_quote, equal, empty,
	UNKNOWN_STRING_FUNCTION
};

enum string_trim_mode { string_trim_mode_all = trim, string_trim_mode_end = trim_end, string_trim_mode_start = trim_start };

uint8_t string_trim_any(struct range* input_output, enum string_trim_mode mode)
{
	static const char chars_to_trim[] = { '\0', '\t', ' ', '\n' };

	if (NULL != input_output && input_output->start == input_output->finish)
	{
		return 1;
	}

	if (range_is_null_or_empty(input_output) ||
		(string_trim_mode_all != mode && string_trim_mode_end != mode && string_trim_mode_start != mode))
	{
		return 0;
	}

	if (string_trim_mode_all == mode || string_trim_mode_start == mode)
	{
		input_output->start = find_any_symbol_like_or_not_like_that(input_output->start, input_output->finish,
							  chars_to_trim, 4, 0, 1);
	}

	if (input_output->start < input_output->finish &&
		(string_trim_mode_all == mode || string_trim_mode_end == mode))
	{
		const char* pos = input_output->start;
		const char* prev_pos = pos;

		while (input_output->finish > (pos = find_any_symbol_like_or_not_like_that(
				pos, input_output->finish, chars_to_trim, 4, 0, 1)))
		{
			prev_pos = pos;
			++pos;
		}

		uint8_t shift = 1;

		for (uint8_t i = 0; i < 4; ++i)
		{
			if (chars_to_trim[i] == (*prev_pos))
			{
				shift = 0;
				break;
			}
		}

		input_output->finish = prev_pos + shift;
	}

	return 1;
}

uint8_t string_trim(struct range* input_output)
{
	return string_trim_any(input_output, string_trim_mode_all);
}

uint8_t string_trim_end(struct range* input_output)
{
	return string_trim_any(input_output, string_trim_mode_end);
}

uint8_t string_trim_start(struct range* input_output)
{
	return string_trim_any(input_output, string_trim_mode_start);
}

uint8_t string_quote(const char* input_start, const char* input_finish,
					 struct buffer* output)
{
	if (NULL == output)
	{
		return 0;
	}

	if (range_in_parts_is_null_or_empty(input_start, input_finish))
	{
		return buffer_push_back(output, '"') && buffer_push_back(output, '"');
	}

	return buffer_push_back(output, '"') &&
		   buffer_append_char(output, input_start, input_finish - input_start) &&
		   buffer_push_back(output, '"');
}

uint8_t string_un_quote(struct range* input_output)
{
	if (NULL == input_output ||
		NULL == input_output->start ||
		NULL == input_output->finish ||
		input_output->finish < input_output->start)
	{
		return 0;
	}

	input_output->start = find_any_symbol_like_or_not_like_that(
							  input_output->start, input_output->finish, "\"", 1, 0, 1);
	input_output->finish = find_any_symbol_like_or_not_like_that(
							   input_output->finish - 1, input_output->start, "\"", 1, 0, -1);

	if (input_output->start < input_output->finish)
	{
		input_output->finish++;
	}

	return 1;
}

uint8_t string_equal(const char* input_1_start, const char* input_1_finish,
					 const char* input_2_start, const char* input_2_finish)
{
	if (NULL == input_1_start || NULL == input_1_finish ||
		NULL == input_2_start || NULL == input_2_finish ||
		input_1_finish < input_1_start || input_2_finish < input_2_start ||
		input_1_finish - input_1_start != input_2_finish - input_2_start)
	{
		return 0;
	}

	if (0 == input_1_finish - input_1_start)
	{
		return 1;
	}

	if (input_1_start == input_2_start)
	{
		return 1;
	}

	return (0 == memcmp(input_1_start, input_2_start, input_1_finish - input_1_start));
}

static const char* string_function_str[] =
{
	"contains", "ends-with", "get-length", "index-of", "last-index-of",
	"pad-left", "pad-right", "replace", "starts-with", "substring",
	"to-lower", "to-upper", "trim", "trim-end", "trim-start",
	"quote", "un-quote", "equal", "empty"
};

uint8_t string_get_function(const char* name_start, const char* name_finish)
{
	return common_string_to_enum(name_start, name_finish, string_function_str, UNKNOWN_STRING_FUNCTION);
}

uint8_t string_exec_function(uint8_t function,
							 const struct buffer* arguments, uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_STRING_FUNCTION <= function || NULL == arguments || 3 < arguments_count || NULL == output)
	{
		return 0;
	}

	const ptrdiff_t current_output_size = buffer_size(output);
	struct range argument1;
	struct range argument2;
	struct range argument3;
	argument1.start = argument2.start = argument3.start = argument1.finish = argument2.finish = argument3.finish =
											NULL;

	switch (arguments_count)
	{
		case 0:
			break;

		case 1:
			if (!common_get_one_argument(arguments, &argument1, 0))
			{
				/*NOTE: allowed to string unit, where empty string can be as input.*/
				argument1.start = argument1.finish = (const char*)&argument1;
			}

			break;

		case 2:
			if (!common_get_two_arguments(arguments, &argument1, &argument2, substring == function ? 1 : 0))
			{
				if (NULL == argument1.start)
				{
					argument1.start = argument1.finish = (const char*)&argument1;
				}

				if (NULL == argument2.start)
				{
					argument2.start = argument2.finish = (const char*)&argument2;
				}
			}

			break;

		case 3:
			if (!common_get_three_arguments(arguments, &argument1, &argument2, &argument3, substring == function ? 1 : 0))
			{
				if (NULL == argument1.start)
				{
					argument1.start = argument1.finish = (const char*)&argument1;
				}

				if (NULL == argument2.start)
				{
					argument2.start = argument2.finish = (const char*)&argument2;
				}

				if (NULL == argument3.start)
				{
					argument3.start = argument3.finish = (const char*)&argument3;
				}
			}

			break;

		default:
			return 0;
	}

	switch (function)
	{
		case contains:
			return (2 == arguments_count) &&
				   bool_to_string(string_contains(argument1.start, argument1.finish, argument2.start, argument2.finish), output);

		case ends_with:
			return (2 == arguments_count) &&
				   bool_to_string(string_ends_with(argument1.start, argument1.finish, argument2.start, argument2.finish),
								  output);

		case get_length:
			return (1 == arguments_count) &&
				   int64_to_string(string_get_length(argument1.start, argument1.finish), output);

		case index_of:
			return (2 == arguments_count) &&
				   int64_to_string(string_index_of_any(argument1.start, argument1.finish, argument2.start, argument2.finish, 1),
								   output);

		case last_index_of:
			return (2 == arguments_count) &&
				   int64_to_string(string_index_of_any(argument1.start, argument1.finish, argument2.start, argument2.finish, -1),
								   output);

		/*TODO:
		case pad_left:
			break;

		case pad_right:
			break;
		*/

		case replace:
			return (3 == arguments_count) && string_replace(
					   argument1.start, argument1.finish,
					   argument2.start, argument2.finish,
					   argument3.start, argument3.finish,
					   output);

		case starts_with:
			return (2 == arguments_count) &&
				   bool_to_string(string_starts_with(argument1.start, argument1.finish, argument2.start, argument2.finish),
								  output);

		case substring:
		{
			if (2 != arguments_count && 3 != arguments_count)
			{
				break;
			}
			else if (2 == arguments_count && range_is_null_or_empty(&argument2))
			{
				break;
			}
			else if (3 == arguments_count && (range_is_null_or_empty(&argument2) || range_is_null_or_empty(&argument3)))
			{
				break;
			}

			if (range_is_null_or_empty(&argument1))
			{
				return 1;
			}

			const ptrdiff_t input_length = range_size(&argument1) - 1;
			const ptrdiff_t index = (ptrdiff_t)int64_parse(argument2.start);
			const ptrdiff_t length = (3 == arguments_count) ? (ptrdiff_t)int64_parse(argument3.start) :
									 (input_length - index);
			/**/
			return string_substring(argument1.start, input_length, index, length, output);
		}

		case to_lower:
			if (1 != arguments_count || (argument1.start != argument1.finish &&
										 !buffer_append_char(output, NULL, range_size(&argument1))))
			{
				break;
			}

			return string_to_lower(argument1.start, argument1.finish, (char*)buffer_data(output, current_output_size));

		case to_upper:
			if (1 != arguments_count || (argument1.start != argument1.finish &&
										 !buffer_append_char(output, NULL, range_size(&argument1))))
			{
				break;
			}

			return string_to_upper(argument1.start, argument1.finish, (char*)buffer_data(output, current_output_size));

		case trim:
		case trim_end:
		case trim_start:
			return (1 == arguments_count) &&
				   string_trim_any(&argument1, function) &&
				   buffer_append_char(output, argument1.start, range_size(&argument1));

		case quote:
			return (1 == arguments_count) &&
				   string_quote(argument1.start, argument1.finish, output);

		case un_quote:
			return (1 == arguments_count) &&
				   string_un_quote(&argument1) &&
				   buffer_append_data_from_range(output, &argument1);

		case equal:
			return (2 == arguments_count) &&
				   bool_to_string(string_equal(argument1.start, argument1.finish, argument2.start, argument2.finish), output);

		case empty:
			return (1 == arguments_count) && bool_to_string(range_is_null_or_empty(&argument1), output);

		case UNKNOWN_STRING_FUNCTION:
		default:
			break;
	}

	return 0;
}
