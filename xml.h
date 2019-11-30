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

const uint8_t* xml_get_tag_finish_pos(const uint8_t* start, const uint8_t* finish);
uint16_t xml_get_sub_nodes_elements(const uint8_t* start, const uint8_t* finish, struct buffer* elements);
uint8_t xml_get_tag_name(const uint8_t* start, const uint8_t* finish, struct range* name);
uint8_t xml_get_attribute_value(const uint8_t* start, const uint8_t* finish,
								const uint8_t* attribute, ptrdiff_t attribute_length, struct buffer* value);
uint8_t xml_get_element_value(const uint8_t* start, const uint8_t* finish, struct buffer* value);

#endif
