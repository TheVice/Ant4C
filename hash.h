/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _HASH_ALGORITHM_
#define _HASH_ALGORITHM_

#include <stdint.h>

struct buffer;

uint8_t hash_algorithm_bytes_to_string(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);

uint8_t hash_algorithm_blake2b_160(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_blake2b_256(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_blake2b_384(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_blake2b_512(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);

uint8_t hash_algorithm_blake3_256(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);

uint8_t hash_algorithm_crc32(
	const uint8_t* start, const uint8_t* finish, uint32_t* output, uint8_t order);

uint8_t hash_algorithm_keccak_224(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_keccak_256(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_keccak_384(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_keccak_512(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);

uint8_t hash_algorithm_sha3_224(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_sha3_256(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_sha3_384(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);
uint8_t hash_algorithm_sha3_512(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);

#endif
