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
#if 0
uint8_t get_crc32_of(const int64_t input, void* output)
{
	if (nullptr == output)
	{
		return 0;
	}

	const auto size = buffer_size(output);

	if (!buffer_append(output, nullptr, 4 * sizeof(uint32_t)))
	{
		return 0;
	}

	auto ptr = reinterpret_cast<const uint64_t*>(&input);
	auto in_ = buffer_uint8_t_data(output, size);

	if (!hash_algorithm_uint64_t_array_to_uint8_t_array(ptr, ptr + 1, in_))
	{
		return 0;
	}

	auto out = buffer_uint8_t_data(output, buffer_size(output) - sizeof(uint32_t));

	if (!hash_algorithm_crc32(in_, in_ + sizeof(int64_t), out, 1))
	{
		return 0;
	}

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	return hash_algorithm_bytes_to_string(out, out + sizeof(uint32_t), output);
}

TEST(TestFileSystem_, directory_create_and_delete)
{
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));
	//
	ASSERT_TRUE(path_get_temp_path(path)) << buffer_free(path);
	//
	ASSERT_TRUE(buffer_push_back(path, PATH_DELIMITER)) << buffer_free(path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, path)) << buffer_free(path);
	const auto size = buffer_size(path);
	//
	ASSERT_TRUE(buffer_push_back(path, PATH_DELIMITER)) << buffer_free(path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	const auto path_ = buffer_uint8_t_data(path, 0);
	ASSERT_FALSE(directory_exists(path_)) << reinterpret_cast<const char*>(path_) << std::endl << buffer_free(
			path);
	ASSERT_TRUE(directory_create(path_)) << reinterpret_cast<const char*>(path_) << std::endl << buffer_free(
			path);
	ASSERT_TRUE(directory_exists(path_)) << reinterpret_cast<const char*>(path_) << std::endl << buffer_free(
			path);
	//
	ASSERT_TRUE(buffer_resize(path, size)) << reinterpret_cast<const char*>(path_) << std::endl <<
										   buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << reinterpret_cast<const char*>(path_) << buffer_free(path);
	//
	ASSERT_TRUE(directory_delete(path_)) << reinterpret_cast<const char*>(path_) << std::endl << buffer_free(
			path);
	ASSERT_FALSE(directory_exists(path_)) << reinterpret_cast<const char*>(path_) << std::endl << buffer_free(
			path);
	//
	buffer_release(path);
}

TEST_F(TestFileSystem, directory_enumerate_file_system_entries)
{
	static const std::map<std::string, uint8_t> entry_types(
	{
		std::make_pair(std::string("directory"), static_cast<uint8_t>(0)),
		std::make_pair(std::string("file"), static_cast<uint8_t>(1)),
		std::make_pair(std::string("all"), static_cast<uint8_t>(2))
	});
	//
	std::string property_value_buffer(buffer_size_of(), 0);
	auto property_value = reinterpret_cast<void*>(&property_value_buffer[0]);
	ASSERT_TRUE(buffer_init(property_value, buffer_size_of()));
	//
	std::string file_tree_buffer(buffer_size_of(), 0);
	auto file_tree = reinterpret_cast<void*>(&file_tree_buffer[0]);
	ASSERT_TRUE(buffer_init(file_tree, buffer_size_of()))
			<< buffer_free(property_value);
	//
	uint8_t verbose = 0;

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		const auto code_node = the_node.select_node("code").node();
		//
		std::string code(code_node.child_value());
		const std::string property_name(code_node.attribute("input").as_string());
		auto content = string_to_range(code);
		//
		std::string the_project_buffer(buffer_size_of(), 0);
		auto the_project = reinterpret_cast<void*>(&the_project_buffer[0]);
		ASSERT_TRUE(buffer_init(the_project, buffer_size_of()))
				<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		auto returned = project_new(&the_project);
		ASSERT_TRUE(returned)
				<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		returned = project_load_from_content(content.start, content.finish, &the_project, 0, verbose);
		ASSERT_TRUE(returned)
				<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);

		for (const auto& output : the_node.select_nodes("output"))
		{
			returned = buffer_resize(&property_value, 0);
			ASSERT_TRUE(returned)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			//
			returned = project_property_get_by_name(
						   &the_project,
						   reinterpret_cast<const uint8_t*>(property_name.c_str()),
						   static_cast<uint8_t>(property_name.size()),
						   &property_value,
						   verbose);
			ASSERT_TRUE(returned)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			//
			returned = buffer_push_back(&property_value, 0);
			ASSERT_TRUE(returned)
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			//
			const auto the_output_node = output.node();
			const std::string entry_type_str(the_output_node.attribute("entry_type").as_string());
			ASSERT_TRUE(entry_types.count(entry_type_str))
					<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
			const auto entry_type = entry_types.at(entry_type_str);
			//
			const uint8_t recurse = the_output_node.attribute("recurse").as_bool();
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
			const auto returned_counter = buffer_size(&file_tree);
			const auto returned_paths(buffer_to_string(&file_tree));
			const auto returned_output = string_to_range(returned_paths);

			for (auto& entry : the_output_node)
			{
				const std::string entry_str(entry.child_value());
				//
				returned = buffer_resize(&file_tree, 0);
				ASSERT_TRUE(returned)
						<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
				//
				returned = project_property_get_by_name(
							   &the_project,
							   reinterpret_cast<const uint8_t*>(entry_str.c_str()),
							   static_cast<uint8_t>(entry_str.size()),
							   &file_tree,
							   verbose);
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
					   &the_project,
					   reinterpret_cast<const uint8_t*>(property_name.c_str()),
					   static_cast<uint8_t>(property_name.size()),
					   &property_value,
					   verbose);
		ASSERT_TRUE(returned)
				<< project_free(&the_project) << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		project_unload(&the_project);
		//
		content = buffer_to_range(&property_value);
		//
		returned = path_get_file_name(&content.start, content.finish);
		ASSERT_TRUE(returned)
				<< buffer_free(&property_value) << buffer_free(&file_tree);
		//
		static const auto* that = reinterpret_cast<const uint8_t*>("?*");

		if (content.finish != string_find_any_symbol_like_or_not_like_that(
				content.start, content.finish, that, that + 2, 1, 1))
		{
			content = buffer_to_range(&property_value);
			returned = path_get_directory_name(content.start, &content.finish);
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
				<< reinterpret_cast<const char*>(content.start)
				<< std::endl << buffer_free(&property_value) << buffer_free(&file_tree);
		//
		--node_count;
	}

	buffer_release(&file_tree);
	buffer_release(&property_value);
}

TEST_F(TestFileSystem, directory_exists)
{
	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		const std::string input(the_node.select_node("input").node().child_value());
		const std::string return_str(the_node.select_node("return").node().child_value());
		const auto return_in_a_range(string_to_range(return_str));
		const auto expected_return =
			static_cast<uint8_t>(int_parse(return_in_a_range.start, return_in_a_range.finish));
		//
		auto returned = directory_exists(reinterpret_cast<const uint8_t*>(input.c_str()));
		ASSERT_EQ(expected_return, returned) << input;
#if defined(_WIN32)
		const auto input_w(char_to_wchar_t(input));
		returned = directory_exists_wchar_t(input_w.c_str());
		ASSERT_EQ(expected_return, returned) << input;
#endif
		--node_count;
	}
}
#endif
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
		const auto* path = reinterpret_cast<const uint8_t*>(input.c_str());

		if (!directory_exists(path))
		{
			continue;
		}

		test_was_run = 1;
		ASSERT_LE(0, directory_get_creation_time(path));
		ASSERT_LE(0, directory_get_creation_time_utc(path));
		ASSERT_LE(0, directory_get_last_access_time(path));
		ASSERT_LE(0, directory_get_last_access_time_utc(path));
		ASSERT_LE(0, directory_get_last_write_time(path));
		ASSERT_LE(0, directory_get_last_write_time_utc(path));
	}

	EXPECT_TRUE(test_was_run) << "[Warning]: No directories from inputs exists at your environment.";
}
//directory_get_current_directory
//directory_get_directory_root
TEST(TestFileSystem_, directory_get_logical_drives)
{
	std::string drives_buffer(buffer_size_of(), 0);
	auto drives = reinterpret_cast<void*>(&drives_buffer[0]);
	ASSERT_TRUE(buffer_init(drives, buffer_size_of()));
	//
	ASSERT_TRUE(directory_get_logical_drives(drives)) << buffer_free(drives);
#if defined(_WIN32)
	ASSERT_TRUE(buffer_size(drives)) << buffer_free(drives);
	uint8_t i = 0;
	const uint8_t* returned;

	for (uint8_t a = 'A'; a < 'Z' + 1; ++a)
	{
		returned = buffer_uint8_t_data(drives, i);
		ASSERT_NE(nullptr, returned) << buffer_free(drives);

		if (*returned == a)
		{
			static const auto* expected_output = reinterpret_cast<const uint8_t*>(":\\\0");
			const auto* expected = expected_output;

			for (uint8_t count = i + 3; i < count;)
			{
				returned = buffer_uint8_t_data(drives, ++i);
				ASSERT_EQ(*expected, *returned) << buffer_free(drives);
				++expected;
			}

			++i;
		}
	}

	returned = buffer_uint8_t_data(drives, i++);
	ASSERT_NE(nullptr, returned) << buffer_free(drives);
	ASSERT_EQ('\0', *returned) << buffer_free(drives);
	returned = buffer_uint8_t_data(drives, i++);
	ASSERT_EQ(nullptr, returned) << buffer_free(drives);
#else
	ASSERT_EQ(3, buffer_size(drives)) << buffer_free(drives);
	std::string expected_output(3, '\0');
	expected_output[0] = '/';
	ASSERT_EQ(expected_output, buffer_to_string(drives)) << buffer_free(drives);
#endif
	buffer_release(drives);
}
#if 0
TEST(TestFileSystem_, directory_move)
{
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));
	//
	ASSERT_TRUE(path_get_temp_path(path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, PATH_DELIMITER)) << buffer_free(path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	const auto size = buffer_size(path);
	//
	ASSERT_TRUE(path_get_temp_path(path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, PATH_DELIMITER)) << buffer_free(path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	ASSERT_EQ(2 * size, buffer_size(path)) << buffer_free(path);
	//
	auto returned = memcmp(buffer_char_data(path, 0), buffer_char_data(path, size), size);
	ASSERT_TRUE(returned) << buffer_free(path);
	//
	const auto path_1 = buffer_uint8_t_data(path, 0);
	const auto path_2 = buffer_uint8_t_data(path, size);
	//
	ASSERT_FALSE(directory_exists(path_1)) << buffer_free(path);
	//
	returned = directory_create(path_1);
	ASSERT_TRUE(returned) << buffer_free(path);
	//
	ASSERT_TRUE(directory_exists(path_1)) << buffer_free(path);
	ASSERT_FALSE(directory_exists(path_2)) << buffer_free(path);
	//
	returned = directory_move(path_1, path_2);
	//
	ASSERT_TRUE(returned) << buffer_free(path);
	//
	ASSERT_FALSE(directory_exists(path_1)) << buffer_free(path);
	ASSERT_TRUE(directory_exists(path_2)) << buffer_free(path);
	//
	ASSERT_TRUE(directory_delete(path_2)) << buffer_free(path);
	//
	buffer_release(path);
}

TEST(TestFileSystem_, directory_set_current_directory)
{
	uint8_t verbose = 0;
	//
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));
	//
	ASSERT_TRUE(directory_get_current_directory(nullptr, nullptr, path, verbose))
			<< buffer_free(path);
	const auto current_directory(buffer_to_string(path));
	//
	ASSERT_TRUE(path_get_temp_path(path)) << buffer_free(path);
	auto path_in_a_range(buffer_to_range(path));
	//
	auto returned = path_get_path_root(path_in_a_range.start, path_in_a_range.finish);
	ASSERT_TRUE(returned) << buffer_free(path);
	//
	returned = buffer_resize(path, range_size(path_in_a_range));
	ASSERT_TRUE(returned) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	returned = directory_set_current_directory(buffer_uint8_t_data(path, 0));
	ASSERT_TRUE(returned) << buffer_free(path);
	//
	returned = buffer_resize(path, buffer_size(path) - 1);
	ASSERT_TRUE(returned) << buffer_free(path);
	//
	const auto path_that_was_set(buffer_to_string(path));
	//
	ASSERT_TRUE(buffer_resize(path, 0)) << buffer_free(path);
	returned = directory_get_current_directory(nullptr, nullptr, path, verbose);
	ASSERT_TRUE(returned) << buffer_free(path);
	//
	const auto current_path(buffer_to_string(path));
	//
	ASSERT_EQ(path_that_was_set, current_path) << buffer_free(path);
	//
	buffer_release(path);
	//
	ASSERT_TRUE(directory_set_current_directory(reinterpret_cast<const uint8_t*>(current_directory.c_str())));
}

TEST(TestFileSystem_, file_copy)
{
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));

	for (uint8_t i = 0; i < 2; ++i)
	{
		ASSERT_TRUE(buffer_resize(path, 0)) << buffer_free(path);
		//
		ASSERT_TRUE(path_get_temp_file_name(path)) << buffer_free(path);
		ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
		//
		const auto size = buffer_size(path);
		//
		ASSERT_TRUE(path_get_temp_path(path)) << buffer_free(path);
		ASSERT_TRUE(buffer_push_back(path, PATH_DELIMITER)) << buffer_free(path);
		ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, path)) << buffer_free(path);
		ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);

		if (2 * size == buffer_size(path))
		{
			auto returned = memcmp(buffer_char_data(path, 0), buffer_char_data(path, size), size);
			ASSERT_TRUE(returned) << buffer_free(path);
		}

		const auto path_1 = buffer_uint8_t_data(path, 0);
		const auto path_2 = buffer_uint8_t_data(path, size);

		if (i)
		{
			ASSERT_TRUE(echo(0, Default, path_1, Info, path_1, size, 0, 0))
					<< buffer_free(path);
		}
		else
		{
			if (!file_exists(path_1))
			{
				ASSERT_TRUE(file_create(path_1)) << buffer_free(path);
			}
		}

		ASSERT_TRUE(file_exists(path_1)) << buffer_free(path);
		ASSERT_FALSE(file_exists(path_2)) << buffer_free(path);
		ASSERT_TRUE(file_copy(path_1, path_2)) << buffer_free(path);
		ASSERT_TRUE(file_exists(path_2)) << buffer_free(path);
		ASSERT_TRUE(file_exists(path_1)) << buffer_free(path);
		//
		ASSERT_TRUE(file_delete(path_2)) << buffer_free(path);
		ASSERT_TRUE(file_delete(path_1)) << buffer_free(path);
		ASSERT_FALSE(file_exists(path_2)) << buffer_free(path);
		ASSERT_FALSE(file_exists(path_1)) << buffer_free(path);
	}

	buffer_release(path);
}

TEST(TestFileSystem_, file_create)
{
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));
	//
	ASSERT_TRUE(path_get_temp_path(path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, PATH_DELIMITER)) << buffer_free(path);
	ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	const auto path_ = buffer_uint8_t_data(path, 0);
	//
	ASSERT_FALSE(file_exists(path_)) << buffer_free(path);
	ASSERT_TRUE(file_create(path_)) << buffer_free(path);
	//
	ASSERT_TRUE(file_exists(path_)) << buffer_free(path);
	ASSERT_TRUE(file_delete(path_)) << buffer_free(path);
	ASSERT_FALSE(file_exists(path_)) << buffer_free(path);
	//
	buffer_release(path);
}

TEST_F(TestFileSystem, file_exists)
{
	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		const std::string input(the_node.select_node("input").node().child_value());
		const std::string return_str(the_node.select_node("return").node().child_value());
		const auto return_in_a_range(string_to_range(return_str));
		const auto expected_return =
			static_cast<uint8_t>(int_parse(return_in_a_range.start, return_in_a_range.finish));
		//
		auto returned = file_exists(reinterpret_cast<const uint8_t*>(input.c_str()));
		ASSERT_EQ(expected_return, returned) << input;
#if defined(_WIN32)
		const auto input_w(char_to_wchar_t(input));
		returned = file_exists_wchar_t(input_w.c_str());
		ASSERT_EQ(expected_return, returned) << input;
#endif
		--node_count;
	}
}

TEST_F(TestFileSystem, file_read_lines)
{
	std::string tmp_buffer(buffer_size_of(), 0);
	auto tmp = reinterpret_cast<void*>(&tmp_buffer[0]);
	ASSERT_TRUE(buffer_init(tmp, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		const std::string input(the_node.select_node("input").node().child_value());
		const std::string output_str(the_node.select_node("output").node().child_value());
		const auto output_in_a_range(string_to_range(output_str));
		const auto expected_output =
			static_cast<uint16_t>(int_parse(output_in_a_range.start, output_in_a_range.finish));
		//
		ASSERT_TRUE(buffer_resize(tmp, 0)) << buffer_free(tmp);
		ASSERT_TRUE(path_get_temp_file_name(tmp)) << buffer_free(tmp);
		ASSERT_TRUE(buffer_push_back(tmp, 0)) << buffer_free(tmp);
		//
		const void* ptr = buffer_data(tmp, 0);
		//
		ASSERT_TRUE(echo(0, Default,
						 reinterpret_cast<const uint8_t*>(ptr),
						 Info,
						 reinterpret_cast<const uint8_t*>(input.c_str()),
						 static_cast<ptrdiff_t>(input.size()),
						 0,
						 0)) << buffer_free(tmp);
		//
		ASSERT_TRUE(buffer_resize(tmp, 0)) << buffer_free(tmp);
		ASSERT_TRUE(file_read_lines(reinterpret_cast<const uint8_t*>(ptr), tmp)) << buffer_free(tmp);
		//
		uint16_t i = 0;
		std::string returned_output;

		while (nullptr != (ptr = buffer_range_data(tmp, i++)))
		{
			returned_output.append(range_to_string(static_cast<const range*>(ptr)));
		}

		const auto input_in_a_range(string_to_range(input));
		static const uint8_t n = '\n';
		//
		ASSERT_TRUE(buffer_resize(tmp, 0)) << buffer_free(tmp);

		if (!range_is_null_or_empty(&input_in_a_range))
		{
			ASSERT_TRUE(string_replace(input_in_a_range.start, input_in_a_range.finish,
									   &n, &n + 1, nullptr, nullptr, tmp)) << buffer_free(tmp);
		}

		ASSERT_EQ(expected_output, i - 1) << buffer_free(tmp);
		ASSERT_EQ(buffer_to_string(tmp), returned_output) << buffer_free(tmp);
		//
		--node_count;
	}

	buffer_release(tmp);
}
#endif
TEST_F(TestFileSystem, file_get_attributes)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const auto path = reinterpret_cast<const uint8_t*>(input.c_str());

		if (!file_exists(path))
		{
			continue;
		}

		ASSERT_LE(0, file_get_creation_time(path)) << input;
		ASSERT_LE(0, file_get_creation_time_utc(path)) << input;
		ASSERT_LE(0, file_get_last_access_time(path)) << input;
		ASSERT_LE(0, file_get_last_access_time_utc(path)) << input;
		ASSERT_LE(0, file_get_last_write_time(path)) << input;
		ASSERT_LE(0, file_get_last_write_time_utc(path)) << input;
		//
		ASSERT_LT(0, file_get_length(path)) << input;
		//
		node_count = 0;
		break;
	}
}
#if 0
TEST(TestFileSystem_, file_move)
{
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));

	for (uint8_t i = 0; i < 2; ++i)
	{
		ASSERT_TRUE(buffer_resize(path, 0)) << buffer_free(path);
		//
		ASSERT_TRUE(path_get_temp_file_name(path)) << buffer_free(path);
		ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
		//
		const auto size = buffer_size(path);
		//
		ASSERT_TRUE(path_get_temp_path(path)) << buffer_free(path);
		ASSERT_TRUE(buffer_push_back(path, PATH_DELIMITER)) << buffer_free(path);
		ASSERT_TRUE(get_crc32_of(datetime_now_utc() + __LINE__, path)) << buffer_free(path);
		ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);

		if (2 * size == buffer_size(path))
		{
			auto returned = memcmp(buffer_char_data(path, 0), buffer_char_data(path, size), size);
			ASSERT_TRUE(returned) << buffer_free(path);
		}

		const auto path_1 = buffer_uint8_t_data(path, 0);
		const auto path_2 = buffer_uint8_t_data(path, size);

		if (i)
		{
			ASSERT_TRUE(echo(0, Default, path_1, Info, path_1, size, 0, 0))
					<< buffer_free(path);
		}
		else
		{
			if (!file_exists(path_1))
			{
				ASSERT_TRUE(file_create(path_1)) << buffer_free(path);
			}
		}

		ASSERT_TRUE(file_exists(path_1)) << buffer_free(path);
		ASSERT_FALSE(file_exists(path_2)) << buffer_free(path);
		ASSERT_TRUE(file_move(path_1, path_2)) << buffer_free(path);
		ASSERT_TRUE(file_exists(path_2)) << buffer_free(path);
		ASSERT_FALSE(file_exists(path_1)) << buffer_free(path);
		//
		ASSERT_TRUE(file_delete(path_2)) << buffer_free(path);
		ASSERT_FALSE(file_exists(path_2)) << buffer_free(path);
	}

	buffer_release(path);
}
#endif
TEST(TestFileSystem_, file_up_to_date)
{
	std::string paths_buffer(buffer_size_of(), 0);
	auto paths = reinterpret_cast<void*>(&paths_buffer[0]);
	ASSERT_TRUE(buffer_init(paths, buffer_size_of()));
	//
	ASSERT_TRUE(path_get_temp_file_name(paths)) << buffer_free(paths);
	ASSERT_TRUE(buffer_push_back(paths, 0)) << buffer_free(paths);
	//
	auto src_file = buffer_uint8_t_data(paths, 0);

	if (!file_exists(src_file))
	{
		ASSERT_TRUE(file_create(src_file)) << buffer_free(paths);
	}

	const auto size = buffer_size(paths);
	ASSERT_TRUE(path_get_temp_file_name(paths)) << buffer_free(paths);
	ASSERT_TRUE(buffer_push_back(paths, 0)) << buffer_free(paths);
	//
	auto target_file = buffer_uint8_t_data(paths, size);

	if (!file_exists(target_file))
	{
		ASSERT_TRUE(file_create(target_file)) << buffer_free(paths);
	}

	src_file = buffer_uint8_t_data(paths, 0);
	target_file = buffer_uint8_t_data(paths, size);
	ASSERT_TRUE(file_up_to_date(src_file, target_file)) << buffer_free(paths);
	//
	ASSERT_TRUE(file_delete(src_file)) << buffer_free(paths);
	ASSERT_TRUE(file_delete(target_file)) << buffer_free(paths);
	//
	buffer_release(paths);
}

/*TEST_F(TestFileSystem, file_replace)
{
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		ASSERT_TRUE(path_get_temp_file_name(output)) << buffer_free(output);
		//
		const auto tmp_path(buffer_to_string(output));
		ASSERT_TRUE(buffer_push_back(output, 0)) << buffer_free(output);
		//
		const auto path = buffer_uint8_t_data(output, 0);
		//
		returned = echo(0, Default, path, Info,
						input_in_a_range.start, range_size(&input_in_a_range), 0, 0);
		//
		ASSERT_TRUE(returned) << tmp_path << std::endl << buffer_free(output);
		//
		returned = file_replace(path,
								to_be_replaced_in_a_range.start, to_be_replaced_in_a_range.finish,
								by_replacement_in_a_range.start, by_replacement_in_a_range.finish);
		//
		ASSERT_EQ(expected_return, returned) << tmp_path << std::endl << buffer_free(output);
		//
		returned = load_file_to_buffer(path, Default, output, 0);
		ASSERT_TRUE(returned) << tmp_path << std::endl << buffer_free(output);
		ASSERT_EQ(expected_output, buffer_to_string(output)) << tmp_path << std::endl << buffer_free(output);
}*/

TEST(TestFileSystem_, file_set_attributes)
{
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));
	//
	ASSERT_TRUE(path_get_temp_file_name(path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	const auto path_ = buffer_uint8_t_data(path, 0);

	if (!file_exists(path_))
	{
		ASSERT_TRUE(file_create(path_)) << buffer_free(path);
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
										path_, archive, hidden, normal, readonly, system_attribute)) << buffer_free(path);
#if TODO
						unsigned long attributes = 0;
						ASSERT_TRUE(file_get_attributes(path_, &attributes)) << buffer_free(path);
						//
						ASSERT_EQ(archive, IS_ARCHIVE(attributes)) << buffer_free(path);
						ASSERT_EQ(hidden, IS_HIDDEN(attributes)) << buffer_free(path);
						ASSERT_EQ(normal, IS_NORMAL(attributes)) << buffer_free(path);
						ASSERT_EQ(readonly, IS_READ_ONLY(attributes)) << buffer_free(path);
						ASSERT_EQ(system_attribute, IS_SYSTEM(attributes)) << buffer_free(path);
#endif
					}
				}
			}
		}
	}

	ASSERT_FALSE(file_delete(path_)) << buffer_free(path);
	ASSERT_TRUE(file_set_attributes(path_, 0, 0, 1, 0, 0)) << buffer_free(path);
#endif
	ASSERT_TRUE(file_delete(path_)) << buffer_free(path);
	//
	buffer_release(path);
}

TEST(TestFileSystem_, file_set_last_access_time)
{
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));
	//
	ASSERT_TRUE(path_get_temp_file_name(path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	const auto path_ = buffer_uint8_t_data(path, 0);

	if (!file_exists(path_))
	{
		ASSERT_TRUE(file_create(path_)) << buffer_free(path);
	}

	const int64_t time_to_set = 1569840495;
	ASSERT_TRUE(file_set_last_access_time_utc(path_, time_to_set)) << buffer_free(path);
	ASSERT_TRUE(file_set_last_access_time(path_, time_to_set)) << buffer_free(path);
	const auto time = file_get_last_access_time(path_);
	//
	ASSERT_EQ(time_to_set, time) << buffer_free(path);
	//
	ASSERT_TRUE(file_delete(path_)) << buffer_free(path);
	buffer_release(path);
}

TEST(TestFileSystem_, file_set_last_write_time)
{
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));
	//
	ASSERT_TRUE(path_get_temp_file_name(path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	const auto path_ = buffer_uint8_t_data(path, 0);

	if (!file_exists(path_))
	{
		ASSERT_TRUE(file_create(path_)) << buffer_free(path);
	}

	const int64_t time_to_set = 1569840495;
	ASSERT_TRUE(file_set_last_write_time_utc(path_, time_to_set)) << buffer_free(path);
	ASSERT_TRUE(file_set_last_write_time(path_, time_to_set)) << buffer_free(path);
	const auto time = file_get_last_write_time(path_);
	//
	ASSERT_EQ(time_to_set, time) << buffer_free(path);
	//
	ASSERT_TRUE(file_set_creation_time_utc(path_, time_to_set)) << buffer_free(path);
	ASSERT_TRUE(file_set_creation_time(path_, time_to_set)) << buffer_free(path);
	//
	ASSERT_TRUE(file_delete(path_)) << buffer_free(path);
	buffer_release(path);
}

TEST(TestFileSystem_, file_write_all_bytes)
{
	static const uint16_t content_size[] = { 0, INT8_MAX, UINT8_MAX, 4096, 7000, 8192, 9000 };
	//
	std::string path_buffer(buffer_size_of(), 0);
	auto path = reinterpret_cast<void*>(&path_buffer[0]);
	ASSERT_TRUE(buffer_init(path, buffer_size_of()));
	//
	ASSERT_TRUE(path_get_temp_file_name(path)) << buffer_free(path);
	ASSERT_TRUE(buffer_push_back(path, 0)) << buffer_free(path);
	//
	const auto path_str(buffer_to_string(path));
	const auto* path_ = reinterpret_cast<const uint8_t*>(path_str.c_str());

	for (uint8_t i = 0, count = COUNT_OF(content_size); i < count; ++i)
	{
		static const uint8_t value = '\0';
		//
		ASSERT_TRUE(buffer_resize(path, 0)) << buffer_free(path);
		const auto returned =
			string_pad_left(&value, &value, &value, &value + 1, content_size[i], path);
		ASSERT_TRUE(returned) << buffer_free(path);
		//
		auto size = buffer_size(path);
		ASSERT_EQ(content_size[i], size) << buffer_free(path);
		//
		ASSERT_TRUE(file_write_all(path_, path));
		//
		size = static_cast<ptrdiff_t>(file_get_length(path_));
		ASSERT_EQ(content_size[i], size) << buffer_free(path);
	}

	ASSERT_TRUE(file_delete(path_)) << buffer_free(path);
	ASSERT_FALSE(file_exists(path_)) << buffer_free(path);
	buffer_release(path);
}
