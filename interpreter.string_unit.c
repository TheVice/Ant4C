/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "string_unit.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"

enum string_function
{
	contains, ends_with, get_length, index_of, last_index_of,
	index_of_any, last_index_of_any,
	pad_left, pad_right, replace, starts_with, substring,
	to_lower, to_upper, trim, trim_end, trim_start,
	quote, un_quote, equal, empty,
	UNKNOWN_STRING_FUNCTION
};

uint8_t string_get_id_of_to_lower_function()
{
	return to_lower;
}

uint8_t string_get_id_of_to_upper_function()
{
	return to_upper;
}

uint8_t string_get_id_of_trim_function()
{
	return trim;
}

uint8_t string_get_id_of_trim_end_function()
{
	return trim_end;
}

uint8_t string_get_id_of_trim_start_function()
{
	return trim_start;
}

uint8_t string_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* string_function_str[] =
	{
		(const uint8_t*)"contains",
		(const uint8_t*)"ends-with",
		(const uint8_t*)"get-length",
		(const uint8_t*)"index-of",
		(const uint8_t*)"last-index-of",
		(const uint8_t*)"index-of-any",
		(const uint8_t*)"last-index-of-any",
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
	/**/
	return common_string_to_enum(name_start, name_finish, string_function_str, UNKNOWN_STRING_FUNCTION);
}

uint8_t string_exec_function(uint8_t function,
							 const struct buffer* arguments, uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_STRING_FUNCTION <= function || NULL == arguments || 3 < arguments_count || NULL == output)
	{
		return 0;
	}

	struct range values[3];

	if (!common_get_arguments(arguments, arguments_count, values,
							  (substring == function || pad_left == function || pad_right == function) ? 1 : 0))
	{
		return 0;
	}

	for (uint8_t i = 0, count = COUNT_OF(values); i < count; ++i)
	{
		if (NULL == values[i].start)
		{
			values[i].start = values[i].finish = (const uint8_t*)&values[i];
		}
	}

	switch (function)
	{
		case contains:
			return (2 == arguments_count) &&
				   bool_to_string(string_contains(values[0].start, values[0].finish,
												  values[1].start, values[1].finish), output);

		case ends_with:
			return (2 == arguments_count) &&
				   bool_to_string(string_ends_with(values[0].start, values[0].finish,
												   values[1].start, values[1].finish), output);

		case get_length:
			return (1 == arguments_count) &&
				   int64_to_string(string_get_length(values[0].start, values[0].finish), output);

		case index_of:
			return (2 == arguments_count) &&
				   int64_to_string(string_index_of_value(values[0].start, values[0].finish, values[1].start, values[1].finish,
								   1),
								   output);

		case last_index_of:
			return (2 == arguments_count) &&
				   int64_to_string(string_index_of_value(values[0].start, values[0].finish, values[1].start, values[1].finish,
								   -1),
								   output);

		case index_of_any:
			return (2 == arguments_count) &&
				   int64_to_string(string_index_of_any(values[0].start, values[0].finish, values[1].start, values[1].finish),
								   output);

		case last_index_of_any:
			return (2 == arguments_count) &&
				   int64_to_string(string_last_index_of_any(values[0].start, values[0].finish, values[1].start,
								   values[1].finish), output);

		case pad_left:
			return (3 == arguments_count) &&
				   string_pad_left(values[0].start, values[0].finish, values[2].start, values[2].finish,
								   (ptrdiff_t)int64_parse(values[1].start, values[1].finish), output);

		case pad_right:
			return (3 == arguments_count) &&
				   string_pad_right(values[0].start, values[0].finish, values[2].start, values[2].finish,
									(ptrdiff_t)int64_parse(values[1].start, values[1].finish), output);

		case replace:
			return (3 == arguments_count) && string_replace(
					   values[0].start, values[0].finish,
					   values[1].start, values[1].finish,
					   values[2].start, values[2].finish,
					   output);

		case starts_with:
			return (2 == arguments_count) &&
				   bool_to_string(string_starts_with(values[0].start, values[0].finish, values[1].start, values[1].finish),
								  output);

		case substring:
		{
			if (2 != arguments_count && 3 != arguments_count)
			{
				break;
			}
			else if (2 == arguments_count && range_is_null_or_empty(&values[1]))
			{
				break;
			}
			else if (3 == arguments_count && (range_is_null_or_empty(&values[1]) || range_is_null_or_empty(&values[2])))
			{
				break;
			}

			if (range_is_null_or_empty(&values[0]))
			{
				return 1;
			}

			const ptrdiff_t index = (ptrdiff_t)int64_parse(values[1].start, values[1].finish);
			ptrdiff_t length = -1;

			if (3 == arguments_count)
			{
				length = (ptrdiff_t)int64_parse(values[2].start, values[2].finish);
			}

			return string_substring(values[0].start, values[0].finish, index, length, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);
		}

		case to_lower:
		case to_upper:
			return (1 == arguments_count) &&
				   string_transform_to_case(values[0].start, values[0].finish, output, function);

		case trim:
		case trim_end:
		case trim_start:
			return (1 == arguments_count) &&
				   string_trim_any(&values[0], function, /*TODO:*/NULL, 0) &&
				   buffer_append(output, values[0].start, range_size(&values[0]));

		case quote:
			return (1 == arguments_count) &&
				   string_quote(values[0].start, values[0].finish, output);

		case un_quote:
			return (1 == arguments_count) &&
				   string_un_quote(&values[0]) &&
				   buffer_append_data_from_range(output, &values[0]);

		case equal:
			return (2 == arguments_count) &&
				   bool_to_string(string_equal(values[0].start, values[0].finish, values[1].start, values[1].finish), output);

		case empty:
			return (1 == arguments_count) && bool_to_string(range_is_null_or_empty(&values[0]), output);

		case UNKNOWN_STRING_FUNCTION:
		default:
			break;
	}

	return 0;
}
