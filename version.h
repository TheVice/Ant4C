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

uint8_t version_init(uint8_t* version, uint8_t size,
					 uint32_t major, uint32_t minor, uint32_t build, uint32_t revision);

uint32_t version_get_major(const uint8_t* version);
uint32_t version_get_minor(const uint8_t* version);
uint32_t version_get_build(const uint8_t* version);
uint32_t version_get_revision(const uint8_t* version);

uint8_t version_parse(const uint8_t* input_start, const uint8_t* input_finish, uint8_t* version);
uint8_t version_to_byte_array(const uint8_t* version, uint8_t* output);
uint8_t version_to_string(const uint8_t* version, struct buffer* output);

uint8_t version_less(const uint8_t* version_a, const uint8_t* version_b);
uint8_t version_greater(const uint8_t* version_a, const uint8_t* version_b);

uint8_t version_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t version_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output);

#endif
