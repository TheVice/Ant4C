/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "common.h"
#include "conversion.h"
#include "buffer.h"
#include "range.h"
#include "string_unit.h"

#include <stdio.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#define FIND_ANY_SYMBOL_LIKE_OR_NOT_LIKE_THAT(start, finish, that, that_length, like, step) \
	if ((NULL == start) || (NULL == finish) ||												\
		(NULL == that) || (that_length < 1) ||												\
		(0 != like && 1 != like) ||															\
		(-1 != step && 1 != step) ||														\
		((0 < step) ? (finish < start) : (finish > start)))									\
	{																						\
		return finish;																		\
	}																						\
	\
	while (start != finish)																	\
	{																						\
		uint8_t is_like = 0;																\
		\
		for (ptrdiff_t i = 0; i < that_length; ++i)											\
		{																					\
			is_like = (that[i] == (*start));												\
			\
			if (is_like)																	\
			{																				\
				break;																		\
			}																				\
		}																					\
		\
		if (like == is_like)																\
		{																					\
			return start;																	\
		}																					\
		\
		start += step;																		\
	}																						\
	\
	return start;

const uint8_t* find_any_symbol_like_or_not_like_that(
	const uint8_t* start, const uint8_t* finish, const uint8_t* that,
	ptrdiff_t that_length, uint8_t like, int8_t step)
{
	FIND_ANY_SYMBOL_LIKE_OR_NOT_LIKE_THAT(start, finish, that, that_length, like, step);
}

const wchar_t* find_any_symbol_like_or_not_like_that_wchar_t(
	const wchar_t* start, const wchar_t* finish, const wchar_t* that,
	ptrdiff_t that_length, uint8_t like, int8_t step)
{
	FIND_ANY_SYMBOL_LIKE_OR_NOT_LIKE_THAT(start, finish, that, that_length, like, step);
}

void replace_double_char_by_single(uint8_t* string, ptrdiff_t* length, uint8_t to_be_replaced)
{
	if (NULL == string || NULL == length || 0 == (*length))
	{
		return;
	}

	ptrdiff_t match = (*length);

	for (ptrdiff_t i = 0, current = -1, count = match; i < count; ++i)
	{
		if (i + 1 < count && to_be_replaced == string[i + 1] && to_be_replaced == string[i])
		{
			--match;

			if (-1 == current)
			{
				current = i;
			}

			continue;
		}

		if (-1 != current && current != i)
		{
			string[current++] = string[i];
		}
	}

	if (match != (*length))
	{
		(*length) = match;
	}
}

/*uint8_t common_string_to_enum_(const uint8_t* string_start, const uint8_t* string_finish,
							   const uint8_t** reference_strings,
							   const ptrdiff_t* reference_strings_lengths,
							   uint8_t max_enum_value)
{
	if (range_in_parts_is_null_or_empty(string_start, string_finish) ||
		NULL == reference_strings ||
		NULL == reference_strings_lengths ||
		0 == max_enum_value)
	{
		return max_enum_value;
	}

	for (uint8_t i = 0; i < max_enum_value; ++i)
	{
		if (string_equal(string_start, string_finish,
			reference_strings[i], reference_strings[i] + reference_strings_lengths[i]))
		{
			return i;
		}
	}

	return max_enum_value;
}*/

ptrdiff_t common_count_bytes_until(const uint8_t* bytes, uint8_t until)
{
	if (NULL == bytes)
	{
		return 0;
	}

	ptrdiff_t count = 0;

	while (until != *bytes)
	{
		++count;
		++bytes;
	}

	return count;
}

uint8_t common_string_to_enum(const uint8_t* string_start, const uint8_t* string_finish,
							  const uint8_t** reference_strings, uint8_t max_enum_value)
{
	if (range_in_parts_is_null_or_empty(string_start, string_finish) ||
		NULL == reference_strings ||
		0 == max_enum_value)
	{
		return max_enum_value;
	}

	for (uint8_t i = 0; i < max_enum_value; ++i)
	{
		const size_t length = common_count_bytes_until(reference_strings[i], 0);

		if (string_equal(string_start, string_finish,
						 reference_strings[i], reference_strings[i] + length))
		{
			return i;
		}
	}

	return max_enum_value;
}

uint8_t common_append_string_to_buffer(const uint8_t* input, struct buffer* output)
{
	if (NULL == input || NULL == output)
	{
		return 0;
	}

	return buffer_append(output, input, common_count_bytes_until(input, 0));
}

uint8_t common_unbox_char_data(const struct buffer* box_with_data, uint8_t i, uint8_t j,
							   struct range* data, uint8_t terminate)
{
	struct buffer* boxed_data = NULL;

	if (NULL == box_with_data ||
		NULL == data ||
		(0 != terminate && 1 != terminate))
	{
		return 0;
	}

	if (NULL == (boxed_data = buffer_buffer_data(box_with_data, i)))
	{
		return 0;
	}

	if (buffer_size(boxed_data))
	{
		if (terminate && !buffer_push_back(boxed_data, 0))
		{
			return 0;
		}
	}

	if (NULL == (data->start = buffer_data(boxed_data, j)))
	{
		return 0;
	}

	data->finish = data->start + buffer_size(boxed_data);
	return 1;
}

uint8_t common_get_one_argument(const struct buffer* arguments,	struct range* argument, uint8_t terminate)
{
	if (NULL == arguments || NULL == argument)
	{
		return 0;
	}

	return common_unbox_char_data(arguments, 0, 0, argument, terminate);
}

uint8_t common_get_two_arguments(const struct buffer* arguments, struct range* argument1,
								 struct range* argument2, uint8_t terminate)
{
	if (NULL == arguments || NULL == argument1 || NULL == argument2)
	{
		return 0;
	}

	const uint8_t ret1 = common_unbox_char_data(arguments, 0, 0, argument1, terminate);
	const uint8_t ret2 = common_unbox_char_data(arguments, 1, 0, argument2, terminate);
	return ret1 && ret2;
}

uint8_t common_get_three_arguments(const struct buffer* arguments, struct range* argument1,
								   struct range* argument2, struct range* argument3, uint8_t terminate)
{
	if (NULL == arguments || NULL == argument1 || NULL == argument2 || NULL == argument3)
	{
		return 0;
	}

	const uint8_t ret1 = common_unbox_char_data(arguments, 0, 0, argument1, terminate);
	const uint8_t ret2 = common_unbox_char_data(arguments, 1, 0, argument2, terminate);
	const uint8_t ret3 = common_unbox_char_data(arguments, 2, 0, argument3, terminate);
	return ret1 && ret2 && ret3;
}

uint8_t common_unbox_bool_data(const struct buffer* box_with_data, uint8_t i, uint8_t j, uint8_t* data)
{
	struct range char_data;

	if (!common_unbox_char_data(box_with_data, i, j, &char_data, 0))
	{
		return 0;
	}

	return bool_parse(char_data.start, range_size(&char_data), data);
}

int64_t common_unbox_int64_data(const struct buffer* box_with_data, uint8_t i, uint8_t j)
{
	struct range int64_data;
	int64_data.start = int64_data.finish = NULL;

	if (common_unbox_char_data(box_with_data, i, j, &int64_data, 1))
	{
		return int64_parse(int64_data.start);
	}

	return 0;
}

uint8_t read_file(const uint8_t* file_path, struct buffer* content)
{
	if (NULL == file_path || NULL == content)
	{
		return 0;
	}

	FILE* file_stream = NULL;
#if __STDC_SEC_API__

	if (0 != fopen_s(&file_stream, (const char*)file_path, "rb") || NULL == file_stream)
#else
	if (NULL == (file_stream = fopen((const char*)file_path, "rb")))
#endif
	{
		return 0;
	}

	uint8_t ret = 0;

	if (0 == fseek(file_stream, 0, SEEK_END))
	{
		const long file_size = ftell(file_stream);

		if (0 < file_size)
		{
			const ptrdiff_t size = buffer_size(content);

			if (buffer_append(content, NULL, file_size))
			{
				uint8_t* ptr = buffer_data(content, size);

				if (NULL != ptr && 0 == fseek(file_stream, 0, SEEK_SET))
				{
#if __STDC_SEC_API__ && defined(_MSC_VER)
					ret = (file_size == (long)fread_s(ptr, file_size, sizeof(uint8_t),
													  file_size, file_stream));
#else
					ret = (file_size == (long)fread(ptr, sizeof(uint8_t),
													file_size, file_stream));
#endif
				}
			}
		}
	}

	fclose(file_stream);
	file_stream = NULL;
	return ret;
}
