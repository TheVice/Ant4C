/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "host_controller.h"

HRESULT STDMETHODCALLTYPE GetHostManager(void* this_, REFIID riid, void** object)
{
	(void)this_;
	(void)riid;

	if (object)
	{
		*object = NULL;
	}

	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE SetAppDomainManager(void* this_, DWORD app_domain_id, void* app_domain_manager)
{
	(void)app_domain_id;

	if (this_ && app_domain_manager)
	{
		struct HostController* real_this = (struct HostController*)this_;
		struct IUnknownStructure* real_app_domain_manager = (struct IUnknownStructure*)app_domain_manager;

		if (real_this->ptr &&
			real_app_domain_manager->ptr &&
			real_app_domain_manager->ptr->QueryInterface)
		{
			return real_app_domain_manager->ptr->QueryInterface(
					   real_app_domain_manager, real_this->ptr->domain_manager_id, &(real_this->ptr->default_domain_manager));
		}
	}

	return E_POINTER;
}

struct IUnknownStructure* STDMETHODCALLTYPE GetDomainManager(void* this_)
{
	if (this_)
	{
		struct HostController* real_this = (struct HostController*)this_;

		if (real_this->ptr &&
			real_this->ptr->AddRef &&
			real_this->ptr->default_domain_manager)
		{
			real_this->ptr->AddRef(this_);
			return real_this->ptr->default_domain_manager;
		}
	}

	return NULL;
}

uint8_t host_control_init(struct HostController* host, const GUID* domain_manager_id)
{
	if (!host ||
		!unknown_structure_init((struct IUnknownStructure*)host))
	{
		return 0;
	}

	host->ptr->domain_manager_id = domain_manager_id;
	host->ptr->default_domain_manager = NULL;
	host->ptr->GetHostManager = GetHostManager;
	host->ptr->SetAppDomainManager = SetAppDomainManager;
	host->ptr->GetDomainManager = GetDomainManager;
	/**/
	return 1;
}
