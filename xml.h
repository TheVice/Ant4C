/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#ifndef _XML_H_
#define _XML_H_

#include <stddef.h>
#include <stdint.h>

/*TODO: xmlpeek
xmlpoke*/

struct range;

const uint8_t* xml_get_tag_finish_pos(
	const uint8_t* start, const uint8_t* finish);
uint16_t xml_get_sub_nodes_elements(
	const uint8_t* start, const uint8_t* finish,
	const struct range* sub_nodes_names, void* elements);
const uint8_t* xml_get_tag_name(const uint8_t* start, const uint8_t* finish);
uint8_t xml_get_attribute_value(
	const uint8_t* start, const uint8_t* finish,
	const uint8_t* attribute, ptrdiff_t attribute_length,
	void* value);
uint8_t xml_get_element_value(
	const uint8_t* start, const uint8_t* finish, void* value);

#endif
