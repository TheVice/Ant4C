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

void bytes_to_string(const uint8_t* input, uint8_t length, char* output)
{
	for (uint8_t j = 0; j < length; ++j)
	{
#if __STDC_SEC_API__
		output += sprintf_s(output, 3, input[j] < 16 ? "0%x" : "%x", input[j]);
#else
		output += sprintf(output, input[j] < 16 ? "0%x" : "%x", input[j]);
#endif
	}
}

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

uint8_t Test_bytes_to_string(struct buffer* output)
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

	char expected[2 * UINT8_MAX + 4];
	bytes_to_string(ptr, UINT8_MAX, expected);
	bytes_to_string(ptr + UINT8_MAX, 1, expected + 2 * UINT8_MAX);
	return 0 == memcmp(expected, ptr + 1 + UINT8_MAX, 2 * UINT8_MAX + 2);
}

uint8_t Test_BLAKE3()
{
	extern uint8_t BLAKE3(const uint8_t* start, const uint8_t* finish, uint8_t hash_length, uint8_t* output);
	uint8_t input[31745];

	for (uint16_t i = 0, j = 0; i < 31745; ++i, j = j < 250 ? j + 1 : 0)
	{
		input[i] = (uint8_t)j;
	}

	const uint16_t lengths[] =
	{
		0,
		1,
		1023,
		1024,
		1025,
		2048,
		2049,
		3072,
		3073,
		4096,
		4097,
		5120,
		5121,
		6144,
		6145,
		7168,
		7169,
		8192,
		8193,
		16384,
		31744,
	};
	/**/
	const uint8_t* expected_outputs[] =
	{
		"af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262e00f03e7b69af26b7faaf09fcd333050338ddfe085b8cc869ca98b206c08243a26f5487789e8f660afe6c99ef9e0c52b92e7393024a80459cf91f476f9ffdbda7001c22e159b402631f277ca96f2defdf1078282314e763699a31c5363165421cce14d",
		"2d3adedff11b61f14c886e35afa036736dcd87a74d27b5c1510225d0f592e213c3a6cb8bf623e20cdb535f8d1a5ffb86342d9c0b64aca3bce1d31f60adfa137b358ad4d79f97b47c3d5e79f179df87a3b9776ef8325f8329886ba42f07fb138bb502f4081cbcec3195c5871e6c23e2cc97d3c69a613eba131e5f1351f3f1da786545e5",
		"10108970eeda3eb932baac1428c7a2163b0e924c9a9e25b35bba72b28f70bd11a182d27a591b05592b15607500e1e8dd56bc6c7fc063715b7a1d737df5bad3339c56778957d870eb9717b57ea3d9fb68d1b55127bba6a906a4a24bbd5acb2d123a37b28f9e9a81bbaae360d58f85e5fc9d75f7c370a0cc09b6522d9c8d822f2f28f485",
		"42214739f095a406f3fc83deb889744ac00df831c10daa55189b5d121c855af71cf8107265ecdaf8505b95d8fcec83a98a6a96ea5109d2c179c47a387ffbb404756f6eeae7883b446b70ebb144527c2075ab8ab204c0086bb22b7c93d465efc57f8d917f0b385c6df265e77003b85102967486ed57db5c5ca170ba441427ed9afa684e",
		"d00278ae47eb27b34faecf67b4fe263f82d5412916c1ffd97c8cb7fb814b8444f4c4a22b4b399155358a994e52bf255de60035742ec71bd08ac275a1b51cc6bfe332b0ef84b409108cda080e6269ed4b3e2c3f7d722aa4cdc98d16deb554e5627be8f955c98e1d5f9565a9194cad0c4285f93700062d9595adb992ae68ff12800ab67a",
		"e776b6028c7cd22a4d0ba182a8bf62205d2ef576467e838ed6f2529b85fba24a9a60bf80001410ec9eea6698cd537939fad4749edd484cb541aced55cd9bf54764d063f23f6f1e32e12958ba5cfeb1bf618ad094266d4fc3c968c2088f677454c288c67ba0dba337b9d91c7e1ba586dc9a5bc2d5e90c14f53a8863ac75655461cea8f9",
		"5f4d72f40d7a5f82b15ca2b2e44b1de3c2ef86c426c95c1af0b687952256303096de31d71d74103403822a2e0bc1eb193e7aecc9643a76b7bbc0c9f9c52e8783aae98764ca468962b5c2ec92f0c74eb5448d519713e09413719431c802f948dd5d90425a4ecdadece9eb178d80f26efccae630734dff63340285adec2aed3b51073ad3",
		"b98cb0ff3623be03326b373de6b9095218513e64f1ee2edd2525c7ad1e5cffd29a3f6b0b978d6608335c09dc94ccf682f9951cdfc501bfe47b9c9189a6fc7b404d120258506341a6d802857322fbd20d3e5dae05b95c88793fa83db1cb08e7d8008d1599b6209d78336e24839724c191b2a52a80448306e0daa84a3fdb566661a37e11",
		"7124b49501012f81cc7f11ca069ec9226cecb8a2c850cfe644e327d22d3e1cd39a27ae3b79d68d89da9bf25bc27139ae65a324918a5f9b7828181e52cf373c84f35b639b7fccbb985b6f2fa56aea0c18f531203497b8bbd3a07ceb5926f1cab74d14bd66486d9a91eba99059a98bd1cd25876b2af5a76c3e9eed554ed72ea952b603bf",
		"015094013f57a5277b59d8475c0501042c0b642e531b0a1c8f58d2163229e9690289e9409ddb1b99768eafe1623da896faf7e1114bebeadc1be30829b6f8af707d85c298f4f0ff4d9438aef948335612ae921e76d411c3a9111df62d27eaf871959ae0062b5492a0feb98ef3ed4af277f5395172dbe5c311918ea0074ce0036454f620",
		"9b4052b38f1c5fc8b1f9ff7ac7b27cd242487b3d890d15c96a1c25b8aa0fb99505f91b0b5600a11251652eacfa9497b31cd3c409ce2e45cfe6c0a016967316c426bd26f619eab5d70af9a418b845c608840390f361630bd497b1ab44019316357c61dbe091ce72fc16dc340ac3d6e009e050b3adac4b5b2c92e722cffdc46501531956",
		"9cadc15fed8b5d854562b26a9536d9707cadeda9b143978f319ab34230535833acc61c8fdc114a2010ce8038c853e121e1544985133fccdd0a2d507e8e615e611e9a0ba4f47915f49e53d721816a9198e8b30f12d20ec3689989175f1bf7a300eee0d9321fad8da232ece6efb8e9fd81b42ad161f6b9550a069e66b11b40487a5f5059",
		"628bd2cb2004694adaab7bbd778a25df25c47b9d4155a55f8fbd79f2fe154cff96adaab0613a6146cdaabe498c3a94e529d3fc1da2bd08edf54ed64d40dcd6777647eac51d8277d70219a9694334a68bc8f0f23e20b0ff70ada6f844542dfa32cd4204ca1846ef76d811cdb296f65e260227f477aa7aa008bac878f72257484f2b6c95",
		"3e2e5b74e048f3add6d21faab3f83aa44d3b2278afb83b80b3c35164ebeca2054d742022da6fdda444ebc384b04a54c3ac5839b49da7d39f6d8a9db03deab32aade156c1c0311e9b3435cde0ddba0dce7b26a376cad121294b689193508dd63151603c6ddb866ad16c2ee41585d1633a2cea093bea714f4c5d6b903522045b20395c83",
		"f1323a8631446cc50536a9f705ee5cb619424d46887f3c376c695b70e0f0507f18a2cfdd73c6e39dd75ce7c1c6e3ef238fd54465f053b25d21044ccb2093beb015015532b108313b5829c3621ce324b8e14229091b7c93f32db2e4e63126a377d2a63a3597997d4f1cba59309cb4af240ba70cebff9a23d5e3ff0cdae2cfd54e070022",
		"61da957ec2499a95d6b8023e2b0e604ec7f6b50e80a9678b89d2628e99ada77a5707c321c83361793b9af62a40f43b523df1c8633cecb4cd14d00bdc79c78fca5165b863893f6d38b02ff7236c5a9a8ad2dba87d24c547cab046c29fc5bc1ed142e1de4763613bb162a5a538e6ef05ed05199d751f9eb58d332791b8d73fb74e4fce95",
		"a003fc7a51754a9b3c7fae0367ab3d782dccf28855a03d435f8cfe74605e781798a8b20534be1ca9eb2ae2df3fae2ea60e48c6fb0b850b1385b5de0fe460dbe9d9f9b0d8db4435da75c601156df9d047f4ede008732eb17adc05d96180f8a73548522840779e6062d643b79478a6e8dbce68927f36ebf676ffa7d72d5f68f050b119c8",
		"aae792484c8efe4f19e2ca7d371d8c467ffb10748d8a5a1ae579948f718a2a635fe51a27db045a567c1ad51be5aa34c01c6651c4d9b5b5ac5d0fd58cf18dd61a47778566b797a8c67df7b1d60b97b19288d2d877bb2df417ace009dcb0241ca1257d62712b6a4043b4ff33f690d849da91ea3bf711ed583cb7b7a7da2839ba71309bbf",
		"bab6c09cb8ce8cf459261398d2e7aef35700bf488116ceb94a36d0f5f1b7bc3bb2282aa69be089359ea1154b9a9286c4a56af4de975a9aa4a5c497654914d279bea60bb6d2cf7225a2fa0ff5ef56bbe4b149f3ed15860f78b4e2ad04e158e375c1e0c0b551cd7dfc82f1b155c11b6b3ed51ec9edb30d133653bb5709d1dbd55f4e1ff6",
		"f875d6646de28985646f34ee13be9a576fd515f76b5b0a26bb324735041ddde49d764c270176e53e97bdffa58d549073f2c660be0e81293767ed4e4929f9ad34bbb39a529334c57c4a381ffd2a6d4bfdbf1482651b172aa883cc13408fa67758a3e47503f93f87720a3177325f7823251b85275f64636a8f1d599c2e49722f42e93893",
		"62b6960e1a44bcc1eb1a611a8d6235b6b4b78f32e7abc4fb4c6cdcce94895c47860cc51f2b0c28a7b77304bd55fe73af663c02d3f52ea053ba43431ca5bab7bfea2f5e9d7121770d88f70ae9649ea713087d1914f7f312147e247f87eb2d4ffef0ac978bf7b6579d57d533355aa20b8b77b13fd09748728a5cc327a8ec470f4013226f",
	};
	const uint8_t count = (uint8_t)(sizeof(lengths) / sizeof(*lengths));

	if (count != sizeof(expected_outputs) / sizeof(*expected_outputs))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	uint8_t output[2 * UINT8_MAX];

	for (uint8_t i = 0; i < count; ++i)
	{
		if (31744 < lengths[i])
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		if (!BLAKE3(input, input + lengths[i], 131, output))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		char* ptr = (char*)(output + 131);
		bytes_to_string(output, 131, ptr);

		if (strlen(expected_outputs[i]) != strlen(ptr) ||
			0 != memcmp(expected_outputs[i], ptr, strlen(expected_outputs[i])))
		{
			printf("%i\nexpected: '%s'\nreturned: '%s'\n%s\n", i, expected_outputs[i], ptr, "FAILURE");
			return 0;
		}

		if (!BLAKE3(input + 1, input + lengths[i] + 1, 131, output))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		bytes_to_string(output, 131, ptr);

		if (strlen(expected_outputs[i]) != strlen(ptr))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		if (0 < lengths[i] && 0 == memcmp(expected_outputs[i], ptr, strlen(expected_outputs[i])))
		{
			printf("%i\nhash '%s' should not be equal to expected at this point.\n%s\n", i, ptr, "FAILURE");
			return 0;
		}
	}

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

	if (!Test_bytes_to_string(&output))
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

	if (!Test_BLAKE3())
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	printf("%s\n", "SUCCESS");
	/**/
	(void)argc;
	(void)argv;
	return EXIT_SUCCESS;
}
