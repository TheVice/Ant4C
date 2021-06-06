/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#ifndef __HOST_CONTROLLER_H__
#define __HOST_CONTROLLER_H__

#include "unknown_structure.h"

#include <stdint.h>

#define IHOSTCONTROL_METHODS(TYPE)																				\
	HRESULT(STDMETHODCALLTYPE* GetHostManager)(TYPE* this_, REFIID riid, void** object);						\
	HRESULT(STDMETHODCALLTYPE* SetAppDomainManager)(TYPE* this_, DWORD app_domain_id, void* app_domain_manager);

struct HostController;

struct HostControllerPtr
{
	IUNKNOWN_METHODS(struct HostController)
	IHOSTCONTROL_METHODS(struct HostController)
	struct IUnknownStructure* (STDMETHODCALLTYPE* GetDomainManager)(struct HostController* this_);
	void* default_domain_manager;
	const GUID* domain_manager_id;
};

struct HostController
{
	struct HostControllerPtr* ptr;
};

uint8_t host_control_init(struct HostController* host, const GUID* domain_manager_id);

#endif
