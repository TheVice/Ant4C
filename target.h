/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _TARGET_H_
#define _TARGET_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

uint8_t target_exists(const struct buffer* targets, const char* name, uint8_t name_length);
uint8_t target_get_current_target(const void* target, const char** name, ptrdiff_t* name_length);
uint8_t target_has_executed(const struct buffer* targets, const char* name, uint8_t name_length);

uint8_t target_get(const struct buffer* targets, const char* name, uint8_t name_length, void** target);
uint8_t target_add(struct buffer* targets,
				   const struct range* name, const struct range* description,
				   const struct range* depends, const struct range* content);
uint8_t target_add_from_xml_tag_record(struct buffer* targets,
									   const char* record_start, const char* record_finish);
void target_clear(struct buffer* targets);

#endif
