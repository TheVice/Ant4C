/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _VERSION_H_
#define _VERSION_H_

#include <stdint.h>

#if !defined(PROGRAM_VERSION)
#define PROGRAM_VERSION					"YYYY.MM.DD.?"
#endif
#define PROGRAM_VERSION_LENGTH			COUNT_OF(PROGRAM_VERSION)

struct Version
{
	uint32_t major;
	uint32_t minor;
	uint32_t build;
	uint32_t revision;
};

struct buffer;

uint8_t version_parse(const uint8_t* input_start, const uint8_t* input_finish, struct Version* version);
uint8_t version_to_byte_array(const struct Version* version, uint8_t* output);
uint8_t version_to_string(const struct Version* version, struct buffer* output);

uint8_t version_less(const struct Version* a, const struct Version* b);
uint8_t version_greater(const struct Version* a, const struct Version* b);

uint8_t version_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t version_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output);

#endif
