/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 https://github.com/TheVice/
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
	//
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
	const auto size = buffer_size(&path);
	//
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	const auto ptr = buffer_data(&path, 0);
	ASSERT_FALSE(directory_exists(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(directory_create(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(directory_exists(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	//
	ASSERT_TRUE(buffer_resize(&path, size)) << (const char*)ptr << std::endl << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << (const char*)ptr << buffer_free(&path);
	//
	ASSERT_TRUE(directory_delete(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	ASSERT_FALSE(directory_exists(ptr)) << (const char*)ptr << std::endl << buffer_free(&path);
	//
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
		const auto code_node = node.node().select_node("code").node();
		//
		std::string code(code_node.child_value());
		const std::string property_name(code_node.attribute("input").as_string());
		auto content = string_to_range(code);
		//
		buffer the_project;
		SET_NULL_TO_BUFFER(the_project);
		//
		auto returned = project_new(&the_project);
		ASSERT_TRUE(returned)
				<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		returned = project_load_from_content(content.start, content.finish, &the_project, 0, verbose);
		ASSERT_TRUE(returned)
				<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);

		for (const auto& output : node.node().select_nodes("output"))
		{
			returned = buffer_resize(&property_value, 0);
			ASSERT_TRUE(returned)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			//
			returned = project_property_get_by_name(
						   &the_project, (const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(), &property_value, verbose);
			ASSERT_TRUE(returned)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			//
			returned = buffer_push_back(&property_value, 0);
			ASSERT_TRUE(returned)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			//
			const std::string entry_type_str(output.node().attribute("entry_type").as_string());
			ASSERT_TRUE(entry_types.count(entry_type_str))
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			const auto entry_type = entry_types.at(entry_type_str);
			//
			const uint8_t recurse = output.node().attribute("recurse").as_bool();
			//
			returned = buffer_resize(&file_tree, 0);
			ASSERT_TRUE(returned)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			//
			returned = directory_enumerate_file_system_entries(&property_value, entry_type, recurse, &file_tree, 1);
			ASSERT_TRUE(returned)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			//
			ptrdiff_t counter = 0;
			const ptrdiff_t returned_counter = buffer_size(&file_tree);
			const auto returned_paths(buffer_to_string(&file_tree));
			const auto returned_output = string_to_range(returned_paths);

			for (auto& entry : output.node())
			{
				const std::string entry_str(entry.child_value());
				//
				returned = buffer_resize(&file_tree, 0);
				ASSERT_TRUE(returned)
						<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
				//
				returned = project_property_get_by_name(&the_project, (const uint8_t*)entry_str.c_str(),
														(uint8_t)entry_str.size(), &file_tree, verbose);
				ASSERT_TRUE(returned)
						<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
				//
				returned = buffer_push_back(&file_tree, 0);
				ASSERT_TRUE(returned)
						<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
				//
				counter += buffer_size(&file_tree);
				//
				const auto expected_path = buffer_to_range(&file_tree);
				returned = string_contains(returned_output.start, returned_output.finish, expected_path.start,
										   expected_path.finish);
				//
				std::string returned_output_str;
				std::string expected_path_str;

				if (!returned)
				{
					returned_output_str = range_to_string(returned_output);
					expected_path_str = range_to_string(expected_path);
				}

				ASSERT_TRUE(returned)
						<< returned_output_str << std::endl << expected_path_str << std::endl
						<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			}

			ASSERT_EQ(counter, returned_counter)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
		}

		returned = buffer_resize(&property_value, 0);
		ASSERT_TRUE(returned)
				<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		returned = project_property_get_by_name(
					   &the_project, (const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(), &property_value, verbose);
		ASSERT_TRUE(returned)
				<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		project_unload(&the_project);
		//
		content = buffer_to_range(&property_value);
		//
		returned = path_get_file_name(content.start, content.finish, &content);
		ASSERT_TRUE(returned)
				<< buffer_free(&property_value) << buffer_free(&file_tree);

		if (content.finish != find_any_symbol_like_or_not_like_that(content.start, content.finish,
				(const uint8_t*)"?*", 2, 1, 1))
		{
			content = buffer_to_range(&property_value);
			returned = path_get_directory_name(content.start, content.finish, &content);
			ASSERT_TRUE(returned)
					<< buffer_free(&property_value) << buffer_free(&file_tree);
			//
			returned = buffer_resize(&property_value, range_size(&content));
			ASSERT_TRUE(returned)
					<< buffer_free(&property_value) << buffer_free(&file_tree);
		}

		returned = buffer_push_back(&property_value, 0);
		ASSERT_TRUE(returned)
				<< buffer_free(&property_value) << buffer_free(&file_tree);
		//
		code = buffer_to_string(&property_value);
		content = string_to_range(code);
		//
		returned = directory_delete(content.start);
		//
		ASSERT_TRUE(returned)
				<< (const char*)(content.start) << std::endl << buffer_free(&property_value) << buffer_free(&file_tree);
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

TEST(TestFileSystem_, directory_move)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	const ptrdiff_t size = buffer_size(&path);
	//
	ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	ASSERT_EQ(2 * size, buffer_size(&path)) << buffer_free(&path);
	//
	auto returned = memcmp(buffer_char_data(&path, 0), buffer_char_data(&path, size), size);
	ASSERT_TRUE(returned) << buffer_free(&path);
	//
	ASSERT_FALSE(directory_exists(buffer_data(&path, 0))) << buffer_free(&path);
	//
	returned = directory_create(buffer_data(&path, 0));
	ASSERT_TRUE(returned) << buffer_free(&path);
	//
	ASSERT_TRUE(directory_exists(buffer_data(&path, 0))) << buffer_free(&path);
	ASSERT_FALSE(directory_exists(buffer_data(&path, size))) << buffer_free(&path);
	//
	returned = directory_move(buffer_data(&path, 0), buffer_data(&path, size));
	//
	ASSERT_TRUE(returned) << buffer_free(&path);
	//
	ASSERT_FALSE(directory_exists(buffer_data(&path, 0))) << buffer_free(&path);
	ASSERT_TRUE(directory_exists(buffer_data(&path, size))) << buffer_free(&path);
	//
	ASSERT_TRUE(directory_delete(buffer_data(&path, size))) << buffer_free(&path);
	//
	buffer_release(&path);
}

TEST(TestFileSystem_, directory_set_current_directory)
{
	uint8_t verbose = 0;
	//
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
	auto path_in_range(buffer_to_range(&path));
	//
	auto returned = path_get_path_root(path_in_range.start, path_in_range.finish, &path_in_range);
	ASSERT_TRUE(returned) << buffer_free(&path);
	//
	returned = buffer_resize(&path, range_size(&path_in_range));
	ASSERT_TRUE(returned) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	returned = directory_set_current_directory(buffer_data(&path, 0));
	ASSERT_TRUE(returned) << buffer_free(&path);
	//
	returned = buffer_resize(&path, buffer_size(&path) - 1);
	ASSERT_TRUE(returned) << buffer_free(&path);
	//
	const auto path_that_was_set(buffer_to_string(&path));
	//
	ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
	returned = directory_get_current_directory(NULL, NULL, &path, verbose);
	ASSERT_TRUE(returned) << buffer_free(&path);
	//
	const auto current_path(buffer_to_string(&path));
	//
	ASSERT_EQ(path_that_was_set, current_path) << buffer_free(&path);
	//
	buffer_release(&path);
}

TEST(TestFileSystem_, file_copy)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);

	for (uint8_t i = 0; i < 2; ++i)
	{
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		//
		ASSERT_TRUE(path_get_temp_file_name(&path)) << buffer_free(&path);
		ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
		//
		const ptrdiff_t size = buffer_size(&path);
		//
		ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
		ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
		ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
		ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);

		if (2 * size == buffer_size(&path))
		{
			auto returned = memcmp(buffer_char_data(&path, 0), buffer_char_data(&path, size), size);
			ASSERT_TRUE(returned) << buffer_free(&path);
		}

		if (i)
		{
			ASSERT_TRUE(echo(0, Default, buffer_data(&path, 0), Info, buffer_data(&path, 0), size, 0, 0))
					<< buffer_free(&path);
		}
		else
		{
			if (!file_exists(buffer_data(&path, 0)))
			{
				ASSERT_TRUE(file_create(buffer_data(&path, 0))) << buffer_free(&path);
			}
		}

		ASSERT_TRUE(file_exists(buffer_data(&path, 0))) << buffer_free(&path);
		ASSERT_FALSE(file_exists(buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_TRUE(file_copy(buffer_data(&path, 0), buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_TRUE(file_exists(buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_TRUE(file_exists(buffer_data(&path, 0))) << buffer_free(&path);
		//
		ASSERT_TRUE(file_delete(buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_TRUE(file_delete(buffer_data(&path, 0))) << buffer_free(&path);
		ASSERT_FALSE(file_exists(buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_FALSE(file_exists(buffer_data(&path, 0))) << buffer_free(&path);
	}

	buffer_release(&path);
}

TEST(TestFileSystem_, file_create)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	ASSERT_FALSE(file_exists(buffer_data(&path, 0))) << buffer_free(&path);
	ASSERT_TRUE(file_create(buffer_data(&path, 0))) << buffer_free(&path);
	//
	ASSERT_TRUE(file_exists(buffer_data(&path, 0))) << buffer_free(&path);
	ASSERT_TRUE(file_delete(buffer_data(&path, 0))) << buffer_free(&path);
	ASSERT_FALSE(file_exists(buffer_data(&path, 0))) << buffer_free(&path);
	//
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

TEST_F(TestFileSystem, file_read_lines)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const auto expected_output = (uint16_t)INT_PARSE(
										 node.node().select_node("output").node().child_value());
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
		ASSERT_TRUE(path_get_temp_file_name(&tmp)) << buffer_free(&tmp);
		ASSERT_TRUE(buffer_push_back(&tmp, 0)) << buffer_free(&tmp);
		//
		const void* ptr = buffer_data(&tmp, 0);
		//
		ASSERT_TRUE(echo(0, Default,
						 (const uint8_t*)ptr, Info,
						 (const uint8_t*)input.c_str(), (ptrdiff_t)input.size(),
						 0, verbose)) << buffer_free(&tmp);
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
		ASSERT_TRUE(file_read_lines((const uint8_t*)ptr, &tmp)) << buffer_free(&tmp);
		//
		uint16_t i = 0;
		std::string returned_output;

		while (NULL != (ptr = buffer_range_data(&tmp, i++)))
		{
			returned_output.append(range_to_string(static_cast<const range*>(ptr)));
		}

		const auto input_in_range(string_to_range(input));
		static const uint8_t n = '\n';
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);

		if (!range_is_null_or_empty(&input_in_range))
		{
			ASSERT_TRUE(string_replace(input_in_range.start, input_in_range.finish,
									   &n, &n + 1, NULL, NULL, &tmp)) << buffer_free(&tmp);
		}

		ASSERT_EQ(expected_output, i - 1) << buffer_free(&tmp);
		ASSERT_EQ(buffer_to_string(&tmp), returned_output) << buffer_free(&tmp);
		//
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

TEST(TestFileSystem_, file_move)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);

	for (uint8_t i = 0; i < 2; ++i)
	{
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		//
		ASSERT_TRUE(path_get_temp_file_name(&path)) << buffer_free(&path);
		ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
		//
		const ptrdiff_t size = buffer_size(&path);
		//
		ASSERT_TRUE(path_get_temp_path(&path)) << buffer_free(&path);
		ASSERT_TRUE(buffer_push_back(&path, PATH_DELIMITER)) << buffer_free(&path);
		ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, &path)) << buffer_free(&path);
		ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);

		if (2 * size == buffer_size(&path))
		{
			auto returned = memcmp(buffer_char_data(&path, 0), buffer_char_data(&path, size), size);
			ASSERT_TRUE(returned) << buffer_free(&path);
		}

		if (i)
		{
			ASSERT_TRUE(echo(0, Default, buffer_data(&path, 0), Info, buffer_data(&path, 0), size, 0, 0))
					<< buffer_free(&path);
		}
		else
		{
			if (!file_exists(buffer_data(&path, 0)))
			{
				ASSERT_TRUE(file_create(buffer_data(&path, 0))) << buffer_free(&path);
			}
		}

		ASSERT_TRUE(file_exists(buffer_data(&path, 0))) << buffer_free(&path);
		ASSERT_FALSE(file_exists(buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_TRUE(file_move(buffer_data(&path, 0), buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_TRUE(file_exists(buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_FALSE(file_exists(buffer_data(&path, 0))) << buffer_free(&path);
		//
		ASSERT_TRUE(file_delete(buffer_data(&path, size))) << buffer_free(&path);
		ASSERT_FALSE(file_exists(buffer_data(&path, size))) << buffer_free(&path);
	}

	buffer_release(&path);
}

TEST(TestFileSystem_, file_up_to_date)
{
	buffer paths;
	SET_NULL_TO_BUFFER(paths);
	//
	ASSERT_TRUE(path_get_temp_file_name(&paths)) << buffer_free(&paths);
	ASSERT_TRUE(buffer_push_back(&paths, 0)) << buffer_free(&paths);

	if (!file_exists(buffer_data(&paths, 0)))
	{
		ASSERT_TRUE(file_create(buffer_data(&paths, 0))) << buffer_free(&paths);
	}

	const auto size = buffer_size(&paths);
	ASSERT_TRUE(path_get_temp_file_name(&paths)) << buffer_free(&paths);
	ASSERT_TRUE(buffer_push_back(&paths, 0)) << buffer_free(&paths);

	if (!file_exists(buffer_data(&paths, size)))
	{
		ASSERT_TRUE(file_create(buffer_data(&paths, size))) << buffer_free(&paths);
	}

	const auto src_file = buffer_data(&paths, 0);
	const auto target_file = buffer_data(&paths, size);
	ASSERT_TRUE(file_up_to_date(src_file, target_file)) << buffer_free(&paths);
	//
	ASSERT_TRUE(file_delete(src_file)) << buffer_free(&paths);
	ASSERT_TRUE(file_delete(target_file)) << buffer_free(&paths);
	//
	buffer_release(&paths);
}

TEST(TestFileSystem_, file_set_attributes)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_file_name(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);

	if (!file_exists(buffer_data(&path, 0)))
	{
		ASSERT_TRUE(file_create(buffer_data(&path, 0))) << buffer_free(&path);
	}

#if defined(_WIN32)

	for (uint8_t archive = 0; archive < 2; ++archive)
	{
		for (uint8_t hidden = 0; hidden < 2; ++hidden)
		{
			for (uint8_t normal = 0; normal < 2; ++normal)
			{
				for (uint8_t readonly = 0; readonly < 2; ++readonly)
				{
					for (uint8_t system_attribute = 0; system_attribute < 2; ++system_attribute)
					{
						ASSERT_TRUE(file_set_attributes(
										buffer_data(&path, 0), archive, hidden, normal, readonly, system_attribute)) << buffer_free(&path);
#if TODO
						unsigned long attributes = 0;
						ASSERT_TRUE(file_get_attributes(buffer_data(&path, 0), &attributes)) << buffer_free(&path);
						//
						ASSERT_EQ(archive, IS_ARCHIVE(attributes)) << buffer_free(&path);
						ASSERT_EQ(hidden, IS_HIDDEN(attributes)) << buffer_free(&path);
						ASSERT_EQ(normal, IS_NORMAL(attributes)) << buffer_free(&path);
						ASSERT_EQ(readonly, IS_READ_ONLY(attributes)) << buffer_free(&path);
						ASSERT_EQ(system_attribute, IS_SYSTEM(attributes)) << buffer_free(&path);
#endif
					}
				}
			}
		}
	}

	ASSERT_FALSE(file_delete(buffer_data(&path, 0))) << buffer_free(&path);
	ASSERT_TRUE(file_set_attributes(buffer_data(&path, 0), 0, 0, 1, 0, 0)) << buffer_free(&path);
#endif
	ASSERT_TRUE(file_delete(buffer_data(&path, 0))) << buffer_free(&path);
	/**/
	buffer_release(&path);
}

TEST(TestFileSystem_, file_set_last_access_time)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_file_name(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);

	if (!file_exists(buffer_data(&path, 0)))
	{
		ASSERT_TRUE(file_create(buffer_data(&path, 0))) << buffer_free(&path);
	}

	const int64_t time_to_set = 1569840495;
	ASSERT_TRUE(file_set_last_access_time_utc(buffer_data(&path, 0), time_to_set)) << buffer_free(&path);
	ASSERT_TRUE(file_set_last_access_time(buffer_data(&path, 0), time_to_set)) << buffer_free(&path);
	const int64_t time = file_get_last_access_time(buffer_data(&path, 0));
	//
	ASSERT_EQ(time_to_set, time) << buffer_free(&path);
	//
	ASSERT_TRUE(file_delete(buffer_data(&path, 0))) << buffer_free(&path);
	buffer_release(&path);
}

TEST(TestFileSystem_, file_set_last_write_time)
{
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_file_name(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);

	if (!file_exists(buffer_data(&path, 0)))
	{
		ASSERT_TRUE(file_create(buffer_data(&path, 0))) << buffer_free(&path);
	}

	const int64_t time_to_set = 1569840495;
	ASSERT_TRUE(file_set_last_write_time_utc(buffer_data(&path, 0), time_to_set)) << buffer_free(&path);
	ASSERT_TRUE(file_set_last_write_time(buffer_data(&path, 0), time_to_set)) << buffer_free(&path);
	const int64_t time = file_get_last_write_time(buffer_data(&path, 0));
	//
	ASSERT_EQ(time_to_set, time) << buffer_free(&path);
	//
	ASSERT_TRUE(file_set_creation_time_utc(buffer_data(&path, 0), time_to_set)) << buffer_free(&path);
	ASSERT_TRUE(file_set_creation_time(buffer_data(&path, 0), time_to_set)) << buffer_free(&path);
	//
	ASSERT_TRUE(file_delete(buffer_data(&path, 0))) << buffer_free(&path);
	buffer_release(&path);
}

TEST(TestFileSystem_, file_write_all_bytes)
{
	static const uint16_t content_size[] = { 0, INT8_MAX, UINT8_MAX, 4096, 7000, 8192, 9000 };
	//
	buffer path;
	SET_NULL_TO_BUFFER(path);
	//
	ASSERT_TRUE(path_get_temp_file_name(&path)) << buffer_free(&path);
	ASSERT_TRUE(buffer_push_back(&path, 0)) << buffer_free(&path);
	//
	const auto path_str(buffer_to_string(&path));
	const uint8_t* ptr = (const uint8_t*)path_str.c_str();

	for (uint8_t i = 0, count = COUNT_OF(content_size); i < count; ++i)
	{
		static const uint8_t value = '\0';
		//
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		const uint8_t returned =
			string_pad_left(&value, &value, &value, &value + 1, content_size[i], &path);
		ASSERT_TRUE(returned) << buffer_free(&path);
		//
		ptrdiff_t size = buffer_size(&path);
		ASSERT_EQ(content_size[i], size) << buffer_free(&path);
		//
		ASSERT_TRUE(file_write_all(ptr, &path));
		//
		size = (ptrdiff_t)file_get_length((const uint8_t*)path_str.c_str());
		ASSERT_EQ(content_size[i], size) << buffer_free(&path);
	}

	ASSERT_TRUE(file_delete(ptr)) << buffer_free(&path);
	ASSERT_FALSE(file_exists(ptr)) << buffer_free(&path);
	buffer_release(&path);
}
