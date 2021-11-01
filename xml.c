/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "xml.h"
#include "buffer.h"
#include "common.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include <stdlib.h>
#include <string.h>

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

static const uint8_t* cdata_start = (const uint8_t*)"<![CDATA[";
static const uint8_t cdata_start_length = 9;

static const uint8_t* cdata_finish = (const uint8_t*)"]]>";
static const uint8_t cdata_finish_length = 3;

#define LESS_POSITION		0
#define GREATER_POSITION	1
#define AMPERSAND_POSITION	2
#define QUOTE_POSITION		4

const uint8_t* xml_skip_comment(const uint8_t* start, const uint8_t* finish)
{
	static const uint8_t* comment_start = (const uint8_t*)"<!--";
	static const uint8_t comment_start_length = 4;
	/**/
	static const uint8_t* comment_finish = (const uint8_t*)"-->";
	static const uint8_t comment_finish_length = 3;

	if (range_in_parts_is_null_or_empty(start, finish))
	{
		return NULL;
	}

	for (uint8_t i = 0; i < 2; ++i)
	{
		const uint8_t* tag_start = i ? cdata_start : comment_start;
		const uint8_t tag_start_length = i ? cdata_start_length : comment_start_length;
		/**/
		const uint8_t* tag_finish = i ? cdata_finish : comment_finish;

		if (!string_starts_with(start, finish, tag_start, tag_start + tag_start_length))
		{
			continue;
		}

		start += tag_start_length;
		uint8_t found = 0;

		while (NULL != start && comment_finish_length <= finish - start)
		{
			if (memcmp(start, tag_finish, comment_finish_length))
			{
				start = string_enumerate(start, finish, NULL);
				continue;
			}

			found = 1;
			break;
		}

		if (!found)
		{
			return NULL;
		}

		start += comment_finish_length;
		static const uint8_t* that = &characters[LESS_POSITION];
		start = string_find_any_symbol_like_or_not_like_that(
					start, finish, that, that + 1, 1, 1);

		if (finish == start)
		{
			return NULL;
		}

		return xml_skip_comment(start, finish);
	}

	return start;
}

const uint8_t* xml_get_tag_finish_pos(
	const uint8_t* start, const uint8_t* finish)
{
	if (range_in_parts_is_null_or_empty(start, finish))
	{
		return finish;
	}

	while (NULL != start && start < finish)
	{
		if (characters[GREATER_POSITION] == *start)
		{
			break;
		}
		else if (characters[QUOTE_POSITION] == *start)
		{
			start = string_enumerate(start, finish, NULL);

			if (!start)
			{
				return NULL;
			}

			static const uint8_t* that = &characters[QUOTE_POSITION];
			start = string_find_any_symbol_like_or_not_like_that(
						start, finish, that, that + 1, 1, 1);

			if (start == finish)
			{
				break;
			}
		}
		else if (characters[LESS_POSITION] == *start)
		{
			return NULL;
		}

		start = string_enumerate(start, finish, NULL);
	}

	return start;
}

uint16_t xml_get_sub_nodes_elements_(const uint8_t* start, const uint8_t* finish,
									 const struct buffer* sub_nodes_names, struct buffer* elements)
{
	struct buffer r;
	SET_NULL_TO_BUFFER(r);
	const struct range* sub_node_name = NULL;
	ptrdiff_t i = 0;

	while (NULL != (sub_node_name = buffer_range_data(sub_nodes_names, i++)))
	{
		if (!buffer_append(&r, sub_node_name->start, range_size(sub_node_name)) ||
			!buffer_push_back(&r, 0))
		{
			buffer_release(&r);
			return 0;
		}
	}

	struct range R;

	BUFFER_TO_RANGE(R, &r);

	i = xml_get_sub_nodes_elements(start, finish, &R, elements);

	buffer_release(&r);

	return (uint16_t)i;
}

uint16_t xml_get_sub_nodes_elements(
	const uint8_t* start, const uint8_t* finish,
	const struct range* sub_nodes_names, struct buffer* elements)
{
	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == elements)
	{
		return 0;
	}

	uint16_t count = 0;
	uint8_t depth = 0;

	while (NULL != start && finish != start)
	{
		static const uint8_t question_mark = '?';
		static const uint8_t* that = &characters[LESS_POSITION];
		start = string_find_any_symbol_like_or_not_like_that(
					start, finish, that, that + 1, 1, 1);

		if (NULL == (start = xml_skip_comment(start, finish)))
		{
			return 0 == depth ? count : 0;
		}

		start = string_enumerate(start, finish, NULL);
		const uint8_t* tag_finish_pos = xml_get_tag_finish_pos(start, finish);

		if (!tag_finish_pos)
		{
			return 0;
		}

		if (question_mark == *start)
		{
			start = tag_finish_pos;
			continue;
		}

		if (tag_close == *start)
		{
			if (!depth)
			{
				break;
			}

			--depth;
		}
		else
		{
			if (!depth)
			{
				if (count < UINT16_MAX &&
					buffer_append_range(elements, NULL, 1))
				{
					++count;
				}
				else
				{
					return 0;
				}

				struct range* element = buffer_range_data(elements, count - 1);

				if (!element)
				{
					return 0;
				}

				element->start = start;
				element->finish = tag_finish_pos;
			}

			const uint8_t* tag_finish_pos_prev =
				string_find_any_symbol_like_or_not_like_that(
					tag_finish_pos, start, &tag_close, &tag_close + 1, 0, -1);

			if (1 == string_get_length(tag_finish_pos_prev, tag_finish_pos) &&
				tag_close != *tag_finish_pos_prev)
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

		if (!depth)
		{
			struct range* element = buffer_range_data(elements, count - 1);
			element->finish = tag_finish_pos;

			if (!range_is_null_or_empty(sub_nodes_names))
			{
				const uint8_t* tag_name_finish = element->finish;

				if (!xml_get_tag_name(element->start, &tag_name_finish))
				{
					return 0;
				}

				uint8_t found = 0;
				const uint8_t* sub_node_name_start = sub_nodes_names->start;
				const uint8_t* sub_node_name_finish = sub_nodes_names->start;
				static const uint8_t zero = '\0';

				while (sub_nodes_names->finish != (sub_node_name_finish = string_find_any_symbol_like_or_not_like_that(
													   sub_node_name_finish, sub_nodes_names->finish, &zero, &zero + 1, 1, 1)))
				{
					if (string_equal(element->start, tag_name_finish, sub_node_name_start, sub_node_name_finish))
					{
						found = 1;
						break;
					}

					sub_node_name_finish = string_find_any_symbol_like_or_not_like_that(
											   sub_node_name_finish, sub_nodes_names->finish, &zero, &zero + 1, 0, 1);
					sub_node_name_start = sub_node_name_finish;
				}

				if (!found)
				{
					if (!buffer_resize(elements, buffer_size(elements) - sizeof(struct range)))
					{
						return 0;
					}

					--count;
				}
			}
		}

		start = tag_finish_pos;
	}

	return 0 == depth ? count : 0;
}

uint8_t xml_get_tag_name(const uint8_t* start, const uint8_t** finish)
{
	static const uint8_t tab_space_close_tag[] = { '\t', ' ', '/', '>', '\r', '\n' };

	if (!finish || range_in_parts_is_null_or_empty(start, *finish))
	{
		return 0;
	}

	*finish = string_find_any_symbol_like_or_not_like_that(
				  start, *finish, tab_space_close_tag, tab_space_close_tag + 6, 1, 1);
	return start < *finish;
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

	struct buffer digits;

	SET_NULL_TO_BUFFER(digits);

	if (!buffer_resize(&digits, (finish - start) / 3) ||
		!buffer_resize(&digits, 0))
	{
		buffer_release(&digits);
		return 0;
	}

	const uint8_t* previous_pos = start;

	while (finish != (start = find_any_symbol_like_or_not_like_that(start, finish,
							  &(characters[AMPERSAND_POSITION]), 1, 1, 1)))
	{
		if (previous_pos < start && buffer_size(&digits))
		{
#if 0

			if (BigEndian)
			{
				...
			}
			else
			{
#endif

				if (!text_encoding_UTF16LE_to_UTF8(
						buffer_uint16_data(&digits, 0),
						(const uint16_t*)(buffer_data(&digits, 0) + buffer_size(&digits)), output) ||
					!buffer_resize(&digits, 0))
				{
					buffer_release(&digits);
					return 0;
				}

#if 0
			}

#endif
		}

		if (previous_pos < start && !buffer_append(output, previous_pos, start - previous_pos))
		{
			buffer_release(&digits);
			return 0;
		}

		static const uint8_t* hex_value_start = (const uint8_t*)"&#x";
		static const uint8_t* hex_value_start_upper = (const uint8_t*)"&#X";
		static const uint8_t* decimal_value_start = (const uint8_t*)"&#";

		if (string_starts_with(start, finish, hex_value_start, hex_value_start + 3) ||
			string_starts_with(start, finish, hex_value_start_upper, hex_value_start_upper + 3) ||
			string_starts_with(start, finish, decimal_value_start, decimal_value_start + 2))
		{
			static const uint8_t* skip_to_digit = (const uint8_t*)"&#xX";
			static const uint8_t number_sign = '#';
			start = find_any_symbol_like_or_not_like_that(start + 2, finish, skip_to_digit, 4, 0, 1);
			const long value = strtol((const char*)start, NULL, number_sign == (*(start - 1)) ? 10 : 16);

			if ((value < 0 || UINT16_MAX < value) ||
				!buffer_push_back_uint16(&digits, (uint16_t)value))
			{
				buffer_release(&digits);
				return 0;
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
						buffer_release(&digits);
						return 0;
					}

					start += predefined_lengths[i] - 1;
					break;
				}
			}

			if (characters[AMPERSAND_POSITION] == *start)
			{
				if (!buffer_push_back(output, characters[AMPERSAND_POSITION]))
				{
					buffer_release(&digits);
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

	if (buffer_size(&digits))
	{
#if 0

		if (BigEndian)
		{
			...
		}
		else
		{
#endif

			if (!text_encoding_UTF16LE_to_UTF8(
					buffer_uint16_data(&digits, 0),
					(const uint16_t*)(buffer_data(&digits, 0) + buffer_size(&digits)), output) ||
				!buffer_resize(&digits, 0))
			{
				buffer_release(&digits);
				return 0;
			}

#if 0
		}

#endif
	}

	buffer_release(&digits);
	return previous_pos < finish ? buffer_append(output, previous_pos, finish - previous_pos) : 1;
}

uint8_t xml_get_attribute_value(const uint8_t* start, const uint8_t* finish,
								const uint8_t* attribute, ptrdiff_t attribute_length, struct buffer* value)
{
	static const uint8_t equal_symbol = '=';

	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == attribute ||
		0 == attribute_length ||
		(finish - start) < attribute_length)
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
		key_finish = find_any_symbol_like_or_not_like_that(key_finish, pos, equal_space_tab, 3, 1, 1);
		/**/
		++pos;

		if (!string_equal(key_start, key_finish, attribute, attribute + attribute_length))
		{
			continue;
		}

		pos = find_any_symbol_like_or_not_like_that(pos, finish, &(characters[QUOTE_POSITION]), 1, 1, 1);

		if (finish != pos && string_starts_with(pos, finish, double_quote, double_quote + 2))
		{
			return 1;
		}

		pos = find_any_symbol_like_or_not_like_that(pos + 1, finish, &(characters[QUOTE_POSITION]), 1, 0, 1);

		if (finish == pos)
		{
			return 0;
		}

		key_finish = find_any_symbol_like_or_not_like_that(pos + 1, finish, &(characters[QUOTE_POSITION]), 1, 1, 1);

		if (finish == key_finish &&
			characters[QUOTE_POSITION] != *key_finish)
		{
			return 0;
		}

		if (NULL == value)
		{
			return 1;
		}

		return xml_read_ampersand_based_data(pos, key_finish, value);
	}

	return 0;
}

uint8_t xml_get_element_value(const uint8_t* start, const uint8_t* finish, struct buffer* value)
{
	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == value)
	{
		return 0;
	}

	start = find_any_symbol_like_or_not_like_that(
				start, finish, &(characters[GREATER_POSITION]), 1, 1, 1);

	if (finish == start)
	{
		return 0;
	}

	if (tag_close == *(start - 1))
	{
		return 1;
	}
	else
	{
		start = find_any_symbol_like_or_not_like_that(
					start + 1, finish, &(characters[GREATER_POSITION]), 1, 0, 1);
	}

	if (string_contains(start, finish, cdata_start, cdata_start + cdata_start_length))
	{
		const uint8_t* start_with_index = start;

		while (NULL != start_with_index && cdata_start_length <= finish - start_with_index)
		{
			if (memcmp(start_with_index, cdata_start, cdata_start_length))
			{
				start_with_index = string_enumerate(start_with_index, finish, NULL);
				continue;
			}

			if (start < start_with_index && !xml_read_ampersand_based_data(start, start_with_index, value))
			{
				return 0;
			}

			start_with_index += cdata_start_length;
			start = start_with_index;
			uint8_t found = 0;

			while (NULL != start_with_index && cdata_finish_length <= finish - start_with_index)
			{
				if (memcmp(start_with_index, cdata_finish, cdata_finish_length))
				{
					start_with_index = string_enumerate(start_with_index, finish, NULL);
					continue;
				}

				found = 1;
				break;
			}

			if (!found)
			{
				if (start < finish)
				{
					if (!buffer_append(value, start, finish - start))
					{
						return 0;
					}

					start = finish;
				}

				break;
			}

			if (!buffer_append(value, start, start_with_index - start))
			{
				return 0;
			}

			start_with_index += cdata_finish_length;
			start = start_with_index;
		}

		if (start < finish)
		{
			if (!xml_read_ampersand_based_data(start, finish, value))
			{
				return 0;
			}
		}
	}
	else
	{
		finish = find_any_symbol_like_or_not_like_that(
					 finish, start, &(characters[LESS_POSITION]), 1, 1, -1);

		if (!range_in_parts_is_null_or_empty(start, finish) &&
			!xml_read_ampersand_based_data(start, finish, value))
		{
			return 0;
		}
	}

	return 1;
}
