/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "buffer.h"
#include "hash.h"

#include <math.h>
#include <time.h>
#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <stdbool.h>
#include <inttypes.h>

#ifdef _WIN32
#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

typedef uint8_t(*Func)(const uint8_t*, const uint8_t*, struct buffer*);
typedef size_t(*get_data)(const Func** functions, uint8_t* functions_count,
						  const uint8_t*** inputs, const uint8_t** inputs_lengths,
						  const char*** expected_outputs);

static const Func BLAKE2b_functions[] =
{
	&hash_algorithm_blake2b_160, &hash_algorithm_blake2b_256,
	&hash_algorithm_blake2b_384, &hash_algorithm_blake2b_512
};

static const uint8_t* BLAKE2b_inputs[] =
{
	(const uint8_t*)"",
	(const uint8_t*)"The quick brown fox jumps over the lazy dog",
	(const uint8_t*)"BLAKE and BLAKE2 are hash functions based on the ChaCha stream cipher, and was one of the finalists in the NIST competition for SHA-3"
};

static const uint8_t BLAKE2b_inputs_lengths[] = { 0, 43, 133 };

static const char* BLAKE2b_expected_outputs[] =
{
	"3345524abf6bbe1809449224b5972c41790b6cf2",
	"0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8",
	"b32811423377f52d7862286ee1a72ee540524380fda1724a6f25d7978c6fd3244a6caf0498812673c5e05ef583825100",
	"786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce",
	/**/
	"3c523ed102ab45a37d54f5610d5a983162fde84f",
	"01718cec35cd3d796dd00020e0bfecb473ad23457d063b75eff29c0ffa2e58a9",
	"b7c81b228b6bd912930e8f0b5387989691c1cee1e65aade4da3b86a3c9f678fc8018f6ed9e2906720c8d2a3aeda9c03d",
	"a8add4bdddfd93e4877d2746e62817b116364a1fa7bc148d95090bc7333b3673f82401cf7aa2e4cb1ecd90296e3f14cb5413f8ed77be73045b13914cdcd6a918",
	/**/
	"5e92b21f7cb0b46d06416c314f41c474e2feaf30",
	"40a9706556c5b70ef10daae4c519c4fcfdee519b3ded9bc6801730b23f2c1d40",
	"1441c65e58b2e9656200df99c9bd1e188a3a4e18e80d0ed9f69d4ce67bde16ef8f726e54800554dda7b29f485e50d0e9",
	"f22f00d4570b502efe801674f6588ad6926b54ef3503d266224b65c257d1422683bfd4a12470ba3d5cb3c73d5016a5a6f1fb96a13b7f0a84d03777bb18e6e859"
};

#define GET_DATA(FUNCTION_FAMILY_TO_TEST, FUNCTIONS, FUNCTIONS_COUNT, INPUTS, INPUTS_LENGTHS, EXPECTED_OUTPUTS)			\
	const size_t inputs_count = sizeof(FUNCTION_FAMILY_TO_TEST##_inputs) / sizeof(*FUNCTION_FAMILY_TO_TEST##_inputs);	\
	\
	if (0 == inputs_count)																								\
	{																													\
		printf("Inputs count can not be zero %i %s\n", __LINE__, "FAILURE");											\
		return 0;																										\
	}																													\
	\
	(FUNCTIONS_COUNT) = sizeof(FUNCTION_FAMILY_TO_TEST##_functions) / sizeof(*FUNCTION_FAMILY_TO_TEST##_functions);		\
	\
	if (0 == (FUNCTIONS_COUNT))																							\
	{																													\
		printf("Functions count can not be zero %i %s\n", __LINE__, "FAILURE");											\
		return 0;																										\
	}																													\
	\
	if (inputs_count != (sizeof(FUNCTION_FAMILY_TO_TEST##_inputs_lengths) / sizeof(*FUNCTION_FAMILY_TO_TEST##_inputs_lengths)))	\
	{																															\
		printf("Count of inputs should be equals to the count of inputs lengths %i %s\n", __LINE__, "FAILURE");					\
		return 0;																												\
	}																															\
	\
	if (inputs_count !=																														\
		((sizeof(FUNCTION_FAMILY_TO_TEST##_expected_outputs) / sizeof(*FUNCTION_FAMILY_TO_TEST##_expected_outputs)) / (FUNCTIONS_COUNT)))	\
	{																																		\
		printf("Count of inputs should be equals to the count of expected outputs %i %s\n", __LINE__, "FAILURE");							\
		return 0;																															\
	}																																		\
	\
	(FUNCTIONS) = FUNCTION_FAMILY_TO_TEST##_functions;																	\
	(INPUTS) = FUNCTION_FAMILY_TO_TEST##_inputs;																		\
	(INPUTS_LENGTHS) = FUNCTION_FAMILY_TO_TEST##_inputs_lengths;														\
	(EXPECTED_OUTPUTS) = FUNCTION_FAMILY_TO_TEST##_expected_outputs;													\
	\
	return inputs_count;

size_t get_data_BLAKE2b(const Func** functions, uint8_t* functions_count,
						const uint8_t*** inputs, const uint8_t** inputs_lengths,
						const char*** expected_outputs)
{
	GET_DATA(BLAKE2b, (*functions), (*functions_count), (*inputs), (*inputs_lengths), (*expected_outputs));
}

static const uint8_t* CRC32_inputs[] =
{
	(const uint8_t*)"",
	(const uint8_t*)"The quick brown fox jumps over the lazy dog"
};

static const uint8_t CRC32_inputs_lengths[] = { 0, 43 };

static const char* CRC32_expected_outputs[] =
{
	"00000000", "00000000",
	"414fa339", "39a34f41"
};

static const Func CRC32_functions[] =
{
	&hash_algorithm_blake2b_160, &hash_algorithm_blake2b_256
};

size_t get_data_CRC32(const Func** functions, uint8_t* functions_count,
					  const uint8_t*** inputs, const uint8_t** inputs_lengths,
					  const char*** expected_outputs)
{
	GET_DATA(CRC32, (*functions), (*functions_count), (*inputs), (*inputs_lengths), (*expected_outputs));
}

static const Func SHA3_functions[] =
{
	&hash_algorithm_keccak_224, &hash_algorithm_keccak_256,
	&hash_algorithm_keccak_384, &hash_algorithm_keccak_512,
	/**/
	&hash_algorithm_sha3_224, &hash_algorithm_sha3_256,
	&hash_algorithm_sha3_384, &hash_algorithm_sha3_512
};

static const uint8_t* SHA3_inputs[] =
{
	(const uint8_t*)"",
	(const uint8_t*)"The quick brown fox jumps over the lazy dog"
};

static const uint8_t SHA3_inputs_lengths[] = { 0, 43 };

static const char* SHA3_expected_outputs[] =
{
	"f71837502ba8e10837bdd8d365adb85591895602fc552b48b7390abd",
	"c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470",
	"2c23146a63a29acf99e73b88f8c24eaa7dc60aa771780ccc006afbfa8fe2479b2dd2b21362337441ac12b515911957ff",
	"0eab42de4c3ceb9235fc91acffe746b29c29a8c366b7c60e4e67c466f36a4304c00fa9caf9d87976ba469bcbe06713b435f091ef2769fb160cdab33d3670680e",
	"6b4e03423667dbb73b6e15454f0eb1abd4597f9a1b078e3f5b5a6bc7",
	"a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a",
	"0c63a75b845e4f7d01107d852e4c2485c51a50aaaa94fc61995e71bbee983a2ac3713831264adb47fb6bd1e058d5f004",
	"a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26",
	/**/
	"310aee6b30c47350576ac2873fa89fd190cdc488442f3ef654cf23fe",
	"4d741b6f1eb29cb2a9b9911c82f56fa8d73b04959d3d9d222895df6c0b28aa15",
	"283990fa9d5fb731d786c5bbee94ea4db4910f18c62c03d173fc0a5e494422e8a0b3da7574dae7fa0baf005e504063b3",
	"d135bb84d0439dbac432247ee573a23ea7d3c9deb2a968eb31d47c4fb45f1ef4422d6c531b5b9bd6f449ebcc449ea94d0a8f05f62130fda612da53c79659f609",
	"d15dadceaa4d5d7bb3b48f446421d542e08ad8887305e28d58335795",
	"69070dda01975c8c120c3aada1b282394e7f032fa9cf32f4cb2259a0897dfc04",
	"7063465e08a93bce31cd89d2e3ca8f602498696e253592ed26f07bf7e703cf328581e1471a7ba7ab119b1a9ebdf8be41",
	"01dedd5de4ef14642445ba5f5b97c15e47b9ad931326e4b0727cd94cefc44fff23f07bf543139939b49128caf436dc1bdee54fcb24023a08d9403f9b4bf0d450"
};

size_t get_data_SHA3(const Func** functions, uint8_t* functions_count,
					 const uint8_t*** inputs, const uint8_t** inputs_lengths,
					 const char*** expected_outputs)
{
	GET_DATA(SHA3, (*functions), (*functions_count), (*inputs), (*inputs_lengths), (*expected_outputs));
}

uint8_t Check_HASH(struct buffer* output)
{
	if (!buffer_append(output, NULL, 3 * UINT8_MAX + 2))
	{
		return 0;
	}

	uint8_t* ptr = buffer_data(output, 0);

	for (uint16_t i = 0, j = 0; i < UINT8_MAX + 1; ++i, ++j)
	{
		ptr[i] = (uint8_t)j;
	}

	if (!buffer_resize(output, UINT8_MAX + 1))
	{
		return 0;
	}

	ptr = buffer_data(output, 0);

	if (!hash_algorithm_bytes_to_string(ptr, ptr + UINT8_MAX + 1, output))
	{
		return 0;
	}

	printf("%s\n", ptr + 1 + UINT8_MAX);
	return 1;
}

#if defined(_MSC_VER)
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
#ifdef _WIN32
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif
	struct buffer output;
	SET_NULL_TO_BUFFER(output);

	if (!Check_HASH(&output))
	{
		buffer_release(&output);
		printf("Check_HASH %i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	static const get_data functions[] =
	{
		&get_data_BLAKE2b, &get_data_SHA3,
	};

	for (uint8_t i = 0, count = sizeof(functions) / sizeof(*functions); i < count; ++i)
	{
		const Func* functions_to_test = NULL;
		uint8_t functions_count = 0;
		const uint8_t** inputs = NULL;
		const uint8_t* lengths = NULL;
		const char** expected_output = NULL;
		const size_t inputs_counts = functions[i](&functions_to_test, &functions_count,
									 &inputs, &lengths, &expected_output);

		if (0 == inputs_counts)
		{
			buffer_release(&output);
			printf("i = %i Line # %i %s\n", (int)i, __LINE__, "FAILURE");
			return EXIT_FAILURE;
		}

		for (uint8_t j = 0; j < functions_count; ++j)
		{
			for (size_t k = 0; k < inputs_counts; ++k)
			{
				if (!buffer_resize(&output, 0) ||
					!functions_to_test[j](inputs[k], inputs[k] + lengths[k], &output))
				{
					buffer_release(&output);
					printf("i = %i j = %i k = %"PRId64" %i %s\n", i, j, k, __LINE__, "FAILURE");
					return EXIT_FAILURE;
				}

				const ptrdiff_t size = buffer_size(&output);

				if (!buffer_append(&output, NULL, 2 * size + 2) ||
					!buffer_resize(&output, size) ||
					!hash_algorithm_bytes_to_string(buffer_data(&output, 0), buffer_data(&output, 0) + size, &output))
				{
					buffer_release(&output);
					printf("i = %i j = %i k = %"PRId64" %i %s\n", i, j, k, __LINE__, "FAILURE");
					return EXIT_FAILURE;
				}

				if (0 != memcmp(expected_output[k * functions_count + j],
								buffer_char_data(&output, 0) + size, buffer_size(&output) - size))
				{
					printf("i = %i j = %i k = %"PRId64" %i %s\n%s\n%s\n", i, j, k, __LINE__, "FAILURE",
						   buffer_char_data(&output, 0) + size, expected_output[k * functions_count + j]);
					buffer_release(&output);
					return EXIT_FAILURE;
				}
			}
		}
	}

	const Func* dummy = NULL;
	uint8_t functions_count_CRC32 = 0;
	const uint8_t** inputs_CRC32 = NULL;
	const uint8_t* inputs_lengths_CRC32 = NULL;
	const char** expected_output_CRC32 = NULL;
	const size_t inputs_counts_CRC32 = get_data_CRC32(
										   &dummy, &functions_count_CRC32, &inputs_CRC32,
										   &inputs_lengths_CRC32, &expected_output_CRC32);

	for (size_t i = 0, o = 0; i < inputs_counts_CRC32; ++i)
	{
		for (uint8_t j = 0; j < functions_count_CRC32; ++j, ++o)
		{
			if (!buffer_resize(&output, 3 * sizeof(uint32_t) + 2) ||
				!hash_algorithm_crc32(inputs_CRC32[i],
									  inputs_CRC32[i] + inputs_lengths_CRC32[i],
									  buffer_uint32_data(&output, 0), j))
			{
				buffer_release(&output);
				printf("%i %s\n", __LINE__, "FAILURE");
				return EXIT_FAILURE;
			}

			if (!buffer_resize(&output, sizeof(uint32_t)) ||
				!hash_algorithm_bytes_to_string(buffer_data(&output, 0),
												buffer_data(&output, 0) + sizeof(uint32_t), &output))
			{
				buffer_release(&output);
				printf("%i %s\n", __LINE__, "FAILURE");
				return EXIT_FAILURE;
			}

			if (0 != memcmp(expected_output_CRC32[o], buffer_data(&output, sizeof(uint32_t)), 2 * sizeof(uint32_t)))
			{
				buffer_release(&output);
				printf("%i %s\n", __LINE__, "FAILURE");
				return EXIT_FAILURE;
			}
		}
	}

	buffer_release(&output);
	printf("%s\n", "SUCCESS");
	/**/
	(void)argc;
	(void)argv;
	return EXIT_SUCCESS;
}
