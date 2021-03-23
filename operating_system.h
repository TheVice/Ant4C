/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 https://github.com/TheVice/
 *
 */

#ifndef _OPERATING_SYSTEM_H_
#define _OPERATING_SYSTEM_H_

#include <stddef.h>
#include <stdint.h>

enum PlatformID
{
	Win32,
	Unix/*,
	MacOSX*/
};

struct buffer;
struct Version;

#if !defined(_WIN32)
uint8_t operating_system_init(uint8_t platformID, const struct Version* version,
							  const uint8_t** version_string, ptrdiff_t size, void* os);
#else
uint8_t operating_system_init(uint8_t platformID, uint8_t is_server,
							  const struct Version* version, ptrdiff_t size, void* os);
#endif

uint8_t operating_system_parse(const uint8_t* start, const uint8_t* finish, ptrdiff_t size, void* os);
enum PlatformID operating_system_get_platform(const void* os);
const struct Version* operating_system_get_version(const void* os);
const uint8_t* operating_system_to_string(const void* os);
uint8_t operating_system_is_windows_server(const void* os);

const uint8_t* platform_get_name();
uint8_t platform_is_unix();
uint8_t platform_is_windows();
uint8_t platform_is_windows_server();

uint8_t os_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t os_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
						 struct buffer* output);
uint8_t platform_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							   struct buffer* output);

#endif
