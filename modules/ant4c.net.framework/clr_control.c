/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, 2022 TheVice
 *
 */

#include "clr_control.h"

#include "buffer.h"
#include "common.h"
#include "text_encoding.h"

#include <metahost.h>

#include <wchar.h>
#include <stddef.h>

uint8_t clr_control_get_clr_manager(void* control, const void* idd, void** manager)
{
	if (!control || !idd || !manager)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRControl*)control)->lpVtbl->GetCLRManager(control, idd, manager);
	return SUCCEEDED(result);
}

uint8_t clr_control_get_debug_manager(void* control, void** manager)
{
	if (!control || !manager)
	{
		return 0;
	}

	ICLRDebugManager* manager_ = NULL;
	const HRESULT result = clr_control_get_clr_manager(control, &IID_ICLRDebugManager, &manager_);
	*manager = manager_;
	return SUCCEEDED(result);
}

uint8_t clr_control_get_error_reporting_manager(void* control, void** manager)
{
	if (!control || !manager)
	{
		return 0;
	}

	ICLRErrorReportingManager* manager_ = NULL;
	const HRESULT result = clr_control_get_clr_manager(control, &IID_ICLRErrorReportingManager, &manager_);
	*manager = manager_;
	return SUCCEEDED(result);
}

uint8_t clr_control_get_gc_manager(void* control, void** manager)
{
	if (!control || !manager)
	{
		return 0;
	}

	ICLRGCManager* manager_ = NULL;
	const HRESULT result = clr_control_get_clr_manager(control, &IID_ICLRGCManager, &manager_);
	*manager = manager_;
	return SUCCEEDED(result);
}

uint8_t clr_control_get_gc_manager2(void* control, void** manager)
{
	if (!control || !manager)
	{
		return 0;
	}

	ICLRGCManager2* manager_ = NULL;
	const HRESULT result = clr_control_get_clr_manager(control, &IID_ICLRGCManager2, &manager_);
	*manager = manager_;
	return SUCCEEDED(result);
}

uint8_t clr_control_get_host_protection_manager(void* control, void** manager)
{
	if (!control || !manager)
	{
		return 0;
	}

	ICLRHostProtectionManager* manager_ = NULL;
	const HRESULT result = clr_control_get_clr_manager(control, &IID_ICLRHostProtectionManager, &manager_);
	*manager = manager_;
	return SUCCEEDED(result);
}

uint8_t clr_control_get_on_event_manager(void* control, void** manager)
{
	if (!control || !manager)
	{
		return 0;
	}

	ICLROnEventManager* manager_ = NULL;
	const HRESULT result = clr_control_get_clr_manager(control, &IID_ICLROnEventManager, &manager_);
	*manager = manager_;
	return SUCCEEDED(result);
}

uint8_t clr_control_get_policy_manager(void* control, void** manager)
{
	if (!control || !manager)
	{
		return 0;
	}

	ICLRPolicyManager* manager_ = NULL;
	const HRESULT result = clr_control_get_clr_manager(control, &IID_ICLRPolicyManager, &manager_);
	*manager = manager_;
	return SUCCEEDED(result);
}

uint8_t clr_control_get_task_manager(void* control, void** manager)
{
	if (!control || !manager)
	{
		return 0;
	}

	ICLRTaskManager* manager_ = NULL;
	const HRESULT result = clr_control_get_clr_manager(control, &IID_ICLRTaskManager, &manager_);
	*manager = manager_;
	return SUCCEEDED(result);
}

uint8_t clr_control_set_app_domain_manager_type_wchar_t(
	void* control, const wchar_t* app_domain_manager_assembly, const wchar_t* app_domain_manager_type)
{
	if (!control ||
		!app_domain_manager_assembly ||
		!app_domain_manager_type)
	{
		return 0;
	}

	const HRESULT result = ((struct ICLRControl*)control)->lpVtbl->SetAppDomainManagerType(
							   control, app_domain_manager_assembly, app_domain_manager_type);
	return SUCCEEDED(result);
}

uint8_t clr_control_set_app_domain_manager_type(
	void* control, const uint8_t* app_domain_manager_assembly, const uint8_t* app_domain_manager_type)
{
	if (!control ||
		!app_domain_manager_assembly ||
		!app_domain_manager_type)
	{
		return 0;
	}

	uint8_t arguments_W_buffer[BUFFER_SIZE_OF];
	void* arguments_W = (void*)arguments_W_buffer;

	if (!buffer_init(arguments_W, BUFFER_SIZE_OF))
	{
		return 0;
	}

	ptrdiff_t app_domain_manager_assembly_size = common_count_bytes_until(app_domain_manager_assembly, 0);
	ptrdiff_t app_domain_manager_type_size = common_count_bytes_until(app_domain_manager_type, 0);
	const ptrdiff_t size = app_domain_manager_assembly_size + app_domain_manager_type_size + 2 * sizeof(uint32_t);

	if (!buffer_append_wchar_t(arguments_W, NULL, size) ||
		!buffer_resize(arguments_W, 0))
	{
		buffer_release(arguments_W);
		return 0;
	}

	if (!text_encoding_UTF8_to_UTF16LE(
			app_domain_manager_assembly, app_domain_manager_assembly + app_domain_manager_assembly_size, arguments_W) ||
		!buffer_push_back_uint16_t(arguments_W, 0))
	{
		buffer_release(arguments_W);
		return 0;
	}

	app_domain_manager_assembly_size = buffer_size(arguments_W);

	if (!text_encoding_UTF8_to_UTF16LE(
			app_domain_manager_type, app_domain_manager_type + app_domain_manager_type_size, arguments_W) ||
		!buffer_push_back_uint16_t(arguments_W, 0))
	{
		buffer_release(arguments_W);
		return 0;
	}

	const wchar_t* app_domain_manager_assembly_w = buffer_wchar_t_data(arguments_W, 0);
	const wchar_t* app_domain_manager_type_w =
		(const wchar_t*)buffer_data(arguments_W, app_domain_manager_assembly_size);
	/**/
	app_domain_manager_type_size = clr_control_set_app_domain_manager_type_wchar_t(
									   control, app_domain_manager_assembly_w, app_domain_manager_type_w);
	/**/
	buffer_release(arguments_W);
	return 0 < app_domain_manager_type_size;
}
