/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "operating_system.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "environment.h"
#include "range.h"
#include "string_unit.h"

uint8_t operating_system_parse(const char* start, const char* finish, struct OperatingSystem* os)
{
	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == os ||
		sizeof(os->VersionString) / sizeof(*os->VersionString) < (size_t)(finish - start))
	{
		return 0;
	}

	static const char* windows_label = Win32NT_str;

	if (string_starts_with(start, finish, windows_label, windows_label + Win32NT_str_length))
	{
		os->Platform = Win32;
	}
	else
	{
		os->Platform = Unix;
	}

	/*TODO: os->Platform = MacOSX;*/

	if (!version_parse(start, finish, &os->Version))
	{
		return 0;
	}

	for (uint16_t i = 0, count = sizeof(os->VersionString) / sizeof(*os->VersionString); i < count; ++i)
	{
		if (start + i < finish)
		{
			os->VersionString[i] = *(start + i);
		}
		else
		{
			os->VersionString[i] = '\0';
			break;
		}
	}

	return 1;
}

enum PlatformID operating_system_get_platform(const struct OperatingSystem* os)
{
	if (NULL == os)
	{
		return UINT8_MAX;
	}

	return os->Platform;
}

struct Version operating_system_get_version(const struct OperatingSystem* os)
{
	if (NULL == os)
	{
		static struct Version ver;
		ver.major = ver.minor = ver.build = ver.revision = 0;
		return ver;
	}

	return os->Version;
}

const char* operating_system_to_string(const struct OperatingSystem* os)
{
	if (NULL == os)
	{
		return NULL;
	}

	return os->VersionString;
}

const char* platform_get_name()
{
	const struct OperatingSystem* os = environment_get_operating_system();

	if (NULL == os)
	{
		return NULL;
	}

	return os->VersionString;
}

uint8_t platform_is_unix()
{
	return Unix == operating_system_get_platform(environment_get_operating_system());
}

uint8_t platform_is_windows()
{
	return Win32 == operating_system_get_platform(environment_get_operating_system());
}

static const char* os_function_str[] =
{
	"get-platform", "get-version", "to-string", "get-name", "is-unix", "is-windows"
};

enum os_function
{
	get_platform, get_version, to_string, get_name, is_unix, is_windows,
	UNKNOWN_OS_FUNCTION
};

uint8_t os_get_function(const char* name_start, const char* name_finish)
{
	return common_string_to_enum(name_start, name_finish, os_function_str, UNKNOWN_OS_FUNCTION);
}

uint8_t os_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						 struct buffer* output)
{
	if (UNKNOWN_OS_FUNCTION <= function || NULL == arguments || 1 < arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument;

	argument.start = argument.finish = NULL;

	if (1 == arguments_count && !common_get_one_argument(arguments, &argument, 0))
	{
		return 0;
	}

	struct OperatingSystem os;

	if (1 == arguments_count && !operating_system_parse(argument.start, argument.finish, &os))
	{
		return 0;
	}

	switch (function)
	{
		case get_platform:
		{
			switch (operating_system_get_platform(&os))
			{
				case Win32:
					return common_append_string_to_buffer("Win32", output);

				case Unix:
					return common_append_string_to_buffer("Unix", output);

				/*TODO: case MacOSX:*/
				default:
					break;
			}
		}
		break;

		case get_version:
		{
			const struct Version ver = operating_system_get_version(&os);
			return version_to_string(&ver, output);
		}

		case to_string:
			return common_append_string_to_buffer(operating_system_to_string(&os), output);

		case UNKNOWN_OS_FUNCTION:
		default:
			break;
	}

	return 0;
}

uint8_t platform_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							   struct buffer* output)
{
	if (UNKNOWN_OS_FUNCTION <= function || NULL == arguments || arguments_count || NULL == output)
	{
		return 0;
	}

	switch (function)
	{
		case get_name:
			return common_append_string_to_buffer(platform_get_name(), output);

		case is_unix:
			return bool_to_string(platform_is_unix(), output);

		case is_windows:
			return bool_to_string(platform_is_windows(), output);
			break;

		case UNKNOWN_OS_FUNCTION:
		default:
			break;
	}

	return 0;
}
