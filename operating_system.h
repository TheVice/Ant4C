/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _OPERATING_SYSTEM_H_
#define _OPERATING_SYSTEM_H_

#include "version.h"

#include <stdint.h>

enum PlatformID
{
	Win32,
	Unix/*,
	MacOSX*/
};

struct OperatingSystem
{
	enum PlatformID Platform;
	struct Version Version;
#if defined(_WIN32)
	char VersionString[INT8_MAX + 1];
#else
	char VersionString[UINT8_MAX + 17];
#endif
};

struct buffer;

#define Win32NT_str			"Microsoft Windows NT"
#define Win32NT_str_length	20

uint8_t operating_system_parse(const char* start, const char* finish, struct OperatingSystem* os);
enum PlatformID operating_system_get_platform(const struct OperatingSystem* os);
struct Version operating_system_get_version(const struct OperatingSystem* os);
const char* operating_system_to_string(const struct OperatingSystem* os);

const char* platform_get_name();
uint8_t platform_is_unix();
uint8_t platform_is_windows();

uint8_t os_get_function(const char* name_start, const char* name_finish);
uint8_t os_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						 struct buffer* output);
uint8_t platform_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							   struct buffer* output);

#endif
