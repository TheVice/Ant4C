/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _HASH_ALGORITHM_
#define _HASH_ALGORITHM_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

uint8_t hash_algorithm_bytes_to_string(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);

uint8_t BLAKE2b_init(uint8_t hash_length, uint64_t* output);
uint8_t BLAKE2b_core(const uint8_t* start, const uint8_t* finish,
					 ptrdiff_t* bytes_compressed, uint64_t* output);
uint8_t BLAKE2b_final(const uint8_t* start, ptrdiff_t* bytes_compressed,
					  uint8_t bytes_remaining, uint64_t* output);
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

uint8_t hash_algorithm_crc32_init(uint32_t* output);
uint8_t hash_algorithm_crc32_core(
	const uint8_t* start, const uint8_t* finish, uint32_t* output);
uint8_t hash_algorithm_crc32_final(uint32_t* output, uint8_t order);
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

uint8_t hash_algorithm_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t hash_algorithm_exec_function(uint8_t function, const struct buffer* arguments,
									 uint8_t arguments_count, struct buffer* output);

uint8_t file_get_checksum(const uint8_t* path, const struct range* algorithm, struct buffer* output);

#endif
