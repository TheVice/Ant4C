/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 https://github.com/TheVice/
 *
 */

#include "version.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "range.h"

#include <stdio.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

struct version_
{
	uint32_t major;
	uint32_t minor;
	uint32_t build;
	uint32_t revision;
};

uint8_t version_init(uint8_t* version, uint8_t size,
					 uint32_t major, uint32_t minor, uint32_t build, uint32_t revision)
{
	if (size < sizeof(struct version_))
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

uint32_t version_get_major(const uint8_t* version)
{
	if (!version)
	{
		return 0;
	}

	return ((const struct version_*)version)->major;
}

uint32_t version_get_minor(const uint8_t* version)
{
	if (!version)
	{
		return 0;
	}

	return ((const struct version_*)version)->minor;
}

uint32_t version_get_build(const uint8_t* version)
{
	if (!version)
	{
		return 0;
	}

	return ((const struct version_*)version)->build;
}

uint32_t version_get_revision(const uint8_t* version)
{
	if (!version)
	{
		return 0;
	}

	return ((const struct version_*)version)->revision;
}

uint8_t version_parse(const uint8_t* input_start, const uint8_t* input_finish, uint8_t* version)
{
	static const uint8_t point = '.';
	static const uint8_t digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
#define COUNT_OF_DIGITS COUNT_OF(digits)

	if (range_in_parts_is_null_or_empty(input_start, input_finish) || NULL == version)
	{
		return 0;
	}

	if (!version_init(version, VERSION_SIZE, 0, 0, 0, 0))
	{
		return 0;
	}

	uint8_t i = 0;

	input_start = find_any_symbol_like_or_not_like_that(
							   input_start, input_finish, digits, COUNT_OF_DIGITS, 1, 1);

	for (; input_start < input_finish && i < 4; ++i, ++input_start, version += sizeof(uint32_t))
	{
		if (input_finish == input_start ||
			input_start + 1 == find_any_symbol_like_or_not_like_that(
				input_start, input_start + 1, digits, COUNT_OF_DIGITS, 1, 1))
		{
			if (i == 0)
			{
				return 0;
			}

			break;
		}

		*((uint32_t*)version) = (uint32_t)int_parse(input_start);
		input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, &point, 1, 1, 1);
	}

	if (i == 0)
	{
		return 0;
	}

	return 1;
}

uint8_t version_to_byte_array(const uint8_t* version, uint8_t* output)
{
	if (NULL == version || NULL == output)
	{
		return 0;
	}

	struct version_* the_version = (struct version_*)version;

#if __STDC_SEC_API__
	const uint8_t length = (uint8_t)sprintf_s(
							   (char* const)output, 64,
#else
	const uint8_t length = (uint8_t)sprintf(
							   (char* const)output,
#endif
							   "%u.%u.%u.%u",
							   the_version->major,
							   the_version->minor,
							   the_version->build,
							   the_version->revision);
	return length;
}

uint8_t version_to_string(const uint8_t* version, struct buffer* output)
{
	if (NULL == version || NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append_char(output, NULL, 64))
	{
		return 0;
	}

	uint8_t* str_version = buffer_data(output, size);
	return buffer_resize(output, size + version_to_byte_array(version, str_version));
}

uint8_t version_less(const uint8_t* version_a, const uint8_t* version_b)
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

uint8_t version_greater(const uint8_t* version_a, const uint8_t* version_b)
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

static const uint8_t* version_function_str[] =
{
	(const uint8_t*)"parse",
	(const uint8_t*)"to-string",
	(const uint8_t*)"get-major",
	(const uint8_t*)"get-minor",
	(const uint8_t*)"get-build",
	(const uint8_t*)"get-revision",
	(const uint8_t*)"less",
	(const uint8_t*)"greater"
};

enum version_function
{
	parse, to_string, get_major, get_minor, get_build, get_revision, less_, greater_,
	UNKNOWN_VERSION_FUNCTION
};

uint8_t version_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, version_function_str, UNKNOWN_VERSION_FUNCTION);
}

uint8_t version_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output)
{
	if (UNKNOWN_VERSION_FUNCTION <= function ||
		NULL == arguments ||
		(1 != arguments_count && 2 != arguments_count) ||
		NULL == output)
	{
		return 0;
	}

	struct range values[2];

	if (arguments_count && !common_get_arguments(arguments, arguments_count, values, 1))
	{
		return 0;
	}

	uint8_t versions[2 * VERSION_SIZE];

	for (ptrdiff_t i = 0, count = 2; i < count; ++i)
	{
		uint8_t* version = versions + i * VERSION_SIZE;

		if (arguments_count <= i)
		{
			if (!version_init(version, VERSION_SIZE, 0, 0, 0, 0))
			{
				return 0;
			}
		}
		else if (!version_parse(values[i].start, values[i].finish, version))
		{
			return 0;
		}
	}

	switch (function)
	{
		case parse:
		case to_string:
			return 1 == arguments_count && version_to_string(versions, output);

		case get_major:
			return 1 == arguments_count && int_to_string(version_get_major(versions), output);

		case get_minor:
			return 1 == arguments_count && int_to_string(version_get_minor(versions), output);

		case get_build:
			return 1 == arguments_count && int_to_string(version_get_build(versions), output);

		case get_revision:
			return 1 == arguments_count && int_to_string(version_get_revision(versions), output);

		case less_:
			return 2 == arguments_count && bool_to_string(version_less(versions, versions + VERSION_SIZE), output);

		case greater_:
			return 2 == arguments_count && bool_to_string(version_greater(versions, versions + VERSION_SIZE), output);

		case UNKNOWN_VERSION_FUNCTION:
		default:
			break;
	}

	return 0;
}
