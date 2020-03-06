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
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"
};

#include <map>
#include <cstdio>
#include <string>
#include <cstdint>
#include <utility>

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
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	const auto ptr = buffer_data(&path, 0);
	ASSERT_FALSE(directory_exists(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(directory_create(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(directory_exists(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(directory_delete(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	ASSERT_FALSE(directory_exists(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	//
	buffer_release(&path);
}

TEST(TestFileSystem_, directory_enumerate_file_system_entries)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	const uint8_t* ptr = buffer_data(&path, 0);
	ASSERT_TRUE(directory_create(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	//
	ASSERT_TRUE(buffer_resize(&path, buffer_size(&path) - 1)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
	ASSERT_TRUE(buffer_append_char(&path, "*.*", 3)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	// Empty folder.
	ptr = buffer_data(&path, 0);
	//
	buffer output;
	SET_NULL_TO_BUFFER(output);
#if 0
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 0, 0, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 0, 1, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 1, 0, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 1, 1, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
#endif
	// With one folder.
	ASSERT_TRUE(buffer_resize(&path, buffer_size(&path) - 4))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(get_crc32_of(__LINE__, &path))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, 0))
			<< buffer_free(&path) << buffer_free(&output);
	//
	char folder_name_1[8];
	ptr = buffer_data(&path, buffer_size(&path) - 9);
#if __STDC_SEC_API__
	ASSERT_EQ(0, memcpy_s(folder_name_1, 8, ptr, 8))
			<< buffer_free(&path) << buffer_free(&output);
#else
	memcpy(folder_name_1, ptr, 8);
#endif
	//
	ptr = buffer_data(&path, 0);
	ASSERT_TRUE(directory_create(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	//
	ASSERT_TRUE(buffer_resize(&path, buffer_size(&path) - 9))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_append_char(&path, "*.*", 3))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, 0))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ptr = buffer_data(&path, 0);
#if 0
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 0, 0, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	/*ASSERT_EQ(9, buffer_size(&output)) << buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(0, memcmp(folder_name_1, buffer_data(&output, 0), 8))
			<< buffer_free(&path) << buffer_free(&output);*/
	//
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 0, 1, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	/*ASSERT_EQ(9, buffer_size(&output)) << buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(0, memcmp(folder_name_1, buffer_data(&output, 0), 8))
			<< buffer_free(&path) << buffer_free(&output);*/
	//
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 1, 0, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 1, 1, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
#endif
	// With one folder and file in it.
	ASSERT_TRUE(buffer_resize(&path, buffer_size(&path) - 4))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_append_char(&path, folder_name_1, 8))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(get_crc32_of(__LINE__, &path))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, 0))
			<< buffer_free(&path) << buffer_free(&output);
	//
	char file_name_1[17];
	ptr = buffer_data(&path, buffer_size(&path) - 18);
#if __STDC_SEC_API__
	ASSERT_EQ(0, memcpy_s(file_name_1, 17, ptr, 17))
			<< buffer_free(&path) << buffer_free(&output);
#else
	memcpy(file_name_1, ptr, 17);
#endif
	//
	ptr = buffer_data(&path, 0);
	ASSERT_TRUE(echo(0, UTF8, ptr, NoLevel, ptr, buffer_size(&path) - 1, 0, NoLevel))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&path, buffer_size(&path) - 18))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_append_char(&path, "*.*", 3))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, 0))
			<< buffer_free(&path) << buffer_free(&output);
#if 0
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 0, 0, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(9, buffer_size(&output)) << buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(0, memcmp(folder_name_1, buffer_data(&output, 0), 8))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 0, 1, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(9, buffer_size(&output)) << buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(0, memcmp(folder_name_1, buffer_data(&output, 0), 8))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 1, 0, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_FALSE(buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 1, 1, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	ASSERT_LT(18, buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(0, memcmp(file_name_1, buffer_data(&output, buffer_size(&output) - 18), 17))
			<< buffer_free(&path) << buffer_free(&output);
#endif
	// With two files and folder.
	ASSERT_TRUE(buffer_resize(&path, buffer_size(&path) - 4))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(get_crc32_of(__LINE__, &path))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, 0))
			<< buffer_free(&path) << buffer_free(&output);
	//
	char file_name_2[8];
	ptr = buffer_data(&path, buffer_size(&path) - 9);
#if __STDC_SEC_API__
	ASSERT_EQ(0, memcpy_s(file_name_2, 8, ptr, 8))
			<< buffer_free(&path) << buffer_free(&output);
#else
	memcpy(file_name_2, ptr, 8);
#endif
	//
	ptr = buffer_data(&path, 0);
	ASSERT_TRUE(echo(0, UTF8, ptr, NoLevel, ptr, buffer_size(&path) - 1, 0, NoLevel))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&path, buffer_size(&path) - 9))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_append_char(&path, "*.*", 3))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, 0))
			<< buffer_free(&path) << buffer_free(&output);
#if 0
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 0, 0, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	/*ASSERT_EQ(9, buffer_size(&output)) << buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(0, memcmp(folder_name_1, buffer_data(&output, 0), 8))
			<< buffer_free(&path) << buffer_free(&output);*/
	//
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 0, 1, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	/*ASSERT_EQ(9, buffer_size(&output)) << buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(0, memcmp(folder_name_1, buffer_data(&output, 0), 8))
			<< buffer_free(&path) << buffer_free(&output);*/
	//
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 1, 0, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	/*ASSERT_EQ(9, buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_EQ(0, memcmp(file_name_2, buffer_data(&output, 0), 8))
			<< buffer_free(&path) << buffer_free(&output);*/
	//
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(ptr, 1, 1, &output))
			<< (const char*)ptr << std::endl << buffer_free(&path) << buffer_free(&output);
	//
	uint8_t i = 0;
	const char* expected_output[] = { file_name_2, file_name_1 };
	const uint8_t expected_lengths[] = { 8, 17 };
	ptr = buffer_data(&output, 0);
	const uint8_t* finish = ptr + buffer_size(&path);

	do
	{
		ptr += common_count_bytes_until(ptr, 0) - expected_lengths[i];
		ASSERT_EQ(0, memcmp(expected_output[i], ptr, expected_lengths[i]))
				<< i << std::endl << buffer_free(&path) << buffer_free(&output);
		ptr += common_count_bytes_until(ptr, 0) + 1;
		++i;
	}
	while (ptr < finish && i < COUNT_OF(expected_lengths));

#endif
	// With two files and two folders.
	//...
	//
	ASSERT_TRUE(buffer_resize(&path, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(path_get_temp_path(&path))
			<< buffer_free(&path) << buffer_free(&output);
	//
	auto root = buffer_to_range(&path);
	ASSERT_TRUE(path_get_path_root(root.start, root.finish, &root))
			<< buffer_free(&path) << buffer_free(&output);
	//
	ASSERT_TRUE(buffer_resize(&path, range_size(&root)))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, '*'))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_push_back(&path, 0))
			<< buffer_free(&path) << buffer_free(&output);
#if 0
	ASSERT_TRUE(buffer_resize(&output, 0))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(directory_enumerate_file_system_entries(root.start, 2, 0, &output))
			<< buffer_free(&path) << buffer_free(&output);
	ASSERT_TRUE(buffer_size(&output))
			<< buffer_free(&path) << buffer_free(&output);
#endif
	buffer_release(&output);
	buffer_release(&path);
}

TEST_F(TestFileSystem, directory_enumerate_file_system_entries)
{
	static const std::map<std::string, uint8_t> entry_types(
	{
		std::make_pair(std::string("directory"), (uint8_t)0),
		std::make_pair(std::string("file"), (uint8_t)1),
		std::make_pair(std::string("all"), (uint8_t)2)
	});
	//
	buffer property_value;
	SET_NULL_TO_BUFFER(property_value);
	//
	buffer file_tree;
	SET_NULL_TO_BUFFER(file_tree);

	for (const auto& node : nodes)
	{
		const std::string code(node.node().select_node("code").node().child_value());
		const std::string property_name(node.node().select_node("code").node().attribute("input").as_string());
		auto content = string_to_range(code);
		//
		void* project = NULL;
		auto returned = project_new(&project);
		ASSERT_TRUE(returned)
				<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		returned = project_load_from_content(content.start, content.finish, project, verbose);
		ASSERT_TRUE(returned)
				<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		returned = buffer_resize(&property_value, 0);
		ASSERT_TRUE(returned)
				<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		returned = project_property_get_by_name(
					   project, (const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(), &property_value, verbose);
		ASSERT_TRUE(returned)
				<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		auto pattern(buffer_to_string(&property_value));
#if 0
		for (const auto& output : node.node().select_nodes("output"))
		{
			const std::string entry_type_str(output.node().attribute("entry_type").as_string());
			ASSERT_TRUE(entry_types.count(entry_type_str))
					<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
			const auto entry_type = entry_types.at(entry_type_str);
			const uint8_t recurse = output.node().attribute("recurse").as_bool();
			//
			returned = buffer_resize(&file_tree, 0);
			ASSERT_TRUE(returned)
					<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
			returned = directory_enumerate_file_system_entries(
						   (const uint8_t*)pattern.c_str(), entry_type, recurse, &file_tree);
			ASSERT_TRUE(returned)
					<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
			const auto returned_output = buffer_to_range(&file_tree);
			ptrdiff_t size = 0;

			for (auto& entry : output.node())
			{
				const std::string entry_str(entry.child_value());
				//
				returned = buffer_resize(&property_value, 0);
				ASSERT_TRUE(returned)
						<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
				ASSERT_TRUE(project_property_get_by_name(project, (const uint8_t*)entry_str.c_str(),
							(uint8_t)entry_str.size(), &property_value, verbose))
						<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
				//
				ASSERT_TRUE(buffer_push_back(&property_value, 0))
						<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
				size += buffer_size(&property_value);
				//
				const auto expected_path = buffer_to_range(&property_value);
				returned = string_contains(returned_output.start, returned_output.finish, expected_path.start,
										   expected_path.finish);
				//
				ASSERT_TRUE(returned)
						<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
			}

			ASSERT_EQ(size, buffer_size(&file_tree))
					<< project_free(project) << buffer_free(&property_value) << buffer_free(&file_tree);
		}
#endif
		project_unload(project);
		//
		content = string_to_range(pattern);
		ASSERT_TRUE(path_get_directory_name(content.start, content.finish, &content))
				<< pattern << std::endl << buffer_free(&property_value) << buffer_free(&file_tree);
		ASSERT_TRUE(buffer_resize(&property_value, 0))
				<< buffer_free(&property_value) << buffer_free(&file_tree);
		ASSERT_TRUE(buffer_append_data_from_range(&property_value, &content))
				<< buffer_free(&property_value) << buffer_free(&file_tree);
		ASSERT_TRUE(buffer_push_back(&property_value, 0))
				<< buffer_free(&property_value) << buffer_free(&file_tree);
		//
		pattern = buffer_to_string(&property_value);
		content = string_to_range(pattern);
#if TODO
		ASSERT_TRUE(directory_delete(content.start))
				<< (const char*)(content.start) << std::endl << buffer_free(&property_value) << buffer_free(&file_tree);
#endif
		//
		--node_count;
	}

	buffer_release(&file_tree);
	buffer_release(&property_value);
}

TEST_F(TestFileSystem, directory_exists)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		uint8_t condition = 0;
		ASSERT_TRUE(is_this_node_pass_by_if_condition(node, &tmp, &condition, verbose)) << buffer_free(&tmp);

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

TEST(TestFileSystem_, directory_get_parent_directory)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);
	ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
	//
	const auto path_in_range = buffer_to_range(&path);
	ASSERT_FALSE(range_is_null_or_empty(&path_in_range)) << buffer_free(&path);
	//
	range parent;
	ASSERT_TRUE(directory_get_parent_directory(path_in_range.start, path_in_range.finish,
				&parent)) << buffer_free(&path);
	//
	ASSERT_FALSE(string_equal(path_in_range.start, path_in_range.finish, parent.start,
							  parent.finish)) << buffer_free(&path);
	buffer_release(&path);
}

TEST_F(TestFileSystem, file_exists)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		uint8_t condition = 0;
		ASSERT_TRUE(is_this_node_pass_by_if_condition(node, &tmp, &condition, verbose)) << buffer_free(&tmp);

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
	buffer_release(&paths);
}
