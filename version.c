/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "version.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"

struct version_
{
	uint32_t major;
	uint32_t minor;
	uint32_t build;
	uint32_t revision;
};

static const uint8_t point = '.';

uint8_t version_init(
	void* version, uint8_t size,
	uint32_t major, uint32_t minor, uint32_t build, uint32_t revision)
{
	if (!version ||
		size < sizeof(struct version_))
	{
		return 0;
	}

	struct version_* the_version = (struct version_*)version;

	the_version->major = major;

	the_version->minor = minor;

	the_version->build = build;

	the_version->revision = revision;

	return 1;
}

uint32_t version_get_major(const void* version)
{
	if (!version)
	{
		return 0;
	}

	return ((const struct version_*)version)->major;
}

uint32_t version_get_minor(const void* version)
{
	if (!version)
	{
		return 0;
	}

	return ((const struct version_*)version)->minor;
}

uint32_t version_get_build(const void* version)
{
	if (!version)
	{
		return 0;
	}

	return ((const struct version_*)version)->build;
}

uint32_t version_get_revision(const void* version)
{
	if (!version)
	{
		return 0;
	}

	return ((const struct version_*)version)->revision;
}

uint8_t version_parse(
	const uint8_t* input_start, const uint8_t* input_finish, uint8_t* version)
{
	static const uint8_t* digits = (const uint8_t*)"0123456789";
	static const uint8_t count_of_digits = 10;

	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == version)
	{
		return 0;
	}

	if (!version_init(version, VERSION_SIZE, 0, 0, 0, 0))
	{
		return 0;
	}

	uint8_t i = 0;
	input_start = find_any_symbol_like_or_not_like_that(
					  input_start, input_finish, digits, count_of_digits, 1, 1);

	for (uint8_t count = VERSION_SIZE / sizeof(uint32_t);
		 input_start < input_finish && i < count;
		 ++i, ++input_start, version += sizeof(uint32_t))
	{
		if (input_finish == input_start ||
			input_start + 1 == find_any_symbol_like_or_not_like_that(
				input_start, input_start + 1, digits, count_of_digits, 1, 1))
		{
			break;
		}

		*((uint32_t*)version) = (uint32_t)int64_parse(input_start, input_finish);
		++input_start;
		/**/
		const uint8_t* start = input_start;
		input_start = find_any_symbol_like_or_not_like_that(input_start, input_finish, &point, 1, 1, 1);

		for (; start < input_start; ++start)
		{
			if (' ' == *start)
			{
				i = count;
				break;
			}
		}
	}

	return 0 < i;
}

uint8_t version_to_byte_array(const void* version, uint8_t* output)
{
	if (NULL == version || NULL == output)
	{
		return 0;
	}

	const struct version_* the_version = (const struct version_*)version;
	const uint32_t* input[4];
	input[0] = &the_version->major;
	input[1] = &the_version->minor;
	input[2] = &the_version->build;
	input[3] = &the_version->revision;
	uint8_t* ptr = output;

	for (uint8_t i = 0; i < 4; ++i)
	{
		static uint8_t zero = '0';
		/**/
		uint8_t* a = ptr;
		uint8_t* b = ptr + 21;
		/**/
		const uint8_t* start = uint64_to_string_to_byte_array(*input[i], a, b, 21);
		const uint8_t* finish = start + 21;
		/**/
		start = find_any_symbol_like_or_not_like_that(start, finish, &zero, 1, 0, 1);
		const uint8_t l = (uint8_t)(finish - start);

		if (l)
		{
			MEM_CPY(ptr, start, l);
		}
		else
		{
			*ptr = zero;
			++ptr;
		}

		*ptr = point;
		++ptr;
	}

	return (uint8_t)(ptr - output - 1);
}

uint8_t version_to_string(const void* version, struct buffer* output)
{
	if (NULL == version || NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, INT8_MAX))
	{
		return 0;
	}

	uint8_t* str_version = buffer_data(output, size);
	return buffer_resize(output, size + version_to_byte_array(version, str_version));
}

uint8_t version_less(const void* version_a, const void* version_b)
{
	if (NULL == version_a ||
		NULL == version_b)
	{
		return 0;
	}

	const struct version_* the_a = (const struct version_*)version_a;
	const struct version_* the_b = (const struct version_*)version_b;

	if (the_a->major < the_b->major)
	{
		return 1;
	}
	else if (the_a->major == the_b->major && the_a->minor < the_b->minor)
	{
		return 1;
	}
	else if (the_a->major == the_b->major && the_a->minor == the_b->minor && the_a->build < the_b->build)
	{
		return 1;
	}
	else if (the_a->major == the_b->major && the_a->minor == the_b->minor &&
			 the_a->build == the_b->build && the_a->revision < the_b->revision)
	{
		return 1;
	}

	return 0;
}

uint8_t version_greater(const void* version_a, const void* version_b)
{
	if (NULL == version_a ||
		NULL == version_b)
	{
		return 0;
	}

	const struct version_* the_a = (const struct version_*)version_a;
	const struct version_* the_b = (const struct version_*)version_b;

	if (the_a->major > the_b->major)
	{
		return 1;
	}
	else if (the_a->major == the_b->major && the_a->minor > the_b->minor)
	{
		return 1;
	}
	else if (the_a->major == the_b->major && the_a->minor == the_b->minor && the_a->build > the_b->build)
	{
		return 1;
	}
	else if (the_a->major == the_b->major && the_a->minor == the_b->minor && the_a->build == the_b->build &&
			 the_a->revision > the_b->revision)
	{
		return 1;
	}

	return 0;
}
