/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_NET_HOST_H__
#define __MODULES_NET_NET_HOST_H__

#include <stdint.h>

uint8_t net_result_to_string(
	const uint8_t* result, uint16_t result_length, void* output);
uint8_t net_host_get_hostfxr_path(
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	void* output);

#endif
