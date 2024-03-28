/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021, 2024 TheVice
 *
 */

#if !defined(_WIN32)
#define _POSIX_SOURCE 1
#endif

#include "stdc_secure_api.h"

#include "operating_system.h"
#include "common.h"
#include "environment.h"
#include "range.h"
#include "string_unit.h"
#include "version.h"

#include <string.h>

#if !defined(_WIN32)
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
static const uint8_t* darwin_label = (const uint8_t*)"Darwin Kernel";
#define Win32NT_str_length	20
#define Server_str_length	6
#define Darwin_str_length	13

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
	operating_system->platform = (enum PlatformID)platformID;
#if defined(_WIN32)
	operating_system->is_server = is_server;
#endif

	if (version)
	{
#if defined(__STDC_LIB_EXT1__)

		if (0 != memcpy_s(operating_system->version, VERSION_SIZE, version, VERSION_SIZE))
		{
			return 0;
		}

		size = (ptrdiff_t)sizeof(operating_system->description);
#else
		memcpy(operating_system->version, version, VERSION_SIZE);
#endif
		uint8_t* description = operating_system->description;

		if (Win32 == platformID)
		{
			const uint8_t* labels[2];
			labels[0] = windows_label;
			labels[1] = server_label;
			const uint8_t labels_lengths[] = { Win32NT_str_length, Server_str_length };

			for (uint8_t i = 0, count = COUNT_OF(labels); i < count; ++i)
			{
#if defined(__STDC_LIB_EXT1__)

				if (0 != memcpy_s(description, size, labels[i], labels_lengths[i]))
				{
					return 0;
				}

				size -= (ptrdiff_t)labels_lengths[i] + 2;
#else
				memcpy(description, labels[i], labels_lengths[i]);
#endif
				description += labels_lengths[i];
				*description = ' ';
				++description;
#if defined(_WIN32)

				if (!is_server)
				{
					break;
				}

#endif
			}

			if (!version_to_byte_array(version, description))
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

			for (uint8_t i = 0; i < 4; ++i)
			{
				const ptrdiff_t length = common_count_bytes_until(version_string[i], 0);

				if (!length)
				{
					continue;
				}

#if defined(__STDC_LIB_EXT1__)

				if (0 != memcpy_s(description, size, version_string[i], length))
				{
					return 0;
				}

				size -= length;
#else
				memcpy(description, version_string[i], length);
#endif
				description += length;
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
	else if (string_contains(start, finish, darwin_label, darwin_label + Darwin_str_length))
	{
		operating_system->platform = macOS;
	}
	else
	{
		operating_system->platform = Unix;
	}

	if (!version_parse(start, finish, operating_system->version))
	{
		return 0;
	}

	size = finish - start;
#if defined(__STDC_LIB_EXT1__)

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
		return (enum PlatformID)UINT8_MAX;
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

const uint8_t* operating_system_get_platform_name(const void* os)
{
	static const uint8_t* IDs_str[] =
	{
		(const uint8_t*)"win32",
		(const uint8_t*)"unix",
		(const uint8_t*)"macos",
		(const uint8_t*)"UNKNOWN_PLATFORM",
	};
	/**/
	const uint8_t count = COUNT_OF(IDs_str) - 1;
	const enum PlatformID platformID = operating_system_get_platform(os);

	for (uint8_t i = 0; i < count; ++i)
	{
		if (platformID == i)
		{
			return IDs_str[i];
		}
	}

	return IDs_str[count];
}

const uint8_t* platform_get_name()
{
	return operating_system_get_platform_name(environment_get_operating_system());
}

uint8_t platform_is_macos()
{
	return macOS == operating_system_get_platform(environment_get_operating_system());
}

uint8_t platform_is_unix()
{
	const enum PlatformID platformID = operating_system_get_platform(environment_get_operating_system());
	return Unix == platformID || macOS == platformID;
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
