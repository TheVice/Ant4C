/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_NET_FILE_H__
#define __MODULES_NET_NET_FILE_H__

#include <stdint.h>

struct buffer;

uint8_t file_is_assembly(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values,
	const uint16_t* values_lengths,
	uint8_t values_count,
	struct buffer* output);

#endif
