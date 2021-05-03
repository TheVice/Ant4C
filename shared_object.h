/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#ifndef _SHARED_OBJECT_H_
#define _SHARED_OBJECT_H_

#include <stdint.h>

void* shared_object_load(const uint8_t* path);
void* shared_object_get_procedure_address(void* object, const uint8_t* procedure_name);
void shared_object_unload(void* object);

#endif
