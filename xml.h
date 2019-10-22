/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _XML_H_
#define _XML_H_

#include <stddef.h>
#include <stdint.h>

/*TODO: xmlpeek
xmlpoke*/

struct buffer;
struct range;

const char* xml_get_tag_finish_pos(const char* start, const char* finish);
uint16_t xml_get_sub_nodes_elements(const char* start, const char* finish, struct buffer* elements);
uint8_t xml_get_tag_name(const char* start, const char* finish, struct range* name);
uint8_t xml_get_attribute_value(const char* start, const char* finish,
								const char* attribute, ptrdiff_t attribute_length, struct range* value);
uint8_t xml_is_attribute_exists(const char* start, const char* finish,
								const char* attribute, ptrdiff_t attribute_length);
uint8_t xml_get_element_value_from_parts(const char* element_start, const char* element_finish,
		struct range* value);
uint8_t xml_get_element_value(const struct range* element, struct range* value);

#endif
