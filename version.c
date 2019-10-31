/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
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

static const uint8_t point = '.';

static const uint8_t digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
#define COUNT_OF_DIGITS COUNT_OF(digits)

uint8_t version_parse(const uint8_t* input_start, const uint8_t* input_finish, struct Version* version)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) || NULL == version)
	{
		return 0;
	}

	uint32_t* ver_in_parts[4];
	ver_in_parts[0] = &version->major;
	ver_in_parts[1] = &version->minor;
	ver_in_parts[2] = &version->build;
	ver_in_parts[3] = &version->revision;
	/**/
	const uint8_t count = sizeof(ver_in_parts) / sizeof(*ver_in_parts);
	uint8_t i = 0;

	for (input_start = find_any_symbol_like_or_not_like_that(input_start, input_finish, digits, COUNT_OF_DIGITS,
					   1, 1);
		 input_start < input_finish && i < count; ++i, ++input_start)
	{
		if (input_finish == input_start ||
			input_start + 1 == find_any_symbol_like_or_not_like_that(input_start, input_start + 1, digits,
					COUNT_OF_DIGITS, 1,
					1))
		{
			if (i == 0)
			{
				return 0;
			}

			break;
		}

		*(ver_in_parts[i]) = (uint32_t)int_parse(input_start);
		input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, &point, 1, 1, 1);
	}

	if (i == 0)
	{
		return 0;
	}

	while (i < count)
	{
		*(ver_in_parts[i++]) = 0;
	}

	return 1;
}

uint8_t version_to_byte_array(const struct Version* version, uint8_t* output)
{
	if (NULL == version || NULL == output)
	{
		return 0;
	}

#if __STDC_SEC_API__
	const uint8_t length = (uint8_t)sprintf_s(
							   (char* const)output, 64, "%u.%u.%u.%u", version->major, version->minor, version->build, version->revision);
#else
	const uint8_t length = (uint8_t)sprintf(
							   (char* const)output, "%u.%u.%u.%u", version->major, version->minor, version->build, version->revision);
#endif
	return length;
}

uint8_t version_to_string(const struct Version* version, struct buffer* output)
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

	uint8_t* ptr = buffer_data(output, size);
	return buffer_resize(output, size + version_to_byte_array(version, ptr));
}

uint8_t version_less(const struct Version* a, const struct Version* b)
{
	if (NULL == a ||
		NULL == b)
	{
		return 0;
	}

	if (a->major < b->major)
	{
		return 1;
	}
	else if (a->major == b->major && a->minor < b->minor)
	{
		return 1;
	}
	else if (a->major == b->major && a->minor == b->minor && a->build < b->build)
	{
		return 1;
	}
	else if (a->major == b->major && a->minor == b->minor && a->build == b->build && a->revision < b->revision)
	{
		return 1;
	}

	return 0;
}

uint8_t version_greater(const struct Version* a, const struct Version* b)
{
	if (NULL == a ||
		NULL == b)
	{
		return 0;
	}

	if (a->major > b->major)
	{
		return 1;
	}
	else if (a->major == b->major && a->minor > b->minor)
	{
		return 1;
	}
	else if (a->major == b->major && a->minor == b->minor && a->build > b->build)
	{
		return 1;
	}
	else if (a->major == b->major && a->minor == b->minor && a->build == b->build && a->revision > b->revision)
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

	struct range argument_1;

	struct range argument_2;

	argument_1.start = argument_2.start = argument_1.finish = argument_2.finish = NULL;

	if (1 == arguments_count && !common_get_one_argument(arguments, &argument_1, 1))
	{
		return 0;
	}

	if (2 == arguments_count && !common_get_two_arguments(arguments, &argument_1, &argument_2, 1))
	{
		return 0;
	}

	struct Version version_1;

	struct Version version_2;

	version_1.major = version_2.major = version_1.minor = version_2.minor = version_1.build = version_2.build =
											version_1.revision = version_2.revision = 0;

	if (!version_parse(argument_1.start, argument_1.finish, &version_1))
	{
		return 0;
	}

	if (2 == arguments_count && !version_parse(argument_2.start, argument_2.finish, &version_2))
	{
		return 0;
	}

	switch (function)
	{
		case parse:
		case to_string:
			return 1 == arguments_count && version_to_string(&version_1, output);

		case get_major:
			return 1 == arguments_count && int_to_string(version_1.major, output);

		case get_minor:
			return 1 == arguments_count && int_to_string(version_1.minor, output);

		case get_build:
			return 1 == arguments_count && int_to_string(version_1.build, output);

		case get_revision:
			return 1 == arguments_count && int_to_string(version_1.revision, output);

		case less_:
			return 2 == arguments_count && bool_to_string(version_less(&version_1, &version_2), output);

		case greater_:
			return 2 == arguments_count && bool_to_string(version_greater(&version_1, &version_2), output);

		case UNKNOWN_VERSION_FUNCTION:
		default:
			break;
	}

	return 0;
}
