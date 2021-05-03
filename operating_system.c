/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "stdc_secure_api.h"

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

struct OperatingSystem
{
	enum PlatformID platform;
	uint8_t version[VERSION_SIZE];
	uint8_t is_server;
#if defined(_WIN32)
	uint8_t description[INT8_MAX + 1];
#else
	uint8_t description[sizeof(struct utsname) + 4];
#endif
};

static const uint8_t* windows_label = (const uint8_t*)"Microsoft Windows NT";
static const uint8_t* server_label = (const uint8_t*)"Server";
#define Win32NT_str_length	20
#define Server_str_length	6

#if !defined(_WIN32)
uint8_t operating_system_init(uint8_t platformID, const uint8_t* version,
							  const uint8_t** version_string, ptrdiff_t size, void* os)
#else
uint8_t operating_system_init(uint8_t platformID, uint8_t is_server,
							  const uint8_t* version, ptrdiff_t size, void* os)
#endif
{
	if (size < (ptrdiff_t)sizeof(struct OperatingSystem) ||
		NULL == os)
	{
		return 0;
	}

	memset(os, 0, size);
	struct OperatingSystem* operating_system = (struct OperatingSystem*)os;
	operating_system->platform = platformID;
#if defined(_WIN32)
	operating_system->is_server = is_server;
#endif

	if (version)
	{
#if __STDC_LIB_EXT1__

		if (0 != memcpy_s(operating_system->version, VERSION_SIZE, version, VERSION_SIZE))
		{
			return 0;
		}

#else
		memcpy(operating_system->version, version, VERSION_SIZE);
#endif
		uint8_t* ptr = operating_system->description;
#if __STDC_LIB_EXT1__
		size = (ptrdiff_t)sizeof(operating_system->description);
#endif

		if (Win32 == platformID)
		{
			const uint8_t* labels[2];
			labels[0] = windows_label;
			labels[1] = server_label;
			uint8_t labels_lengths[] = { Win32NT_str_length, Server_str_length };

			for (uint8_t i = 0, count = COUNT_OF(labels); i < count; ++i)
			{
#if __STDC_LIB_EXT1__

				if (0 != memcpy_s(ptr, size, labels[i], labels_lengths[i]))
				{
					return 0;
				}

#else
				memcpy(ptr, labels[i], labels_lengths[i]);
#endif
				ptr += labels_lengths[i];
				*ptr = ' ';
				++ptr;
#if __STDC_LIB_EXT1__
				size -= (ptrdiff_t)labels_lengths[i] + 2;
#endif
#if defined(_WIN32)

				if (!is_server)
				{
					break;
				}

#endif
			}

			if (!version_to_byte_array(version, ptr))
			{
				return 0;
			}
		}

#if !defined(_WIN32)
		else
		{
			if (!version_string)
			{
				return 0;
			}

#if __STDC_LIB_EXT1__
			size = sprintf_s(
					   (char* const)ptr, size,
#else
			size = sprintf(
					   (char* const)ptr,
#endif
					   "%s %s %s %s",
					   version_string[0],
					   version_string[1],
					   version_string[2],
					   version_string[3]);

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
		operating_system->platform = Win32;
		operating_system->is_server = string_contains(
										  start + Win32NT_str_length, finish, server_label,
										  server_label + Server_str_length);
	}
	else
	{
		operating_system->platform = Unix;
	}

	/*TODO: os->Platform = MacOSX;*/

	if (!version_parse(start, finish, operating_system->version))
	{
		return 0;
	}

	size = finish - start;
#if __STDC_LIB_EXT1__

	if (0 != memcpy_s(operating_system->description, sizeof(operating_system->description),
					  start, MIN((ptrdiff_t)sizeof(operating_system->description), size)))
	{
		return 0;
	}

#else
	memcpy(operating_system->description, start,
		   MIN((ptrdiff_t)sizeof(operating_system->description), size));
#endif
	return 1;
}

enum PlatformID operating_system_get_platform(const void* os)
{
	if (NULL == os)
	{
		return UINT8_MAX;
	}

	return ((const struct OperatingSystem*)os)->platform;
}

const uint8_t* operating_system_get_version(const void* os)
{
	if (NULL == os)
	{
		static uint8_t version[VERSION_SIZE];
		memset(version, 0, VERSION_SIZE);
		return version;
	}

	return ((const struct OperatingSystem*)os)->version;
}

const uint8_t* operating_system_to_string(const void* os)
{
	if (NULL == os)
	{
		return NULL;
	}

	return ((const struct OperatingSystem*)os)->description;
}

uint8_t operating_system_is_windows_server(const void* os)
{
	if (NULL == os)
	{
		return 0;
	}

	return ((const struct OperatingSystem*)os)->is_server;
}

const uint8_t* platform_get_name()
{
	const struct OperatingSystem* os = (const struct OperatingSystem*)environment_get_operating_system();

	if (NULL == os)
	{
		return NULL;
	}

	return os->description;
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

	return os->is_server;
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

	if (!common_get_arguments(arguments, arguments_count, &argument, 1) ||
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
