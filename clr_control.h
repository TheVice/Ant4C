/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef __CLR_CONTROL_H__
#define __CLR_CONTROL_H__

#include <stdint.h>

uint8_t clr_control_get_clr_manager(void* control, const void* idd, void** manager);
uint8_t clr_control_get_debug_manager(void* control, void** manager);
uint8_t clr_control_get_error_reporting_manager(void* control, void** manager);
uint8_t clr_control_get_gc_manager(void* control, void** manager);
uint8_t clr_control_get_gc_manager2(void* control, void** manager);
uint8_t clr_control_get_host_protection_manager(void* control, void** manager);
uint8_t clr_control_get_on_event_manager(void* control, void** manager);
uint8_t clr_control_get_policy_manager(void* control, void** manager);
uint8_t clr_control_get_task_manager(void* control, void** manager);

uint8_t clr_control_set_app_domain_manager_type(
	void* control, const uint8_t* app_domain_manager_assembly, const uint8_t* app_domain_manager_type);

#endif
