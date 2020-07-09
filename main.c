/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "hash.h"
#include "path.h"
#include "range.h"

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

uint8_t bytes_to_string(const uint8_t* input, uint8_t length, char* output)
{
	if (NULL == input ||
		NULL == output)
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	for (uint8_t i = 0; i < length; ++i)
	{
#if __STDC_SEC_API__
		output += sprintf_s(output, 3, input[i] < 16 ? "0%x" : "%x", input[i]);
#else
		output += snprintf(output, 3, input[i] < 16 ? "0%x" : "%x", input[i]);
#endif
	}

	return 1;
}

uint8_t Test_bytes_to_string(struct buffer* output)
{
	if (!buffer_resize(output, 3 * UINT8_MAX + 2))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	uint8_t* ptr = buffer_data(output, 0);

	for (uint16_t i = 0; i < UINT8_MAX + 1; ++i)
	{
		ptr[i] = (uint8_t)i;
	}

	if (!buffer_resize(output, UINT8_MAX + 1))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	ptr = buffer_data(output, 0);

	if (!hash_algorithm_bytes_to_string(ptr, ptr + UINT8_MAX + 1, output))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	char expected[2 * UINT8_MAX + 4];

	if (!bytes_to_string(ptr, UINT8_MAX, expected))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	if (!bytes_to_string(ptr + UINT8_MAX, 1, expected + 2 * UINT8_MAX))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	return 0 == memcmp(expected, ptr + 1 + UINT8_MAX, 2 * UINT8_MAX + 2);
}

uint8_t Test_BLAKE2b_1(struct buffer* output)
{
	static const uint8_t* BLAKE2b_inputs[] =
	{
		(const uint8_t*)"",
		(const uint8_t*)"The quick brown fox jumps over the lazy dog",
		(const uint8_t*)"BLAKE and BLAKE2 are hash functions based on the ChaCha stream cipher, and was one of the finalists in the NIST competition for SHA-3"
	};
	/**/
	static const uint8_t BLAKE2b_inputs_lengths[] = { 0, 43, 133 };
	static const uint16_t BLAKE2b_hash_lengths[] = { 160, 256, 384, 512 };
	/**/
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

	if (!buffer_resize(output, UINT8_MAX))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	for (uint8_t i = 0, count_i = COUNT_OF(BLAKE2b_inputs_lengths); i < count_i; ++i)
	{
		const uint8_t* start = BLAKE2b_inputs[i];
		const uint8_t* finish = start + BLAKE2b_inputs_lengths[i];

		for (uint8_t j = 0, count_j = COUNT_OF(BLAKE2b_hash_lengths); j < count_j; ++j)
		{
			if (!buffer_resize(output, 0))
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			if (!hash_algorithm_blake2b(start, finish, BLAKE2b_hash_lengths[j], output))
			{
				printf("%i\n\tfunction = hash_algorithm_blake2b\n\tinput = '%s'\n\thash length = %i\n%s\n",
					   __LINE__, start, BLAKE2b_hash_lengths[j], "FAILURE");
				return 0;
			}

			const uint8_t size = (uint8_t)buffer_size(output);
			char* out = (char*)buffer_data(output, 0) + size;

			if (!bytes_to_string(buffer_data(output, 0), size, out))
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			const size_t index = (size_t)i + (size_t)count_i * i + j;

			if (COUNT_OF(BLAKE2b_expected_outputs) <= index)
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			const char* expected = BLAKE2b_expected_outputs[index];
			const uint8_t expected_length = (uint8_t)strlen(expected);
			const uint8_t returned_length = (uint8_t)strlen(out);

			if (expected_length != returned_length ||
				0 != memcmp(expected, out, returned_length))
			{
				printf("%i\nexpected: '%s' (%i)\nreturned: '%s' (%i)\n%s\n", i, expected, expected_length, out,
					   returned_length, "FAILURE");
				return 0;
			}
		}
	}

	return 1;
}

uint8_t Test_BLAKE2b_2(struct buffer* output)
{
	if (!buffer_append(output, NULL, 31745 + 3 * 4096) ||
		!buffer_resize(output, 31745))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	uint8_t* in = buffer_data(output, 0);

	for (uint16_t i = 0, j = 0; i < 31745; ++i, j = j < 250 ? j + 1 : 0)
	{
		in[i] = (uint8_t)j;
	}

	const uint16_t lengths[] =
	{
		0,
		1,
		69,
		70,
		71,
		101,
		102,
		103,
		133,
		134,
		135,
		141,
		142,
		143,
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
	const char* expected_outputs[] =
	{
		"786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce",
		"2fa3f686df876995167e7c2e5d74c4c7b6e48f8068fe0e44208344d480f7904c36963e44115fe3eb2a3ac8694c28bcb4f5a0f3276f2e79487d8219057a506e4b",
		"78a9fc048e25c6dcb5de45667de8ffdd3a93711141d594e9fa62a959475da6075ea8f0916e84e45ad911b75467077ee52d2c9aebf4d58f20ce4a3a00458b05d4",
		"45813f441769ab6ed37d349ff6e72267d76ae6bb3e3c612ec05c6e02a12af5a37c918b52bf74267c3f6a3f183a8064ff84c07b193d08066789a01accdb6f9340",
		"956da1c68d83a7b881e01b9a966c3c0bf27f68606a8b71d457bd016d4c41dd8a380c709a296cb4c6544792920fd788835771a07d4a16fb52ed48050331dc4c8b",
		"965f20f139765fcc4ce4ba3794675863cac24db472cd2b799d035bce3dbea502da7b524865f6b811d8c5828d3a889646fe64a380da1aa7c7044e9f245dced128",
		"ec295b5783601244c30e4641e3b45be222c4dce77a58700f53bc8ec52a941690b4d0b087fb6fcb3f39832b9de8f75ec20bd43079811749cdc907edb94157d180",
		"61c72f8ccc91dbb54ca6750bc489672de09faedb8fdd4f94ff2320909a303f5d5a98481c0bc1a625419fb4debfbf7f8a53bb07ec3d985e8ea11e72d559940780",
		"e59b9987d428b3eda37d80abdb16cd2b0aef674c2b1dda4432ea91ee6c935c684b48b4428a8cc740e579a30deff35a803013820dd23f14ae1d8413b5c8672aec",
		"cd9fcc99f99d4cc16d031900b2a736e1508db4b586814e6345857f354a70ccecb1df3b50a19adaf43c278efa423ff4bb6c523ec7fd7859b97b168a7ebff8467c",
		"0602185d8c3a78738b99164b8bc6ffb21c7debebbf806372e0da44d121545597b9c662a255dc31542cf995ecbe6a50fb5e6e0ee4ef240fe557eded1188087e86",
		"75351313b52a8529298d8c186b1768666dcca8595317d7a4816eb88c062020c0c8efc554bb341b64688db5ccafc35f3c3cd09d6564b36d7b04a248e146980d4b",
		"e3128b1d311d02179d7f25f97a5a8bee2cc8c86303644fcd664e157d1fef00f23e46f9a5e8e5c890ce565bb6abd4302ce06469d52a5bd53e1c5a54d04649dc03",
		"c2382a72d2d3ace9d5933d00b60827ed380cda08d0ba5f6dd41e29ee6dbe8ecb9235f06be95d83b6816a2fb7a5ad47035e8a4b69a4884b99e4bece58cab25d44",
		"e55fd611a16696f8295ea5120a151e312e5dfb1488ac74be64118ffe1bc1d539e725ad0440e5213de297ba435d381c66edf88eebf28b8d640e31103842d3be29",
		"8d1090909017add40e749df2d0ebac43273d6fc816bc4ffaf2a6dfabe4206dea13677d2002399e4a38e700d8083db4af8341ee9b3a5147110b6a963a3894e4e2",
		"7a9e5283a15d13b995755360fde4c65c2ae1bc0cf33e8db2ce8416e5d10697c73fc4b2622a29b938a1faec43d931b02e71ad8635e071265633643a9d9396ec28",
		"84ef376f8080d5d19a6914c9b8e8eaf71b3f716f5b4f0da4fdf81b6c465a5656e01b52807011e1fce05e77729aae5422c6424fe241f7ba93da39456e5c5448d9",
		"146560fd774a01704fcce96f5f9b4b042ae43c928ad6546fb070b0ec18d2a4ac592578af038a1f6c5b79144fb16a0c6428999d518384d8349a3ec3707aa50ac2",
		"6a4fd5fd8cc0a8e717b28757c896096b0452750684cf7c6c3636f51a98beb32c88f32c9ed7140f90a2cdff2fc4ff49bcaa257f14a6bf6f926530cb47cc7aa340",
		"8f204a6e0204733471290db377aff78f069bc2d3d943de81f9a0b71764204c71fb3b1c09a3cd7f3b9f290b7325afb591597ebebc853657acff0fdb242f745d16",
		"c7a3d6a53bd11772ecf077c1dc9633a39c6fe691ec07a530e0e765c0a9d5a01a16f00995536578b83e54c2821766ac7ac6ae86e22269a5d14208ccac954cc95f",
		"a1aca2bd515e5a87ed22476d9209f748754ebaeddef9cd1e1d57c12cc4b9029342cb74899a9f23cfece0ee8be2fd86e9e72a9289921231a6e40883d01694e0dd",
		"c4ce856743cab7efd10591a0b43e0049dee967baaf0ac042fe571a62b01687c99ac345fd6bb3a5ffec83b31f96f92bd00337a59bb6f066696c2968e624461644",
		"92fecb457c0e6c3de55bf61fbabdcf805191fd53aa8d46efbf7a4e76828ac79cd10b5a00aab74e3c9c5bc053cfb742e8170604a85087bf9aa634729b2a7939c6",
		"71b0c49f685263009535c8d90d3cb3983eb39840f5b6e32072ac239f2a5f7fc72d1ef13a5a765a82f1485dfe63b0f5145726940848ca2390a57dc5e719f17b4e",
		"39b663dd381569a0b708005e423b1813dd90e2a0b3716995a7366ce226f2d624c5826aa40b5eee94003ccab32ddf940d3b8bb7312643d1e1bb3ae10228fd0d84",
		"643e53172be0fd9c4a9480ee718205fa593b0c0ee325d92248bceac3f8c7b31a4fb23b9465df1df8f822da17b7f7ec0d538e5a462e2b559578b313b84392ea2e",
		"ec21a001756ee29de1e67ad1b5fa4e5f5ef702332ac30a38fd3e8765355bdbcd8052ef4343bd9b9f9ef577cd93b8c20c64646ab48950922d0a64f11aa60abc04",
		"6e02a28235a5fea5bb41fe376b384a8f83376b633ae67572d73b4152c94b07a5fadb1478a2debefb3ac30cb5594e0352b108b73163f9e09f260e4f483900a039",
		"00f382e50aa061d8e3eac0a7bec89c711d2ec4c315d894fef92a8c71d79b4f9b8b6182bd2965b2428c12001c0748efff0e7a9610cea33f83a055c695d5ab767f",
		"fdaf9dca1aaf9c01e65379b5b17dffc40f890721627bf5eca54558245324ad8983b7f445a642f9d9388367226e4a1d2fb15591ac0cbeec886c247eee76d3a576",
		"c3494504df969632ed8a0827ab8508354f31059e7cd44ae21a27a510793cffb53fc21dd1a42efbb0dfe7b22435450056443a2090992ff6043818fb0b25b1d5d0"
	};
	/**/
	const uint8_t count = (uint8_t)(sizeof(lengths) / sizeof(*lengths));

	if (count != sizeof(expected_outputs) / sizeof(*expected_outputs))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	if (!path_get_temp_file_name(output))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	static const uint8_t* algorithm = (const uint8_t*)"blake2b-512";
	struct range algorithm_in_range;
	algorithm_in_range.start = algorithm;
	algorithm_in_range.finish = algorithm + 7;
	/**/
	struct range algorith_parameter_in_range;
	algorith_parameter_in_range.start = algorithm + 8;
	algorith_parameter_in_range.finish = algorithm + 11;
	/**/
	const uint8_t* path = buffer_data(output, 31745);

	for (uint8_t i = 0; i < count; ++i)
	{
		if (31744 < lengths[i])
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		void* file = NULL;

		if (!file_open(path, (const uint8_t*)"wb", &file))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		if (lengths[i] != file_write(in, sizeof(uint8_t), lengths[i], file))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			file_close(file);
			return 0;
		}

		if (!file_close(file))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		const ptrdiff_t size = buffer_size(output);

		if (!file_get_checksum(path, &algorithm_in_range, &algorith_parameter_in_range, output) ||
			!buffer_push_back(output, 0))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		const char* ptr = (const char*)buffer_data(output, size);

		if (strlen(expected_outputs[i]) != strlen(ptr) ||
			0 != memcmp(expected_outputs[i], ptr, strlen(expected_outputs[i])))
		{
			printf("%i\nexpected: '%s'\nreturned: '%s'\n%s\n", i, expected_outputs[i], ptr, "FAILURE");
			return 0;
		}

		if (!buffer_resize(output, size))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}
	}

	return 1;
}

extern uint8_t BLAKE3(const uint8_t* start, const uint8_t* finish, uint8_t hash_length, uint8_t* output);

uint8_t Test_BLAKE3(struct buffer* output)
{
	if (!buffer_append(output, NULL, 31745 + 3 * 4096) ||
		!buffer_resize(output, 31745))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	uint8_t* in = buffer_data(output, 0);

	for (uint16_t i = 0, j = 0; i < 31745; ++i, j = j < 250 ? j + 1 : 0)
	{
		in[i] = (uint8_t)j;
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
	const char* expected_outputs[] =
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
	/**/
	const uint8_t count = (uint8_t)(sizeof(lengths) / sizeof(*lengths));

	if (count != sizeof(expected_outputs) / sizeof(*expected_outputs))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	const uint8_t* start = buffer_data(output, 0);
	uint8_t* out = buffer_data(output, 0) + 31746;

	for (uint8_t i = 0; i < count; ++i)
	{
		if (31744 < lengths[i])
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		const uint8_t* finish = buffer_data(output, 0) + lengths[i];

		if (!BLAKE3(start, finish, 131, out))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		char* ptr = (char*)(out + 131);

		if (!bytes_to_string(out, 131, ptr))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		if (strlen(expected_outputs[i]) != strlen(ptr) ||
			0 != memcmp(expected_outputs[i], ptr, strlen(expected_outputs[i])))
		{
			printf("%i\nexpected: '%s'\nreturned: '%s'\n%s\n", i, expected_outputs[i], ptr, "FAILURE");
			return 0;
		}

		if (!BLAKE3(start + 1, finish + 1, 131, out))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

		if (!bytes_to_string(out, 131, ptr))
		{
			printf("%i %s\n", __LINE__, "FAILURE");
			return 0;
		}

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

uint8_t Test_CRC32(struct buffer* output)
{
	static const uint8_t* CRC32_inputs[] =
	{
		(const uint8_t*)"",
		(const uint8_t*)"The quick brown fox jumps over the lazy dog"
	};
	/**/
	static const uint8_t CRC32_inputs_lengths[] = { 0, 43 };
	/**/
	static const char* CRC32_expected_outputs[] =
	{
		"00000000", "00000000",
		"414fa339", "39a34f41"
	};

	if (!buffer_resize(output, UINT8_MAX))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	uint8_t* out = buffer_data(output, 0);

	for (uint8_t i = 0, count_i = COUNT_OF(CRC32_inputs_lengths); i < count_i; ++i)
	{
		const uint8_t* start = CRC32_inputs[i];
		const uint8_t* finish = start + CRC32_inputs_lengths[i];

		for (uint8_t order = 0; order < 2; ++order)
		{
			if (!hash_algorithm_crc32(start, finish, (uint32_t*)out, order))
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			if (!buffer_resize(output, sizeof(uint32_t)))
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			if (!hash_algorithm_bytes_to_string(out, out + sizeof(uint32_t), output))
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			const size_t index = (size_t)count_i * i + order;

			if (COUNT_OF(CRC32_expected_outputs) <= index)
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			const char* expected = CRC32_expected_outputs[index];
			const uint8_t expected_length = (uint8_t)strlen(expected);
			const uint8_t returned_length = (uint8_t)strlen((char*)out + sizeof(uint32_t));

			if (expected_length != returned_length ||
				0 != memcmp(expected, out + sizeof(uint32_t), returned_length))
			{
				printf("%i\nexpected: '%s' (%i)\nreturned: '%s' (%i)\n%s\n", i,
					   expected, expected_length,
					   (char*)out + sizeof(uint32_t),
					   returned_length, "FAILURE");
				return 0;
			}
		}
	}

	return 1;
}

uint8_t Test_Keccak(struct buffer* output)
{
	typedef uint8_t(*keccak_function)(const uint8_t*, const uint8_t*, uint16_t hash_length,
			struct buffer*);
	/**/
	static const keccak_function keccak_functions[] =
	{
		&hash_algorithm_keccak, &hash_algorithm_sha3
	};
	/**/
	static const uint8_t* Keccak_inputs[] =
	{
		(const uint8_t*)"",
		(const uint8_t*)"The quick brown fox jumps over the lazy dog"
	};
	/**/
	static const uint8_t Keccak_inputs_lengths[] = { 0, 43 };
	/**/
	static const uint16_t Keccak_hash_lengths[] = { 224, 256, 384, 512 };
	/**/
	static const char* Keccak_expected_outputs[] =
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
	/**/
	const char** expected = Keccak_expected_outputs;

	if (!buffer_resize(output, UINT8_MAX))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	for (uint8_t i = 0, count_i = COUNT_OF(Keccak_inputs_lengths); i < count_i; ++i)
	{
		const uint8_t* start = Keccak_inputs[i];
		const uint8_t* finish = start + Keccak_inputs_lengths[i];

		for (uint8_t k = 0, count_k = COUNT_OF(keccak_functions); k < count_k; ++k)
		{
			for (uint8_t j = 0, count_j = COUNT_OF(Keccak_hash_lengths); j < count_j; ++j)
			{
				if (!buffer_resize(output, 0))
				{
					printf("%i %s\n", __LINE__, "FAILURE");
					return 0;
				}

				uint8_t returned = keccak_functions[k](start, finish, Keccak_hash_lengths[j], output);

				if (!returned)
				{
					printf("%i %s\n", __LINE__, "FAILURE");
					return 0;
				}

				const uint8_t* input_start = buffer_data(output, 0);
				const uint8_t* input_finish = buffer_data(output, 0) + buffer_size(output);
				/**/
				returned = hash_algorithm_bytes_to_string(input_start, input_finish, output);

				if (!returned)
				{
					printf("%i %s\n", __LINE__, "FAILURE");
					return 0;
				}

				const uint8_t expected_length = (uint8_t)strlen(*expected);
				const uint8_t returned_length = (uint8_t)strlen((const char*)input_finish);

				if (expected_length != returned_length ||
					0 != memcmp(*expected, input_finish, returned_length))
				{
					printf("%i\nexpected: '%s' (%i)\nreturned: '%s' (%i)\n%s\n", i,
						   *expected, expected_length,
						   (const char*)input_finish,
						   returned_length, "FAILURE");
					return 0;
				}

				++expected;
			}
		}
	}

	return 1;
}

uint8_t Test_SHA3(struct buffer* output)
{
	if (!buffer_append(output, NULL, 31745 + 3 * 4096) ||
		!buffer_resize(output, 31745))
	{
		printf("%i %s\n", __LINE__, "FAILURE");
		return 0;
	}

	uint8_t* in = buffer_data(output, 0);

	for (uint16_t i = 0, j = 0; i < 31745; ++i, j = j < 250 ? j + 1 : 0)
	{
		in[i] = (uint8_t)j;
	}

	const uint16_t lengths[] =
	{
		0,
		1,
		69,
		70,
		71,
		101,
		102,
		103,
		133,
		134,
		135,
		141,
		142,
		143,
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
	static const uint16_t SHA3_hash_lengths[] = { 224, 256, 384, 512 };
	/**/
	const char* expected_outputs[] =
	{
		"6b4e03423667dbb73b6e15454f0eb1abd4597f9a1b078e3f5b5a6bc7",
		"a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a",
		"0c63a75b845e4f7d01107d852e4c2485c51a50aaaa94fc61995e71bbee983a2ac3713831264adb47fb6bd1e058d5f004",
		"a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26",
		"bdd5167212d2dc69665f5a8875ab87f23d5ce7849132f56371a19096",
		"5d53469f20fef4f8eab52b88044ede69c77a6a68a60728609fc4a65ff531e7d0",
		"127677f8b66725bbcb7c3eae9698351ca41e0eb6d66c784bd28dcdb3b5fb12d0c8e840342db03ad1ae180b92e3504933",
		"7127aab211f82a18d06cf7578ff49d5089017944139aa60d8bee057811a15fb55a53887600a3eceba004de51105139f32506fe5b53e1913bfa6b32e716fe97da",
		"bb41c858301cb57230fd85083ba5c76044282170123a990f461e3b14",
		"ed379e9012f1d3a4fef5096688a2557b3ceb68c619245bffcf05a14a5a846fd9",
		"71007b69c9b15fa519af8cab73514a8b92534513aa19153e7723e56ab54c34bce618bafef9ab92d13298bd35cb6e692a",
		"65e34948011bf0715855139bd03afcd01c10a0cba4a6c237cf38a79977080eace8f47e1e9ee14e7d60dcc2a9fe2e9dab47da5bd0ea1cd3932fa788f07e04d7a0",
		"c885e6c9be4a56615adc88657907b0449ff80a0b14459096caa14999",
		"97a26b0e8066f35d400b7f12a6ae62a290bc1ca68660b4da8bf17afad6b8c948",
		"f82eab30f732330e9692860682ea083586216ee761ea08c014c8a8dd2dc08fc9901942f311f63f63bf3ab8b4dab14975",
		"351164e46da292fd3d75132545401498d8983be9d143732de2df21fbf975a5e2c81584d90a773d34d7895d4cece328637ad2ffa973e21d4b6d03f39141cf44c8",
		"5e31d4bc8904e6e77531e6b975d3dcdd4330c03620e5204bc047ce2e",
		"881ad9ffbd7f090efa51cbdfe93da23a0401f4446f7adf150d1c226851cbfff2",
		"4bb4db01ac1c1d1a5de657436aca5275e4cae772bd6ab9b358e0ed094202be9600724a5bdfef0461ba7f1dc2427cf155",
		"3ccc850d53a1287af7b4560b2ef0d43eb5d9a80d62a0e9cf1dbc040135921104d4395168e90bfc871773ebb34bca1bd67056e1cc7dc7a48ff7c3167d389f117c",
		"03b96617edf75c33e4949e2b462eb75828546a2eb8308bdd4c0859cf",
		"af504dd36feb666b16fe553116adbdd604e449ca783e54a83171aee7ddc7e7b1",
		"1c3a5ccc5dc4030ba16305a85f59dced8cb5023db4479e8b3749a7004ae77bb18996931af4c16409efcc0c72caa4c1a1",
		"c8560c1f73d8f309676b71c239c06fcf16a7a8fc710c593b0f83b32606d6d7c85b66f9ad987a4c24bad2f4a03a20da89d5f1f0987f60fc97f40f306e221edbef",
		"c556db7b33e53bf1736d9bc84f30aa936a8ea4afcc590d70fbe647ab",
		"986b81944604ef3a1f26032a04537777c0ecd1cb66b37e3ca6e9b108befaf56c",
		"ee8a6c983daaec5976cb0ed4446494d348d0d571eb4c77fd8cf7eebdaa0733f96de2bdd8cdd0c70f5fa4391a336107b9",
		"9d6df09db9028fb57b45e346886fc0a6ba37d81607bf21b4c7e5ef811197e01eefd4be9c22e68a1b7c714f8f7924f2ea699b0e83a3c95379f7e8dd0ce20ca8b6",
		"8b59fefcaf88187d29eb7b887f5c066475168a4348deac4f34993d63",
		"120a055c592d237c0f535eebfc05673374fe4a50e1330293ef2c1ab611e0d0ba",
		"1f91ee551ad18f268876d1fc262f137fe196580216c5193819a95ec5222537d2a658dd129c3d8080e65ec7460f1f4704",
		"c2204bc64deef9249ab2e29187e31db863002b0ab130c1189d76d8e41f40aab95606b1cc3955788fd6e377d438c2c88c4aecf8f2096a5c8d214863cfbaa3ef5e",
		"67dd1365e9e2ec75f4d96a8460a0a9bf686827920e2b7fbbb0ef2925",
		"721f0e936b3b93c0384f970c07680a8a6293e5012295e83615ea4657ed5d7e17",
		"72d7f791dd6b7e70cb118634b458bf6977a64b1a6c27a136464ae12db9e7ecc5cb2d7d73c8faf07d4d93d1b2e2f1c0da",
		"df7f2a48b2d154de7d3adf710cdd3d1528ebcb06371a72ae607e6e6836163b179800e1ef1ba08649ed405637c6617d6600b0afcdfd7fd9e39a1240f45c21ff96",
		"6d8a43354b6c4e4e28e93ad60e16df666eeca61b360f718009c445a1",
		"644e15224f5597351aef5c4bdd22b27ca0c19db2244431534c2a4a0bebfdf39c",
		"0dd4000d8a25d0cffdfe2b7151fc610cb5bf25d9249d855ce8d0d647d2151235a12092a75233efa7820fcdfedc59041f",
		"76dc7b65f9c314c810c75a7c7a520ca9bc750d14fb4984f982e86e4bb1e46f3df3ce529bfa7b52366884ec66a1fb6dcdb9ec04a72d9405e0af9fa1bde9054a32",
		"3174e5126280625ecdfdb238d6a10d1c8b071031870ed197640cad90",
		"fded8fd9d6551c601eeb3b7c6bc5e5cfd8aad1d015b7e9aaa9c9b9475231d5e2",
		"25d2be1f5a681f8135992e0cc18a7b0758d1a880eb3c64d9c722f80d68b744bb3a89c223a0c38164cf4a423f91f8a533",
		"d942df0df09ac042cd3b641144c98d8fda0980bb037fc5c0e7f2e9a073b073dc4bb8a8c1f4cb5b45f5805c6523741ed0571d6779b15829b2faa280fc60b50645",
		"a8df4dbdc4c5d535eb3f590dd7e097e0b1b3ace931447d987ac25e94",
		"4134fa637cc87ac52320f311f4a681ef740b58da8ce2c09c721eedd720179c4f",
		"cb6b185e7f5e232db435f2d02f6e8bf16db60f895fc92cbc7fbad356807c5d693853fe638305c4d163d7b35d51b9041c",
		"3e92887ff9919469c7927cfc56523492abdcfda14d7fad2bf1de875fa05e2416674ae1793799b7a0703d4da88cf67d09ac64004dc4e332adb6b199ba6ee52479",
		"4353e7ed38eb9cc41abbf0709c6ab707de70d759e5d6eedf09043fd0",
		"4996d371abd506e72178b4cbea8e9f5ad781a5a566543d97f89a4efb13d5bb5f",
		"abe7a3a2309b13851157043658589d3f011cf2b58ded164cefc56654146f1884ed73d195c5176ea879f7821e9c332b3a",
		"64e5a4e32097cf9eeb8170b582503af020076f2856d7e67c0acb4c53c5f7fcb6bed9e330bcaac7c9229d7875efbe233877d5d3a43a25376eaf7804320599676e",
		"64d0e8a1be3cf30ef6727b30a6e428f7f068d44634c943d277ad8e7f",
		"295fef4d46110ee21fba0d1798a1bb7c1bbc88306bc9b7661b18ace7170f02ae",
		"f25214f92d3b1ccc162c46a74ad8fafd33e00abdcb3048744d93d36bc77f2796f92d91cea8946b357f14f249792dd8de",
		"eb9748309c6b70ffe82820052ad26ea99f43968d2af359adc804b2a76741a62ea8d710f018ea113c2259d0bd6687e3838602ae6c1dff727ae985f059141c7217",
		"e7d72a8635ec1a6b177046d8f98c60bb934204fc7a6990469bed72cd",
		"3456dac4a1209693b8c2bd2cd27fc64c330ce283b8d43a0d9662761825865cbc",
		"db04f83cdfeb0bc711e9e2aca51f838e8dd725155daa64cbd3b913d40e56475ecc7b4ecab75b771a5f7ccc365ef3a5ae",
		"703be67f6d9c1a2d99b1995e60744a05097ddde919f9a4eb4ee3fe0088c9e5a502daa96064f284fbda40d1b22cc70d273b46bb4e4f48fe0e5a2d29f296bfe8a6",
		"59e82bec68b80a525fd12a0408580ecda62ee6d4c8b12d6529ce0c20",
		"ba7f1834cddbba9f82cd4dcf7a106bb2e615fec90020f5a5de8efff8d49198b6",
		"885b25cbe374a0e76d495caea95475a2b870909abe434035316d4c60dcade94b8b18b18102059034de194e9f1e682811",
		"8f0c52262455dca2c7d7a6157bbc545cb34cea699290735fbc46b5c83173a458d53666e2137eb66e3b672070a97134775e3006a8ee542585ddb54183932f768d",
		"faa2566329eef816510312ec08e329cdc855e96a440e47e92da4cc92",
		"413cf357775aef534fcd49da91a30f7877b50bbd924a20649315a4827f79cac0",
		"d00dfd2110cb761ca3f3f3195037e89b29660805e6ce37e9f3164a22517e1c1ac937adbd38c78ad568ada4340f039e58",
		"4d77323a341a3c8edd736ed718cb07deb66bcc5317dbc6f73da4c686dcec8440e414a2ed46ed5219cc226160b2416b276d9d9d95ad83c4c9e301397c89e37864",
		"f761d1bb315dd7d743de1e76b6fe0e8a9f32c74be8819d2f09b49349",
		"51a0283a8332e99679957152f59acd063596f607f567053ef538e64c9f75ecf2",
		"44f9abfc39f11d63ab2e92f5b2c62b770a76bcedee29e2674c658088c39e383c03ed2c3d9746af7aadf76c0cc18144f9",
		"746a42aeef3cdf3f3deb899628f20c36c92e2268fb0ad45883cfbfe74394ff84a15a6f421d26f46c231a911ddae489049019c610e2c0a309e4b9704e57035cd8",
		"13d59fd552e8a83f759fa10b443d560e36f9b32d3d54553690036282",
		"aee1e49906909f36267064b9e924a42cc84832c16f2b453ce3dc90ed09431ee9",
		"5e3347f462f9514df49d6d6d9f2df802144318dcb22fef6add07ddfb46f72bdf1c9bb9323bcaee870039af86978f1068",
		"0354e1653a3c8f4307f4d31f83e84f532598b69211618a73088969768ed995a718fb1da5feaddb4ad79600ee76efe2a52b58bd33029ee291af5419f392f8c16e",
		"7e28e44f1df828b246b1952d24c7de5faa98d139a9daee2e491d6edb",
		"3d6f9314a37c0dec3cd87ccfb4b5d515c3449e40c1640c0123e511134fd3371c",
		"49b4adb6da7229dd2803835c7f3846376742ec22728aaf9117ebe4f1ed9812409781400c206f0132b1d992d2794c3ec6",
		"776f1df7cbffdff1df64d2fdcb3b3a238667916f901b27b5cddc54f2664572ac6326459b44ab1d28dd023b2a44a868cb6a950ceec4f4720c58cc2c8ad03f5c55",
		"d5756b31553455e694bde2514720f02e6eff5004df280db6741731c0",
		"5f24381c43d4473a3a65dba2430023c3415eb682d7ab0baffec2576c6e20649e",
		"2d9861baf20a6d4d094c8eb8e2f4462492856567fafc8e23b5d9bd696098561315202928c5bae4dd73dc0ed0dce7bc6e",
		"b53488fe0c03f3ddd91cf153e7cc33701cc4789bb8a02c3a4c8215d6abab0589dffc7778058b606c20725cca5fe1b3c480269eaa43677eb1b5ba84c9ee4c6ef1",
		"e710eeadf70563847a754d60ba7c97362b08b0eec91e08b7f628dce6",
		"40e655a0042c7fc243710579c0d6fad05daceba7d474de35cccb17d194c2cda2",
		"de7e4e4470424afae94fea724569c73744f766215c48e5314dd0b2256f8c2a5848bbe3f341f32a59e6b4d055f2fde812",
		"ac8fc5c0a7dc20b9234524accd6000bcafbad2850a66455600873c13d1cb6875824f6888630829896eb411ee4973896e0fb6487d8be89fcc3dfd9eed6c93fe90",
		"ed37079a71b014db056fba28be20e275bef7002152685f55563ee7ba",
		"c1743d3e79e9a1516251b027c4210f8b592630bfc9456d4ce36b17e9908bf8fa",
		"958c222c76083e19e2e64edf9b71a73844090592f6282874dcad59ff94c078a315c5f16f625643a8c052f306cfc280fb",
		"61d8587ac16b5f06e2010ab90e4e369d609ba8e995cb5a65098ef4d681d4c11ef46f1a62bb897981f5e8bce8496645885302ca8269a4042c337e7564e4075bf0",
		"c5bb921beb40e901bb6a8b9ff87f54acd102b31617b69dae27d80edc",
		"791faf2ff8ab27eb6ea5b860baa8db20336213bbe1da02e050f4da3451d868e4",
		"f4edc71aa7bcf93bf466b278a378711b4333ab7c8bf6a0b597d93a6068785ad95a390b6d7e0cf21ee611e0fe5cb4b620",
		"161e9fe6165a4e9cc91779fe436e5b8e59d11bfb2328bdf389de7cefa2d613f36d8c4249400ae1b2bfed5ebbb52ab2c164b7498a7c187d4f52e7d98a7ef77ffb",
		"cefb803e3da97cca87920e3142f84f9601eb3f1a68383d05c9c71e09",
		"d07ffdb737ad569dc7f648f0bc6dd0968475a7c72dc8aaf8a40184829d9e276d",
		"24395363a35adc5c9cc1fab5346afdacc0432727c3ff6b5456b966fd8b168ba0c6151d4853959175c7fb83845fc5c430",
		"ff6253c8686d52864b21b6fac5bd91dcadddb30daf39e8a70f07c26904c49889f32449e5cbeb8781c21dff89dce285783bba9df10d6c86fcd24064ed970da13d",
		"e46947a65472b14a893436f8d8aeb434a324470d21b85993122a25d3",
		"2d31fe5b4080fb6a965196936406cb33d3a929030ad9b151dde066e533bb4b99",
		"891902665b409c474d75a53985a5e5e324cb05f95edeb0e8df714a8895353faa5c8aa88849189aa8a78328ab899a70d9",
		"2cfea6de3c394e88ffbc2236d8b87381cb14e6e5a23235e5d03fa2f395383dd239df59807edd148d39bb271d64e1ab6f7746a9bbd16e553a538ea32bebd3bfee",
		"9eaa48302221fbd4458032b63c2a0fd656f66baf06be0e727c56410a",
		"2277dd53396cef38fe8aa8716739e7d2f4fa33a55c697f21409e96715eca91dc",
		"159b07f2aeac0a2aee7f65a367003acfd3f42efce3799170bc8cf29d2bad1eeccdae3b35b86d5c95b980d5d026e68153",
		"d5fdcdb2a4cf311ab061ccf062a62bbb2802103f30b87e077ef066f5b20e01b32e4a7443d5bfa518767c92c76b0835a3bebd013d5749fd5ab9586b72824b5a1f",
		"2e4f3b82daecda1e8f546590c2006625001fd3802b04781233355d38",
		"de9a4c24206979eea656585ffc5877faf6203b1bc90a4fbde802f05c36d44c73",
		"b1bd8d0584707f9cca3adc794da83a54d268b0512e755d8e9eb7e8f209189de3447fb14cf3e3b0e34634c62b96e09d80",
		"3d29a32838788f1068a017d30abbde48438e200164d16810cfc5e72131ca0493c3f147d77e58b594f3dc6dce0fe6d157edb37ddd16fbafa8892d03dd3b7438a6",
		"ce0dc42b066c8d4aa4c412e625896c3253a371fba7647049689d269e",
		"1c76a266bcfc80883d589fbd593ccd628d0b368ecbb40bd9e932d9def73598c9",
		"bd3c93282ea84a25b0096468012511b8b2f8217e185d63800511048bd0da36c0d8e1af3113a68f68f0fdaca07fb4fe4f",
		"c04d24a5098866a4689f8dea4bae1ca23a86ae20c38550b3ff42694730fcc71fce3a13e6eddd1e948cc5f42a6b479612ec8f5055eb541de81a72e4cf99c35a3b",
		"b6b0c36fe2b845f393199b5896c06ea2203535fa40f61454f66be7ec",
		"ae1e2d41aabdd5f20028d82dbd03bb02c64de6021b9c5afb5db3bec5b415528c",
		"dfd45758ce0596a69b5ebcfbd41fd82ab73336b0798882a45b0fa19bdbc31cbfd2dbfc21b599d7baaaf0aa6db172d384",
		"54944f3d94baf90603f2d2536d7612aced75dee0e8f1e9e70490729f33539e87a87c1db5eb12779802e47a9a2c4629f00d6363ea282bf876c28779140ab73d17",
		"a7232b923c520ff46e1d3cf1c21fb712308b37c54ddf49b8a57618b0",
		"91cbc140c95662bd52cfb156b36fe9eed52f264dd7babde25cb5e84837633b88",
		"d68d76b03025d73fad0495718910f5167bca3667e7dcd90a91f721ec8d28f79d8ae7575db6ddc0d889b50619c3aed756",
		"757d20247c70b52e77308ef1bc1eb3dcb5ccd04f9e22c62fb2c62ad49350f336a08d126e9d00edb85af69ff92524b477b9f515c4ce8742cfa2a9c6b243cbe6a0",
		"ee18735acfa752902c13bd2fa0472b2fb11687f80bd1b717edd92a5e",
		"8f8eeb8c5f4c7ca72654a2f6b8ee7c84e0367846655af46494e3e98f91208a43",
		"6cbb72f240f59e8acef6e7995430e27d277b0c838f6fe2873ce8ae2ca05ce74c66d89d1a9b974026de3d8694be65ac73",
		"7e94924b09f131d8e32a1c6ed0f5340e55aae3bbfd3619cd297acf40730d1ebf92acd763a06a25d58e94d214cacca52542768e78851ba1d82f56c0154d2fd5c8",
		"314056dd3d94ca15731815ab1b85322ba07ff26e90555d435822e1af",
		"cbc6fd3ce90dafe74a83bb29674e48082f59efd400b989214a63e85ae1984d43",
		"93115f5c2f020a9c6cd4bf4a20b09abf0096b78b0e2646e64aeee0d87498d2b2cdb5458daf0253273942499a757cf352",
		"827727b142e9a09e066f3847c800912b7d76c26cc22874e92b359880b4dc2f593cf13c473004d0033b7afe218db7b58792b5545c812baeaf6d096cbe9f36ded2",
	};
	/**/
	const char** expected = expected_outputs;

	for (uint8_t i = 0, count_i = COUNT_OF(lengths); i < count_i; ++i)
	{
		const uint8_t* finish = in + lengths[i];

		for (uint8_t j = 0, count_j = COUNT_OF(SHA3_hash_lengths); j < count_j; ++j)
		{
			if (!buffer_resize(output, 31745))
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			if (!hash_algorithm_sha3(in, finish, SHA3_hash_lengths[j], output))
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			const uint8_t* input_start = buffer_data(output, 0) + 31745;
			char* input_finish = (char*)buffer_data(output, 0) + buffer_size(output);

			if (!bytes_to_string(input_start, (uint8_t)(buffer_size(output) - 31745), input_finish))
			{
				printf("%i %s\n", __LINE__, "FAILURE");
				return 0;
			}

			const uint8_t expected_length = (uint8_t)strlen(*expected);
			const uint8_t returned_length = (uint8_t)strlen(input_finish);

			if (expected_length != returned_length ||
				0 != memcmp(*expected, input_finish, returned_length))
			{
				printf("%i\nexpected: '%s' (%i)\nreturned: '%s' (%i)\n%s\n", i,
					   *expected, expected_length,
					   input_finish, returned_length, "FAILURE");
				return 0;
			}

			++expected;
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
	(void)argc;
	(void)argv;
	/**/
	struct buffer output;
	SET_NULL_TO_BUFFER(output);

	if (!Test_bytes_to_string(&output))
	{
		buffer_release(&output);
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	if (!Test_BLAKE2b_1(&output) ||
		!Test_BLAKE2b_2(&output))
	{
		buffer_release(&output);
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	if (!Test_BLAKE3(&output))
	{
		buffer_release(&output);
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	if (!Test_CRC32(&output))
	{
		buffer_release(&output);
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	if (!Test_Keccak(&output))
	{
		buffer_release(&output);
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	if (!Test_SHA3(&output))
	{
		buffer_release(&output);
		printf("%i %s\n", __LINE__, "FAILURE");
		return EXIT_FAILURE;
	}

	buffer_release(&output);
	return 8 == printf("%s\n", "SUCCESS") ? EXIT_SUCCESS : EXIT_FAILURE;
}
