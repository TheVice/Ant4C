/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "conversion.h"
#include "file_system.h"
};

#include <string>
#include <cstdint>

class TestFileSystem : public TestsBaseXml
{
};

TEST_F(TestFileSystem, directory_exists)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		uint8_t condition = 0;
		ASSERT_TRUE(is_this_node_pass_by_if_condition(node, &tmp, &condition)) << buffer_free(&tmp);

		if (!condition)
		{
			--node_count;
			continue;
		}

		const std::string input(node.node().select_node("input").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const uint8_t returned = directory_exists(input.c_str());
		ASSERT_EQ(expected_return, returned) << input;
		//
		--node_count;
	}

	buffer_release(&tmp);
}
//directory_get_current_directory
//directory_get_parent_directory

TEST_F(TestFileSystem, file_exists)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		uint8_t condition = 0;
		ASSERT_TRUE(is_this_node_pass_by_if_condition(node, &tmp, &condition)) << buffer_free(&tmp);

		if (!condition)
		{
			--node_count;
			continue;
		}

		const std::string input(node.node().select_node("input").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const uint8_t returned = file_exists(input.c_str());
		ASSERT_EQ(expected_return, returned) << input;
		//
		--node_count;
	}

	buffer_release(&tmp);
}
//file_get_length
