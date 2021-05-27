/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __HOST_POLICY_H__
#define __HOST_POLICY_H__

#include "net.common.h"

#include <stddef.h>
#include <stdint.h>

uint8_t host_policy_load(
	const type_of_element* path_to_host_policy, void* ptr_to_host_policy_object, ptrdiff_t size);
void host_policy_unload(void* ptr_to_host_policy_object);

#endif
