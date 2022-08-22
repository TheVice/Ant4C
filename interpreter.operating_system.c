/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#if !defined(_WIN32)
#define _POSIX_SOURCE 1
#endif

#include "operating_system.h"

#include "common.h"
#include "conversion.h"
#include "range.h"
#include "version.h"

#if !defined(_WIN32)
#include <sys/utsname.h>
#endif

enum os_function
{
	get_platform, get_version, to_string, get_name,
	is_macos, is_unix, is_windows, is_windows_server,
	UNKNOWN_OS_FUNCTION
};

uint8_t os_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* os_function_str[] =
	{
		(const uint8_t*)"get-platform",
		(const uint8_t*)"get-version",
		(const uint8_t*)"to-string",
		(const uint8_t*)"get-name",
		(const uint8_t*)"is-macos",
		(const uint8_t*)"is-unix",
		(const uint8_t*)"is-windows",
		(const uint8_t*)"is-windows-server"
	};
	/**/
	return common_string_to_enum(name_start, name_finish, os_function_str, UNKNOWN_OS_FUNCTION);
}

uint8_t os_exec_function(
	uint8_t function, const void* arguments, uint8_t arguments_count,
	void* output)
{
	if (UNKNOWN_OS_FUNCTION <= function || NULL == arguments || 1 != arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument;

	uint8_t os[OPERATING_SYSTEM_SIZE];

	if (!common_get_arguments(arguments, arguments_count, &argument, 0) ||
		!operating_system_parse(argument.start, argument.finish, OPERATING_SYSTEM_SIZE, os))
	{
		return 0;
	}

	switch (function)
	{
		case get_platform:
			return common_append_string_to_buffer(operating_system_get_platform_name(os), output);

		case get_version:
			return version_to_string(operating_system_get_version(os), output);

		case to_string:
			return common_append_string_to_buffer(operating_system_to_string(os), output);

		case is_windows_server:
			return bool_to_string(operating_system_is_windows_server(os), output);

		case UNKNOWN_OS_FUNCTION:
		default:
			break;
	}

	return 0;
}

uint8_t platform_exec_function(
	uint8_t function, const void* arguments, uint8_t arguments_count,
	void* output)
{
	if (UNKNOWN_OS_FUNCTION <= function || NULL == arguments || arguments_count || NULL == output)
	{
		return 0;
	}

	switch (function)
	{
		case get_name:
			return common_append_string_to_buffer(platform_get_name(), output);

		case is_macos:
			return bool_to_string(platform_is_macos(), output);

		case is_unix:
			return bool_to_string(platform_is_unix(), output);

		case is_windows:
			return bool_to_string(platform_is_windows(), output);

		case is_windows_server:
			return bool_to_string(platform_is_windows_server(), output);

		case UNKNOWN_OS_FUNCTION:
		default:
			break;
	}

	return 0;
}
