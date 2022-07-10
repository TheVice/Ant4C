/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "echo.h"
#include "hash.h"
#include "path.h"
#include "range.h"
#include "text_encoding.h"
};

#include <string>
#include <cstdint>
#include <iostream>
#include <algorithm>

class TestHashAlgorithm : public TestsBaseXml
{
};

uint8_t input_generator(buffer* input, ptrdiff_t count)
{
	if (nullptr == input || count < 1)
	{
		return 0;
	}

	if (!buffer_resize(input, count))
	{
		return 0;
	}

	auto ptr = buffer_data(input, 0);

	for (ptrdiff_t i = 0, j = 0; i < count; ++i, j = j < 250 ? j + 1 : 0)
	{
		ptr[i] = static_cast<uint8_t>(j);
	}

	return 1;
}

static const uint8_t input_array_to[] =
{
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

TEST(TestHashAlgorithm_, hash_algorithm_uint8_t_array_to_uint32_t)
{
	uint32_t output;

	for (uint8_t i = 0, count = COUNT_OF(input_array_to); i < count; ++i)
	{
		ASSERT_TRUE(
			hash_algorithm_uint8_t_array_to_uint32_t(
				input_array_to, input_array_to + i, &output));
	}
}

TEST(TestHashAlgorithm_, hash_algorithm_uint8_t_array_to_uint64_t)
{
	uint64_t output;

	for (uint8_t i = 0, count = COUNT_OF(input_array_to); i < count; ++i)
	{
		ASSERT_TRUE(
			hash_algorithm_uint8_t_array_to_uint64_t(
				input_array_to, input_array_to + i, &output));
	}
}

TEST(TestHashAlgorithm_, hash_algorithm_bytes_to_string)
{
	std::string input(UINT8_MAX + 1, '\0');
	input.clear();

	for (uint16_t i = 0; i < UINT8_MAX + 1; ++i)
	{
		input.push_back(static_cast<char>(i));
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
	//
	const auto input_in_a_range(string_to_range(input));
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_TRUE(
		hash_algorithm_bytes_to_string(
			input_in_a_range.start, input_in_a_range.finish, &output))
			<< buffer_free(&output);
	//
	const auto returned_output(buffer_to_string(&output));
	buffer_release(&output);
	ASSERT_EQ(expected_output, returned_output);
}

TEST_F(TestHashAlgorithm, BLAKE2)
{
	static const uint8_t hash_sizes[] =
	{
		20, 32, 48, 64
	};
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	uint64_t output_[16];

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const std::string return_str(
			the_node.select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const std::string input(
			the_node.select_node("input").node().child_value());
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string BLAKE2b_160(the_node.select_node("BLAKE2b_160").node().child_value());
		const std::string BLAKE2b_256(the_node.select_node("BLAKE2b_256").node().child_value());
		const std::string BLAKE2b_384(the_node.select_node("BLAKE2b_384").node().child_value());
		const std::string BLAKE2b_512(the_node.select_node("BLAKE2b_512").node().child_value());
		//
		const std::string* outputs[] =
		{
			&BLAKE2b_160, &BLAKE2b_256,
			&BLAKE2b_384, &BLAKE2b_512
		};

		for (uint8_t i = 0, count = COUNT_OF(outputs); i < count; ++i)
		{
			if (outputs[i]->empty())
			{
				continue;
			}

			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
			//
			input_in_a_range = string_to_range(input);
			null_range_to_empty(input_in_a_range);
			//
			auto returned = hash_algorithm_blake2b(
								input_in_a_range.start, input_in_a_range.finish,
								8 * hash_sizes[i], &output);
			ASSERT_EQ(expected_return, returned)
					<< input << std::endl << static_cast<int>(i) << buffer_free(&output);
			//
			returned = static_cast<uint8_t>(buffer_size(&output));
			ASSERT_TRUE(buffer_append(&output, nullptr, 2 * returned + 2)) << buffer_free(&output);
			ASSERT_TRUE(buffer_resize(&output, returned)) << buffer_free(&output);
			//
			input_in_a_range = buffer_to_range(&output);
			returned = hash_algorithm_bytes_to_string(
						   input_in_a_range.start, input_in_a_range.finish, &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl <<
												 static_cast<int>(i) << buffer_free(&output);
			//
			const auto str_output(
				buffer_to_string(&output).substr(range_size(&input_in_a_range)));
			ASSERT_EQ(*(outputs[i]), str_output) << input << std::endl <<
												 static_cast<int>(i) << buffer_free(&output);
			//
			ptrdiff_t bytes_compressed = 0;
			//
			ASSERT_TRUE(BLAKE2b_init(hash_sizes[i], output_)) << buffer_free(&output);
			//
			input_in_a_range = string_to_range(input);
			null_range_to_empty(input_in_a_range);

			if (128 < input.size())
			{
				const ptrdiff_t bytes_to_compress = 128 * (input.size() / 128);
				//
				ASSERT_TRUE(BLAKE2b_core(
								input_in_a_range.start, input_in_a_range.start + bytes_to_compress,
								&bytes_compressed, output_)) << buffer_free(&output);
				//
				input_in_a_range.start += bytes_to_compress;
			}

			ASSERT_TRUE(BLAKE2b_final(
							input_in_a_range.start, &bytes_compressed,
							static_cast<uint8_t>(range_size(&input_in_a_range)),
							output_)) << buffer_free(&output);
			//
			ASSERT_EQ(bytes_compressed, static_cast<ptrdiff_t>(input.size())) << buffer_free(&output);
			ASSERT_TRUE(buffer_resize(&output, hash_sizes[i])) << buffer_free(&output);
			//
			input_in_a_range.start = reinterpret_cast<const uint8_t*>(output_);
			input_in_a_range.finish = input_in_a_range.start + hash_sizes[i];
			returned = hash_algorithm_bytes_to_string(input_in_a_range.start, input_in_a_range.finish, &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl <<
												 static_cast<int>(i) << buffer_free(&output);
			//
			const auto str_output_(buffer_to_string(&output).substr(range_size(&input_in_a_range)));
			ASSERT_EQ(*(outputs[i]), str_output_) << input << std::endl <<
												  static_cast<int>(i) << buffer_free(&output);
		}

		ASSERT_NE(0, buffer_size(&output)) << buffer_free(&output);
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}

TEST_F(TestHashAlgorithm, hash_algorithm_blake3)
{
	buffer input;
	SET_NULL_TO_BUFFER(input);
	ASSERT_TRUE(input_generator(&input, 31745)) << buffer_free(&input);
	auto ptr = buffer_data(&input, 0);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	ASSERT_TRUE(buffer_resize(&output, 2 * UINT8_MAX)) << buffer_free(&input) << buffer_free(&output);
	//
	const auto start = buffer_data(&output, 0);
	//
	ASSERT_NE(nullptr, start) << buffer_free(&input) << buffer_free(&output);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const std::string input_str(the_node.select_node("input").node().child_value());
		auto input_in_a_range = string_to_range(input_str);
		const auto input_length =
			static_cast<uint16_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		ASSERT_LT(input_length, 31745) << buffer_free(&input) << buffer_free(&output);
		//
		const std::string hash_length_str(the_node.select_node("hash_length").node().child_value());
		input_in_a_range = string_to_range(hash_length_str);
		const auto hash_length =
			static_cast<uint16_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const auto expected_output(get_data_from_nodes(the_node, "output"));
		//
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&input) << buffer_free(&output);
		auto returned = hash_algorithm_blake3(ptr, ptr + input_length, hash_length, &output);
		ASSERT_TRUE(returned) << buffer_free(&input) << buffer_free(&output);
		//
		const auto finish = buffer_data(&output, 0) + buffer_size(&output);
		ASSERT_NE(nullptr, finish) << buffer_free(&input) << buffer_free(&output);
		ASSERT_LT(start, finish) << buffer_free(&input) << buffer_free(&output);
		//
		const auto size = buffer_size(&output);
		//
		returned = hash_algorithm_bytes_to_string(start, finish, &output);
		ASSERT_TRUE(returned) << buffer_free(&input) << buffer_free(&output);
		//
		std::string returned_hash(reinterpret_cast<const char*>(finish), buffer_size(&output) - size);
		//
		const auto length = hash_length / sizeof(uint32_t);
		ASSERT_EQ(expected_output.substr(0, length), returned_hash) << buffer_free(&input) << buffer_free(&output);

		if (0 == input_length)
		{
			--node_count;
			continue;
		}

		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&input) << buffer_free(&output);
		returned = hash_algorithm_blake3(ptr + 1, ptr + input_length + 1, hash_length, &output);
		ASSERT_TRUE(returned) << buffer_free(&input) << buffer_free(&output);
		//
		returned = hash_algorithm_bytes_to_string(start, finish, &output);
		ASSERT_TRUE(returned) << buffer_free(&input) << buffer_free(&output);
		//
		returned_hash.clear();
		returned_hash.append(reinterpret_cast<const char*>(finish), buffer_size(&output) - size);
		//
		ASSERT_NE(expected_output.substr(0, length), returned_hash) << buffer_free(&input) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
	buffer_release(&input);
}

TEST_F(TestHashAlgorithm, crc32)
{
	static const std::string empty(sizeof(uint64_t), '0');
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const std::string input(the_node.select_node("input").node().child_value());
		//
		const std::string return_str(the_node.select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string decreasing(the_node.select_node("decreasing").node().child_value());
		const std::string increasing(the_node.select_node("increasing").node().child_value());
		//
		const std::string* outputs[] =
		{
			&decreasing, &increasing
		};

		for (uint8_t i = 0, count = COUNT_OF(outputs); i < count; ++i)
		{
			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
			//
			input_in_a_range = string_to_range(input);
			null_range_to_empty(input_in_a_range);
			ASSERT_TRUE(buffer_append(&output, NULL, UINT8_MAX)) << buffer_free(&output);
			auto digit_output = buffer_data(&output, UINT8_MAX - sizeof(uint32_t));
			//
			auto returned = hash_algorithm_crc32(input_in_a_range.start, input_in_a_range.finish, digit_output, i);
			ASSERT_EQ(expected_return, returned) << input << std::endl <<
												 static_cast<int>(i) << std::endl << buffer_free(&output);
			//
			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
			returned = hash_algorithm_bytes_to_string(digit_output, digit_output + sizeof(uint32_t), &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&output);
			const auto str_output(buffer_to_string(&output));

			if (outputs[i]->empty())
			{
				ASSERT_EQ(empty, str_output) <<
											 input << std::endl <<
											 static_cast<int>(i) << std::endl << buffer_free(&output);
			}
			else
			{
				ASSERT_EQ(*(outputs[i]), str_output) <<
													 input << std::endl <<
													 static_cast<int>(i) << std::endl << buffer_free(&output);
			}
		}

		--node_count;
	}

	buffer_release(&output);
}

typedef uint8_t(*Keccak)(
	const uint8_t* start, const uint8_t* finish,
	uint16_t hash_length, buffer* output);
static const Keccak Keccak_functions[] =
{
	&hash_algorithm_keccak, &hash_algorithm_sha3
};

TEST_F(TestHashAlgorithm, Keccak)
{
	static const uint16_t hash_length[] = { 224, 256, 384, 512, 224, 256, 384, 512 };
	//
	buffer input;
	SET_NULL_TO_BUFFER(input);
	ASSERT_TRUE(input_generator(&input, 31745)) << buffer_free(&input);
	auto ptr = buffer_data(&input, 0);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_TRUE(buffer_resize(&output, 2 * UINT8_MAX)) << buffer_free(&input) << buffer_free(&output);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const std::string input_str(the_node.select_node("input").node().child_value());
		auto input_in_a_range = string_to_range(input_str);
		const auto input_length =
			static_cast<uint16_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		ASSERT_LT(input_length, 31745) << buffer_free(&input) << buffer_free(&output);
		//
		const std::string return_str(the_node.select_node("return").node().child_value());
		input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string Keccak_224(the_node.select_node("Keccak_224").node().child_value());
		const std::string Keccak_256(the_node.select_node("Keccak_256").node().child_value());
		const std::string Keccak_384(the_node.select_node("Keccak_384").node().child_value());
		const std::string Keccak_512(the_node.select_node("Keccak_512").node().child_value());
		//
		const std::string SHA3_224(the_node.select_node("SHA3_224").node().child_value());
		const std::string SHA3_256(the_node.select_node("SHA3_256").node().child_value());
		const std::string SHA3_384(the_node.select_node("SHA3_384").node().child_value());
		const std::string SHA3_512(the_node.select_node("SHA3_512").node().child_value());
		//
		const std::string* expected_result[] =
		{
			&Keccak_224, &Keccak_256, &Keccak_384, &Keccak_512,
			&SHA3_224, &SHA3_256, &SHA3_384, &SHA3_512
		};

		for (uint8_t i = 0, count = COUNT_OF(expected_result); i < count; ++i)
		{
			if (expected_result[i]->empty())
			{
				continue;
			}

			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&input) << buffer_free(&output);
			//
			auto returned = (Keccak_functions[3 < i])(ptr, ptr + input_length, hash_length[i], &output);
			//
			ASSERT_EQ(expected_return, returned) << input_length << std::endl <<
												 static_cast<int>(i) << buffer_free(&input) << buffer_free(&output);
			//
			returned = static_cast<uint8_t>(hash_length[i] / 8);
			ASSERT_TRUE(buffer_append(&output, nullptr, 2 * returned + 2)) << buffer_free(&input) << buffer_free(&output);
			ASSERT_TRUE(buffer_resize(&output, returned)) << buffer_free(&input) << buffer_free(&output);
			//
			input_in_a_range = buffer_to_range(&output);
			returned = hash_algorithm_bytes_to_string(input_in_a_range.start, input_in_a_range.finish, &output);
			ASSERT_EQ(expected_return, returned) << input_length << std::endl <<
												 static_cast<int>(i) << buffer_free(&input) << buffer_free(&output);
			//
			const auto result(buffer_to_string(&output).substr(range_size(&input_in_a_range)));
			ASSERT_EQ(*(expected_result[i]), result) << input_length << std::endl <<
					static_cast<int>(i) << buffer_free(&input) << buffer_free(&output);
		}

		ASSERT_NE(0, buffer_size(&output)) << buffer_free(&input) << buffer_free(&output);
		--node_count;
	}

	buffer_release(&output);
	buffer_release(&input);
}

TEST_F(TestHashAlgorithm, sha3)
{
	static const uint16_t hash_lengths[] =
	{
		224, 256, 384, 512,
		224, 256, 384, 512
	};
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const auto input(get_data_from_nodes(the_node, "input"));
		const std::string return_str(the_node.select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string Keccak_224(the_node.select_node("Keccak_224").node().child_value());
		const std::string Keccak_256(the_node.select_node("Keccak_256").node().child_value());
		const std::string Keccak_384(the_node.select_node("Keccak_384").node().child_value());
		const std::string Keccak_512(the_node.select_node("Keccak_512").node().child_value());
		//
		const std::string SHA3_224(the_node.select_node("SHA3_224").node().child_value());
		const std::string SHA3_256(the_node.select_node("SHA3_256").node().child_value());
		const std::string SHA3_384(the_node.select_node("SHA3_384").node().child_value());
		const std::string SHA3_512(the_node.select_node("SHA3_512").node().child_value());
		//
		const std::string* expected_result[] =
		{
			&Keccak_224, &Keccak_256, &Keccak_384, &Keccak_512,
			&SHA3_224, &SHA3_256, &SHA3_384, &SHA3_512
		};

		for (uint8_t i = 0, count = COUNT_OF(expected_result); i < count; ++i)
		{
			if (expected_result[i]->empty())
			{
				continue;
			}

			ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
			//
			input_in_a_range = string_to_range(input);
			null_range_to_empty(input_in_a_range);
			//
			auto returned = (Keccak_functions[i < (count / 2) ? 0 : 1])(input_in_a_range.start, input_in_a_range.finish,
							hash_lengths[i], &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl <<
												 static_cast<int>(i) << buffer_free(&output);
			//
			returned = static_cast<uint8_t>(buffer_size(&output));
			ASSERT_TRUE(buffer_append(&output, nullptr, 2 * returned + 2)) << buffer_free(&output);
			ASSERT_TRUE(buffer_resize(&output, returned)) << buffer_free(&output);
			//
			input_in_a_range = buffer_to_range(&output);
			returned = hash_algorithm_bytes_to_string(input_in_a_range.start, input_in_a_range.finish, &output);
			ASSERT_EQ(expected_return, returned) << input << std::endl <<
												 static_cast<int>(i) << buffer_free(&output);
			//
			const auto result(buffer_to_string(&output).substr(range_size(&input_in_a_range)));
			ASSERT_EQ(*(expected_result[i]), result) << input << std::endl <<
					static_cast<int>(i) << buffer_free(&output);
		}

		ASSERT_NE(0, buffer_size(&output)) << buffer_free(&output);
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}

TEST_F(TestHashAlgorithm, xxHash)
{
	static const std::string algorithms[] =
	{ "XXH32", "XXH64" };
	std::string input(256, '\0');
	uint32_t base = 2654435761;

	for (size_t i = 0, count = input.size(); i < count; ++i)
	{
		input[i] = static_cast<uint32_t>(base >> 24);
		base *= base;
	}

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const auto length = the_node.attribute("length").as_ullong();
		ASSERT_LT(length, input.size()) << std::to_string(node_count);
		//
		const std::string algorithm(the_node.attribute("algorithm").as_string());
		const auto seed = the_node.attribute("seed").as_ullong();
		const auto expected_return = the_node.attribute("return").as_ullong();
		//
		auto input_in_a_range = string_to_range(input);
		input_in_a_range.finish = input_in_a_range.start + length;
		//
		uint64_t returned = 0;

		if (algorithms[0] == algorithm)
		{
			ASSERT_LE(length, UINT32_MAX) <<
										  std::to_string(node_count) << std::endl << algorithm << std::endl <<
										  seed << std::endl << length << std::endl << expected_return;
			ASSERT_LE(seed, UINT32_MAX) <<
										std::to_string(node_count) << std::endl << algorithm << std::endl <<
										seed << std::endl << length << std::endl << expected_return;
			//
			uint32_t result = 0;
			ASSERT_TRUE(hash_algorithm_XXH32(
							input_in_a_range.start,
							input_in_a_range.finish,
							static_cast<uint32_t>(seed),
							&result)) <<
									  std::to_string(node_count) << std::endl << algorithm << std::endl <<
									  seed << std::endl << length << std::endl << expected_return;
			returned = result;
		}
		else if (algorithms[1] == algorithm)
		{
			uint64_t result = 0;
			ASSERT_TRUE(hash_algorithm_XXH64(
							input_in_a_range.start,
							input_in_a_range.finish,
							seed,
							&result)) <<
									  std::to_string(node_count) << std::endl << algorithm << std::endl <<
									  seed << std::endl << length << std::endl << expected_return;
			returned = result;
		}

		ASSERT_EQ(expected_return, returned) <<
											 std::to_string(node_count) << std::endl <<
											 algorithm << std::endl <<
											 seed << std::endl << length;
		//
		--node_count;
	}
}

TEST_F(TestHashAlgorithm, file_get_checksum)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_TRUE(input_generator(&output, 31744)) << buffer_free(&output);
	ASSERT_TRUE(buffer_append(&output, nullptr, 3 * 4096)) << buffer_free(&output);
	ASSERT_TRUE(buffer_resize(&output, 31744)) << buffer_free(&output);
	ASSERT_TRUE(path_get_temp_file_name(&output)) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&output, 0)) << buffer_free(&output);
	const auto size = buffer_size(&output);
	//
	const auto input = buffer_data(&output, 0);
	const auto path = buffer_data(&output, 31744);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();

		for (const auto& algorithm_node : the_node)
		{
			const std::string algorithm(algorithm_node.name());
			auto algorithm_in_a_range(string_to_range(algorithm));
			//
			range algorithm_parameter;
			algorithm_parameter.start = algorithm_parameter.finish = nullptr;
			//
			const auto pos = algorithm.find('-');

			if (std::string::npos != pos)
			{
				algorithm_parameter.finish = algorithm_in_a_range.finish;
				algorithm_in_a_range.finish = algorithm_in_a_range.start + pos;
				algorithm_parameter.start = algorithm_in_a_range.start + pos + 1;
			}

			const auto input_length = static_cast<uint16_t>(algorithm_node.attribute("input").as_uint());
			const std::string expected_output(algorithm_node.child_value());
			//
			ASSERT_LT(input_length, 31745) << buffer_free(&output);
			//
			ASSERT_TRUE(echo(0, Default, path, Info, input, input_length, 0, 0)) << buffer_free(&output);
			//
			ASSERT_TRUE(buffer_resize(&output, size)) << buffer_free(&output);
			ASSERT_TRUE(file_get_checksum(path, &algorithm_in_a_range, &algorithm_parameter,
										  &output)) << buffer_free(&output);
			//
			const std::string output_str(reinterpret_cast<const char*>(buffer_data(&output, size)),
										 buffer_size(&output) - size);
			ASSERT_EQ(expected_output, output_str) << buffer_free(&output);
		}

		--node_count;
	}

	buffer_release(&output);
}
