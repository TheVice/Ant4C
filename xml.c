/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "xml.h"
#include "buffer.h"
#include "common.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

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

		while (finish != (start = string_find_any_symbol_like_or_not_like_that(
									  start, finish, tag_finish, tag_finish + 1, 1, 1)))
		{
			if (!string_starts_with(start, finish, tag_finish, tag_finish + comment_finish_length))
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
		uint32_t char_set;
		const uint8_t* pos = string_enumerate(start, finish, &char_set);

		if (!pos)
		{
			return NULL;
		}

		if (characters[GREATER_POSITION] == char_set)
		{
			break;
		}
		else if (characters[QUOTE_POSITION] == char_set)
		{
			static const uint8_t* that = &characters[QUOTE_POSITION];
			start = string_find_any_symbol_like_or_not_like_that(
						pos, finish, that, that + 1, 1, 1);

			if (start == finish)
			{
				break;
			}

			pos = string_enumerate(start, finish, NULL);
		}
		else if (characters[LESS_POSITION] == char_set)
		{
			return NULL;
		}

		start = pos;
	}

	return start;
}

uint16_t xml_get_sub_nodes_elements(
	const uint8_t* start, const uint8_t* finish,
	const struct range* sub_nodes_names, void* elements)
{
	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == elements)
	{
		return 0;
	}

	uint16_t count = 0;
	uint16_t depth = 0;

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

		uint32_t char_set;

		if (!string_enumerate(start, finish, &char_set))
		{
			return 0;
		}

		if (question_mark == char_set)
		{
			start = tag_finish_pos;
			continue;
		}

		if (tag_close == char_set)
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
				if (depth < UINT16_MAX)
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
				uint8_t found = 0;
				static const uint8_t zero = '\0';
				const uint8_t* tag_name_finish = xml_get_tag_name(element->start, element->finish);
				const uint8_t* sub_node_name_start = sub_nodes_names->start;
				const uint8_t* sub_node_name_finish = sub_nodes_names->start;

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

const uint8_t* xml_get_tag_name(const uint8_t* start, const uint8_t* finish)
{
	static const uint8_t tab_space_close_tag[] = { '\t', ' ', '/', '>', '\r', '\n' };
	return string_find_any_symbol_like_or_not_like_that(
			   start, finish, tab_space_close_tag, tab_space_close_tag + 6, 1, 1);
}

uint8_t xml_read_ampersand_based_data(
	const uint8_t* start, const uint8_t* finish, void* output)
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

	uint8_t digits_buffer[BUFFER_SIZE_OF];
	void* digits = (void*)digits_buffer;

	if (!buffer_init(digits, BUFFER_SIZE_OF) ||
		!buffer_resize(digits, (finish - start) / 3) ||
		!buffer_resize(digits, 0))
	{
		buffer_release(digits);
		return 0;
	}

	const uint8_t* pos = start;
	static const uint8_t* that = &characters[AMPERSAND_POSITION];

	while (finish != (start = string_find_any_symbol_like_or_not_like_that(
								  start, finish, that, that + 1, 1, 1)))
	{
		if (pos < start && buffer_size(digits))
		{
			const uint16_t* digits_start = buffer_uint16_t_data(digits, 0);
			const uint16_t* digits_finish = (const uint16_t*)(buffer_uint8_t_data(digits, 0) + buffer_size(digits));

			if (!text_encoding_UTF16LE_to_UTF8(digits_start, digits_finish, output) ||
				!buffer_resize(digits, 0))
			{
				buffer_release(digits);
				return 0;
			}
		}

		if (pos < start && !buffer_append(output, pos, start - pos))
		{
			buffer_release(digits);
			return 0;
		}

		static const uint8_t* ampersand_based_start[] =
		{
			(const uint8_t*)"&#x",
			(const uint8_t*)"&#X",
			(const uint8_t*)"&#"
		};
		/**/
		static const uint8_t ampersand_based_start_lengths[] = { 3, 3, 2 };
		uint8_t found = 0;

		for (uint8_t i = 0, count = COUNT_OF(ampersand_based_start_lengths); i < count; ++i)
		{
			if (string_starts_with(
					start, finish,
					ampersand_based_start[i],
					ampersand_based_start[i] + ampersand_based_start_lengths[i]))
			{
				found = i + 1;
				break;
			}
		}

		if (found)
		{
			static const uint8_t* skip_to_digit = (const uint8_t*)"&#xX";
			start = string_find_any_symbol_like_or_not_like_that(
						start + 2, finish, skip_to_digit, skip_to_digit + 4, 0, 1);
			const long value = strtol((const char*)start, NULL, 3 == found ? 10 : 16);

			if ((value < 0 || UINT16_MAX < value) ||
				!buffer_push_back_uint16_t(digits, (uint16_t)value))
			{
				buffer_release(digits);
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
						buffer_release(digits);
						return 0;
					}

					start += predefined_lengths[i] - 1;
					break;
				}
			}

			uint32_t char_set;
			pos = string_enumerate(start, finish, &char_set);

			if (characters[AMPERSAND_POSITION] == char_set)
			{
				if (!buffer_push_back(output, characters[AMPERSAND_POSITION]))
				{
					buffer_release(digits);
					return 0;
				}

				start = pos;
				continue;
			}
		}

		static const uint8_t semicolon = ';';
		start = string_find_any_symbol_like_or_not_like_that(start, finish, &semicolon, &semicolon + 1, 1, 1);
		start = string_enumerate(start, finish, NULL);
		pos = start;
	}

	if (buffer_size(digits))
	{
		const uint16_t* digits_start = buffer_uint16_t_data(digits, 0);
		const uint16_t* digits_finish = (const uint16_t*)(buffer_uint8_t_data(digits, 0) + buffer_size(digits));

		if (!text_encoding_UTF16LE_to_UTF8(digits_start, digits_finish, output) ||
			!buffer_resize(digits, 0))
		{
			buffer_release(digits);
			return 0;
		}
	}

	buffer_release(digits);
	return pos < finish ? buffer_append(output, pos, finish - pos) : 1;
}

uint8_t xml_get_attribute_value(
	const uint8_t* start, const uint8_t* finish,
	const uint8_t* attribute, ptrdiff_t attribute_length,
	void* value)
{
	static const uint8_t* equal_space_tab = (const uint8_t*)"= \t";

	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == attribute ||
		0 == attribute_length ||
		(finish - start) < attribute_length)
	{
		return 0;
	}

	const uint8_t* pos = start;

	while (finish != (pos = (string_find_any_symbol_like_or_not_like_that(
								 pos, finish,
								 equal_space_tab, equal_space_tab + 1,
								 1, 1))))
	{
		static const uint8_t* double_quote = (const uint8_t*)"\"\"";
		const uint8_t* space_tab = equal_space_tab + 1;
		/**/
		const uint8_t* key_finish =
			string_find_any_symbol_like_or_not_like_that(
				pos, start, equal_space_tab, equal_space_tab + 3, 0, -1);
		const uint8_t* key_start =
			string_find_any_symbol_like_or_not_like_that(
				key_finish, start, space_tab, space_tab + 2, 1, -1);
		/**/
		key_start =
			string_find_any_symbol_like_or_not_like_that(
				key_start, key_finish, space_tab, space_tab + 2, 0, 1);
		key_finish =
			string_find_any_symbol_like_or_not_like_that(
				key_finish, pos, equal_space_tab, equal_space_tab + 3, 1, 1);
		/**/
		pos = string_enumerate(pos, finish, NULL);

		if (!string_equal(
				key_start, key_finish,
				attribute, attribute + attribute_length))
		{
			continue;
		}

		static const uint8_t* that = &characters[QUOTE_POSITION];
		pos = string_find_any_symbol_like_or_not_like_that(
				  pos, finish, that, that + 1, 1, 1);

		if (finish != pos &&
			string_starts_with(pos, finish, double_quote, double_quote + 2))
		{
			return 1;
		}

		pos = string_enumerate(pos, finish, NULL);
		pos = string_find_any_symbol_like_or_not_like_that(
				  pos, finish, that, that + 1, 0, 1);

		if (finish == pos)
		{
			return 0;
		}

		key_finish = string_enumerate(pos, finish, NULL);
		key_finish = string_find_any_symbol_like_or_not_like_that(
						 key_finish, finish, that, that + 1, 1, 1);

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

uint8_t xml_get_element_value(
	const uint8_t* start, const uint8_t* finish, void* value)
{
	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == value)
	{
		return 0;
	}

	const uint8_t* pos = start;
	const uint8_t* that = &characters[GREATER_POSITION];
	start = string_find_any_symbol_like_or_not_like_that(
				start, finish, that, that + 1, 1, 1);

	if (finish == start)
	{
		return 0;
	}

	pos = string_find_any_symbol_like_or_not_like_that(
			  start, pos, &tag_close, &tag_close + 1, 1, -1);
	uint32_t char_set;

	if (1 == string_get_length(pos, start) &&
		string_enumerate(pos, finish, &char_set) &&
		tag_close == char_set)
	{
		return 1;
	}

	start = string_enumerate(start, finish, NULL);
	start = string_find_any_symbol_like_or_not_like_that(
				start, finish, that, that + 1, 0, 1);

	if (string_contains(start, finish, cdata_start, cdata_start + cdata_start_length))
	{
		pos = start;

		while (finish != (pos = string_find_any_symbol_like_or_not_like_that(
									pos, finish, cdata_start, cdata_start + 1, 1, 1)))
		{
			if (!string_starts_with(pos, finish, cdata_start, cdata_start + cdata_start_length))
			{
				pos = string_enumerate(pos, finish, NULL);
				continue;
			}

			if (start < pos && !xml_read_ampersand_based_data(start, pos, value))
			{
				return 0;
			}

			pos += cdata_start_length;
			start = pos;
			uint8_t found = 0;

			while (finish != (pos = string_find_any_symbol_like_or_not_like_that(
										pos, finish, cdata_finish, cdata_finish + 1, 1, 1)))
			{
				if (!string_starts_with(pos, finish, cdata_finish, cdata_finish + cdata_finish_length))
				{
					pos = string_enumerate(pos, finish, NULL);
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

			if (!buffer_append(value, start, pos - start))
			{
				return 0;
			}

			pos += cdata_finish_length;
			start = pos;
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
		that = &characters[LESS_POSITION];
		finish = string_find_any_symbol_like_or_not_like_that(
					 finish, start, that, that + 1, 1, -1);

		if (!range_in_parts_is_null_or_empty(start, finish) &&
			!xml_read_ampersand_based_data(start, finish, value))
		{
			return 0;
		}
	}

	return 1;
}
