/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef __UNKNOWN_STRUCTURE_H__
#define __UNKNOWN_STRUCTURE_H__

#include <stdint.h>

#include <unknwn.h>

#define IUNKNOWN_METHODS(TYPE)															\
	HRESULT(STDMETHODCALLTYPE* QueryInterface)(TYPE* this_, REFIID iid, void** object);	\
	ULONG(STDMETHODCALLTYPE* AddRef)(TYPE* this_);										\
	ULONG(STDMETHODCALLTYPE* Release)(TYPE* this_);

struct IUnknownStructure;

struct IUnknownStructurePtr
{
	IUNKNOWN_METHODS(struct IUnknownStructure)
};

struct IUnknownStructure
{
	struct IUnknownStructurePtr* ptr;
};

uint8_t unknown_structure_init(struct IUnknownStructure* structure);

#endif
