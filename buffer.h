/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <wchar.h>
#include <stddef.h>
#include <stdint.h>

struct buffer
{
	uint8_t* data;
	ptrdiff_t size;
	ptrdiff_t capacity;
};

#define SET_NULL_TO_BUFFER(A) \
	(A).data = NULL; \
	(A).size = 0; \
	(A).capacity = 0;

uint8_t buffer_init(void** the_buffer);

ptrdiff_t buffer_size(const struct buffer* the_buffer);

uint8_t buffer_resize(struct buffer* the_buffer, ptrdiff_t size);
void buffer_release(struct buffer* the_buffer);
void buffer_release_inner_buffers(struct buffer* buffer);
void buffer_release_with_inner_buffers(struct buffer* the_buffer);

uint8_t buffer_append(struct buffer* the_buffer, const uint8_t* data, ptrdiff_t size);
uint8_t buffer_append_char(struct buffer* the_buffer, const char* data, ptrdiff_t data_count);
uint8_t buffer_append_wchar_t(struct buffer* the_buffer, const wchar_t* data, ptrdiff_t data_count);
uint8_t buffer_append_buffer(struct buffer* the_buffer, const struct buffer* data, ptrdiff_t data_count);
uint8_t buffer_append_data_from_buffer(struct buffer* the_buffer, const struct buffer* data);

uint8_t* buffer_data(const struct buffer* the_buffer, ptrdiff_t index);
char* buffer_char_data(const struct buffer* the_buffer, ptrdiff_t data_position);
wchar_t* buffer_wchar_t_data(const struct buffer* the_buffer, ptrdiff_t data_position);
uint16_t* buffer_uint16_data(const struct buffer* the_buffer, ptrdiff_t data_position);
uint32_t* buffer_uint32_data(const struct buffer* the_buffer, ptrdiff_t data_position);
struct buffer* buffer_buffer_data(const struct buffer* the_buffer, ptrdiff_t data_position);

uint8_t buffer_push_back(struct buffer* the_buffer, uint8_t data);
uint8_t buffer_push_back_uint16(struct buffer* the_buffer, uint16_t data);
uint8_t buffer_push_back_uint32(struct buffer* the_buffer, uint32_t data);

uint8_t buffer_assing_to_uint16(struct buffer* the_buffer, const uint8_t* data, ptrdiff_t size);

uint8_t buffer_shrink_to_fit(struct buffer* the_buffer);

uint8_t buffer_return_to_pool(void* the_buffer);
uint8_t buffer_release_pool();

#endif
