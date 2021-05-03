/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#include "unknown_structure.h"

#define COUNT	8

static uint8_t used = 0;
static ULONG counts[COUNT];
static void* pointers[COUNT];

ULONG* get_count(void* pointer)
{
	if (NULL == pointer)
	{
		return NULL;
	}

	for (uint8_t i = 0; i < used; ++i)
	{
		if (pointer == pointers[i])
		{
			return &(counts[i]);
		}
	}

	if (used < COUNT)
	{
		counts[used] = 0;
		pointers[used] = pointer;
		++used;
		/**/
		return &(counts[used - 1]);
	}

	return NULL;
}

ULONG STDMETHODCALLTYPE AddRef(void* this_)
{
	ULONG* count = get_count(this_);

	if (!count)
	{
		return 0;
	}

	(*count) = (*count) + 1;
	return (*count);
}

HRESULT STDMETHODCALLTYPE QueryInterface(void* this_, REFIID iid, void** object)
{
	(void)iid;

	if (!this_ || !object)
	{
		return E_POINTER;
	}

	*object = this_;
	AddRef(this_);
	return S_OK;
}

ULONG STDMETHODCALLTYPE Release(void* this_)
{
	ULONG* count = get_count(this_);

	if (!count)
	{
		return 0;
	}

	if (0 < (*count))
	{
		(*count) = (*count) - 1;
	}

	return (*count);
}

uint8_t unknown_structure_init(struct IUnknownStructure* structure)
{
	if (!structure ||
		!(structure->ptr) ||
		!get_count(structure))
	{
		return 0;
	}

	structure->ptr->AddRef = AddRef;
	structure->ptr->QueryInterface = QueryInterface;
	structure->ptr->Release = Release;
	/**/
	return 1;
}
