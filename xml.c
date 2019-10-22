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

uint8_t go_to_comment_end_if_it_exists(const char** start, const char* finish)
{
	static const char* comment_start = "<!--";
	static const uint8_t comment_start_length = 4;
	const char* start_ = NULL;

	if (NULL == start || NULL == finish ||
		NULL == (start_ = *start) || finish <= start_)
	{
		return 0;
	}

	if (comment_start_length < finish - start_ &&
		string_equal(start_, start_ + comment_start_length, comment_start, comment_start + comment_start_length))
	{
		static const char* comment_end = "-->";
		static const uint8_t comment_end_length = 3;
		/**/
		start_ += comment_start_length;
		const ptrdiff_t index = string_index_of(start_, finish, comment_end, comment_end + comment_end_length);

		if (-1 == index)
		{
			return 0;
		}

		start_ += index + comment_end_length;

		if (finish == (start_ = find_any_symbol_like_or_not_like_that(start_, finish, "<", 1, 1, 1)))
		{
			return 0;
		}

		*start = start_;
		return go_to_comment_end_if_it_exists(start, finish);
	}

	return finish != start_;
}

const char* xml_get_tag_finish_pos(const char* start, const char* finish)
{
	if (range_in_parts_is_null_or_empty(start, finish))
	{
		return finish;
	}

	while (start != finish)
	{
		if ('>' == *start)
		{
			break;
		}
		else if ('"' == *start)
		{
			start = find_any_symbol_like_or_not_like_that(start + 1, finish, "\"", 1, 1, 1);

			if (start == finish)
			{
				break;
			}
		}

		++start;
	}

	return start;
}

uint16_t xml_get_sub_nodes_elements(const char* start, const char* finish, struct buffer* elements)
{
	static const char tag_close = '/';

	if (NULL == start || NULL == finish || finish < start || NULL == elements)
	{
		return 0;
	}

	uint16_t count = 0;
	uint8_t depth = 0;
	const char* pos = start;

	while (finish != pos)
	{
		while (finish != pos)
		{
			if ('<' == *pos)
			{
				break;
			}

			++pos;
		}

		if (!go_to_comment_end_if_it_exists(&pos, finish))
		{
			return count;
		}
		else
		{
			++pos;
		}

		const char* tag_finish_pos = xml_get_tag_finish_pos(pos, finish);

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

			const char* tag_finish_prev_pos = tag_finish_pos;
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

uint8_t xml_get_tag_name(const char* start, const char* finish, struct range* name)
{
	static const char tab_space_close_tag[] = { '\t', ' ', '/', '>', '\r', '\n' };

	if (NULL == start || NULL == finish || NULL == name || finish < start)
	{
		return 0;
	}

	name->start = start;
	name->finish = find_any_symbol_like_or_not_like_that(start, finish, tab_space_close_tag, 6, 1, 1);
	return start < name->finish;
}

uint8_t xml_get_attribute_value(const char* start, const char* finish,
								const char* attribute, ptrdiff_t attribute_length, struct range* value)
{
	if (range_in_parts_is_null_or_empty(start, finish) || NULL == attribute ||
		0 == attribute_length || NULL == value || (finish - start) < attribute_length)
	{
		return 0;
	}

	const char* pos = start;

	while (finish != (pos = (find_any_symbol_like_or_not_like_that(pos, finish, "=", 1, 1, 1))))
	{
		const char* key_finish = find_any_symbol_like_or_not_like_that(pos, start, "= \t", 3, 0, -1);
		const char* key_start = find_any_symbol_like_or_not_like_that(key_finish, start, " \t", 2, 1, -1);
		++key_finish;
		const char ch = *key_start;

		if (' ' == ch || '\t' == ch)
		{
			++key_start;
		}

		if (string_equal(key_start, key_finish, attribute, attribute + attribute_length))
		{
			value->start = find_any_symbol_like_or_not_like_that(pos, finish, "\"", 1, 1, 1);

			if (finish == value->start)
			{
				break;
			}

			++value->start;
			value->finish = find_any_symbol_like_or_not_like_that(value->start, finish, "\"", 1, 1, 1);
			return 1;
		}

		++pos;
	}

	return 0;
}

uint8_t xml_is_attribute_exists(const char* start, const char* finish, const char* attribute,
								ptrdiff_t attribute_length)
{
	struct range attribute_value;
	return xml_get_attribute_value(start, finish, attribute, attribute_length, &attribute_value);
}

uint8_t xml_get_element_value_from_parts(const char* element_start, const char* element_finish,
		struct range* value)
{
	if (range_in_parts_is_null_or_empty(element_start, element_finish) || NULL == value)
	{
		return 0;
	}

	value->start = find_any_symbol_like_or_not_like_that(element_start, element_finish, ">", 1, 1, 1);
	value->finish = element_finish;

	if (value->finish == value->start)
	{
		return 0;
	}

	if (element_start < value->start && ('/' == *(value->start - 1)))
	{
		value->finish = value->start;
	}
	else
	{
		++value->start;
		value->finish = find_any_symbol_like_or_not_like_that(value->finish, value->start, "<", 1, 1, -1);
	}

	return value->start <= value->finish;
}

uint8_t xml_get_element_value(const struct range* element, struct range* value)
{
	if (range_is_null_or_empty(element) || NULL == value)
	{
		return 0;
	}

	return xml_get_element_value_from_parts(element->start, element->finish, value);
}