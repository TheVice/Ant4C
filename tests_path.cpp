/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
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

#include <cstring>
#include <string>

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
		const uint8_t expected_return = (uint8_t)INT_PARSE(
											node.node().select_node("return").node().child_value());
		//
		const range input_in_range = string_to_range(input);
		const range ext_in_range = string_to_range(ext);
		//
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		//
		const uint8_t returned = path_change_extension(input_in_range.start, input_in_range.finish,
								 ext_in_range.start, ext_in_range.finish, &path);
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
		const uint8_t expected_return = (uint8_t)INT_PARSE(
											node.node().select_node("return").node().child_value());
		//
		const range path1_in_range = string_to_range(path1);
		const range path2_in_range = string_to_range(path2);
		//
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		const uint8_t returned = path_combine(path1_in_range.start, path1_in_range.finish,
											  path2_in_range.start, path2_in_range.finish, &path);
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
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const range path_in_range = string_to_range(path);
		//
		range directory;
		directory.start = directory.finish = NULL;
		const uint8_t returned = path_get_directory_name(path_in_range.start, path_in_range.finish, &directory);
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_directory, range_to_string(directory));
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
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const range path_in_range = string_to_range(path);
		//
		range ext;
		ext.start = ext.finish = NULL;
		const uint8_t returned = path_get_extension(path_in_range.start, path_in_range.finish, &ext);
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_ext, range_to_string(ext));
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
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const range path_in_range = string_to_range(path);
		//
		range file_name;
		file_name.start = file_name.finish = NULL;
		const uint8_t returned = path_get_file_name(path_in_range.start, path_in_range.finish, &file_name);
		ASSERT_EQ(expected_return, returned);
		ASSERT_EQ(expected_file_name, range_to_string(file_name));
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
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const range path_in_range = string_to_range(path);
		//
		range file_name;
		file_name.start = file_name.finish = NULL;
		const uint8_t returned = path_get_file_name_without_extension(path_in_range.start, path_in_range.finish,
								 &file_name);
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
		ASSERT_TRUE(select_nodes_by_condition(node.node().select_nodes("full_path"), expected_full_path, &full_path))
				<< buffer_free(&full_path);
		ASSERT_EQ(1ull, expected_full_path.size());
		//
		const std::string expected_full_path_str(expected_full_path.cbegin()->node().child_value());
		const std::string root_path(node.node().select_node("root_path").node().child_value());
		const std::string path(node.node().select_node("path").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		const range root_path_in_range(string_to_range(root_path));
		const range path_in_range(string_to_range(path));
		//
		ASSERT_TRUE(buffer_resize(&full_path, 0)) << buffer_free(&full_path);
		const uint8_t returned = path_get_full_path(root_path_in_range.start, root_path_in_range.finish,
								 path_in_range.start, path_in_range.finish, &full_path);
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
		const auto input_in_range = string_to_range(input);
		const auto wild_card_in_range = string_to_range(wild_card);
		const auto returned = path_glob(input_in_range.start, input_in_range.finish,
										wild_card_in_range.start, wild_card_in_range.finish);
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

TEST(TestPath_, path_get_path_root)
{
#if defined(_WIN32)
	const std::string input[] = { "C:", "C:\\", "C:/", "C:\\Windows", "C:/Windows", "Windows" };
	const char* expected_root[] = { "C:", "C:\\", "C:/", "C:\\", "C:/", NULL };
	const uint8_t expected_return[] = { 1, 1, 1, 1, 1, 0 };
#else
	const std::string input[] = { "\\", "/", "/tmp", "\tmp" };
	const char* expected_root[] = { NULL, "/", "/", NULL };
	const uint8_t expected_return[] = { 0, 1, 1, 0 };
#endif

	for (uint8_t i = 0, count = sizeof(input) / sizeof(*input); i < count; ++i)
	{
		const range input_in_range = string_to_range(input[i]);
		range root;
		root.start = root.finish = NULL;
		const uint8_t returned = path_get_path_root(input_in_range.start, input_in_range.finish, &root);
		ASSERT_EQ(expected_return[i], returned);
		ASSERT_EQ(NULL != expected_root[i] ? std::string(expected_root[i]) : std::string(), range_to_string(root));
	}
}

TEST(TestPath_, path_get_temp_file_name)
{
	buffer temp_file_name;
	SET_NULL_TO_BUFFER(temp_file_name);
	//
	ASSERT_TRUE(path_get_temp_file_name(&temp_file_name)) << buffer_free(&temp_file_name);
	const std::string temp_file_name_str(buffer_to_string(&temp_file_name));
	const range temp_file_name_range = string_to_range(temp_file_name_str);
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

	ASSERT_TRUE(path_is_path_rooted(temp_file_name_range.start,
									temp_file_name_range.finish)) << buffer_free(&temp_file_name);
	buffer_release(&temp_file_name);
	//
	ASSERT_FALSE(path_get_temp_file_name(NULL));
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
	const std::string temp_in_string(buffer_char_data(&temp_path, 0), size);
	const auto temp_in_range = string_to_range(temp_in_string);
	//
	buffer_release(&temp_path);
	//
	ASSERT_TRUE(path_is_path_rooted(temp_in_range.start, temp_in_range.finish)) << temp_in_string;
	ASSERT_NE(PATH_DELIMITER, *(temp_in_range.finish - 1)) << temp_in_string;
	//
	ASSERT_FALSE(path_get_temp_path(NULL));
}

TEST(TestPath_, path_has_extension)
{
	const uint8_t* input[] =
	{
		(const uint8_t*)"a.txt",
		(const uint8_t*)"b.exe",
		(const uint8_t*)"c"
	};
	const uint8_t expected_return[] = { 1, 1, 0 };

	for (uint8_t i = 0, count = sizeof(input) / sizeof(*input); i < count; ++i)
	{
		const uint8_t returned = path_has_extension(input[i], input[i] + common_count_bytes_until(input[i], 0));
		ASSERT_EQ(expected_return[i], returned);
	}

	ASSERT_FALSE(path_has_extension(NULL, NULL));
	ASSERT_FALSE(path_has_extension(NULL, input[0]));
	ASSERT_FALSE(path_has_extension(input[0], NULL));
}

TEST(TestPath_, path_is_path_rooted)
{
#if defined(_WIN32)
	const uint8_t* input[] =
	{
		(const uint8_t*)"C:",
		(const uint8_t*)"C:\\",
		(const uint8_t*)"C:/",
		(const uint8_t*)"C:\\Windows",
		(const uint8_t*)"C:/Windows",
		(const uint8_t*)"Windows"
	};
	const uint8_t expect_return[] = { 1, 1, 1, 1, 1, 0 };
#else
	const uint8_t* input[] =
	{
		(const uint8_t*)"\\",
		(const uint8_t*)"/",
		(const uint8_t*)"/tmp",
		(const uint8_t*)"\tmp"
	};
	const uint8_t expect_return[] = { 0, 1, 1, 0 };
#endif

	for (uint8_t i = 0, count = sizeof(input) / sizeof(*input); i < count; ++i)
	{
		const uint8_t returned = path_is_path_rooted(input[i], input[i] + common_count_bytes_until(input[i], 0));
		ASSERT_EQ(expect_return[i], returned);
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
	ASSERT_FALSE(path_exec_function(NULL, path_get_id_of_get_full_path_function(), &output, 1,
									&output)) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output)) << buffer_free(&output);
	//
	range function;
	function.start = (const uint8_t*)"path::get-full-path('.')";
	function.finish = function.start + 24;
	ASSERT_TRUE(interpreter_evaluate_function(NULL, NULL, &function, &output, verbose)) << buffer_free(&output);
	ASSERT_TRUE(buffer_size(&output)) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
	//
	function.start = (const uint8_t*)"path::get-full-path('abcdef')";
	function.finish = function.start + 29;
	ASSERT_TRUE(interpreter_evaluate_function(NULL, NULL, &function, &output, verbose)) << buffer_free(&output);
	ASSERT_TRUE(buffer_size(&output)) << buffer_free(&output);
	//
	buffer_release(&output);
}

TEST(TestPath_, path_get_directory_for_current_process)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
	ASSERT_TRUE(path_get_directory_for_current_process(&output)) << buffer_free(&output);
	const auto path_in_range = buffer_to_range(&output);
	ASSERT_TRUE(path_is_path_rooted(path_in_range.start, path_in_range.finish)) << buffer_free(&output);
	buffer_release(&output);
}

TEST(TestPath_, path_get_directory_for_current_image)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);
#if !defined(__OpenBSD__)
	ASSERT_TRUE(path_get_directory_for_current_image(&output)) << buffer_free(&output);
	const auto path_in_range = buffer_to_range(&output);
	ASSERT_TRUE(path_is_path_rooted(path_in_range.start, path_in_range.finish)) << buffer_free(&output);
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

		if (!directory_exists(reinterpret_cast<const uint8_t*>(input.data())))
		{
			--node_count;
			continue;
		}

		const auto input_in_range(string_to_range(input));
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		ASSERT_TRUE(cygpath_get_dos_path(input_in_range.start, input_in_range.finish, &output))
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
	const uint8_t* path_start = (const uint8_t*)"/";
	ASSERT_FALSE(cygpath_get_dos_path(path_start, path_start + 1, &output)) << buffer_free(&output);//TODO:
	node_count = 0;
#endif
	buffer_release(&output);
}
/*cygpath_get_unix_path
cygpath_get_windows_path
cygpath_exec_function*/
