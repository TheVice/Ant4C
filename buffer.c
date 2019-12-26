/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "buffer.h"

#include <stdlib.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

static struct buffer pool;
static uint8_t is_pool_init = 0;

#if defined(_WIN64) || defined(__amd64) || defined(__x86_64)
#define CAPACITY_TYPE uint32_t
static const uint32_t maximum_capacity = (uint32_t)(INT32_MAX) + (uint32_t)1;
#else
#define CAPACITY_TYPE int32_t
static const int32_t maximum_capacity = 1073741824;
#endif

CAPACITY_TYPE get_capacity(ptrdiff_t size)
{
	CAPACITY_TYPE capacity = 2;

	while (capacity < size && capacity < maximum_capacity)
	{
		capacity = capacity << 1;
	}

	return capacity < maximum_capacity ? capacity : maximum_capacity;
}

struct buffer* buffer_to_real_buffer(const void* the_buffer)
{
	struct buffer* real_buffer = NULL;
	ptrdiff_t i = 0;

	if (NULL == the_buffer)
	{
		return real_buffer;
	}

	while (NULL != (real_buffer = buffer_buffer_data(&pool, i++)))
	{
		if (the_buffer == real_buffer)
		{
			break;
		}
	}

	return real_buffer;
}

ptrdiff_t buffer_size(const struct buffer* the_buffer)
{
	return NULL == the_buffer ? 0 : the_buffer->size;
}

uint8_t buffer_resize(struct buffer* the_buffer, ptrdiff_t size)
{
	if (NULL == the_buffer || size < 0 || the_buffer->size < 0)
	{
		return 0;
	}

	if (the_buffer->capacity < size)
	{
		const CAPACITY_TYPE capacity = get_capacity(size);

		if (capacity < size)
		{
			return 0;
		}

		if (NULL != the_buffer->data)
		{
			free(the_buffer->data);
			the_buffer->data = NULL;
			the_buffer->size = 0;
			the_buffer->capacity = 0;
		}

		the_buffer->data = (uint8_t*)malloc(capacity);

		if (NULL == the_buffer->data)
		{
			return 0;
		}

		the_buffer->capacity = capacity;
	}

	the_buffer->size = size;
	return 1;
}

void buffer_release(struct buffer* buffer)
{
	if (NULL == buffer)
	{
		return;
	}

	if (NULL != buffer->data)
	{
		free(buffer->data);
		buffer->data = NULL;
	}

	buffer->size = 0;
	buffer->capacity = 0;
}

void buffer_release_inner_buffers(struct buffer* buffer)
{
	ptrdiff_t i = 0;
	struct buffer* inner_buffer = NULL;

	while (NULL != (inner_buffer = buffer_buffer_data(buffer, i++)))
	{
		buffer_release(inner_buffer);
	}
}

void buffer_release_with_inner_buffers(struct buffer* buffer)
{
	buffer_release_inner_buffers(buffer);
	buffer_release(buffer);
}

uint8_t buffer_append(struct buffer* the_buffer, const uint8_t* data, ptrdiff_t size)
{
	if (NULL == the_buffer || size < 0 || the_buffer->size < 0)
	{
		return 0;
	}

	if (!size)
	{
		return 1;
	}

	if (the_buffer->capacity - the_buffer->size < size)
	{
		if ((maximum_capacity - the_buffer->capacity) < size)
		{
			return 0;
		}

		const ptrdiff_t new_size = the_buffer->capacity + size;
		const CAPACITY_TYPE capacity = get_capacity(new_size);

		if (capacity < new_size)
		{
			return 0;
		}

		uint8_t* new_data = (uint8_t*)malloc(capacity);

		if (NULL == new_data)
		{
			return 0;
		}

		if (0 < the_buffer->size && NULL != the_buffer->data)
		{
#if __STDC_SEC_API__

			if (0 != memcpy_s(new_data, capacity, the_buffer->data, the_buffer->size))
			{
				free(new_data);
				new_data = NULL;
				return 0;
			}

#else
			memcpy(new_data, the_buffer->data, the_buffer->size);
#endif
		}

		if (NULL != the_buffer->data)
		{
			free(the_buffer->data);
		}

		the_buffer->data = new_data;
		the_buffer->capacity = capacity;
	}

	if (NULL != data)
	{
#if __STDC_SEC_API__

		if (0 != memcpy_s(&the_buffer->data[the_buffer->size], the_buffer->capacity - the_buffer->size, data, size))
		{
			return 0;
		}

#else
		memcpy(&the_buffer->data[the_buffer->size], data, size);
#endif
	}

	the_buffer->size += size;
	return 1;
}

uint8_t buffer_append_char(struct buffer* the_buffer, const char* data, ptrdiff_t data_count)
{
	return buffer_append(the_buffer, (const uint8_t*)data, sizeof(char) * data_count);
}

uint8_t buffer_append_wchar_t(struct buffer* the_buffer, const wchar_t* data, ptrdiff_t data_count)
{
	return buffer_append(the_buffer, (const uint8_t*)data, sizeof(wchar_t) * data_count);
}

uint8_t buffer_append_buffer(struct buffer* the_buffer, const struct buffer* data, ptrdiff_t data_count)
{
	return buffer_append(the_buffer, (const uint8_t*)data, sizeof(struct buffer) * data_count);
}

uint8_t buffer_append_data_from_buffer(struct buffer* the_buffer, const struct buffer* data)
{
	return NULL == data ? 0 : buffer_append(the_buffer, data->data, data->size);
}

uint8_t* buffer_data(const struct buffer* the_buffer, ptrdiff_t index)
{
	if (NULL == the_buffer ||
		NULL == the_buffer->data ||
		index < 0 ||
		the_buffer->size <= index)
	{
		return NULL;
	}

	return &the_buffer->data[index];
}

char* buffer_char_data(const struct buffer* the_buffer, ptrdiff_t data_position)
{
	return (char*)buffer_data(the_buffer, sizeof(char) * data_position);
}

wchar_t* buffer_wchar_t_data(const struct buffer* the_buffer, ptrdiff_t data_position)
{
	return (wchar_t*)buffer_data(the_buffer, sizeof(wchar_t) * data_position);
}

uint16_t* buffer_uint16_data(const struct buffer* the_buffer, ptrdiff_t data_position)
{
	return (uint16_t*)buffer_data(the_buffer, sizeof(uint16_t) * data_position);
}

uint32_t* buffer_uint32_data(const struct buffer* the_buffer, ptrdiff_t data_position)
{
	return (uint32_t*)buffer_data(the_buffer, sizeof(uint32_t) * data_position);
}

struct buffer* buffer_buffer_data(const struct buffer* the_buffer, ptrdiff_t data_position)
{
	return (struct buffer*)buffer_data(the_buffer, sizeof(struct buffer) * data_position);
}

uint8_t buffer_push_back(struct buffer* the_buffer, uint8_t data)
{
	return buffer_append(the_buffer, &data, 1);
}

uint8_t buffer_push_back_uint16(struct buffer* the_buffer, uint16_t data)
{
	return buffer_append(the_buffer, (const uint8_t*)&data, sizeof(uint16_t));
}

uint8_t buffer_push_back_uint32(struct buffer* the_buffer, uint32_t data)
{
	return buffer_append(the_buffer, (const uint8_t*)&data, sizeof(uint32_t));
}

uint8_t buffer_assing_to_uint16(struct buffer* the_buffer, const uint8_t* data, ptrdiff_t size)
{
	if (NULL == the_buffer ||
		NULL == data)
	{
		return 0;
	}

	ptrdiff_t i = buffer_size(the_buffer);

	if (!buffer_append(the_buffer, NULL, sizeof(uint16_t) * size) ||
		!buffer_resize(the_buffer, i))
	{
		return 0;
	}

	for (i = 0; i < size; ++i)
	{
		if (!buffer_push_back_uint16(the_buffer, data[i]))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t buffer_shrink_to_fit(struct buffer* the_buffer)
{
	if (NULL == the_buffer || the_buffer->size < 0 || the_buffer->capacity < 512)
	{
		return 0;
	}

	const CAPACITY_TYPE capacity = get_capacity(the_buffer->size);

	if (capacity < the_buffer->capacity)
	{
		uint8_t* new_data = (uint8_t*)realloc(the_buffer->data, capacity);

		if (NULL == new_data)
		{
			return 0;
		}

		free(the_buffer->data);
		the_buffer->data = new_data;
		the_buffer->capacity = capacity;
		return 1;
	}

	return 0;
}

uint8_t buffer_init(void** the_buffer)
{
	static const ptrdiff_t pool_limit = (ptrdiff_t)(sizeof(struct buffer) * UINT8_MAX);
	struct buffer* current_buffer = NULL;
	struct buffer* candidate_buffer = NULL;
	ptrdiff_t i = 0;
	ptrdiff_t capacity = 0;

	if (NULL == the_buffer)
	{
		return 0;
	}

	if (!is_pool_init)
	{
		SET_NULL_TO_BUFFER(pool);

		if (!buffer_append(&pool, NULL, pool_limit) ||
			!buffer_resize(&pool, 0))
		{
			buffer_release(&pool);
			return 0;
		}

		is_pool_init = 1;
	}

	while (NULL != (current_buffer = buffer_buffer_data(&pool, i++)))
	{
		if (buffer_size(current_buffer) < 0 && capacity < current_buffer->capacity)
		{
			capacity = current_buffer->capacity;
			candidate_buffer = current_buffer;
		}
	}

	if (NULL != candidate_buffer)
	{
		candidate_buffer->size = 0;
		*the_buffer = candidate_buffer;
	}
	else if (buffer_size(&pool) < pool_limit)
	{
		struct buffer new_buffer;
		SET_NULL_TO_BUFFER(new_buffer);

		if (!buffer_append_buffer(&pool, &new_buffer, 1))
		{
			return 0;
		}
		else
		{
			*the_buffer = buffer_buffer_data(&pool, i - 1);
		}
	}
	else
	{
		return 0;
	}

	return NULL != *the_buffer;
}

uint8_t buffer_return_to_pool(void* the_buffer)
{
	struct buffer* real_buffer = buffer_to_real_buffer(the_buffer);

	if (NULL == real_buffer)
	{
		return 0;
	}

	real_buffer->size = -1;
	return 1;
}

uint8_t buffer_release_pool()
{
	if (is_pool_init)
	{
		struct buffer* current_buffer = NULL;
		ptrdiff_t i = 0;
		uint8_t pool_is_free = 1;

		while (NULL != (current_buffer = buffer_buffer_data(&pool, i++)))
		{
			if (pool_is_free && !(buffer_size(current_buffer) < 0))
			{
				pool_is_free = 0;
			}

			buffer_release(current_buffer);
		}

		buffer_release(&pool);
		SET_NULL_TO_BUFFER(pool);
		return pool_is_free;
	}
	else
	{
		SET_NULL_TO_BUFFER(pool);
		is_pool_init = 1;
	}

	return 1;
}
