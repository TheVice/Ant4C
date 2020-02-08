/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "conversion.h"
#include "hash.h"
#include "range.h"
};

#include <string>
#include <cstdint>

class TestHashAlgorithm : public TestsBaseXml
{
};

uint8_t input_generator(buffer* input, ptrdiff_t count)
{
	if (NULL == input || count < 1)
	{
		return 0;
	}

	if (!buffer_resize(input, count))
	{
		return 0;
	}

	uint8_t* ptr = buffer_data(input, 0);

	for (ptrdiff_t i = 0, j = 0; i < count; ++i, j = j < 250 ? j + 1 : 0)
	{
		ptr[i] = (uint8_t)j;
	}

	return 1;
}

TEST(TestHashAlgorithm_, hash_algorithm_bytes_to_string)
{
	std::string input(UINT8_MAX + 1, '\0');
	input.clear();

	for (uint16_t i = 0; i < UINT8_MAX + 1; ++i)
	{
		input.push_back((char)i);
	}

	const std::string expected_output(
		"000102030405060708090a0b0c0d0e0f"
		"101112131415161718191a1b1c1d1e1f"
		"202122232425262728292a2b2c2d2e2f"
		"303132333435363738393a3b3c3d3e3f"
		"404142434445464748494a4b4c4d4e4f"
		"505152535455565758595a5b5c5d5e5f"
		"606162636465666768696a6b6c6d6e6f"
		"707172737475767778797a7b7c7d7e7f"
		"808182838485868788898a8b8c8d8e8f"
		"909192939495969798999a9b9c9d9e9f"
		"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
		"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
		"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
		"d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
		"e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
		"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff");
	const auto input_in_range(string_to_range(input));
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_TRUE(hash_algorithm_bytes_to_string(
					input_in_range.start, input_in_range.finish, &output)) << buffer_free(&output);
	//
	const auto returned_output(buffer_to_string(&output));
	buffer_release(&output);
	ASSERT_EQ(expected_output, returned_output);
}

TEST_F(TestHashAlgorithm, BLAKE2)
{
	typedef uint8_t (*BLAKE2b)(const uint8_t*, const uint8_t*, struct buffer*);
	static const BLAKE2b functions[] =
	{
		&hash_algorithm_blake2b_160, &hash_algorithm_blake2b_256,
		&hash_algorithm_blake2b_384, &hash_algorithm_blake2b_512
	};
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const std::string BLAKE2b_160(node.node().select_node("BLAKE2b_160").node().child_value());
		const std::string BLAKE2b_256(node.node().select_node("BLAKE2b_256").node().child_value());
		const std::string BLAKE2b_384(node.node().select_node("BLAKE2b_384").node().child_value());
		const std::string BLAKE2b_512(node.node().select_node("BLAKE2b_512").node().child_value());
		//
		const std::string* outputs[] =
		{
			&BLAKE2b_160, &BLAKE2b_256,
			&BLAKE2b_384, &BLAKE2b_512
		};

		for (uint8_t i = 0; i < 4; ++i)
		{
			if (outputs[i]->empty())
			{
				continue;
			}

			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
			//
			auto input_in_range(string_to_range(input));
			null_range_to_empty(input_in_range);
			//
			auto returned = (functions[i])(input_in_range.start, input_in_range.finish, &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl << (int)i << buffer_free(&output);
			//
			returned = (uint8_t)buffer_size(&output);
			ASSERT_TRUE(buffer_append(&output, NULL, 2 * returned + 2)) << buffer_free(&output);
			ASSERT_TRUE(buffer_resize(&output, returned)) << buffer_free(&output);
			//
			input_in_range = buffer_to_range(&output);
			returned = hash_algorithm_bytes_to_string(input_in_range.start, input_in_range.finish, &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl << (int)i << buffer_free(&output);
			//
			const std::string str_output(buffer_to_string(&output).substr(range_size(&input_in_range)));
			ASSERT_EQ(*(outputs[i]), str_output) << input << std::endl << (int)i << buffer_free(&output);
		}

		ASSERT_NE(0, buffer_size(&output)) << buffer_free(&output);
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}

extern "C" {
	extern uint8_t BLAKE3(const uint8_t* start, const uint8_t* finish, uint8_t hash_length, uint8_t* output);
};

TEST_F(TestHashAlgorithm, BLAKE3)
{
	buffer input;
	SET_NULL_TO_BUFFER(input);
	ASSERT_TRUE(input_generator(&input, 31745)) << buffer_free(&input);
	uint8_t* ptr = buffer_data(&input, 0);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	ASSERT_TRUE(buffer_resize(&output, 2 * UINT8_MAX)) << buffer_free(&input) << buffer_free(&output);

	for (const auto& node : nodes)
	{
		const auto input_length = (uint16_t)INT_PARSE(node.node().select_node("input").node().child_value());
		ASSERT_LT(input_length, 31745) << buffer_free(&input) << buffer_free(&output);
		//
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		const auto expected_output(get_data_from_nodes(node, "output"));
		//
		const uint8_t* start = buffer_data(&output, 0);
		const uint8_t* finish = buffer_data(&output, 131);
		//
		ASSERT_NE(nullptr, start) << buffer_free(&input) << buffer_free(&output);
		ASSERT_NE(nullptr, finish) << buffer_free(&input) << buffer_free(&output);
		ASSERT_LT(start, finish) << buffer_free(&input) << buffer_free(&output);
		//
		auto returned = BLAKE3(ptr, ptr + input_length, 131, buffer_data(&output, 0));
		ASSERT_EQ(expected_return, returned) << buffer_free(&input) << buffer_free(&output);
		//
		ASSERT_TRUE(buffer_resize(&output, 131)) << buffer_free(&input) << buffer_free(&output);
		returned = hash_algorithm_bytes_to_string(start, finish, &output);
		ASSERT_EQ(expected_return, returned) << buffer_free(&input) << buffer_free(&output);
		//
		ASSERT_STREQ(expected_output.c_str(), (const char*)finish) << buffer_free(&input) << buffer_free(&output);

		if (0 == input_length)
		{
			--node_count;
			continue;
		}

		returned = BLAKE3(ptr + 1, ptr + input_length + 1, 131, buffer_data(&output, 0));
		ASSERT_TRUE(returned) << buffer_free(&input) << buffer_free(&output);
		//
		ASSERT_TRUE(buffer_resize(&output, 131)) << buffer_free(&input) << buffer_free(&output);
		returned = hash_algorithm_bytes_to_string(start, finish, &output);
		ASSERT_TRUE(returned) << buffer_free(&input) << buffer_free(&output);
		//
		ASSERT_STRNE(expected_output.c_str(), (const char*)finish) << buffer_free(&input) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
	buffer_release(&input);
}

TEST_F(TestHashAlgorithm, crc32)
{
	static const std::string empty(8, '0');
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		const std::string decreasing(node.node().select_node("decreasing").node().child_value());
		const std::string increasing(node.node().select_node("increasing").node().child_value());
		//
		const std::string* outputs[] =
		{
			&decreasing, &increasing
		};

		for (uint8_t i = 0; i < 2; ++i)
		{
			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
			//
			auto input_in_range(string_to_range(input));
			null_range_to_empty(input_in_range);
			uint32_t digit_output = 0;
			//
			auto returned = hash_algorithm_crc32(input_in_range.start, input_in_range.finish, &digit_output, i);
			ASSERT_EQ(expected_return, returned) << input << std::endl << (int)i << std::endl << buffer_free(&output);
			//
			returned = hash_algorithm_bytes_to_string(
						   (const uint8_t*)&digit_output, (const uint8_t*)((&digit_output) + 1), &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&output);
			const std::string str_output(buffer_to_string(&output));

			if (outputs[i]->empty())
			{
				ASSERT_EQ(empty, str_output)
						<< input << std::endl << (int)i << std::endl << buffer_free(&output);
			}
			else
			{
				ASSERT_EQ(*(outputs[i]), str_output)
						<< input << std::endl << (int)i << std::endl << buffer_free(&output);
			}
		}

		--node_count;
	}

	buffer_release(&output);
}

extern "C" {
	uint8_t Keccak(const uint8_t* input, const ptrdiff_t length, uint8_t is_sha3,
				   uint16_t rate, uint16_t capacity, uint8_t* output);
};

TEST_F(TestHashAlgorithm, Keccak)
{
	static const uint16_t rate[] = { 1152, 1088, 832, 576, 1152, 1088, 832, 576 };
	static const uint16_t capacity[] = { 448, 512, 768, 1024, 448, 512, 768, 1024 };
	//
	buffer input;
	SET_NULL_TO_BUFFER(input);
	ASSERT_TRUE(input_generator(&input, 31745)) << buffer_free(&input);
	uint8_t* ptr = buffer_data(&input, 0);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const auto input_length = (uint16_t)INT_PARSE(node.node().select_node("input").node().child_value());
		ASSERT_LT(input_length, 31745) << buffer_free(&input) << buffer_free(&output);
		//
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const std::string Keccak_224(node.node().select_node("Keccak_224").node().child_value());
		const std::string Keccak_256(node.node().select_node("Keccak_256").node().child_value());
		const std::string Keccak_384(node.node().select_node("Keccak_384").node().child_value());
		const std::string Keccak_512(node.node().select_node("Keccak_512").node().child_value());
		//
		const std::string SHA3_224(node.node().select_node("SHA3_224").node().child_value());
		const std::string SHA3_256(node.node().select_node("SHA3_256").node().child_value());
		const std::string SHA3_384(node.node().select_node("SHA3_384").node().child_value());
		const std::string SHA3_512(node.node().select_node("SHA3_512").node().child_value());
		//
		const std::string* expected_result[] =
		{
			&Keccak_224, &Keccak_256, &Keccak_384, &Keccak_512,
			&SHA3_224, &SHA3_256, &SHA3_384, &SHA3_512
		};

		for (uint8_t i = 0; i < 8; ++i)
		{
			if (expected_result[i]->empty())
			{
				continue;
			}

			ASSERT_TRUE(buffer_resize(&output, 2 * UINT8_MAX)) << buffer_free(&input) << buffer_free(&output);
			uint8_t* o = buffer_data(&output, 0);
			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&input) << buffer_free(&output);
			uint8_t returned = Keccak(ptr, input_length, 3 < i, rate[i], capacity[i], o);
			ASSERT_EQ(expected_return, returned) <<
												 input_length << std::endl << (int)i << buffer_free(&input) << buffer_free(&output);
			//
			returned = (uint8_t)(capacity[i] / (2 * 8));
			ASSERT_TRUE(buffer_append(&output, NULL, 2 * returned + 2)) << buffer_free(&input) << buffer_free(&output);
			ASSERT_TRUE(buffer_resize(&output, returned)) << buffer_free(&input) << buffer_free(&output);
			//
			const auto input_in_range = buffer_to_range(&output);
			returned = hash_algorithm_bytes_to_string(input_in_range.start, input_in_range.finish, &output);
			ASSERT_EQ(expected_return, returned) <<
												 input_length << std::endl << (int)i << buffer_free(&input) << buffer_free(&output);
			//
			const std::string result(buffer_to_string(&output).substr(range_size(&input_in_range)));
			ASSERT_EQ(*(expected_result[i]), result) <<
					input_length << std::endl << (int)i << buffer_free(&input) << buffer_free(&output);
		}

		ASSERT_NE(0, buffer_size(&output)) << buffer_free(&input) << buffer_free(&output);
		--node_count;
	}

	buffer_release(&output);
	buffer_release(&input);
}

TEST_F(TestHashAlgorithm, sha3)
{
	typedef uint8_t(*Keccak)(const uint8_t*, const uint8_t*, struct buffer*);
	static const Keccak functions[] =
	{
		&hash_algorithm_keccak_224, &hash_algorithm_keccak_256,
		&hash_algorithm_keccak_384, &hash_algorithm_keccak_512,
		&hash_algorithm_sha3_224, &hash_algorithm_sha3_256,
		&hash_algorithm_sha3_384, &hash_algorithm_sha3_512
	};
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const auto input(get_data_from_nodes(node, "input"));
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const std::string Keccak_224(node.node().select_node("Keccak_224").node().child_value());
		const std::string Keccak_256(node.node().select_node("Keccak_256").node().child_value());
		const std::string Keccak_384(node.node().select_node("Keccak_384").node().child_value());
		const std::string Keccak_512(node.node().select_node("Keccak_512").node().child_value());
		//
		const std::string SHA3_224(node.node().select_node("SHA3_224").node().child_value());
		const std::string SHA3_256(node.node().select_node("SHA3_256").node().child_value());
		const std::string SHA3_384(node.node().select_node("SHA3_384").node().child_value());
		const std::string SHA3_512(node.node().select_node("SHA3_512").node().child_value());
		//
		const std::string* expected_result[] =
		{
			&Keccak_224, &Keccak_256, &Keccak_384, &Keccak_512,
			&SHA3_224, &SHA3_256, &SHA3_384, &SHA3_512
		};

		for (uint8_t i = 0; i < 8; ++i)
		{
			if (expected_result[i]->empty())
			{
				continue;
			}

			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
			//
			auto input_in_range(string_to_range(input));
			null_range_to_empty(input_in_range);
			//
			auto returned = (functions[i])(input_in_range.start, input_in_range.finish, &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl << (int)i << buffer_free(&output);
			//
			returned = (uint8_t)buffer_size(&output);
			ASSERT_TRUE(buffer_append(&output, NULL, 2 * returned + 2)) << buffer_free(&output);
			ASSERT_TRUE(buffer_resize(&output, returned)) << buffer_free(&output);
			//
			input_in_range = buffer_to_range(&output);
			returned = hash_algorithm_bytes_to_string(input_in_range.start, input_in_range.finish, &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl << (int)i << buffer_free(&output);
			//
			const std::string result(buffer_to_string(&output).substr(range_size(&input_in_range)));
			ASSERT_EQ(*(expected_result[i]), result) << input << std::endl << (int)i << buffer_free(&output);
		}

		ASSERT_NE(0, buffer_size(&output)) << buffer_free(&output);
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}
