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

static const uint8_t quote_symbol = '"';

ptrdiff_t string_index_of_any(const uint8_t* input_start, const uint8_t* input_finish,
							  const uint8_t* value_start, const uint8_t* value_finish, int8_t step)
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

uint8_t string_contains(const uint8_t* input_start, const uint8_t* input_finish,
						const uint8_t* value_start, const uint8_t* value_finish)
{
	return -1 != string_index_of_any(input_start, input_finish, value_start, value_finish, 1);
}

uint8_t string_ends_with(const uint8_t* input_start, const uint8_t* input_finish,
						 const uint8_t* value_start, const uint8_t* value_finish)
{
	ptrdiff_t index = 0;

	if (-1 == (index = string_index_of_any(input_start, input_finish, value_start, value_finish, 1)))
	{
		return 0;
	}

	const ptrdiff_t expected_index = (input_finish - input_start) - (value_finish - value_start);
	return expected_index == index;
}

ptrdiff_t string_get_length(const uint8_t* input_start, const uint8_t* input_finish)
{
	if (input_start == input_finish)
	{
		return 0;
	}

	if (NULL == input_start || NULL == input_finish || input_finish < input_start)
	{
		return -1;
	}

	/*FIXME: encode from bytes into string and calculate string length. Current form correct for ASCII.*/
	return input_finish - input_start;
}

ptrdiff_t string_index_of(const uint8_t* input_start, const uint8_t* input_finish,
						  const uint8_t* value_start, const uint8_t* value_finish)
{
	return string_index_of_any(input_start, input_finish, value_start, value_finish, 1);
}

ptrdiff_t string_last_index_of(const uint8_t* input_start, const uint8_t* input_finish,
							   const uint8_t* value_start, const uint8_t* value_finish)
{
	return string_index_of_any(input_start, input_finish, value_start, value_finish, -1);
}
/*TODO:string_pad_left
string_pad_right*/
uint8_t string_replace(const uint8_t* input_start, const uint8_t* input_finish,
					   const uint8_t* to_be_replaced_start, const uint8_t* to_be_replaced_finish,
					   const uint8_t* by_replacement_start, const uint8_t* by_replacement_finish,
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

	if (!buffer_append(output, input_start, input_length))
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
			if (to_be_replaced_start[0] == *(buffer_data(output, size + i)))
			{
				*(buffer_data(output, size + i)) = by_replacement_start[0];
			}
		}

		return 1;
	}

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	const uint8_t* previous_position = input_start;
	ptrdiff_t index = -1;

	while (-1 != (index = string_index_of_any(
							  previous_position, input_finish, to_be_replaced_start, to_be_replaced_finish, 1)))
	{
		if (!buffer_append(output, previous_position, index))
		{
			return 0;
		}

		if (-1 != by_replacement_length)
		{
			if (!buffer_append(output, by_replacement_start, by_replacement_length))
			{
				return 0;
			}
		}

		previous_position += index + to_be_replaced_length;
	}

	if (!buffer_append(output, previous_position, input_finish - previous_position))
	{
		return 0;
	}

	return 1;
}

uint8_t string_starts_with(const uint8_t* input_start, const uint8_t* input_finish,
						   const uint8_t* value_start, const uint8_t* value_finish)
{
	return 0 == string_index_of_any(input_start, input_finish, value_start, value_finish, 1);
}

uint8_t string_substring(const uint8_t* input, ptrdiff_t input_length,
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

	/*FIXME: encode from bytes into string and substring string. Current form correct for ASCII.*/
	return buffer_append(output, &input[index], length);
}

uint8_t string_to_lower(const uint8_t* input_start, const uint8_t* input_finish, uint8_t* output)
{
	if (NULL == input_start || NULL == input_finish || NULL == output ||
		input_finish <= input_start)
	{
		return 0;
	}

	const uint8_t* pos = input_start;
	/*TODO: encode may required.*/

	while (input_finish != pos)
	{
		*output = (uint8_t)tolower(*pos);
		++pos;
		++output;
	}

	return 1;
}

uint8_t string_to_upper(const uint8_t* input_start, const uint8_t* input_finish, uint8_t* output)
{
	if (NULL == input_start || NULL == input_finish || NULL == output ||
		input_finish <= input_start)
	{
		return 0;
	}

	const uint8_t* pos = input_start;
	/*TODO: encode may required.*/

	while (input_finish != pos)
	{
		*output = (uint8_t)toupper(*pos);
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
	static const uint8_t trim_symbols[] = { '\0', '\t', ' ', '\n' };

	if (NULL != input_output && input_output->start == input_output->finish)
	{
		return 1;
	}

	if (NULL == input_output ||
		NULL == input_output->start ||
		NULL == input_output->finish ||
		input_output->finish < input_output->start ||
		(string_trim_mode_all != mode && string_trim_mode_end != mode && string_trim_mode_start != mode))
	{
		return 0;
	}

	if (string_trim_mode_all == mode || string_trim_mode_start == mode)
	{
		input_output->start = find_any_symbol_like_or_not_like_that(
								  input_output->start, input_output->finish, trim_symbols, 4, 0, 1);
	}

	if (input_output->start < input_output->finish &&
		(string_trim_mode_all == mode || string_trim_mode_end == mode))
	{
		const uint8_t* pos = input_output->start;
		const uint8_t* prev_pos = pos;

		while (input_output->finish > (pos = find_any_symbol_like_or_not_like_that(
				pos, input_output->finish, trim_symbols, 4, 0, 1)))
		{
			prev_pos = pos;
			++pos;
		}

		uint8_t shift = 1;

		for (uint8_t i = 0; i < 4; ++i)
		{
			if (trim_symbols[i] == (*prev_pos))
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

uint8_t string_quote(const uint8_t* input_start, const uint8_t* input_finish,
					 struct buffer* output)
{
	if (NULL == output)
	{
		return 0;
	}

	if (range_in_parts_is_null_or_empty(input_start, input_finish))
	{
		return buffer_push_back(output, quote_symbol) && buffer_push_back(output, quote_symbol);
	}

	return buffer_push_back(output, quote_symbol) &&
		   buffer_append(output, input_start, input_finish - input_start) &&
		   buffer_push_back(output, quote_symbol);
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
							  input_output->start, input_output->finish, &quote_symbol, 1, 0, 1);
	input_output->finish = find_any_symbol_like_or_not_like_that(
							   input_output->finish - 1, input_output->start, &quote_symbol, 1, 0, -1);

	if (input_output->start < input_output->finish)
	{
		input_output->finish++;
	}

	return 1;
}

uint8_t string_equal(const uint8_t* input_1_start, const uint8_t* input_1_finish,
					 const uint8_t* input_2_start, const uint8_t* input_2_finish)
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

static const uint8_t* string_function_str[] =
{
	(const uint8_t*)"contains",
	(const uint8_t*)"ends-with",
	(const uint8_t*)"get-length",
	(const uint8_t*)"index-of",
	(const uint8_t*)"last-index-of",
	(const uint8_t*)"pad-left",
	(const uint8_t*)"pad-right",
	(const uint8_t*)"replace",
	(const uint8_t*)"starts-with",
	(const uint8_t*)"substring",
	(const uint8_t*)"to-lower",
	(const uint8_t*)"to-upper",
	(const uint8_t*)"trim",
	(const uint8_t*)"trim-end",
	(const uint8_t*)"trim-start",
	(const uint8_t*)"quote",
	(const uint8_t*)"un-quote",
	(const uint8_t*)"equal",
	(const uint8_t*)"empty"
};

uint8_t string_get_function(const uint8_t* name_start, const uint8_t* name_finish)
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
				argument1.start = argument1.finish = (const uint8_t*)&argument1;
			}

			break;

		case 2:
			if (!common_get_two_arguments(arguments, &argument1, &argument2, substring == function ? 1 : 0))
			{
				if (NULL == argument1.start)
				{
					argument1.start = argument1.finish = (const uint8_t*)&argument1;
				}

				if (NULL == argument2.start)
				{
					argument2.start = argument2.finish = (const uint8_t*)&argument2;
				}
			}

			break;

		case 3:
			if (!common_get_three_arguments(arguments, &argument1, &argument2, &argument3, substring == function ? 1 : 0))
			{
				if (NULL == argument1.start)
				{
					argument1.start = argument1.finish = (const uint8_t*)&argument1;
				}

				if (NULL == argument2.start)
				{
					argument2.start = argument2.finish = (const uint8_t*)&argument2;
				}

				if (NULL == argument3.start)
				{
					argument3.start = argument3.finish = (const uint8_t*)&argument3;
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
										 !buffer_append(output, NULL, range_size(&argument1))))
			{
				break;
			}

			return string_to_lower(argument1.start, argument1.finish, buffer_data(output, current_output_size));

		case to_upper:
			if (1 != arguments_count || (argument1.start != argument1.finish &&
										 !buffer_append(output, NULL, range_size(&argument1))))
			{
				break;
			}

			return string_to_upper(argument1.start, argument1.finish, buffer_data(output, current_output_size));

		case trim:
		case trim_end:
		case trim_start:
			return (1 == arguments_count) &&
				   string_trim_any(&argument1, function) &&
				   buffer_append(output, argument1.start, range_size(&argument1));

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
