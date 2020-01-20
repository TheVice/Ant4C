/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef _HASH_ALGORITHM_
#define _HASH_ALGORITHM_

#include <stdint.h>

/*struct buffer;*/

uint8_t BLAKE3(const uint8_t* start, const uint8_t* finish, uint8_t hash_length, uint8_t* output);
/*uint8_t hash_algorithm_blake3_256(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);*/

#endif
