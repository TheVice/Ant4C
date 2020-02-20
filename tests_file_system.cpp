/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "echo.h"
#include "file_system.h"
#include "hash.h"
#include "path.h"
#include "project.h"
#include "text_encoding.h"
};

#include <cstdio>
#include <string>
#include <cstdint>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

class TestFileSystem : public TestsBaseXml
{
};

uint8_t get_crc32_of(const int64_t input, buffer* output)
{
	if (NULL == output)
	{
		return 0;
	}

	const auto size = buffer_size(output);

	if (!buffer_append(output, NULL, 4 * sizeof(uint32_t)))
	{
		return 0;
	}

	auto ptr = (const uint8_t*)&input;
	auto out = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));

	if (!hash_algorithm_crc32(ptr, ptr + sizeof(int64_t), out, 1))
	{
		return 0;
	}

	ptr = (const uint8_t*)out;

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	return hash_algorithm_bytes_to_string(ptr, ptr + sizeof(uint32_t), output);
}

TEST(TestFileSystem_, directory_create_and_delete)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc(), &path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	const auto ptr = buffer_data(&path, 0);
	ASSERT_FALSE(directory_exists(ptr)) << ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(directory_create(ptr)) << ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(directory_exists(ptr)) << ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(directory_delete(ptr)) << ptr << std::endl << buffer_free(&path);
	ASSERT_FALSE(directory_exists(ptr)) << ptr << std::endl << buffer_free(&path);
	//
	buffer_release(&path);
}

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
		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		//
		auto returned = directory_exists((const uint8_t*)input.c_str());
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&tmp);
#if defined(_WIN32)
		const auto input_in_range(string_to_range(input));
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
		returned = text_encoding_UTF8_to_UTF16LE(input_in_range.start, input_in_range.finish, &tmp);
		ASSERT_EQ(!input.empty(), returned) << input << std::endl << buffer_free(&tmp);

		if (!input.empty())
		{
			ASSERT_TRUE(buffer_push_back_uint16(&tmp, 0)) << buffer_free(&tmp);
		}

		returned = directory_exists_wchar_t(buffer_wchar_t_data(&tmp, 0));
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&tmp);
#endif
		--node_count;
	}

	buffer_release(&tmp);
}

TEST(TestFileSystem_, directory_get_time_attributes)
{
	const std::string inputs[] =
	{
#if defined(_WIN32)
		"C:\\Windows",
		"C:\\Users"
#else
		"/sys",
		"/home"
#endif
	};
	//
	uint8_t test_was_run = 0;

	for (auto& input : inputs)
	{
		const uint8_t* path = (const uint8_t*)input.c_str();

		if (!directory_exists(path))
		{
			continue;
		}

		test_was_run = 1;
		ASSERT_TRUE(directory_get_creation_time(path));
		ASSERT_TRUE(directory_get_creation_time_utc(path));
		ASSERT_TRUE(directory_get_last_access_time(path));
		ASSERT_TRUE(directory_get_last_access_time_utc(path));
		ASSERT_TRUE(directory_get_last_write_time(path));
		ASSERT_TRUE(directory_get_last_write_time_utc(path));
	}

	EXPECT_TRUE(test_was_run) << "[Warning]: No directories from inputs exists at your environment.";
}
//directory_get_current_directory
//directory_get_directory_root
TEST(TestFileSystem_, directory_get_logical_drives)
{
	buffer drives;
	SET_NULL_TO_BUFFER(drives);
	ASSERT_TRUE(directory_get_logical_drives(&drives)) << buffer_free(&drives);
#if defined(_WIN32)
	ASSERT_TRUE(buffer_size(&drives)) << buffer_free(&drives);
	uint8_t i = 0;
	const uint8_t* returned = NULL;

	for (uint8_t a = 'A'; a < 'Z' + 1; ++a)
	{
		returned = buffer_data(&drives, i);
		ASSERT_NE(nullptr, returned) << buffer_free(&drives);

		if (*returned == a)
		{
			static const uint8_t* expected_output = (const uint8_t*)":\\\0";
			const uint8_t* expected = expected_output;

			for (uint8_t count = i + 3; i < count;)
			{
				returned = buffer_data(&drives, ++i);
				ASSERT_EQ(*expected, *returned) << buffer_free(&drives);
				++expected;
			}

			++i;
		}
	}

	returned = buffer_data(&drives, i++);
	ASSERT_NE(nullptr, returned) << buffer_free(&drives);
	ASSERT_EQ('\0', *returned) << buffer_free(&drives);
	returned = buffer_data(&drives, i++);
	ASSERT_EQ(nullptr, returned) << buffer_free(&drives);
#else
	ASSERT_EQ(3, buffer_size(&drives)) << buffer_free(&drives);
	std::string expected_output(3, '\0');
	expected_output[0] = '/';
	ASSERT_EQ(expected_output, buffer_to_string(&drives)) << buffer_free(&drives);
#endif
	buffer_release(&drives);
}
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
		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		//
		auto returned = file_exists((const uint8_t*)input.c_str());
		ASSERT_EQ(expected_return, returned) << input << buffer_free(&tmp);
#if defined(_WIN32)
		const auto input_in_range(string_to_range(input));
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
		returned = text_encoding_UTF8_to_UTF16LE(input_in_range.start, input_in_range.finish, &tmp);
		ASSERT_EQ(!input.empty(), returned) << input << std::endl << buffer_free(&tmp);

		if (!input.empty())
		{
			ASSERT_TRUE(buffer_push_back_uint16(&tmp, 0)) << buffer_free(&tmp);
		}

		returned = file_exists_wchar_t(buffer_wchar_t_data(&tmp, 0));
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&tmp);
#endif
		--node_count;
	}

	buffer_release(&tmp);
}

TEST(TestFileSystem_, file_get_attributes)
{
	const std::string inputs[] =
	{
#if defined(_WIN32)
		"C:\\Windows\\notepad.exe",
		"C:\\Windows\\regedit.exe"
#else
		"/bin/uname",
		"/sbin/halt"
#endif
	};
	//
	uint8_t test_was_run = 0;

	for (auto& input : inputs)
	{
		const uint8_t* path = (const uint8_t*)input.c_str();

		if (!file_exists(path))
		{
			continue;
		}

		test_was_run = 1;
		ASSERT_TRUE(file_get_creation_time(path));
		ASSERT_TRUE(file_get_creation_time_utc(path));
		ASSERT_TRUE(file_get_last_access_time(path));
		ASSERT_TRUE(file_get_last_access_time_utc(path));
		ASSERT_TRUE(file_get_last_write_time(path));
		ASSERT_TRUE(file_get_last_write_time_utc(path));
		//
		ASSERT_LT(0, file_get_length(path));
	}

	EXPECT_TRUE(test_was_run) << "[Warning]: No files from inputs exists at your environment.";
}

TEST(TestFileSystem_, file_up_to_date)
{
	buffer paths;
	SET_NULL_TO_BUFFER(paths);
	ASSERT_TRUE(path_get_temp_file_name(&paths)) << buffer_free(&paths);
	ASSERT_TRUE(buffer_push_back(&paths, 0)) << buffer_free(&paths);
	//
	const auto size = buffer_size(&paths);
	ASSERT_TRUE(path_get_temp_file_name(&paths)) << buffer_free(&paths);
	ASSERT_TRUE(buffer_push_back(&paths, 0)) << buffer_free(&paths);
	//
	const auto src_file = buffer_data(&paths, 0);
	const auto target_file = buffer_data(&paths, size);
	ASSERT_TRUE(file_up_to_date(src_file, target_file)) << buffer_free(&paths);
	//
	buffer_free(&paths);
}
