/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019, 2022 TheVice
 *
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <wchar.h>
#include <stddef.h>
#include <stdint.h>

#define BUFFER_SIZE_OF (2 * sizeof(ptrdiff_t) + sizeof(void*))

uint8_t buffer_size_of();

uint8_t buffer_init(void* the_buffer, uint8_t size_of_buffer);

ptrdiff_t buffer_size(const void* the_buffer);

uint8_t buffer_resize(void* the_buffer, ptrdiff_t size);
void buffer_release(void* the_buffer);
void buffer_release_inner_buffers(void* the_buffer);
void buffer_release_with_inner_buffers(void* the_buffer);

uint8_t buffer_append(void* the_buffer, const void* data, ptrdiff_t size);
uint8_t buffer_append_char(void* the_buffer, const char* data, ptrdiff_t data_count);
uint8_t buffer_append_wchar_t(void* the_buffer, const wchar_t* data, ptrdiff_t data_count);
uint8_t buffer_append_buffer(void* the_buffer, const void* data, ptrdiff_t data_count);
uint8_t buffer_append_data_from_buffer(void* the_buffer, const void* the_source_buffer);

void* buffer_data(const void* the_buffer, ptrdiff_t index);
char* buffer_char_data(const void* the_buffer, ptrdiff_t data_position);
uint8_t* buffer_uint8_t_data(const void* the_buffer, ptrdiff_t data_position);
wchar_t* buffer_wchar_t_data(const void* the_buffer, ptrdiff_t data_position);
uint16_t* buffer_uint16_t_data(const void* the_buffer, ptrdiff_t data_position);
uint32_t* buffer_uint32_t_data(const void* the_buffer, ptrdiff_t data_position);
void* buffer_buffer_data(const void* the_buffer, ptrdiff_t data_position);

uint8_t buffer_push_back(void* the_buffer, uint8_t data);
uint8_t buffer_push_back_uint16_t(void* the_buffer, uint16_t data);
uint8_t buffer_push_back_uint32_t(void* the_buffer, uint32_t data);

uint8_t buffer_shrink_to_fit(void* the_buffer);

uint8_t buffer_init_pool(void** the_buffer);
uint8_t buffer_return_to_pool(void* the_buffer);
uint8_t buffer_release_pool();

#endif
