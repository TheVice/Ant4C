/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _HASH_ALGORITHM_
#define _HASH_ALGORITHM_

#include <stdint.h>

struct buffer;

uint8_t hash_algorithm_bytes_to_string(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);

uint8_t hash_algorithm_crc32(
	const uint8_t* start, const uint8_t* finish, uint32_t* output, uint8_t order);

#endif
