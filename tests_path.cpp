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
#include "file_system.h"
#include "interpreter.h"
#include "path.h"
#include "range.h"
#include "string_unit.h"
};

#include <list>
#include <string>
#include <cstring>

class TestPath : public TestsBaseXml
{
};

TEST_F(TestPath, path_change_extension)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string ext(node.node().select_node("ext").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		const auto ext_in_a_range(string_to_range(ext));
		//
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		//
		const auto returned = path_change_extension(input_in_a_range.start, input_in_a_range.finish,
							  ext_in_a_range.start, ext_in_a_range.finish, &path);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&path);
		ASSERT_STREQ(expected_output.c_str(), buffer_to_string(&path).c_str()) << buffer_free(&path);
		//
		--node_count;
	}

	buffer_release(&path);
}

TEST_F(TestPath, path_combine)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);

	for (const auto& node : nodes)
	{
		const std::string path1(node.node().select_node("path1").node().child_value());
		const std::string path2(node.node().select_node("path2").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto path1_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(path1_in_a_range.start, path1_in_a_range.finish));
		//
		path1_in_a_range = string_to_range(path1);
		const auto path2_in_a_range(string_to_range(path2));
		//
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		const auto returned = path_combine(path1_in_a_range.start, path1_in_a_range.finish,
										   path2_in_a_range.start, path2_in_a_range.finish, &path);
		//
		ASSERT_EQ(expected_return, returned) << path1 << std::endl << path2 << buffer_free(&path);
		ASSERT_EQ(expected_output, buffer_to_string(&path)) << buffer_free(&path);
		//
		--node_count;
	}

	buffer_release(&path);
}

TEST_F(TestPath, path_get_directory_name)
{
	for (const auto& node : nodes)
	{
		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_directory(node.node().select_node("directory").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto path_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(path_in_a_range.start, path_in_a_range.finish));
		//
		path_in_a_range = string_to_range(path);
		//
		const auto returned = path_get_directory_name(path_in_a_range.start, &path_in_a_range.finish);
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_directory, range_to_string(path_in_a_range));
		//
		--node_count;
	}
}

TEST_F(TestPath, path_get_extension)
{
	for (const auto& node : nodes)
	{
		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_ext(node.node().select_node("ext").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto path_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(path_in_a_range.start, path_in_a_range.finish));
		//
		path_in_a_range = string_to_range(path);
		//
		const auto returned = path_get_extension(&path_in_a_range.start, path_in_a_range.finish);
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_ext, range_to_string(path_in_a_range));
		//
		--node_count;
	}
}

TEST_F(TestPath, path_get_file_name)
{
	for (const auto& node : nodes)
	{
		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_file_name(node.node().select_node("file_name").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto path_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(path_in_a_range.start, path_in_a_range.finish));
		//
		path_in_a_range = string_to_range(path);
		//
		const auto returned = path_get_file_name(&path_in_a_range.start, path_in_a_range.finish);
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_file_name, range_to_string(path_in_a_range));
		//
		--node_count;
	}
}

TEST_F(TestPath, path_get_file_name_without_extension)
{
	for (const auto& node : nodes)
	{
		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_file_name(node.node().select_node("file_name").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto path_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(path_in_a_range.start, path_in_a_range.finish));
		//
		path_in_a_range = string_to_range(path);
		//
		range file_name;
		file_name.start = file_name.finish = nullptr;
		const auto returned = path_get_file_name_without_extension(
								  path_in_a_range.start, path_in_a_range.finish, &file_name);
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_file_name, range_to_string(file_name));
		//
		--node_count;
	}
}

TEST_F(TestPath, path_get_full_path)
{
	buffer full_path;
	SET_NULL_TO_BUFFER(full_path);

	for (const auto& node : nodes)
	{
		std::list<pugi::xpath_node> expected_full_path;
		ASSERT_TRUE(select_nodes_by_condition(
						nullptr, node.node().select_nodes("full_path"),
						expected_full_path, &full_path))
				<< buffer_free(&full_path);
		ASSERT_EQ(1ull, expected_full_path.size());
		//
		const std::string expected_full_path_str(expected_full_path.cbegin()->node().child_value());
		const std::string root_path(node.node().select_node("root_path").node().child_value());
		const std::string path(node.node().select_node("path").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto root_path_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(root_path_in_a_range.start, root_path_in_a_range.finish));
		//
		root_path_in_a_range = string_to_range(root_path);
		const auto path_in_a_range(string_to_range(path));
		//
		ASSERT_TRUE(buffer_resize(&full_path, 0)) << buffer_free(&full_path);
		const auto returned = path_get_full_path(root_path_in_a_range.start, root_path_in_a_range.finish,
							  path_in_a_range.start, path_in_a_range.finish, &full_path);
		ASSERT_EQ(expected_return, returned) << buffer_to_string(&full_path)
											 << path << std::endl
											 << root_path << std::endl
											 << std::endl << buffer_free(&full_path);

		if (returned)
		{
			ASSERT_EQ(expected_full_path_str, buffer_to_string(&full_path)) << buffer_free(&full_path);
		}

		--node_count;
	}

	buffer_release(&full_path);
}

TEST_F(TestPath, path_glob)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().attribute("input").as_string());
		const std::string wild_card(node.node().attribute("wild_card").as_string());
		const uint8_t expected_return = node.node().attribute("result").as_bool();
		//
		const auto input_in_a_range = string_to_range(input);
		const auto wild_card_in_a_range = string_to_range(wild_card);
		const auto returned = path_glob(input_in_a_range.start, input_in_a_range.finish,
										wild_card_in_a_range.start, wild_card_in_a_range.finish);
		//
		ASSERT_EQ(expected_return, returned) << input << std::endl << wild_card << std::endl;
		//
		--node_count;
	}
}

TEST(TestPath_, path_delimiter)
{
	ASSERT_NE(path_posix_delimiter, path_windows_delimiter);
#if defined(_WIN32)
	ASSERT_EQ(PATH_DELIMITER, path_windows_delimiter);
#else
	ASSERT_EQ(PATH_DELIMITER, path_posix_delimiter);
#endif
}

TEST_F(TestPath, path_get_path_root)
{
	for (const auto& node : nodes)
	{
		const std::string return_str(
			node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string input(node.node().select_node("input").node().child_value());
		input_in_a_range = string_to_range(input);
		//
		const std::string expected_root(node.node().select_node("root").node().child_value());
		//
		const auto returned =
			path_get_path_root(input_in_a_range.start, &input_in_a_range.finish);
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_root, range_to_string(input_in_a_range));
		//
		--node_count;
	}
}

TEST(TestPath_, path_get_temp_file_name)
{
	buffer temp_file_name;
	SET_NULL_TO_BUFFER(temp_file_name);
	//
	ASSERT_TRUE(path_get_temp_file_name(&temp_file_name)) << buffer_free(&temp_file_name);
	const auto temp_file_name_str(buffer_to_string(&temp_file_name));
	const auto temp_file_name_range(string_to_range(temp_file_name_str));
	//
	char chars[4];
	chars[0] = '.';
	chars[1] = path_posix_delimiter;
	chars[2] = path_windows_delimiter;
	chars[3] = '\0';

	for (const auto& ch : chars)
	{
		ASSERT_NE(ch, *(temp_file_name_range.finish - 1)) <<
				temp_file_name_range.start << std::endl << ch << std::endl;
	}

	uint8_t is_path_rooted;
	//
	ASSERT_TRUE(
		path_is_path_rooted(
			temp_file_name_range.start,
			temp_file_name_range.finish,
			&is_path_rooted)) << buffer_free(&temp_file_name);
	//
	ASSERT_TRUE(is_path_rooted) << buffer_free(&temp_file_name);
	//
	buffer_release(&temp_file_name);
	//
	ASSERT_FALSE(path_get_temp_file_name(nullptr));
}

TEST(TestPath_, path_get_temp_path)
{
	buffer temp_path;
	SET_NULL_TO_BUFFER(temp_path);
	//
	ASSERT_TRUE(path_get_temp_path(&temp_path)) << buffer_free(&temp_path);
	const auto size = buffer_size(&temp_path);
	ASSERT_TRUE(path_get_temp_path(&temp_path)) << buffer_free(&temp_path);
	//
	const std::string temp_in_a_string(buffer_char_data(&temp_path, 0), size);
	const auto temp_in_a_range = string_to_range(temp_in_a_string);
	//
	buffer_release(&temp_path);
	//
	uint8_t is_path_rooted;
	//
	ASSERT_TRUE(path_is_path_rooted(temp_in_a_range.start, temp_in_a_range.finish,
									&is_path_rooted)) << temp_in_a_string;
	ASSERT_TRUE(is_path_rooted) << temp_in_a_string;
	ASSERT_NE(PATH_DELIMITER, *(temp_in_a_range.finish - 1)) << temp_in_a_string;
	//
	ASSERT_FALSE(path_get_temp_path(nullptr));
}

TEST_F(TestPath, path_has_extension)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		const auto returned =
			path_has_extension(input_in_a_range.start, input_in_a_range.finish);
		ASSERT_EQ(expected_return, returned);
		//
		--node_count;
	}

	ASSERT_FALSE(path_has_extension(nullptr, nullptr));
	ASSERT_FALSE(path_has_extension(nullptr, reinterpret_cast<const uint8_t*>(&node_count)));
	ASSERT_FALSE(path_has_extension(reinterpret_cast<const uint8_t*>(&node_count), nullptr));
}

TEST_F(TestPath, path_is_path_rooted)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		uint8_t is_path_rooted = 0;
		//
		ASSERT_NE(input.empty(),
				  path_is_path_rooted(input_in_a_range.start, input_in_a_range.finish, &is_path_rooted));
		ASSERT_EQ(expected_return, is_path_rooted);
		//
		--node_count;
	}
}

TEST(TestPath_, path_exec_function_get_full_path)
{
	static const uint8_t verbose = 0;
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
	ASSERT_FALSE(path_exec_function(
					 nullptr, path_get_id_of_get_full_path_function(),
					 &output, 1, &output)) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	range function;
	function.start = reinterpret_cast<const uint8_t*>("path::get-full-path('.')");
	function.finish = function.start + 24;
	ASSERT_TRUE(interpreter_evaluate_function(
					nullptr, nullptr, &function, &output, verbose)) << buffer_free(&output);
	ASSERT_TRUE(buffer_size(&output)) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
	//
	function.start = reinterpret_cast<const uint8_t*>("path::get-full-path('abcdef')");
	function.finish = function.start + 29;
	ASSERT_TRUE(interpreter_evaluate_function(
					nullptr, nullptr, &function, &output, verbose)) << buffer_free(&output);
	ASSERT_TRUE(buffer_size(&output)) << buffer_free(&output);
	//
	buffer_release(&output);
}

TEST(TestPath_, path_get_directory_for_current_process)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
	ASSERT_TRUE(path_get_directory_for_current_process(&output)) << buffer_free(&output);
	const auto path_in_a_range = buffer_to_range(&output);
	uint8_t is_path_rooted;
	//
	ASSERT_TRUE(path_is_path_rooted(path_in_a_range.start, path_in_a_range.finish,
									&is_path_rooted)) << buffer_free(&output);
	ASSERT_TRUE(is_path_rooted) << buffer_free(&output);
	buffer_release(&output);
}

TEST(TestPath_, path_get_directory_for_current_image)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
#if !defined(__OpenBSD__)
	ASSERT_TRUE(path_get_directory_for_current_image(&output)) << buffer_free(&output);
	const auto path_in_a_range = buffer_to_range(&output);
	uint8_t is_path_rooted;
	//
	ASSERT_TRUE(path_is_path_rooted(path_in_a_range.start, path_in_a_range.finish,
									&is_path_rooted)) << buffer_free(&output);
	ASSERT_TRUE(is_path_rooted) << buffer_free(&output);
#else
	ASSERT_FALSE(path_get_directory_for_current_image(&output)) << buffer_free(&output);
#endif
	buffer_release(&output);
}

TEST_F(TestPath, cygpath_get_dos_path)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
#if defined(_WIN32)

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());

		if (!directory_exists(reinterpret_cast<const uint8_t*>(input.c_str())))
		{
			--node_count;
			continue;
		}

		const auto input_in_a_range(string_to_range(input));
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		ASSERT_TRUE(cygpath_get_dos_path(input_in_a_range.start, input_in_a_range.finish, &output))
				<< input << std::endl << buffer_free(&output);
		//
		EXPECT_EQ(expected_output, buffer_to_string(&output))
				<< input << std::endl << buffer_free(&output);
		//
		node_count = 0;
		break;
	}

	ASSERT_LT(0L, buffer_size(&output)) << buffer_free(&output);
#else
	const auto* path_start = reinterpret_cast<const uint8_t*>("/");
	ASSERT_FALSE(cygpath_get_dos_path(path_start, path_start + 1, &output)) << buffer_free(&output);//TODO:
	node_count = 0;
#endif
	buffer_release(&output);
}
/*cygpath_get_unix_path
cygpath_get_windows_path
cygpath_exec_function*/
