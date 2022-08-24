/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "version.h"

#include "common.h"
#include "conversion.h"
#include "range.h"

enum version_function
{
	parse, to_string, get_major, get_minor, get_build, get_revision, less_, greater_,
	UNKNOWN_VERSION_FUNCTION
};

uint8_t version_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
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
	/**/
	return common_string_to_enum(
			   name_start, name_finish, version_function_str, UNKNOWN_VERSION_FUNCTION);
}

uint8_t version_exec_function(
	uint8_t function, const void* arguments, uint8_t arguments_count,
	void* output)
{
	if (UNKNOWN_VERSION_FUNCTION <= function ||
		NULL == arguments ||
		(1 != arguments_count && 2 != arguments_count) ||
		NULL == output)
	{
		return 0;
	}

	struct range values[2];

	if (!common_get_arguments(arguments, arguments_count, values, 0))
	{
		return 0;
	}

	uint8_t versions[2 * VERSION_SIZE];

	for (uint8_t i = 0, count = 2; i < count; ++i)
	{
		uint8_t* version = versions + (ptrdiff_t)i * VERSION_SIZE;

		if (arguments_count <= i || range_is_null_or_empty(&values[i]))
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
