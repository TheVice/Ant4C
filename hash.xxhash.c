/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

/*
 * As reference used:
 * xxHash fast digest algorithm. Version 0.1.1 (10/10/18)
 * https://github.com/Cyan4973/xxHash/blob/ff5df558b7bad19bc060d756f4dbd528b202c820/doc/xxhash_spec.md
 */

#include "stdc_secure_api.h"

#include "hash.h"
#include "common.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ROTATE_LEFT_UINT32_T(VALUE, OFFSET)		\
	(((VALUE) << (OFFSET)) | (VALUE) >> (32 - (OFFSET)))

static const uint32_t PRIME32_1 = 0x9E3779B1U;
static const uint32_t PRIME32_2 = 0x85EBCA77U;
static const uint32_t PRIME32_3 = 0xC2B2AE3DU;
static const uint32_t PRIME32_4 = 0x27D4EB2FU;
static const uint32_t PRIME32_5 = 0x165667B1U;

#define XXH32_INIT_ACCUMULATORS(ACCUMULATORS, SEED, IS_CHUNK_ONE)							\
	if (!(ACCUMULATORS))																	\
	{																						\
		return 0;																			\
	}																						\
	\
	if (IS_CHUNK_ONE)																		\
	{																						\
		(ACCUMULATORS)[0] = (SEED) + PRIME32_5;												\
	}																						\
	else																					\
	{																						\
		(ACCUMULATORS)[0] = (SEED) + PRIME32_1 + PRIME32_2;									\
		(ACCUMULATORS)[1] = (SEED) + PRIME32_2;												\
		(ACCUMULATORS)[2] = (SEED);															\
		(ACCUMULATORS)[3] = (SEED) - PRIME32_1;												\
	}

#define XXH32_CORE_CHUNKS(START, FINISH, ACCUMULATORS)										\
	while (16 <= (FINISH) - (START))\
	{\
		for (uint8_t i = 0; i < 4; ++i, (START) += 4)\
		{\
			uint32_t result;																\
			\
			if (!hash_algorithm_uint8_t_array_to_uint32_t((START), (START) + 4, &result))	\
			{																				\
				return 0;																	\
			}																				\
			\
			(ACCUMULATORS)[i] += result * PRIME32_2;										\
			(ACCUMULATORS)[i] = ROTATE_LEFT_UINT32_T((ACCUMULATORS)[i], 13);				\
			(ACCUMULATORS)[i] *= PRIME32_1;													\
		}																					\
	}

#define XXH32_CORE_CHUNKS_POST(ACCUMULATORS)				\
	(ACCUMULATORS)[4] +=									\
			ROTATE_LEFT_UINT32_T((ACCUMULATORS)[0], 1) +	\
			ROTATE_LEFT_UINT32_T((ACCUMULATORS)[1], 7) +	\
			ROTATE_LEFT_UINT32_T((ACCUMULATORS)[2], 12) +	\
			ROTATE_LEFT_UINT32_T((ACCUMULATORS)[3], 18);

#define XXH32_FINAL(START, FINISH, LAST_ACCUMULATOR)									\
	for (; 4 <= (FINISH) - (START); (START) += 4)										\
	{																					\
		uint32_t result;																\
		\
		if (!hash_algorithm_uint8_t_array_to_uint32_t((START), (START) + 4, &result))	\
		{																				\
			return 0;																	\
		}																				\
		\
		(LAST_ACCUMULATOR) += result * PRIME32_3;										\
		(LAST_ACCUMULATOR) = ROTATE_LEFT_UINT32_T((LAST_ACCUMULATOR), 17) * PRIME32_4;	\
	}																					\
	\
	for (; 1 <= (FINISH)- (START); ++(START))											\
	{																					\
		(LAST_ACCUMULATOR) += ((uint32_t)(*(START))) * PRIME32_5;						\
		(LAST_ACCUMULATOR) = ROTATE_LEFT_UINT32_T((LAST_ACCUMULATOR), 11) * PRIME32_1;	\
	}																					\
	\
	(LAST_ACCUMULATOR) ^= (LAST_ACCUMULATOR) >> 15;										\
	(LAST_ACCUMULATOR) *= PRIME32_2;													\
	(LAST_ACCUMULATOR) ^= (LAST_ACCUMULATOR) >> 13;										\
	(LAST_ACCUMULATOR) *= PRIME32_3;													\
	(LAST_ACCUMULATOR) ^= (LAST_ACCUMULATOR) >> 16;

#define XXHASH_CORE(START, FINISH, QUEUE, QUEUE_SIZE, MAX_QUEUE_SIZE,								\
					ACCUMULATORS, IS_ACCUMULATORS_INITIALIZED, SEED,								\
					TYPE, INIT_ACCUMULATORS, CORE_CHUNKS)											\
if (!(START) ||																						\
	!(FINISH) ||																					\
	!(QUEUE) ||																						\
	!(ACCUMULATORS) ||																				\
	!(IS_ACCUMULATORS_INITIALIZED) ||																\
	(FINISH) < (START))																				\
{																									\
	return 0;																						\
}																									\
\
(ACCUMULATORS)[4] += (TYPE)((FINISH) - (START));													\
\
if (*(QUEUE_SIZE))																					\
{																									\
	const ptrdiff_t size = MIN((MAX_QUEUE_SIZE) - *(QUEUE_SIZE), (FINISH) - (START));				\
	uint8_t* queue_finish = (QUEUE) + *(QUEUE_SIZE);												\
	MEM_CPY_C(queue_finish, (START), size);															\
	*(QUEUE_SIZE) += (uint8_t)size;																	\
	(START) += size;																				\
}																									\
\
if (!(*(IS_ACCUMULATORS_INITIALIZED)))																\
{																									\
	if ((MAX_QUEUE_SIZE) == *(QUEUE_SIZE) || (MAX_QUEUE_SIZE) <= (FINISH) - (START))				\
	{																								\
		INIT_ACCUMULATORS((ACCUMULATORS), (SEED), 0)												\
		*(IS_ACCUMULATORS_INITIALIZED) = 1;															\
	}																								\
}																									\
\
if ((MAX_QUEUE_SIZE) == *(QUEUE_SIZE))																\
{																									\
	const uint8_t* queue_start = (QUEUE);															\
	const uint8_t* queue_finish = (QUEUE) + *(QUEUE_SIZE);											\
	CORE_CHUNKS(queue_start, queue_finish, (ACCUMULATORS))											\
	*(QUEUE_SIZE) = 0;																				\
}																									\
\
CORE_CHUNKS((START), (FINISH), (ACCUMULATORS));														\
\
if (start < finish)																					\
{																									\
	const uint8_t size = MIN((MAX_QUEUE_SIZE), (uint8_t)((FINISH) - (START)));						\
	MEM_CPY_C((QUEUE), (START), size);																\
	*(QUEUE_SIZE) += size;																			\
	(START) += size;																				\
}																									\
\
return (START) == (FINISH);

uint8_t hash_algorithm_XXH32_core(
	const uint8_t* start, const uint8_t* finish,
	uint8_t* queue, uint8_t* queue_size, uint8_t max_queue_size,
	uint32_t* accumulators,	uint8_t* is_accumulators_initialized,
	uint32_t seed)
{
	XXHASH_CORE(
		start, finish,
		queue, queue_size, max_queue_size,
		accumulators, is_accumulators_initialized,
		seed,
		uint32_t,
		XXH32_INIT_ACCUMULATORS,
		XXH32_CORE_CHUNKS);
}

#define XXHASH_FINAL(QUEUE_START, QUEUE_FINISH,														\
					 ACCUMULATORS, IS_ACCUMULATORS_INITIALIZED,										\
					 SEED, OUTPUT,																	\
					 INIT_ACCUMULATORS, CORE_CHUNKS, CORE_CHUNKS_POST,								\
					 FINAL)																			\
if (!(QUEUE_START) ||																				\
	!(QUEUE_FINISH) ||																				\
	!(ACCUMULATORS) ||																				\
	!(OUTPUT) ||																					\
	(QUEUE_FINISH) < (QUEUE_START))																	\
{																									\
	return 0;																						\
}																									\
\
if (!(IS_ACCUMULATORS_INITIALIZED))																	\
{																									\
	INIT_ACCUMULATORS((ACCUMULATORS), (SEED), 1)													\
	(ACCUMULATORS)[4] += (ACCUMULATORS)[0];															\
}																									\
else																								\
{																									\
	CORE_CHUNKS((QUEUE_START), (QUEUE_FINISH), (ACCUMULATORS));										\
	CORE_CHUNKS_POST(ACCUMULATORS);																	\
}																									\
\
FINAL((QUEUE_START), (QUEUE_FINISH), (ACCUMULATORS)[4]);											\
*(OUTPUT) = (ACCUMULATORS)[4];																		\
return 1;

uint8_t hash_algorithm_XXH32_final(
	const uint8_t* queue_start, const uint8_t* queue_finish,
	uint32_t* accumulators, uint8_t is_accumulators_initialized,
	uint32_t seed, uint32_t* output)
{
	XXHASH_FINAL(
		queue_start, queue_finish,
		accumulators, is_accumulators_initialized,
		seed, output,
		XXH32_INIT_ACCUMULATORS,
		XXH32_CORE_CHUNKS,
		XXH32_CORE_CHUNKS_POST,
		XXH32_FINAL)
}

uint8_t hash_algorithm_XXH32(
	const uint8_t* start, const uint8_t* finish,
	uint32_t seed, uint32_t* output)
{
	if (!start ||
		!finish ||
		!output ||
		finish < start)
	{
		return 0;
	}

	uint8_t queue[16];
	uint8_t queue_size = 0;
	const uint8_t max_queue_size = 16;
	uint32_t accumulators[5];
	uint8_t is_accumulators_initialized = 0;
	/**/
	accumulators[4] = 0;

	if (!hash_algorithm_XXH32_core(start, finish,
								   queue, &queue_size, max_queue_size,
								   accumulators, &is_accumulators_initialized, seed))
	{
		return 0;
	}

	return hash_algorithm_XXH32_final(
			   queue, queue + queue_size,
			   accumulators, is_accumulators_initialized, seed, output);
}

#define ROTATE_LEFT_UINT64_T(VALUE, OFFSET)		\
	(((VALUE) << (OFFSET)) | (VALUE) >> (64 - (OFFSET)))

#define XXH64_ROUND(OUT, IN, LINE)				\
	(OUT) = (IN) + (LINE) * PRIME64_2;			\
	(OUT) = ROTATE_LEFT_UINT64_T((OUT), 31);	\
	(OUT) *= PRIME64_1;

#define XXH64_MERGE(OUT, SUB_OUT, IN)			\
	XXH64_ROUND((SUB_OUT), 0, (IN));			\
	(OUT) ^= (SUB_OUT);							\
	(OUT) *= PRIME64_1;							\
	(OUT) += PRIME64_4;

static const uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;
static const uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
static const uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
static const uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
static const uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;

#define XXH64_INIT_ACCUMULATORS(ACCUMULATORS, SEED, IS_CHUNK_ONE)							\
	if (!(ACCUMULATORS))																	\
	{																						\
		return 0;																			\
	}																						\
	\
	if (IS_CHUNK_ONE)																		\
	{																						\
		(ACCUMULATORS)[0] = (SEED) + PRIME64_5;												\
	}																						\
	else																					\
	{																						\
		(ACCUMULATORS)[0] = (SEED) + PRIME64_1 + PRIME64_2;									\
		(ACCUMULATORS)[1] = (SEED) + PRIME64_2;												\
		(ACCUMULATORS)[2] = (SEED);															\
		(ACCUMULATORS)[3] = (SEED) - PRIME64_1;												\
	}

#define XXH64_CORE_CHUNKS(START, FINISH, ACCUMULATORS)										\
	while (32 <= (FINISH) - (START))														\
	{																						\
		for (uint8_t i = 0; i < 4; ++i, (START) += 8)										\
		{\
			uint64_t result;																\
			\
			if (!hash_algorithm_uint8_t_array_to_uint64_t((START), (START) + 8, &result))	\
			{																				\
				return 0;																	\
			}																				\
			\
			XXH64_ROUND((ACCUMULATORS)[i], (ACCUMULATORS)[i], result);						\
		}																					\
	}

#define XXH64_CORE_CHUNKS_POST(ACCUMULATORS)						\
	uint64_t result_1 =												\
			ROTATE_LEFT_UINT64_T((ACCUMULATORS)[0], 1) +			\
			ROTATE_LEFT_UINT64_T((ACCUMULATORS)[1], 7) +			\
			ROTATE_LEFT_UINT64_T((ACCUMULATORS)[2], 12) +			\
			ROTATE_LEFT_UINT64_T((ACCUMULATORS)[3], 18);			\
	\
	for (uint8_t i = 0; i < 4; ++i)									\
	{																\
		uint64_t result_2;											\
		XXH64_MERGE(result_1, result_2, (ACCUMULATORS)[i]);			\
	}																\
	\
	(ACCUMULATORS)[4] += result_1;

#define XXH64_FINAL(START, FINISH, LAST_ACCUMULATOR)									\
	for (; 8 <= (FINISH) - (START); (START) += 8)										\
	{																					\
		uint64_t result;																\
		\
		if (!hash_algorithm_uint8_t_array_to_uint64_t((START), (START) + 8, &result))	\
		{																				\
			return 0;																	\
		}																				\
		\
		XXH64_ROUND(result, 0, result);													\
		(LAST_ACCUMULATOR) ^= result;													\
		(LAST_ACCUMULATOR) = ROTATE_LEFT_UINT64_T((LAST_ACCUMULATOR), 27) * PRIME64_1;	\
		(LAST_ACCUMULATOR) += PRIME64_4;												\
	}																					\
	\
	for (; 4 <= (FINISH) - (START); (START) += 4)										\
	{																					\
		uint32_t result;																\
		\
		if (!hash_algorithm_uint8_t_array_to_uint32_t((START), (START) + 4, &result))	\
		{																				\
			return 0;																	\
		}																				\
		\
		(LAST_ACCUMULATOR) ^= (uint64_t)result * PRIME64_1;								\
		(LAST_ACCUMULATOR) = ROTATE_LEFT_UINT64_T((LAST_ACCUMULATOR), 23) * PRIME64_2;	\
		(LAST_ACCUMULATOR) += PRIME64_3;												\
	}																					\
	\
	for (; 1 <= (FINISH) - (START); ++(START))											\
	{																					\
		(LAST_ACCUMULATOR) ^= ((uint64_t)(*(START))) * PRIME64_5;						\
		(LAST_ACCUMULATOR) = ROTATE_LEFT_UINT64_T((LAST_ACCUMULATOR), 11) * PRIME64_1;	\
	}																					\
	\
	(LAST_ACCUMULATOR) ^= (LAST_ACCUMULATOR) >> 33;										\
	(LAST_ACCUMULATOR) *= PRIME64_2;													\
	(LAST_ACCUMULATOR) ^= (LAST_ACCUMULATOR) >> 29;										\
	(LAST_ACCUMULATOR) *= PRIME64_3;													\
	(LAST_ACCUMULATOR) ^= (LAST_ACCUMULATOR) >> 32;

uint8_t hash_algorithm_XXH64_core(
	const uint8_t* start, const uint8_t* finish,
	uint8_t* queue, uint8_t* queue_size, uint8_t max_queue_size,
	uint64_t* accumulators, uint8_t* is_accumulators_initialized,
	uint64_t seed)
{
	XXHASH_CORE(
		start, finish,
		queue, queue_size, max_queue_size,
		accumulators, is_accumulators_initialized,
		seed,
		uint64_t,
		XXH64_INIT_ACCUMULATORS,
		XXH64_CORE_CHUNKS);
}

uint8_t hash_algorithm_XXH64_final(
	const uint8_t* queue_start, const uint8_t* queue_finish,
	uint64_t* accumulators, uint8_t is_accumulators_initialized,
	uint64_t seed, uint64_t* output)
{
	XXHASH_FINAL(
		queue_start, queue_finish,
		accumulators, is_accumulators_initialized,
		seed, output,
		XXH64_INIT_ACCUMULATORS,
		XXH64_CORE_CHUNKS,
		XXH64_CORE_CHUNKS_POST,
		XXH64_FINAL)
}

uint8_t hash_algorithm_XXH64(
	const uint8_t* start, const uint8_t* finish,
	uint64_t seed, uint64_t* output)
{
	if (!start ||
		!finish ||
		!output ||
		finish < start)
	{
		return 0;
	}

	uint8_t queue[32];
	uint8_t queue_size = 0;
	const uint8_t max_queue_size = 32;
	uint64_t accumulators[5];
	uint8_t is_accumulators_initialized = 0;
	/**/
	accumulators[4] = 0;

	if (!hash_algorithm_XXH64_core(start, finish,
								   queue, &queue_size, max_queue_size,
								   accumulators, &is_accumulators_initialized, seed))
	{
		return 0;
	}

	return hash_algorithm_XXH64_final(
			   queue, queue + queue_size,
			   accumulators, is_accumulators_initialized, seed, output);
}
