/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 TheVice
 *
 */

#ifndef _BIT_CONVERTER_H_
#define _BIT_CONVERTER_H_

#include <stdint.h>

uint8_t bit_converter_is_little_endian();

uint8_t bit_converter_get_bytes_from_uint16_t(uint16_t input, uint8_t* output);
uint8_t bit_converter_get_bytes_from_uint32_t(uint32_t input, uint8_t* output);
uint8_t bit_converter_get_bytes_from_uint64_t(uint64_t input, uint8_t* output);
uint8_t bit_converter_to_uint16_t(const uint8_t* start, const uint8_t* finish, uint16_t* output);
uint8_t bit_converter_to_uint32_t(const uint8_t* start, const uint8_t* finish, uint32_t* output);
uint8_t bit_converter_to_uint64_t(const uint8_t* start, const uint8_t* finish, uint64_t* output);

#endif
