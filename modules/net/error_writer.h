/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_ERROR_WRITER_H__
#define __MODULES_NET_ERROR_WRITER_H__

#include "net.common.h"

#include <stdint.h>

typedef error_writer_type(*setter_type)(const void* library_object, error_writer_type writer);

error_writer_type set_error_writer(
	const void* library_object, error_writer_type writer,
	setter_type setter_function, const type_of_element* path,
	uint8_t writer_number);

void host_fx_resolver_error_writer(const type_of_element* message);
void host_policy_error_writer(const type_of_element* message);
void error_writer_release_buffers(void* host_fxr_object, void* host_policy_object);

#endif
