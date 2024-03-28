/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022, 2024 TheVice
 *
 */

#include "stdc_secure_api.h"

#include "buffer.h"
#if !defined(_WIN32) && defined(NDEBUG) && defined(__ALPINE_PREVENT_ILLEGAL_INSTRUCTION__)
#include "common.h"
#endif

#include <stdlib.h>
#include <string.h>

struct buffer
{
	void* data;
	ptrdiff_t size;
	ptrdiff_t capacity;
};

static struct buffer pool;
static uint8_t is_pool_init = 0;

#if defined(_WIN64) || defined(__amd64) || defined(__x86_64)
static const ptrdiff_t maximum_capacity = (ptrdiff_t)(INT32_MAX) + 1;
#else
static const ptrdiff_t maximum_capacity = 1073741824;
#endif

ptrdiff_t buffer_calculate_capacity(const ptrdiff_t size)
{
	ptrdiff_t capacity = 2;

	while (capacity < size && capacity < maximum_capacity)
	{
		capacity = capacity << 1;
	}

	return capacity < maximum_capacity ? capacity : maximum_capacity;
}

struct buffer* buffer_to_real_buffer(const void* the_buffer)
{
	void* real_buffer = NULL;
	ptrdiff_t i = 0;

	if (NULL == the_buffer)
	{
		return (struct buffer*)real_buffer;
	}

	while (NULL != (real_buffer = buffer_buffer_data(&pool, i++)))
	{
		if (the_buffer == real_buffer)
		{
			break;
		}
	}

	return (struct buffer*)real_buffer;
}

uint8_t buffer_size_of()
{
	return (uint8_t)sizeof(struct buffer);
}

uint8_t buffer_init(void* the_buffer, uint8_t size_of_buffer)
{
	if (NULL == the_buffer || size_of_buffer < sizeof(struct buffer))
	{
		return 0;
	}

	memset(the_buffer, 0, sizeof(struct buffer));
	return 1;
}

ptrdiff_t buffer_size(const void* the_buffer)
{
	return NULL == the_buffer ? 0 : ((const struct buffer*)the_buffer)->size;
}

uint8_t buffer_resize(void* the_buffer_, ptrdiff_t size)
{
	struct buffer* the_buffer = (struct buffer*)the_buffer_;

	if (NULL == the_buffer || size < 0 || the_buffer->size < 0)
	{
		return 0;
	}

	if (the_buffer->capacity < size)
	{
		const ptrdiff_t capacity = buffer_calculate_capacity(size);

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

		the_buffer->data = malloc(capacity);

		if (NULL == the_buffer->data)
		{
			return 0;
		}

		the_buffer->capacity = capacity;
	}

	the_buffer->size = size;
	return 1;
}

void buffer_release(void* the_buffer_)
{
	struct buffer* the_buffer = (struct buffer*)the_buffer_;

	if (NULL == the_buffer)
	{
		return;
	}

	if (NULL != the_buffer->data)
	{
		free(the_buffer->data);
		the_buffer->data = NULL;
	}

	the_buffer->size = 0;
	the_buffer->capacity = 0;
}

void buffer_release_inner_buffers(void* the_buffer)
{
	ptrdiff_t i = 0;
	void* inner_buffer = NULL;

	while (NULL != (inner_buffer = buffer_buffer_data((struct buffer*)the_buffer, i++)))
	{
		buffer_release(inner_buffer);
	}
}

void buffer_release_with_inner_buffers(void* the_buffer)
{
	buffer_release_inner_buffers(the_buffer);
	buffer_release(the_buffer);
}

uint8_t buffer_append(void* the_buffer_, const void* data, ptrdiff_t size)
{
	struct buffer* the_buffer = (struct buffer*)the_buffer_;

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
		const ptrdiff_t capacity = buffer_calculate_capacity(new_size);

		if (capacity < new_size)
		{
			return 0;
		}

		void* new_data = malloc(capacity);

		if (NULL == new_data)
		{
			return 0;
		}

		if (0 < the_buffer->size && NULL != the_buffer->data)
		{
#if defined(__STDC_LIB_EXT1__)

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
#if !defined(_WIN32) && defined(NDEBUG) && defined(__ALPINE_PREVENT_ILLEGAL_INSTRUCTION__)
		uint8_t* dst = (uint8_t*)the_buffer->data + the_buffer->size;
		MEM_CPY(dst, data, size);
#else
#if defined(__STDC_LIB_EXT1__)

		if (0 != memcpy_s((uint8_t*)the_buffer->data + the_buffer->size,
						  the_buffer->capacity - the_buffer->size, data, size))
		{
			return 0;
		}

#else
		memcpy((uint8_t*)the_buffer->data + the_buffer->size, data, size);
#endif
#endif
	}

	the_buffer->size += size;
	return 1;
}

uint8_t buffer_append_char(void* the_buffer, const char* data, ptrdiff_t data_count)
{
	return buffer_append(the_buffer, (const void*)data, sizeof(char) * data_count);
}

uint8_t buffer_append_wchar_t(void* the_buffer, const wchar_t* data, ptrdiff_t data_count)
{
	return buffer_append(the_buffer, (const void*)data, sizeof(wchar_t) * data_count);
}

uint8_t buffer_append_buffer(void* the_buffer, const void* data, ptrdiff_t data_count)
{
	return buffer_append(the_buffer, (const void*)data, sizeof(struct buffer) * data_count);
}

uint8_t buffer_append_data_from_buffer(void* the_buffer, const void* the_source_buffer)
{
	const struct buffer* the_source_buffer_ = (const struct buffer*)the_source_buffer;
	return NULL == the_source_buffer_ ?
		   0 : buffer_append(the_buffer, the_source_buffer_->data, the_source_buffer_->size);
}

void* buffer_data(const void* the_buffer_, ptrdiff_t index)
{
	const struct buffer* the_buffer = (const struct buffer*)the_buffer_;

	if (NULL == the_buffer ||
		NULL == the_buffer->data ||
		index < 0 ||
		the_buffer->size <= index)
	{
		return NULL;
	}

	return (void*)((uint8_t*)the_buffer->data + index);
}

char* buffer_char_data(const void* the_buffer, ptrdiff_t data_position)
{
	return (char*)buffer_data(the_buffer, sizeof(char) * data_position);
}

uint8_t* buffer_uint8_t_data(const void* the_buffer, ptrdiff_t data_position)
{
	return (uint8_t*)buffer_data(the_buffer, sizeof(uint8_t) * data_position);
}

wchar_t* buffer_wchar_t_data(const void* the_buffer, ptrdiff_t data_position)
{
	return (wchar_t*)buffer_data(the_buffer, sizeof(wchar_t) * data_position);
}

uint16_t* buffer_uint16_t_data(const void* the_buffer, ptrdiff_t data_position)
{
	return (uint16_t*)buffer_data(the_buffer, sizeof(uint16_t) * data_position);
}

uint32_t* buffer_uint32_t_data(const void* the_buffer, ptrdiff_t data_position)
{
	return (uint32_t*)buffer_data(the_buffer, sizeof(uint32_t) * data_position);
}

void* buffer_buffer_data(const void* the_buffer, ptrdiff_t data_position)
{
	return buffer_data(the_buffer, sizeof(struct buffer) * data_position);
}

uint8_t buffer_push_back(void* the_buffer, uint8_t data)
{
	return buffer_append(the_buffer, &data, 1);
}

uint8_t buffer_push_back_uint16_t(void* the_buffer, uint16_t data)
{
	return buffer_append(the_buffer, (const void*)&data, sizeof(uint16_t));
}

uint8_t buffer_push_back_uint32_t(void* the_buffer, uint32_t data)
{
	return buffer_append(the_buffer, (const void*)&data, sizeof(uint32_t));
}

uint8_t buffer_shrink_to_fit(void* the_buffer_)
{
	struct buffer* the_buffer = (struct buffer*)the_buffer_;

	if (NULL == the_buffer || the_buffer->size < 0 || the_buffer->capacity < 512)
	{
		return 0;
	}

	const ptrdiff_t capacity = buffer_calculate_capacity(the_buffer->size);

	if (capacity < the_buffer->capacity)
	{
		void* new_data = realloc(the_buffer->data, capacity);

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

uint8_t buffer_init_pool(void** the_buffer)
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
		if (!buffer_init(&pool, sizeof(struct buffer)) ||
			!buffer_append(&pool, NULL, pool_limit) ||
			!buffer_resize(&pool, 0))
		{
			buffer_release(&pool);
			return 0;
		}

		is_pool_init = 1;
	}

	while (NULL != (current_buffer = (struct buffer*)buffer_buffer_data(&pool, i++)))
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

		if (!buffer_init(&new_buffer, sizeof(struct buffer)) ||
			!buffer_append_buffer(&pool, &new_buffer, 1))
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
		void* current_buffer = NULL;
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
		return pool_is_free;
	}
	else
	{
		is_pool_init = buffer_init(&pool, sizeof(struct buffer));
	}

	return is_pool_init;
}
