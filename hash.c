/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 https://github.com/TheVice/
 *
 */

#include "hash.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "range.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

uint8_t hash_algorithm_uint8_t_array_to_uint32_t(
	const uint8_t* start, const uint8_t* finish, uint32_t* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	uint8_t j = 0;
	(*output) = 0;

	while ((--finish) > (start - 1) && j < 8)
	{
		(*output) += (uint32_t)(((*finish) & 0xF0) >> 4) * ((uint32_t)1 << (4 * (7 - (j++))));
		(*output) += (uint32_t)((*finish) & 0x0F) * ((uint32_t)1 << (4 * (7 - (j++))));
	}

	return 1;
}

uint8_t hash_algorithm_uint8_t_array_to_uint64_t(
	const uint8_t* start, const uint8_t* finish, uint64_t* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	uint8_t j = 0;
	(*output) = 0;

	while ((--finish) > (start - 1) && j < 16)
	{
		(*output) += (uint64_t)(((*finish) & 0xF0) >> 4) * ((uint64_t)1 << (4 * (15 - (j++))));
		(*output) += (uint64_t)((*finish) & 0x0F) * ((uint64_t)1 << (4 * (15 - (j++))));
	}

	return 1;
}

uint8_t hash_algorithm_bytes_to_string(
	const uint8_t* start, const uint8_t* finish, struct buffer* output)
{
	if (NULL == start ||
		NULL == finish ||
		finish < start ||
		NULL == output)
	{
		return 0;
	}

	if (finish == start)
	{
		return 1;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 2 * (finish - start) + 2))
	{
		return 0;
	}

	const ptrdiff_t max_size = buffer_size(output) - 3;
	char* ptr = (char*)buffer_data(output, size);

	while (start < finish && size < max_size)
	{
#if __STDC_SEC_API__
		const int32_t sz = sprintf_s(ptr, 3, (*start) < 16 ? "0%x" : "%x", *start);
#else
		const int32_t sz = sprintf(ptr, (*start) < 16 ? "0%x" : "%x", *start);
#endif
		ptr += sz;
		size += sz;
		++start;
	}

	return buffer_resize(output, size);
}

static const uint8_t* crc32_parameters_str[] =
{
	(const uint8_t*)"decreasing",
	(const uint8_t*)"increasing"
};

enum crc32_parameters
{
	decreasing,
	increasing,
	UNKNOWN_CRC32_PARAMETER
};

static const uint8_t* hash_function_str[] =
{
	(const uint8_t*)"blake2b",
	(const uint8_t*)"blake3",
	(const uint8_t*)"bytes-to-string",
	(const uint8_t*)"crc32",
	(const uint8_t*)"keccak",
	(const uint8_t*)"sha3",
	(const uint8_t*)"xxh32",
	(const uint8_t*)"xxh64"
};

enum hash_function
{
	blake2b, blake3,
	bytes_to_string,
	crc32,
	keccak, sha3,
	xxh32, xxh64,
	UNKNOWN_HASH_FUNCTION
};

uint8_t hash_algorithm_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	return common_string_to_enum(name_start, name_finish, hash_function_str, UNKNOWN_HASH_FUNCTION);
}

uint8_t hash_algorithm_exec_function(uint8_t function, const struct buffer* arguments,
									 uint8_t arguments_count, struct buffer* output)
{
	if (UNKNOWN_HASH_FUNCTION <= function ||
		NULL == arguments ||
		arguments_count < 1 || 2 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range values[2];

	if (!common_get_arguments(arguments, arguments_count, values,
							  crc32 != function &&
							  bytes_to_string != function))
	{
		return 0;
	}

	uint16_t hash_length = 256;

	if (2 == arguments_count &&
		crc32 != function &&
		bytes_to_string != function)
	{
		hash_length = (uint16_t)int_parse(values[1].start);
	}

	if (NULL == values[0].start)
	{
		values[0].start = values[0].finish = (const uint8_t*)&values[0];
	}

	switch (function)
	{
		case blake2b:
			return (1 == arguments_count || 2 == arguments_count) &&
				   hash_algorithm_blake2b(values[0].start, values[0].finish, hash_length, output);

		case blake3:
			return (1 == arguments_count || 2 == arguments_count) &&
				   hash_algorithm_blake3(values[0].start, values[0].finish, hash_length, output);

		case bytes_to_string:
			return 1 == arguments_count && hash_algorithm_bytes_to_string(values[0].start, values[0].finish, output);

		case crc32:
		{
			hash_length = 1;

			if (2 == arguments_count)
			{
				hash_length = common_string_to_enum(
								  values[1].start, values[1].finish,
								  crc32_parameters_str, UNKNOWN_CRC32_PARAMETER);

				if (UNKNOWN_CRC32_PARAMETER == hash_length)
				{
					break;
				}
			}

			if (!buffer_push_back_uint32(output, 0))
			{
				break;
			}

			if (NULL == values[0].start)
			{
				values[0].start = values[0].finish = (const uint8_t*)&values[0];
			}

			uint32_t* ptr = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));
			return hash_algorithm_crc32(values[0].start, values[0].finish, ptr, (uint8_t)hash_length);
		}

		case keccak:
			return (1 == arguments_count || 2 == arguments_count) &&
				   hash_algorithm_keccak(values[0].start, values[0].finish, hash_length, output);

		case sha3:
			return (1 == arguments_count || 2 == arguments_count) &&
				   hash_algorithm_sha3(values[0].start, values[0].finish, hash_length, output);

		case xxh32:
		{
			if (!buffer_push_back_uint32(output, 0))
			{
				return 0;
			}

			if (!arguments_count || 2 < arguments_count)
			{
				break;
			}

			uint32_t seed = 0;

			if (2 == arguments_count)
			{
				seed = (uint32_t)uint64_parse(values[1].start, values[1].finish);
			}

			uint32_t* ptr = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));
			return hash_algorithm_XXH32(values[0].start, values[0].finish, seed, ptr);
		}

		case xxh64:
		{
			if (!buffer_append(output, NULL, sizeof(uint64_t)))
			{
				return 0;
			}

			if (!arguments_count || 2 < arguments_count)
			{
				break;
			}

			uint64_t seed = 0;

			if (2 == arguments_count)
			{
				seed = uint64_parse(values[1].start, values[1].finish);
			}

			uint64_t* ptr = (uint64_t*)buffer_data(output, buffer_size(output) - sizeof(uint64_t));
			return hash_algorithm_XXH64(values[0].start, values[0].finish, seed, ptr);
		}

		case UNKNOWN_HASH_FUNCTION:
		default:
			break;
	}

	return 0;
}

uint8_t file_get_checksum_(const uint8_t* path, uint8_t algorithm,
						   const struct range* algorithm_parameter, struct buffer* output)
{
	if (NULL == path ||
		bytes_to_string == algorithm ||
		UNKNOWN_HASH_FUNCTION == algorithm ||
		NULL == output)
	{
		return 0;
	}

	uint16_t hash_length = crc32 == algorithm ? 1 : 256;

	if (!range_is_null_or_empty(algorithm_parameter))
	{
		if (crc32 == algorithm)
		{
			hash_length = common_string_to_enum(
							  algorithm_parameter->start, algorithm_parameter->finish,
							  crc32_parameters_str, UNKNOWN_CRC32_PARAMETER);

			if (UNKNOWN_CRC32_PARAMETER == hash_length)
			{
				return 0;
			}
		}
		else if (xxh32 != algorithm &&
				 xxh64 != algorithm)
		{
			hash_length = (uint16_t)int_parse(algorithm_parameter->start);

			if (hash_length < 8 || 1024 < hash_length)
			{
				return 0;
			}
		}
	}

	void* file = NULL;

	if (!file_open(path, (const uint8_t*)"rb", &file))
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	switch (algorithm)
	{
		case blake2b:
		{
			hash_length = hash_length / 8;

			if (!buffer_append(output, NULL, 64 + 128 + 4096 + 128))
			{
				break;
			}

			uint64_t* out = (uint64_t*)buffer_data(output, size + 64);

			if (!BLAKE2b_init((uint8_t)hash_length, out))
			{
				break;
			}

			size_t readed = 0;
			uint8_t* last = NULL;
			ptrdiff_t bytes_compressed = 0;
			uint8_t* file_content = buffer_data(output, size + 64 + 128);

			while (0 < (readed = file_read(file_content, sizeof(uint8_t), 4096, file)))
			{
				if (last && !BLAKE2b_core(last, last + 128, &bytes_compressed, out))
				{
					file_close(file);
					return 0;
				}

				last = NULL;

				if (4096 != readed)
				{
					if (128 < readed)
					{
						uint16_t bytes_to_compress = (uint16_t)(128 * (readed / 128));

						if (0 == readed % 128)
						{
							bytes_to_compress -= 128;
						}

						if (!BLAKE2b_core(file_content, file_content + bytes_to_compress, &bytes_compressed, out))
						{
							file_close(file);
							return 0;
						}

						readed -= bytes_compressed;
						file_content += bytes_to_compress;
					}

					break;
				}

				if (!BLAKE2b_core(file_content, file_content + 4096 - 128, &bytes_compressed, out))
				{
					file_close(file);
					return 0;
				}

				last = file_content + 4096;
#if __STDC_SEC_API__
				memcpy_s(last, 128, file_content + 4096 - 128, 128);
#else
				memcpy(last, file_content + 4096 - 128, 128);
#endif
			}

			if (last)
			{
				if (!readed)
				{
					file_content = last;
					readed = 128;
				}
				else
				{
					if (!BLAKE2b_core(last, last + 128, &bytes_compressed, out))
					{
						break;
					}
				}
			}

			if (!BLAKE2b_final(file_content, &bytes_compressed, (uint8_t)readed, out))
			{
				break;
			}

			return file_close(file) &&
				   buffer_resize(output, size) &&
				   hash_algorithm_bytes_to_string((const uint8_t*)out, ((const uint8_t*)out) + hash_length, output);
		}

		case blake3:
		{
			static const uint8_t d = 0;
			static const uint8_t h_size = 8 * sizeof(uint32_t);
			static const uint8_t m_size = 16 * sizeof(uint32_t);

			if (!buffer_append(output, NULL,
							   (ptrdiff_t)4096 + h_size + m_size + 1024))
			{
				break;
			}

			uint32_t* h = (uint32_t*)buffer_data(output, size + 4096);
			uint32_t* m = (uint32_t*)buffer_data(output, size + 4096 + h_size);
			uint8_t* stack = buffer_data(output, size + 4096 + h_size + m_size);

			if (!BLAKE3_init(h, 8, m, 16, 1024))
			{
				break;
			}

			uint8_t l = 0;
			uint32_t t[2];
			t[0] = t[1] = 0;
			/**/
			uint8_t compressed = 0;
			uint8_t stack_length = 0;
			size_t readed = 0;
			/**/
			uint8_t* file_content = buffer_data(output, size);

			while (0 < (readed = file_read(file_content, sizeof(uint8_t), 4096, file)))
			{
				if (!BLAKE3_core(file_content, readed, m, &l, h, &compressed, t, stack, &stack_length, d))
				{
					file_close(file);
					return 0;
				}
			}

			hash_length = hash_length / 8;
			file_content = buffer_data(output, size + 64);

			if (!file_close(file) ||
				!BLAKE3_final(stack, stack_length, compressed, t, h, m, l, d, (uint8_t)hash_length, file_content))
			{
				break;
			}

			return buffer_resize(output, size) &&
				   hash_algorithm_bytes_to_string(file_content, file_content + hash_length, output);
		}

		case crc32:
		{
			uint32_t out = 0;

			if (!hash_algorithm_crc32_init(&out))
			{
				break;
			}

			if (!buffer_append(output, NULL, 4096))
			{
				break;
			}

			size_t readed = 0;
			uint8_t* file_content = buffer_data(output, size);

			while (0 < (readed = file_read(file_content, sizeof(uint8_t), 4096, file)))
			{
				if (!hash_algorithm_crc32_core(file_content, file_content + readed, &out))
				{
					file_close(file);
					return 0;
				}
			}

			if (!file_close(file) ||
				!hash_algorithm_crc32_final(&out, (uint8_t)hash_length))
			{
				break;
			}

			return buffer_resize(output, size) &&
				   hash_algorithm_bytes_to_string((const uint8_t*)&out, ((const uint8_t*)&out) + sizeof(uint32_t), output);
		}

		case keccak:
		case sha3:
		{
			uint8_t rate_on_w;
			uint8_t maximum_delta;

			if (!hash_algorithm_sha3_init(
					hash_length, &rate_on_w, &maximum_delta))
			{
				break;
			}

			if (!buffer_append(output, NULL, 4096))
			{
				break;
			}

			uint8_t queue[192];
			uint8_t queue_size = 0;
			uint64_t S[] =
			{
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0,
				0, 0, 0, 0, 0
			};
			/**/
			size_t readed = 0;
			uint8_t* file_content = buffer_data(output, size);

			while (0 < (readed = file_read(file_content, sizeof(uint8_t), 4096, file)))
			{
				const uint8_t* finish = file_content + readed;
				readed = hash_algorithm_sha3_core(file_content, finish, queue, &queue_size, maximum_delta, S, rate_on_w);

				if (!readed)
				{
					file_close(file);
					return 0;
				}
			}

			hash_length = hash_length / 8;
			file_content += 64;

			if (!file_close(file) ||
				!hash_algorithm_sha3_final(
					sha3 == algorithm, queue, queue_size, maximum_delta, S, rate_on_w,
					(uint8_t)hash_length, file_content))
			{
				break;
			}

			return buffer_resize(output, size) &&
				   hash_algorithm_bytes_to_string(file_content, file_content + hash_length, output);
		}

		case xxh32:
		{
			if (!buffer_append(output, NULL, 4096))
			{
				break;
			}

			uint8_t queue[16];
			uint8_t queue_size = 0;
			const uint8_t max_queue_size = 16;
			uint32_t accumulators[5];
			uint8_t is_accumulators_initialized = 0;
			/**/
			accumulators[4] = 0;
			/**/
			uint32_t seed = 0;

			if (!range_is_null_or_empty(algorithm_parameter))
			{
				seed = (uint32_t)uint64_parse(algorithm_parameter->start, algorithm_parameter->finish);
			}

			size_t readed = 0;
			uint8_t* file_content = buffer_data(output, size);

			while (0 < (readed = file_read(file_content, sizeof(uint8_t), 4096, file)))
			{
				const uint8_t* finish = file_content + readed;

				if (!hash_algorithm_XXH32_core(file_content, finish,
											   queue, &queue_size, max_queue_size,
											   accumulators, &is_accumulators_initialized, seed))
				{
					file_close(file);
					return 0;
				}
			}

			if (!file_close(file) ||
				!hash_algorithm_XXH32_final(
					queue, queue + queue_size,
					accumulators, is_accumulators_initialized, seed, &seed))
			{
				return 0;
			}

			file_content = (uint8_t*)&seed;
			return buffer_resize(output, size) &&
				   hash_algorithm_bytes_to_string(file_content, file_content + sizeof(uint32_t), output);
		}

		case xxh64:
		{
			if (!buffer_append(output, NULL, 4096))
			{
				break;
			}

			uint8_t queue[32];
			uint8_t queue_size = 0;
			const uint8_t max_queue_size = 32;
			uint64_t accumulators[5];
			uint8_t is_accumulators_initialized = 0;
			/**/
			accumulators[4] = 0;
			/**/
			uint64_t seed = 0;

			if (!range_is_null_or_empty(algorithm_parameter))
			{
				seed = uint64_parse(algorithm_parameter->start, algorithm_parameter->finish);
			}

			size_t readed = 0;
			uint8_t* file_content = buffer_data(output, size);

			while (0 < (readed = file_read(file_content, sizeof(uint8_t), 4096, file)))
			{
				const uint8_t* finish = file_content + readed;

				if (!hash_algorithm_XXH64_core(file_content, finish,
											   queue, &queue_size, max_queue_size,
											   accumulators, &is_accumulators_initialized, seed))
				{
					file_close(file);
					return 0;
				}
			}

			if (!file_close(file) ||
				!hash_algorithm_XXH64_final(
					queue, queue + queue_size,
					accumulators, is_accumulators_initialized, seed, &seed))
			{
				return 0;
			}

			file_content = (uint8_t*)&seed;
			return buffer_resize(output, size) &&
				   hash_algorithm_bytes_to_string(file_content, file_content + sizeof(uint64_t), output);
		}

		default:
			break;
	}

	file_close(file);
	return 0;
}

uint8_t file_get_checksum(const uint8_t* path, const struct range* algorithm,
						  const struct range* algorithm_parameter, struct buffer* output)
{
	return !range_is_null_or_empty(algorithm) &&
		   file_get_checksum_(path, common_string_to_enum(algorithm->start, algorithm->finish,
							  hash_function_str, UNKNOWN_HASH_FUNCTION),
							  algorithm_parameter, output);
}
