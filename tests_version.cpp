/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "version.h"
}

#include <string>

class TestVersion : public TestsBaseXml
{
};

TEST_F(TestVersion, version_init)
{
	uint8_t version[VERSION_SIZE];
	std::string output(INT8_MAX, 0);

	for (const auto& node : nodes)
	{
		const std::string major_str(node.node().select_node("major").node().child_value());
		const std::string minor_str(node.node().select_node("minor").node().child_value());
		const std::string build_str(node.node().select_node("build").node().child_value());
		const std::string revision_str(node.node().select_node("revision").node().child_value());
		//
		const auto major_in_range(string_to_range(major_str));
		const auto minor_in_range(string_to_range(minor_str));
		const auto build_in_range(string_to_range(build_str));
		const auto revision_in_range(string_to_range(revision_str));
		//
		const auto major = static_cast<uint32_t>(uint64_parse(major_in_range.start, major_in_range.finish));
		const auto minor = static_cast<uint32_t>(uint64_parse(minor_in_range.start, minor_in_range.finish));
		const auto build = static_cast<uint32_t>(uint64_parse(build_in_range.start, build_in_range.finish));
		const auto revision = static_cast<uint32_t>(uint64_parse(revision_in_range.start, revision_in_range.finish));
		//
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		ASSERT_TRUE(version_init(version, VERSION_SIZE, major, minor, build, revision)) << node_count;
		//
		ASSERT_EQ(major, version_get_major(version)) << node_count;
		ASSERT_EQ(minor, version_get_minor(version)) << node_count;
		ASSERT_EQ(build, version_get_build(version)) << node_count;
		ASSERT_EQ(revision, version_get_revision(version)) << node_count;
		//
		output.clear();
		output.resize(INT8_MAX, 0);
		//
		const auto length = version_to_byte_array(version, reinterpret_cast<uint8_t*>(&output[0]));
		ASSERT_LT(0, length) << node_count;
		ASSERT_LT(length, 44) << node_count;
		output.resize(length);
		//
		ASSERT_EQ(expected_output, output) << node_count;
		//
		--node_count;
	}
}

TEST_F(TestVersion, version_parse)
{
	uint8_t version[VERSION_SIZE];

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const auto input_in_range(string_to_range(input));
		//
		const std::string major_str(node.node().select_node("major").node().child_value());
		const std::string minor_str(node.node().select_node("minor").node().child_value());
		const std::string build_str(node.node().select_node("build").node().child_value());
		const std::string revision_str(node.node().select_node("revision").node().child_value());
		//
		const auto major_in_range(string_to_range(major_str));
		const auto minor_in_range(string_to_range(minor_str));
		const auto build_in_range(string_to_range(build_str));
		const auto revision_in_range(string_to_range(revision_str));
		//
		const auto major = static_cast<uint32_t>(uint64_parse(major_in_range.start, major_in_range.finish));
		const auto minor = static_cast<uint32_t>(uint64_parse(minor_in_range.start, minor_in_range.finish));
		const auto build = static_cast<uint32_t>(uint64_parse(build_in_range.start, build_in_range.finish));
		const auto revision = static_cast<uint32_t>(uint64_parse(revision_in_range.start, revision_in_range.finish));
		//
		ASSERT_TRUE(
			version_parse(input_in_range.start, input_in_range.finish, version))
				<< node_count;
		//
		ASSERT_EQ(major, version_get_major(version)) << node_count;
		ASSERT_EQ(minor, version_get_minor(version)) << node_count;
		ASSERT_EQ(build, version_get_build(version)) << node_count;
		ASSERT_EQ(revision, version_get_revision(version)) << node_count;
		//
		--node_count;
	}
}

TEST_F(TestVersion, version_less)
{
	uint8_t version_a[VERSION_SIZE];
	uint8_t version_b[VERSION_SIZE];

	for (const auto& node : nodes)
	{
		const std::string version_a_str(node.node().select_node("version_a").node().child_value());
		const std::string version_b_str(node.node().select_node("version_b").node().child_value());
		//
		auto input_in_range = string_to_range(version_a_str);
		ASSERT_TRUE(
			version_parse(input_in_range.start, input_in_range.finish, version_a))
				<< node_count;
		//
		input_in_range = string_to_range(version_b_str);
		ASSERT_TRUE(
			version_parse(input_in_range.start, input_in_range.finish, version_b))
				<< node_count;
		//
		const auto expected_return =
			static_cast<uint8_t>(INT_PARSE(node.node().select_node("output").node().child_value()));
		ASSERT_EQ(expected_return, version_less(version_a, version_b))
				<< version_a_str << " " << version_b_str << " " << node_count;
		//
		--node_count;
	}
}

TEST_F(TestVersion, version_greater)
{
	uint8_t version_a[VERSION_SIZE];
	uint8_t version_b[VERSION_SIZE];

	for (const auto& node : nodes)
	{
		const std::string version_a_str(node.node().select_node("version_a").node().child_value());
		const std::string version_b_str(node.node().select_node("version_b").node().child_value());
		//
		auto input_in_range = string_to_range(version_a_str);
		ASSERT_TRUE(
			version_parse(input_in_range.start, input_in_range.finish, version_a))
				<< node_count;
		//
		input_in_range = string_to_range(version_b_str);
		ASSERT_TRUE(
			version_parse(input_in_range.start, input_in_range.finish, version_b))
				<< node_count;
		//
		const auto expected_return =
			static_cast<uint8_t>(INT_PARSE(node.node().select_node("output").node().child_value()));
		ASSERT_EQ(expected_return, version_greater(version_a, version_b))
				<< version_a_str << " " << version_b_str << " " << node_count;
		//
		--node_count;
	}
}

TEST(TestVersion_, version_to_string)
{
	uint8_t version[VERSION_SIZE];
	ASSERT_TRUE(version_init(version, VERSION_SIZE, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX));
	buffer output;
	SET_NULL_TO_BUFFER(output);
	ASSERT_TRUE(version_to_string(version, &output)) << buffer_free(&output);
	const auto returned(buffer_to_string(&output));
	buffer_release(&output);
	static const std::string expected_return("4294967295.4294967295.4294967295.4294967295");
	ASSERT_EQ(expected_return, returned);
}
