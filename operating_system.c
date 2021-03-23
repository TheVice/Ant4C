/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 https://github.com/TheVice/
 *
 */

#include "operating_system.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "environment.h"
#include "range.h"
#include "string_unit.h"
#include "version.h"

#include <string.h>

#if !defined(_WIN32)
#include <stdio.h>

#define _POSIXSOURCE 1
#include <sys/utsname.h>
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

struct OperatingSystem
{
	enum PlatformID Platform;
	struct Version Version;
	uint8_t IsServer;
#if defined(_WIN32)
	uint8_t VersionString[INT8_MAX + 1];
#else
	uint8_t VersionString[sizeof(struct utsname) + 4];
#endif
};

static const uint8_t* windows_label = (const uint8_t*)"Microsoft Windows NT";
static const uint8_t* server_label = (const uint8_t*)"Server";
#define Win32NT_str_length	20
#define Server_str_length	6

#if !defined(_WIN32)
uint8_t operating_system_init(uint8_t platformID, const struct Version* version,
							  const uint8_t** version_string, ptrdiff_t size, void* os)
#else
uint8_t operating_system_init(uint8_t platformID, uint8_t is_server,
							  const struct Version* version, ptrdiff_t size, void* os)
#endif
{
	if (size < (ptrdiff_t)sizeof(struct OperatingSystem) ||
		NULL == os)
	{
		return 0;
	}

	memset(os, 0, size);
	struct OperatingSystem* operating_system = (struct OperatingSystem*)os;
	operating_system->Platform = platformID;
#if defined(_WIN32)
	operating_system->IsServer = is_server;
#endif

	if (version)
	{
#if __STDC_SEC_API__

		if (0 != memcpy_s(&operating_system->Version, sizeof(struct Version), version, sizeof(struct Version)))
		{
			return 0;
		}

		if (Win32 == platformID &&
			0 != memcpy_s(operating_system->VersionString,
						  INT8_MAX, windows_label, Win32NT_str_length))
		{
			return 0;
		}

#else
		memcpy(&operating_system->Version, version, sizeof(struct Version));

		if (Win32 == platformID)
		{
			memcpy(operating_system->VersionString, windows_label, Win32NT_str_length);
		}

#endif
		uint8_t* str_version = operating_system->VersionString;

		if (Win32 == platformID)
		{
			str_version += Win32NT_str_length;
			*str_version = ' ';
			++str_version;
#if defined(_WIN32)

			if (is_server)
			{
#if __STDC_SEC_API__

				if (0 != memcpy_s(str_version, INT8_MAX - Win32NT_str_length, server_label, Server_str_length))
				{
					return 0;
				}

#else
				memcpy(str_version, INT8_MAX - Win32NT_str_length, server_label, Server_str_length);
#endif
				str_version += Server_str_length;
				*str_version = ' ';
				++str_version;
			}

#endif

			if (!version_to_byte_array(version, str_version))
			{
				return 0;
			}
		}

#if !defined(_WIN32)
		else
		{
#if __STDC_SEC_API__
			size = sprintf_s(
					   (char* const)str_version, sizeof(operating_system->VersionString),
					   "%s %s %s %s",
					   version_string[0], version_string[1],
					   version_string[2], version_string[3]);
#else
			size = sprintf(
					   (char* const)str_version, "%s %s %s %s",
					   version_string[0], version_string[1],
					   version_string[2], version_string[3]);
#endif

			if (size < 1)
			{
				return 0;
			}
		}

#endif
	}

	return 1;
}

uint8_t operating_system_parse(const uint8_t* start, const uint8_t* finish, ptrdiff_t size, void* os)
{
	if (size < (ptrdiff_t)sizeof(struct OperatingSystem) ||
		NULL == os)
	{
		return 0;
	}

	memset(os, 0, size);

	if (range_in_parts_is_null_or_empty(start, finish))
	{
		return 1;
	}

	struct OperatingSystem* operating_system = (struct OperatingSystem*)os;

	if (string_starts_with(start, finish, windows_label, windows_label + Win32NT_str_length))
	{
		operating_system->Platform = Win32;
		operating_system->IsServer = string_contains(
										 start + Win32NT_str_length, finish, server_label,
										 server_label + Server_str_length);
	}
	else
	{
		operating_system->Platform = Unix;
	}

	/*TODO: os->Platform = MacOSX;*/

	if (!version_parse(start, finish, &operating_system->Version))
	{
		return 0;
	}

#if __STDC_SEC_API__

	if (0 != memcpy_s(&operating_system->VersionString, sizeof(operating_system->VersionString), start,
					  MIN((ptrdiff_t)sizeof(operating_system->VersionString), finish - start)))
	{
		return 0;
	}

#else
	memcpy(&operating_system->VersionString, start,
		   MIN((ptrdiff_t)sizeof(operating_system->VersionString), finish - start));
#endif
	return 1;
}

enum PlatformID operating_system_get_platform(const void* os)
{
	if (NULL == os)
	{
		return UINT8_MAX;
	}

	const struct OperatingSystem* operating_system = (const struct OperatingSystem*)os;
	return operating_system->Platform;
}

const struct Version* operating_system_get_version(const void* os)
{
	if (NULL == os)
	{
		static struct Version ver;
		ver.major = ver.minor = ver.build = ver.revision = 0;
		return &ver;
	}

	const struct OperatingSystem* operating_system = (const struct OperatingSystem*)os;
	return &operating_system->Version;
}

const uint8_t* operating_system_to_string(const void* os)
{
	if (NULL == os)
	{
		return NULL;
	}

	const struct OperatingSystem* operating_system = (const struct OperatingSystem*)os;
	return operating_system->VersionString;
}

uint8_t operating_system_is_windows_server(const void* os)
{
	if (NULL == os)
	{
		return 0;
	}

	const struct OperatingSystem* operating_system = (const struct OperatingSystem*)os;
	return operating_system->IsServer;
}

const uint8_t* platform_get_name()
{
	const struct OperatingSystem* os = (const struct OperatingSystem*)environment_get_operating_system();

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

uint8_t platform_is_windows_server()
{
#if defined(_WIN32)
	const struct OperatingSystem* os = (const struct OperatingSystem*)environment_get_operating_system();

	if (NULL == os)
	{
		return 0;
	}

	return os->IsServer;
#else
	return 0;
#endif
}

static const uint8_t* os_function_str[] =
{
	(const uint8_t*)"get-platform",
	(const uint8_t*)"get-version",
	(const uint8_t*)"to-string",
	(const uint8_t*)"get-name",
	(const uint8_t*)"is-unix",
	(const uint8_t*)"is-windows",
	(const uint8_t*)"is-windows-server"
};

enum os_function
{
	get_platform, get_version, to_string,
	get_name, is_unix, is_windows, is_windows_server,
	UNKNOWN_OS_FUNCTION
};

uint8_t os_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, os_function_str, UNKNOWN_OS_FUNCTION);
}

uint8_t os_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						 struct buffer* output)
{
	if (UNKNOWN_OS_FUNCTION <= function || NULL == arguments || 1 != arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument;

	struct OperatingSystem os;

	if (!common_get_arguments(arguments, arguments_count, &argument, 0) ||
		!operating_system_parse(argument.start, argument.finish, sizeof(struct OperatingSystem), &os))
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
					return common_append_string_to_buffer((const uint8_t*)"Win32", output);

				case Unix:
					return common_append_string_to_buffer((const uint8_t*)"Unix", output);

				/*TODO: case MacOSX:*/
				default:
					break;
			}
		}
		break;

		case get_version:
			return version_to_string(operating_system_get_version(&os), output);

		case to_string:
			return common_append_string_to_buffer(operating_system_to_string(&os), output);

		case is_windows_server:
			return bool_to_string(operating_system_is_windows_server(&os), output);

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

		case is_windows_server:
			return bool_to_string(platform_is_windows_server(), output);

		case UNKNOWN_OS_FUNCTION:
		default:
			break;
	}

	return 0;
}
