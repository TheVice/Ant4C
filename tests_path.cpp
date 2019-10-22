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
		const uint8_t expected_return = (uint8_t)int_parse(
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
		uint8_t condition = 0;
		ASSERT_TRUE(is_this_node_pass_by_if_condition(node, &path, &condition)) << buffer_free(&path);

		if (!condition)
		{
			--node_count;
			continue;
		}

		const std::string path1(node.node().select_node("path1").node().child_value());
		const std::string path2(node.node().select_node("path2").node().child_value());
		const std::string expected_output(node.node().select_node("output").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const range path1_in_range = string_to_range(path1);
		const range path2_in_range = string_to_range(path2);
		//
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		const uint8_t returned = path_combine(path1_in_range.start, path1_in_range.finish,
											  path2_in_range.start, path2_in_range.finish, &path);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&path);
		ASSERT_EQ(expected_output, buffer_to_string(&path)) << buffer_free(&path);
		//
		--node_count;
	}

	buffer_release(&path);
}

TEST_F(TestPath, path_get_directory_name)
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

		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_directory(node.node().select_node("directory").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		//
		const range path_in_range = string_to_range(path);
		//
		range directory;
		directory.start = directory.finish = NULL;
		const uint8_t returned = path_get_directory_name(path_in_range.start, path_in_range.finish, &directory);
		ASSERT_EQ(expected_return, returned) << buffer_free(&tmp);
		ASSERT_EQ(expected_directory, range_to_string(directory)) << buffer_free(&tmp);
		//
		--node_count;
	}

	buffer_release(&tmp);
}

TEST_F(TestPath, path_get_extension)
{
	for (const auto& node : nodes)
	{
		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_ext(node.node().select_node("ext").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
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

		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_file_name(node.node().select_node("file_name").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		//
		const range path_in_range = string_to_range(path);
		//
		range file_name;
		file_name.start = file_name.finish = NULL;
		const uint8_t returned = path_get_file_name(path_in_range.start, path_in_range.finish, &file_name);
		ASSERT_EQ(expected_return, returned) << buffer_free(&tmp);
		ASSERT_EQ(expected_file_name, range_to_string(file_name)) << buffer_free(&tmp);
		//
		--node_count;
	}

	buffer_release(&tmp);
}

TEST_F(TestPath, path_get_file_name_without_extension)
{
	for (const auto& node : nodes)
	{
		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_file_name(node.node().select_node("file_name").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
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
		uint8_t condition = 0;
		ASSERT_TRUE(is_this_node_pass_by_if_condition(node, &full_path, &condition)) << buffer_free(&full_path);

		if (!condition)
		{
			--node_count;
			continue;
		}

		const std::string root_path(node.node().select_node("root_path").node().child_value());
		const std::string path(node.node().select_node("path").node().child_value());
		const std::string expected_full_path(node.node().select_node("full_path").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		//
		const range root_path_in_range(string_to_range(root_path));
		const range path_in_range(string_to_range(path));
		//
		ASSERT_TRUE(buffer_resize(&full_path, 0)) << buffer_free(&full_path);
		const uint8_t returned = path_get_full_path(root_path_in_range.start, root_path_in_range.finish,
								 path_in_range.start, path_in_range.finish, &full_path);
		ASSERT_EQ(expected_return, returned) << buffer_to_string(&full_path) << std::endl << buffer_free(&full_path);

		if (returned)
		{
			ASSERT_EQ(expected_full_path, buffer_to_string(&full_path)) << buffer_free(&full_path);
		}

		--node_count;
	}

	buffer_release(&full_path);
}

TEST(TestPath_, path_delimiter)
{
	ASSERT_NE(path_posix_delimiter(), path_windows_delimiter());
#if defined(_WIN32)
	ASSERT_EQ(path_delimiter(), path_windows_delimiter());
#else
	ASSERT_EQ(path_delimiter(), path_posix_delimiter());
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
	char chars[3];
	chars[0] = '.';
	chars[1] = path_posix_delimiter();
	chars[2] = path_windows_delimiter();

	for (const char& ch : chars)
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
	const std::string temp_in_string(buffer_to_string(&temp_path));
	const range temp_in_range = string_to_range(temp_in_string);
	buffer_release(&temp_path);
	//
	ASSERT_TRUE(path_is_path_rooted(temp_in_range.start, temp_in_range.finish));
	static const char del = path_delimiter();
	ASSERT_FALSE(string_ends_with(temp_in_range.start, temp_in_range.finish, &del, &del + 1));
	//
	ASSERT_FALSE(path_get_temp_path(NULL));
}

TEST(TestPath_, path_has_extension)
{
	const char* input[] = { "a.txt", "b.exe", "c" };
	const uint8_t expected_return[] = { 1, 1, 0 };

	for (uint8_t i = 0, count = sizeof(input) / sizeof(*input); i < count; ++i)
	{
		const uint8_t returned = path_has_extension(input[i], input[i] + strlen(input[i]));
		ASSERT_EQ(expected_return[i], returned);
	}

	ASSERT_FALSE(path_has_extension(NULL, NULL));
	ASSERT_FALSE(path_has_extension(NULL, input[0]));
	ASSERT_FALSE(path_has_extension(input[0], NULL));
}

TEST(TestPath_, path_is_path_rooted)
{
#if defined(_WIN32)
	const char* input[] = { "C:", "C:\\", "C:/", "C:\\Windows", "C:/Windows", "Windows" };
	const uint8_t expect_return[] = { 1, 1, 1, 1, 1, 0 };
#else
	const char* input[] = { "\\", "/", "/tmp", "\tmp" };
	const uint8_t expect_return[] = { 0, 1, 1, 0 };
#endif

	for (uint8_t i = 0, count = sizeof(input) / sizeof(*input); i < count; ++i)
	{
		const uint8_t returned = path_is_path_rooted(input[i], input[i] + strlen(input[i]));
		ASSERT_EQ(expect_return[i], returned);
	}
}

TEST(TestPath_, path_exec_function_get_full_path)
{
	/*get_full_path path::get-full-path('')*/
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
	ASSERT_TRUE(path_exec_function(NULL, 6,
								   &output, 1, &output)) << buffer_free(&output);
	const ptrdiff_t size = buffer_size(&output);
	ASSERT_TRUE(size) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
	buffer arguments;
	SET_NULL_TO_BUFFER(arguments);
	//
	const char* folder_name = "abcdef";
	const ptrdiff_t folder_name_length = 6;
	//
	ASSERT_TRUE(buffer_append_char(&arguments, folder_name,
								   folder_name_length)) << buffer_free(&arguments) << buffer_free(&output);
	ASSERT_TRUE(buffer_append_buffer(&output, &arguments, 1)) << buffer_free(&arguments) << buffer_free(&output);
	//
	const ptrdiff_t size_ = buffer_size(&output);
	ASSERT_TRUE(buffer_append(&output, NULL,
							  size_ + 4 + folder_name_length + size)) << buffer_free(&arguments) << buffer_free(&output);
	ASSERT_TRUE(buffer_resize(&output, size_)) << buffer_free(&arguments) << buffer_free(&output);
	//
	ASSERT_TRUE(path_exec_function(NULL, 6, &output, 1,
								   &output)) << buffer_free(&arguments) << buffer_free(&output);
	ASSERT_TRUE(buffer_size(&output)) << buffer_free(&arguments) << buffer_free(&output);
	//
	const char* path = (const char*)buffer_data(&output, size_);
	const ptrdiff_t path_length = (buffer_size(&output) - size_);
	ASSERT_TRUE(string_ends_with(path, path + path_length,
								 folder_name, folder_name + folder_name_length)) << buffer_free(&arguments) << buffer_free(&output);
	//
	buffer_release(&arguments);
	buffer_release(&output);
	//
	ASSERT_EQ(size + 1 + folder_name_length, path_length);
}

/*
path_get_directory_for_current_process
path_get_directory_for_current_image
cygpath_get_dos_path
cygpath_get_unix_path
cygpath_get_windows_path
path_exec_function
cygpath_exec_function
*/