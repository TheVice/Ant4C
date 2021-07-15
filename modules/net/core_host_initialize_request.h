/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_CORE_HOST_INITIALIZE_REQUEST_H__
#define __MODULES_NET_CORE_HOST_INITIALIZE_REQUEST_H__

#include <stddef.h>
#include <stdint.h>

#define CORE_HOST_INITIALIZE_REQUEST_SIZE 64

void core_host_initialize_request_release_buffers();

uint8_t core_host_initialize_request_init(
	void* initialize_request, size_t size);

uint8_t core_host_initialize_request_set_config_keys(
	void* initialize_request,
	const uint8_t** keys, const uint16_t* keys_lengths,
	uint8_t count);
uint8_t core_host_initialize_request_set_config_values(
	void* initialize_request,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t count);

void* core_host_initialize_request_get();

#endif
