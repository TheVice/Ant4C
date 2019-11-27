/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "xml.h"
#include "buffer.h"
#include "common.h"
#include "range.h"
#include "string_unit.h"

#include <stdlib.h>

static const uint8_t tag_close = '/';

static const uint8_t* predefined_entities[] =
{
	(const uint8_t*)"&lt;",
	(const uint8_t*)"&gt;",
	(const uint8_t*)"&amp;",
	(const uint8_t*)"&apos;",
	(const uint8_t*)"&quot;"
};
static const uint8_t predefined_lengths[] =
{
	4, 4, 5, 6, 6
};
static const uint8_t characters[] = { '<', '>', '&', '\'', '"' };

#define LESS_POSITION		0
#define GREATER_POSITION	1
#define AMPERSAND_POSITION	2
#define QUOTE_POSITION		4

uint8_t xml_skip_comment(const uint8_t** start, const uint8_t* finish)
{
	static const uint8_t* comment_start = (const uint8_t*)"<!--";
	static const uint8_t comment_start_length = 4;
	/**/
	const uint8_t* start_ = NULL;

	if (NULL == start || NULL == finish ||
		NULL == (start_ = *start) || finish <= start_)
	{
		return 0;
	}

	if ((ptrdiff_t)comment_start_length < finish - start_ &&
		string_equal(start_, start_ + comment_start_length, comment_start, comment_start + comment_start_length))
	{
		static const uint8_t* comment_end = (const uint8_t*)"-->";
		static const uint8_t comment_end_length = 3;
		/**/
		start_ += comment_start_length;
		const ptrdiff_t index = string_index_of(start_, finish, comment_end, comment_end + comment_end_length);

		if (-1 == index)
		{
			return 0;
		}

		start_ += index + comment_end_length;

		if (finish == (start_ = find_any_symbol_like_or_not_like_that(start_, finish, &(characters[LESS_POSITION]), 1,
								1, 1)))
		{
			return 0;
		}

		*start = start_;
		return xml_skip_comment(start, finish);
	}

	return finish != start_;
}

const uint8_t* xml_get_tag_finish_pos(const uint8_t* start, const uint8_t* finish)
{
	if (range_in_parts_is_null_or_empty(start, finish))
	{
		return finish;
	}

	while (start != finish)
	{
		if (characters[GREATER_POSITION] == *start)
		{
			break;
		}
		else if (characters[QUOTE_POSITION] == *start)
		{
			start = find_any_symbol_like_or_not_like_that(start + 1, finish, &(characters[QUOTE_POSITION]), 1, 1, 1);

			if (start == finish)
			{
				break;
			}
		}

		++start;
	}

	return start;
}

uint16_t xml_get_sub_nodes_elements(const uint8_t* start, const uint8_t* finish, struct buffer* elements)
{
	if (NULL == start || NULL == finish || finish < start || NULL == elements)
	{
		return 0;
	}

	uint16_t count = 0;
	uint8_t depth = 0;
	const uint8_t* pos = start;

	while (finish != pos)
	{
		while (finish != pos)
		{
			if (characters[LESS_POSITION] == *pos)
			{
				break;
			}

			++pos;
		}

		if (!xml_skip_comment(&pos, finish))
		{
			return count;
		}
		else
		{
			++pos;
		}

		const uint8_t* tag_finish_pos = xml_get_tag_finish_pos(pos, finish);

		if ('?' == (*pos))
		{
			pos = tag_finish_pos;
			continue;
		}

		if (tag_close == (*pos))
		{
			if (0 == depth)
			{
				break;
			}

			--depth;
		}
		else
		{
			if (0 == depth)
			{
				struct range element;
				element.start = pos;
				element.finish = tag_finish_pos;

				if (!buffer_append_range(elements, &element, 1))
				{
					return 0;
				}

				if (count < UINT16_MAX)
				{
					++count;
				}
				else
				{
					return 0;
				}
			}

			const uint8_t* tag_finish_prev_pos = tag_finish_pos;
			--tag_finish_prev_pos;

			if (tag_close != (*tag_finish_prev_pos))
			{
				if (depth < UINT8_MAX)
				{
					++depth;
				}
				else
				{
					return 0;
				}
			}
		}

		if (0 == depth)
		{
			struct range* element = buffer_range_data(elements, count - 1);
			element->finish = tag_finish_pos;
		}

		pos = tag_finish_pos;
	}

	return count;
}

uint8_t xml_get_tag_name(const uint8_t* start, const uint8_t* finish, struct range* name)
{
	static const uint8_t tab_space_close_tag[] = { '\t', ' ', '/', '>', '\r', '\n' };

	if (NULL == start || NULL == finish || NULL == name || finish < start)
	{
		return 0;
	}

	name->start = start;
	name->finish = find_any_symbol_like_or_not_like_that(start, finish, tab_space_close_tag, 6, 1, 1);
	return start < name->finish;
}

uint8_t xml_read_ampersand_based_data(const uint8_t* start, const uint8_t* finish, struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, finish - start) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	const uint8_t* previous_pos = start;

	while (finish != (start = find_any_symbol_like_or_not_like_that(start, finish,
							  &(characters[AMPERSAND_POSITION]), 1, 1, 1)))
	{
		if (!buffer_append(output, previous_pos, start - previous_pos))
		{
			return 0;
		}

		static const uint8_t* hex_value_start = (const uint8_t*)"&#x";
		static const uint8_t* hex_value_start_upper = (const uint8_t*)"&#X";
		static const uint8_t* decimal_value_start = (const uint8_t*)"&#";
		static const uint8_t* skip_to_digit = (const uint8_t*)"&#xX";

		if (string_starts_with(start, finish, hex_value_start, hex_value_start + 3) ||
			string_starts_with(start, finish, hex_value_start_upper, hex_value_start_upper + 3) ||
			string_starts_with(start, finish, decimal_value_start, decimal_value_start + 2))
		{
			start = find_any_symbol_like_or_not_like_that(start + 2, finish, skip_to_digit, 4, 0, 1);
			const long val = strtol((const char*)start, NULL, '#' == (*(start - 1)) ? 10 : 16);

			if (val < 0)
			{
				if (!buffer_push_back(output, 0))
				{
					return 0;
				}
			}
			else if (val < UINT8_MAX + 1)
			{
				if (!buffer_push_back(output, (uint8_t)val))
				{
					return 0;
				}
			}
			else if (val < UINT16_MAX + 1)
			{
				if (!buffer_push_back_uint16(output, (uint16_t)val))
				{
					return 0;
				}
			}
			else
			{
				if (!buffer_push_back_uint32(output, (uint32_t)val))
				{
					return 0;
				}
			}
		}
		else
		{
			for (uint8_t i = 0, count = COUNT_OF(characters); i < count; ++i)
			{
				if (string_starts_with(start, finish, predefined_entities[i], predefined_entities[i] + predefined_lengths[i]))
				{
					if (!buffer_push_back(output, characters[i]))
					{
						return 0;
					}

					start += predefined_lengths[i] - 1;
					break;
				}
			}

			if (characters[AMPERSAND_POSITION] == (*start))
			{
				if (!buffer_push_back(output, characters[AMPERSAND_POSITION]))
				{
					return 0;
				}

				++start;
				previous_pos = start;
				continue;
			}
		}

		static const uint8_t semicolon = ';';
		start = 1 + find_any_symbol_like_or_not_like_that(start, finish, &semicolon, 1, 1, 1);
		previous_pos = start;
	}

	return previous_pos < finish ? buffer_append(output, previous_pos, finish - previous_pos) : 1;
}

uint8_t xml_get_attribute_value(const uint8_t* start, const uint8_t* finish,
								const uint8_t* attribute, ptrdiff_t attribute_length, struct buffer* value)
{
	static const uint8_t equal_symbol = '=';

	if (range_in_parts_is_null_or_empty(start, finish) || NULL == attribute ||
		0 == attribute_length || (finish - start) < attribute_length)
	{
		return 0;
	}

	const uint8_t* pos = start;

	while (finish != (pos = (find_any_symbol_like_or_not_like_that(pos, finish, &equal_symbol, 1, 1, 1))))
	{
		static const uint8_t* equal_space_tab = (const uint8_t*)"= \t";
		static const uint8_t* space_tab = (const uint8_t*)" \t";
		static const uint8_t* double_quote = (const uint8_t*)"\"\"";
		/**/
		const uint8_t* key_finish = find_any_symbol_like_or_not_like_that(pos, start, equal_space_tab, 3, 0, -1);
		const uint8_t* key_start = find_any_symbol_like_or_not_like_that(key_finish, start, space_tab, 2, 1, -1);
		/**/
		key_start = find_any_symbol_like_or_not_like_that(key_start, key_finish, space_tab, 2, 0, 1);
		key_finish = find_any_symbol_like_or_not_like_that(key_finish, pos, equal_space_tab, 2, 1, 1);
		/**/
		++pos;

		if (!string_equal(key_start, key_finish, attribute, attribute + attribute_length))
		{
			continue;
		}

		if (NULL == value)
		{
			return 1;
		}

		pos = find_any_symbol_like_or_not_like_that(pos, finish, &(characters[QUOTE_POSITION]), 1, 1, 1);

		if (finish != pos && string_starts_with(pos, finish, double_quote, double_quote + 2))
		{
			return 1;
		}

		pos = find_any_symbol_like_or_not_like_that(pos + 1, finish, &(characters[QUOTE_POSITION]), 1, 0, 1);
		key_finish = find_any_symbol_like_or_not_like_that(pos + 1, finish, &(characters[QUOTE_POSITION]), 1, 1, 1);
		/**/
		return xml_read_ampersand_based_data(pos, key_finish, value);
	}

	return 0;
}

uint8_t xml_get_element_value(const uint8_t* element_start, const uint8_t* element_finish,
							  struct buffer* value)
{
	if (range_in_parts_is_null_or_empty(element_start, element_finish) || NULL == value)
	{
		return 0;
	}

#if TODO
	value->start = find_any_symbol_like_or_not_like_that(element_start, element_finish,
				   &(characters[GREATER_POSITION]), 1, 1, 1);
	value->finish = element_finish;

	if (value->finish == value->start)
	{
		return 0;
	}

	if (element_start < value->start && (tag_close == *(value->start - 1)))
	{
		value->finish = value->start;
	}
	else
	{
		++value->start;
		value->finish = find_any_symbol_like_or_not_like_that(value->finish, value->start,
						&(characters[LESS_POSITION]), 1, 1, -1);
	}

	return value->start <= value->finish;
#endif
	return 0;
}
