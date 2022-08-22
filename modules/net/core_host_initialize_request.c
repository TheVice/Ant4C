/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "core_host_initialize_request.h"
#include "arguments.h"
#include "net.common.h"

#include "buffer.h"

#include <string.h>

struct initialize_request_type
{
	size_t version;
	struct string_arguments_type config_keys;
	struct string_arguments_type config_values;
};

static uint8_t is_buffer_init = 0;

static uint8_t b_config_keys_[BUFFER_SIZE_OF];
static uint8_t b_config_values_[BUFFER_SIZE_OF];

void core_host_initialize_request_init_buffers()
{
	if (!is_buffer_init)
	{
		is_buffer_init =
			buffer_init((void*)b_config_keys_, BUFFER_SIZE_OF) &&
			buffer_init((void*)b_config_values_, BUFFER_SIZE_OF);
	}
}

void core_host_initialize_request_release_buffers()
{
	if (is_buffer_init)
	{
		buffer_release((void*)b_config_keys_);
		buffer_release((void*)b_config_values_);
	}
}

uint8_t core_host_initialize_request_init(
	void* initialize_request, size_t size)
{
	if (!initialize_request || size < sizeof(struct initialize_request_type))
	{
		return 0;
	}

	memset(initialize_request, 0, sizeof(struct initialize_request_type));
	return 1;
}

uint8_t core_host_initialize_request_set_config_keys(
	void* initialize_request,
	const uint8_t** keys, const uint16_t* keys_lengths,
	uint8_t count)
{
	core_host_initialize_request_init_buffers();
	SET_DATA_FOR_STRING_MEMBER_OF_STRUCTURE(
		initialize_request_type,
		initialize_request,
		keys,
		keys_lengths,
		count,
		config_keys,
		(void*)b_config_keys_);
}

uint8_t core_host_initialize_request_set_config_values(
	void* initialize_request,
	const uint8_t** values, const uint16_t* values_lengths,
	uint8_t count)
{
	core_host_initialize_request_init_buffers();
	SET_DATA_FOR_STRING_MEMBER_OF_STRUCTURE(
		initialize_request_type,
		initialize_request,
		values,
		values_lengths,
		count,
		config_values,
		(void*)b_config_values_);
}

static uint8_t g_core_host_initialize_request[CORE_HOST_INITIALIZE_REQUEST_SIZE];

void* core_host_initialize_request_get()
{
	return (void*)g_core_host_initialize_request;
}
