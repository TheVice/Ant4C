/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_NET_DELEGATE_H__
#define __MODULES_NET_NET_DELEGATE_H__

enum net_delegate_types
{
	net_hdt_com_activation,
	net_hdt_load_in_memory_assembly,
	net_hdt_winrt_activation,
	net_hdt_com_register,
	net_hdt_com_unregister,
	net_hdt_load_assembly_and_get_function_pointer,
	net_hdt_get_function_pointer
};

#define NET_DELEGATE_MAX_VALUE (net_hdt_get_function_pointer + 1)

static const uint8_t* net_delegate_types_str[] =
{
	(const uint8_t*)"net_hdt_com_activation",
	(const uint8_t*)"net_hdt_load_in_memory_assembly",
	(const uint8_t*)"net_hdt_winrt_activation",
	(const uint8_t*)"net_hdt_com_register",
	(const uint8_t*)"net_hdt_com_unregister",
	(const uint8_t*)"net_hdt_load_assembly_and_get_function_pointer",
	(const uint8_t*)"net_hdt_get_function_pointer"
};

#endif
