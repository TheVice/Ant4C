/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_ARGUMENTS_H__
#define __MODULES_NET_ARGUMENTS_H__

#include "net.common.h"

#include <stddef.h>
#include <stdint.h>

struct buffer;

uint8_t values_to_arguments(
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count,
	struct buffer* output, const type_of_element*** argv);
uint8_t value_to_system_path(
	const uint8_t* input, ptrdiff_t length, struct buffer* output);
uint8_t values_to_system_paths(
	const uint8_t** values, const uint16_t* values_lengths, ptrdiff_t* positions,
	uint8_t values_count, struct buffer* output);
uint8_t values_to_system_paths_(
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t values_count,
	struct buffer* output, const type_of_element*** argv);

#endif
