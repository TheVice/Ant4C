/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#ifndef _HASH_ALGORITHM_
#define _HASH_ALGORITHM_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

uint8_t hash_algorithm_uint8_t_array_to_uint32_t(
	const uint8_t* start, const uint8_t* finish, uint32_t* output);
uint8_t hash_algorithm_uint8_t_array_to_uint64_t(
	const uint8_t* start, const uint8_t* finish, uint64_t* output);
uint8_t hash_algorithm_bytes_to_string(
	const uint8_t* start, const uint8_t* finish, struct buffer* output);

uint8_t BLAKE2b_init(uint8_t hash_length, uint64_t* output);
uint8_t BLAKE2b_core(const uint8_t* start, const uint8_t* finish,
					 ptrdiff_t* bytes_compressed, uint64_t* output);
uint8_t BLAKE2b_final(const uint8_t* start, ptrdiff_t* bytes_compressed,
					  uint8_t bytes_remaining, uint64_t* output);

uint8_t hash_algorithm_blake2b(
	const uint8_t* start, const uint8_t* finish,
	uint16_t hash_length, struct buffer* output);

uint8_t BLAKE3_init(uint32_t* h, uint8_t h_length,
					uint32_t* m, uint8_t m_length,
					uint16_t stack_length);
uint8_t BLAKE3_core(const uint8_t* input, uint64_t length, uint32_t* m, uint8_t* l, uint32_t* h,
					uint8_t* compressed, uint32_t* t, uint8_t* stack, uint8_t* stack_length, uint8_t d);
uint8_t BLAKE3_final(const uint8_t* stack, uint8_t stack_length,
					 uint8_t compressed, uint32_t* t, uint32_t* h, uint32_t* m,
					 uint8_t l, uint8_t d, uint8_t hash_length, uint8_t* output);

uint8_t hash_algorithm_blake3(
	const uint8_t* start, const uint8_t* finish,
	uint16_t hash_length, struct buffer* output);

uint8_t hash_algorithm_crc32_init(uint32_t* output);
uint8_t hash_algorithm_crc32_core(
	const uint8_t* start, const uint8_t* finish, uint32_t* output);
uint8_t hash_algorithm_crc32_final(uint32_t* output, uint8_t order);

uint8_t hash_algorithm_crc32(
	const uint8_t* start, const uint8_t* finish, uint32_t* output, uint8_t order);

uint8_t hash_algorithm_sha3_init(
	uint16_t hash_length, uint8_t* rate_on_w, uint8_t* maximum_delta);
uint8_t hash_algorithm_sha3_core(
	const uint8_t* start, const uint8_t* finish,
	uint8_t* queue, uint8_t* queue_size, uint8_t maximum_delta,
	uint64_t* S, uint8_t rate_on_w);
uint8_t hash_algorithm_sha3_final(
	uint8_t is_sha3,
	uint8_t* queue, uint8_t queue_size, uint8_t maximum_delta,
	uint64_t* S, uint8_t rate_on_w, uint8_t d_max,
	uint8_t* output);

uint8_t hash_algorithm_keccak(
	const uint8_t* start, const uint8_t* finish,
	uint16_t hash_length, struct buffer* output);

uint8_t hash_algorithm_sha3(
	const uint8_t* start, const uint8_t* finish,
	uint16_t hash_length, struct buffer* output);

uint8_t hash_algorithm_XXH32_core(
	const uint8_t* start, const uint8_t* finish,
	uint8_t* queue, uint8_t* queue_size, uint8_t max_queue_size,
	uint32_t* accumulators,
	uint8_t* is_accumulators_initialized,
	uint32_t seed);
uint8_t hash_algorithm_XXH32_final(
	const uint8_t* queue_start, const uint8_t* queue_finish,
	uint32_t* accumulators, uint8_t is_accumulators_initialized,
	uint32_t seed, uint32_t* output);

uint8_t hash_algorithm_XXH32(
	const uint8_t* start, const uint8_t* finish,
	uint32_t seed, uint32_t* output);

uint8_t hash_algorithm_XXH64_core(
	const uint8_t* start, const uint8_t* finish,
	uint8_t* queue, uint8_t* queue_size, uint8_t max_queue_size,
	uint64_t* accumulators, uint8_t* is_accumulators_initialized,
	uint64_t seed);
uint8_t hash_algorithm_XXH64_final(
	const uint8_t* queue_start, const uint8_t* queue_finish,
	uint64_t* accumulators, uint8_t is_accumulators_initialized,
	uint64_t seed, uint64_t* output);

uint8_t hash_algorithm_XXH64(
	const uint8_t* start, const uint8_t* finish,
	uint64_t seed, uint64_t* output);

uint8_t hash_algorithm_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t hash_algorithm_exec_function(uint8_t function, const struct buffer* arguments,
									 uint8_t arguments_count, struct buffer* output);

uint8_t file_get_checksum(
	const uint8_t* path, const struct range* algorithm,
	const struct range* algorithm_parameter, struct buffer* output);

#endif
