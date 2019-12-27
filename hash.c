/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "hash.h"
#include "buffer.h"

#include <stdio.h>
#include <stddef.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

uint8_t hash_algorithm_bytes_to_string(const uint8_t* start, const uint8_t* finish,
									   struct buffer* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 2 * (finish - start) + 2))
	{
		return 0;
	}

	const ptrdiff_t max_size = buffer_size(output) - 3;
	char* ptr = (char*)buffer_data(output, size);

	while (start < finish && size < max_size)
	{
		if ((*start) < 16)
		{
			(*ptr) = '0';
			++ptr;
			++size;
		}

#if __STDC_SEC_API__
		const int32_t sz = sprintf_s(ptr, 3, "%x", *start);
#else
		const int32_t sz = sprintf(ptr, "%x", *start);
#endif
		ptr += sz;
		size += sz;
		++start;
	}

	return buffer_resize(output, size);
}
