/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _VERSION_H_
#define _VERSION_H_

#include <stdint.h>

#if !defined(PROGRAM_VERSION)
#define PROGRAM_VERSION					"YYYY.MM.DD.?"
#endif
#define PROGRAM_VERSION_LENGTH			COUNT_OF(PROGRAM_VERSION)

#define VERSION_SIZE 4 * sizeof(uint32_t)

struct buffer;

uint8_t version_init(
	void* version, uint8_t size,
	uint32_t major, uint32_t minor, uint32_t build, uint32_t revision);

uint32_t version_get_major(const void* version);
uint32_t version_get_minor(const void* version);
uint32_t version_get_build(const void* version);
uint32_t version_get_revision(const void* version);

uint8_t version_parse(
	const uint8_t* input_start, const uint8_t* input_finish, uint8_t* version);
uint8_t version_to_byte_array(const void* version, uint8_t* output);
uint8_t version_to_string(const void* version, struct buffer* output);

uint8_t version_less(const void* version_a, const void* version_b);
uint8_t version_greater(const void* version_a, const void* version_b);

#endif
