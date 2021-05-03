/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#ifndef _DEFAULT_LISTENER_H_
#define _DEFAULT_LISTENER_H_

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)

#  ifndef DEFAULT_LISTENER_EXPORT
#    ifdef default_listener_EXPORTS
#      define DEFAULT_LISTENER_EXPORT __declspec(dllexport)
#    else
#      define DEFAULT_LISTENER_EXPORT __declspec(dllimport)
#    endif
#  endif

#else

#  ifndef DEFAULT_LISTENER_EXPORT
#    ifdef example_of_the_module_EXPORTS
#      define DEFAULT_LISTENER_EXPORT __attribute__((visibility("default")))
#    else
#      define DEFAULT_LISTENER_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#endif

void DEFAULT_LISTENER_EXPORT listener_project_started(const uint8_t* source, const uint8_t* the_project);
void DEFAULT_LISTENER_EXPORT listener_project_finished(
	const uint8_t* source, const uint8_t* the_project, uint8_t result);

void DEFAULT_LISTENER_EXPORT listener_target_started(const uint8_t* source, ptrdiff_t offset,
		const uint8_t* the_project, const uint8_t* the_target);
void DEFAULT_LISTENER_EXPORT listener_target_finished(const uint8_t* source, ptrdiff_t offset,
		const uint8_t* the_project, const uint8_t* the_target, uint8_t result);

void DEFAULT_LISTENER_EXPORT listener_task_started(const uint8_t* source, ptrdiff_t offset,
		const uint8_t* the_project, const uint8_t* the_target, const uint8_t* task);
void DEFAULT_LISTENER_EXPORT listener_task_finished(const uint8_t* source, ptrdiff_t offset,
		const uint8_t* the_project, const uint8_t* the_target, const uint8_t* task, uint8_t result);

#endif
