/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _OPERATING_SYSTEM_H_
#define _OPERATING_SYSTEM_H_

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#define OPERATING_SYSTEM_SIZE UINT8_MAX
#else
#define OPERATING_SYSTEM_SIZE (sizeof(struct utsname) + INT8_MAX)
#endif

enum PlatformID
{
	Win32,
	Unix,
	macOS
};

struct buffer;

#if !defined(_WIN32)
uint8_t operating_system_init(
	uint8_t platformID, const uint8_t* version,
	const uint8_t** version_string, ptrdiff_t size, void* os);
#else
uint8_t operating_system_init(
	uint8_t platformID, uint8_t is_server,
	const uint8_t* version, ptrdiff_t size, void* os);
#endif

uint8_t operating_system_parse(
	const uint8_t* start, const uint8_t* finish, ptrdiff_t size, void* os);
enum PlatformID operating_system_get_platform(const void* os);
const uint8_t* operating_system_get_version(const void* os);
const uint8_t* operating_system_to_string(const void* os);
uint8_t operating_system_is_windows_server(const void* os);
const uint8_t* operating_system_get_platform_name(const void* os);

const uint8_t* platform_get_name();
uint8_t platform_is_macos();
uint8_t platform_is_unix();
uint8_t platform_is_windows();
uint8_t platform_is_windows_server();

#endif
