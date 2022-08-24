/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020, 2022 TheVice
 *
 */

#ifndef _ECHO_H_
#define _ECHO_H_

#include <stddef.h>
#include <stdint.h>

enum Level { Debug, Error, Info, None, Verbose, Warning };

#define ECHO_UNKNOWN_LEVEL (Warning + 1)

void echo_set_level(uint8_t level, uint8_t enable);

uint8_t echo(
	uint8_t append, uint8_t encoding, const uint8_t* file, uint8_t level,
	const uint8_t* message, ptrdiff_t message_length, uint8_t new_line, uint8_t verbose);

#endif
