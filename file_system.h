/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#if defined(_WIN32)
#include <wchar.h>
#endif

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

uint8_t file_close(void* stream);
uint8_t file_open(const uint8_t* path, const uint8_t* mode, void** output);
size_t file_read(void* content, const size_t size_of_content_element,
				 const size_t count_of_elements, void* stream);
uint8_t file_read_with_several_steps(void* stream, struct buffer* content);
size_t file_write(const void* content, const size_t size_of_content_element,
				  const size_t count_of_elements, void* stream);

#endif
