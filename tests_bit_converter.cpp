/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "bit_converter.h"
#include "common.h"
}

#include <cstdint>

static const uint64_t inputs[] =
{
	0,
	INT8_MAX / 2 - 1,
	INT8_MAX / 2,
	INT8_MAX / 2 + 1,
	INT8_MAX - 1,
	INT8_MAX,
	INT8_MAX + 1,
	UINT8_MAX - 1,
	UINT8_MAX,
	UINT8_MAX + 1,
	INT16_MAX / 2 - 1,
	INT16_MAX / 2,
	INT16_MAX / 2 + 1,
	INT16_MAX - 1,
	INT16_MAX,
	INT16_MAX + 1,
	UINT16_MAX - 1,
	UINT16_MAX,
	UINT16_MAX + 1,
	INT32_MAX / 2 - 1,
	INT32_MAX / 2,
	INT32_MAX / 2 + 1,
	INT32_MAX - 1,
	INT32_MAX,
	INT32_MAX + static_cast<uint64_t>(1),
	UINT32_MAX - static_cast<uint64_t>(1),
	UINT32_MAX,
	UINT32_MAX + static_cast<uint64_t>(1),
	INT64_MAX / static_cast<uint64_t>(2) - static_cast<uint64_t>(1),
	INT64_MAX / static_cast<uint64_t>(2),
	INT64_MAX / static_cast<uint64_t>(2) + static_cast<uint64_t>(1),
	INT64_MAX - static_cast<uint64_t>(1),
	INT64_MAX,
	INT64_MAX + static_cast<uint64_t>(1),
	UINT64_MAX - static_cast<uint64_t>(1),
	UINT64_MAX
};

TEST(TestBitConverter_, bit_converter_is_little_endian)
{
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	ASSERT_FALSE(bit_converter_is_little_endian());
#else
	ASSERT_TRUE(bit_converter_is_little_endian());
#endif
}

TEST(TestBitConverter_, bit_converter_from_and_to_uint16_t)
{
	uint8_t output[2];
	const uint8_t* finish = output + COUNT_OF(output);

	for (const uint64_t input : inputs)
	{
		if (input <= UINT16_MAX)
		{
			ASSERT_TRUE(bit_converter_get_bytes_from_uint16_t(
							static_cast<uint16_t>(input), output));
			uint16_t out;
			ASSERT_TRUE(bit_converter_to_uint16_t(output, finish, &out));
			ASSERT_EQ(static_cast<uint16_t>(input), out);
		}
	}
}

TEST(TestBitConverter_, bit_converter_from_and_to_uint32_t)
{
	uint8_t output[4];
	const uint8_t* finish = output + COUNT_OF(output);

	for (const uint64_t input : inputs)
	{
		if (input <= UINT32_MAX)
		{
			ASSERT_TRUE(bit_converter_get_bytes_from_uint32_t(
							static_cast<uint32_t>(input), output));
			uint32_t out;
			ASSERT_TRUE(bit_converter_to_uint32_t(output, finish, &out));
			ASSERT_EQ(static_cast<uint32_t>(input), out);
		}
	}
}

TEST(TestBitConverter_, bit_converter_from_and_to_uint64_t)
{
	uint8_t output[8];
	const uint8_t* finish = output + COUNT_OF(output);

	for (const uint64_t input : inputs)
	{
		ASSERT_TRUE(bit_converter_get_bytes_from_uint64_t(
						input, output));
		uint64_t out;
		ASSERT_TRUE(bit_converter_to_uint64_t(output, finish, &out));
		ASSERT_EQ(input, out);
	}
}
