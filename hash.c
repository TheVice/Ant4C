/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "hash.h"
#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "range.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

uint8_t hash_algorithm_bytes_to_string(const uint8_t* start, const uint8_t* finish,
									   struct buffer* output)
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

static const uint8_t* hash_function_str[] =
{
	(const uint8_t*)"blake2b-160",
	(const uint8_t*)"blake2b-256",
	(const uint8_t*)"blake2b-384",
	(const uint8_t*)"blake2b-512",
	(const uint8_t*)"blake3-256",
	(const uint8_t*)"bytes-to-string",
	(const uint8_t*)"crc32",
	(const uint8_t*)"keccak-224",
	(const uint8_t*)"keccak-256",
	(const uint8_t*)"keccak-384",
	(const uint8_t*)"keccak-512",
	(const uint8_t*)"sha3-224",
	(const uint8_t*)"sha3-256",
	(const uint8_t*)"sha3-384",
	(const uint8_t*)"sha3-512"
};

enum hash_function
{
	blake2b_160, blake2b_256, blake2b_384, blake2b_512,
	blake3_256,
	bytes_to_string,
	crc32,
	keccak_224, keccak_256, keccak_384, keccak_512,
	sha3_224, sha3_256, sha3_384, sha3_512,
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

	struct range argument1;

	struct range argument2;

	argument1.start = argument2.start = argument1.finish = argument2.finish = NULL;

	if (1 == arguments_count)
	{
		if (!common_get_one_argument(arguments, &argument1, 0))
		{
			argument1.start = argument1.finish = (const uint8_t*)&argument1;
		}
	}
	else if (2 == arguments_count)
	{
		if (!common_get_two_arguments(arguments, &argument1, &argument2, 0))
		{
			if (range_is_null_or_empty(&argument1))
			{
				argument1.start = argument1.finish = (const uint8_t*)&argument1;
			}

			arguments_count = 1;
		}
	}

	switch (function)
	{
		case blake2b_160:
			return 1 == arguments_count && hash_algorithm_blake2b_160(argument1.start, argument1.finish, output);

		case blake2b_256:
			return 1 == arguments_count && hash_algorithm_blake2b_256(argument1.start, argument1.finish, output);

		case blake2b_384:
			return 1 == arguments_count && hash_algorithm_blake2b_384(argument1.start, argument1.finish, output);

		case blake2b_512:
			return 1 == arguments_count && hash_algorithm_blake2b_512(argument1.start, argument1.finish, output);

		case blake3_256:
			return 1 == arguments_count && hash_algorithm_blake3_256(argument1.start, argument1.finish, output);

		case bytes_to_string:
			return 1 == arguments_count && hash_algorithm_bytes_to_string(argument1.start, argument1.finish, output);

		case crc32:
		{
			uint8_t order = 1;

			if (2 == arguments_count)
			{
				if (10 != range_size(&argument2))
				{
					break;
				}

				order = argument2.start ?
						(0 == memcmp("decreasing", argument2.start, 10) ? 0 :
						 (0 == memcmp("increasing", argument2.start, 10) ? 1 : 2)) : 2;

				if (1 < order)
				{
					break;
				}
			}

			if (!buffer_push_back_uint32(output, 0))
			{
				break;
			}

			uint32_t* ptr = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));
			return hash_algorithm_crc32(argument1.start, argument1.finish, ptr, order);
		}

		case keccak_224:
			return 1 == arguments_count && hash_algorithm_keccak_224(argument1.start, argument1.finish, output);

		case keccak_256:
			return 1 == arguments_count && hash_algorithm_keccak_256(argument1.start, argument1.finish, output);

		case keccak_384:
			return 1 == arguments_count && hash_algorithm_keccak_384(argument1.start, argument1.finish, output);

		case keccak_512:
			return 1 == arguments_count && hash_algorithm_keccak_512(argument1.start, argument1.finish, output);

		case sha3_224:
			return 1 == arguments_count && hash_algorithm_sha3_224(argument1.start, argument1.finish, output);

		case sha3_256:
			return 1 == arguments_count && hash_algorithm_sha3_256(argument1.start, argument1.finish, output);

		case sha3_384:
			return 1 == arguments_count && hash_algorithm_sha3_384(argument1.start, argument1.finish, output);

		case sha3_512:
			return 1 == arguments_count && hash_algorithm_sha3_512(argument1.start, argument1.finish, output);

		case UNKNOWN_HASH_FUNCTION:
		default:
			break;
	}

	return 0;
}

uint8_t file_get_checksum(const uint8_t* path, const struct range* algorithm, struct buffer* output)
{
	if (NULL == path ||
		range_is_null_or_empty(algorithm) ||
		NULL == output)
	{
		return 0;
	}

	const uint8_t algorithm_value = common_string_to_enum(algorithm->start, algorithm->finish,
									hash_function_str, UNKNOWN_HASH_FUNCTION);

	if (blake3_256 == algorithm_value ||
		bytes_to_string == algorithm_value ||
		crc32 < algorithm_value)
	{
		return 0;
	}

	uint8_t hash_size = 0;
	const ptrdiff_t size = buffer_size(output);
	void* file = NULL;

	if (!file_open(path, (const uint8_t*)"rb", &file))
	{
		return 0;
	}

	switch (algorithm_value)
	{
		case blake2b_160:
			hash_size = 20;
			break;

		case blake2b_256:
			hash_size = 32;
			break;

		case blake2b_384:
			hash_size = 48;
			break;

		case blake2b_512:
			hash_size = 64;
			break;

		case crc32:
		{
			uint32_t out = 0;

			if (!hash_algorithm_crc32_init(&out))
			{
				file_close(file);
				return 0;
			}

			if (!buffer_append(output, NULL, 4096))
			{
				file_close(file);
				return 0;
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
				!hash_algorithm_crc32_final(&out, 1))
			{
				file_close(file);
				return 0;
			}

			return buffer_resize(output, size) &&
				   hash_algorithm_bytes_to_string((const uint8_t*)&out, ((const uint8_t*)&out) + sizeof(uint32_t), output);
		}

		default:
			file_close(file);
			return 0;
	}

	if (algorithm_value < blake3_256)
	{
		if (!buffer_append(output, NULL, 64 + 128 + 4096 + 128))
		{
			file_close(file);
			return 0;
		}

		uint64_t* out = (uint64_t*)buffer_data(output, size + 64);

		if (!BLAKE2b_init(hash_size, out))
		{
			file_close(file);
			return 0;
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

		if (!file_close(file))
		{
			return 0;
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
					return 0;
				}
			}
		}

		if (!BLAKE2b_final(file_content, &bytes_compressed, (uint8_t)readed, out))
		{
			return 0;
		}

		return buffer_resize(output, size) &&
			   hash_algorithm_bytes_to_string((const uint8_t*)out, ((const uint8_t*)out) + hash_size, output);
	}

	file_close(file);
	return 0;
}
