/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _ECHO_H_
#define _ECHO_H_

#include <stddef.h>
#include <stdint.h>

enum Level { Debug, Error, Info, None, Verbose, Warning, NoLevel };

uint8_t echo(uint8_t append, uint8_t encoding, const uint8_t* file, uint8_t level,
			 const uint8_t* message, ptrdiff_t message_length, uint8_t new_line, uint8_t verbose);
uint8_t echo_evaluate_task(const void* project, const void* target,
						   const uint8_t* attributes_start, const uint8_t* attributes_finish,
						   const uint8_t* element_finish);

#endif
