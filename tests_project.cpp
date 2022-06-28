/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "echo.h"
#include "file_system.h"
#include "hash.h"
#include "interpreter.h"
#include "load_file.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "target.h"
#include "text_encoding.h"
#include "xml.h"
};

#include <map>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <utility>
#include <iostream>
#include <algorithm>

extern "C" {
	extern uint8_t program_get_properties(
		const void* the_project,
		const void* the_target,
		const buffer* properties_elements,
		buffer* properties,
		uint8_t is_root, uint8_t verbose);
};

class TestProject : public TestsBaseXml
{
protected:
	static std::string tests_base_directory;

	uint8_t verbose;

protected:
	TestProject() :
		TestsBaseXml(),
		verbose()
	{
		predefine_arguments.emplace(std::make_pair("--tests_base_directory=", &tests_base_directory));
	}

	virtual void SetUp() override
	{
		TestsBaseXml::SetUp();

		if (tests_base_directory.empty())
		{
			auto result = parse_input_arguments();
			assert(result);
			ASSERT_TRUE(result);
			//
			result = tests_base_directory.empty();
			assert(!result);
			ASSERT_FALSE(result);
		}
	}
};

std::string TestProject::tests_base_directory;

TEST_F(TestProject, project_property_set_value)
{
	for (const auto& node : nodes)
	{
		const auto code(get_data_from_nodes(node, "code"));
		//
		const auto code_in_a_range = string_to_range(code);
		//
		buffer project_;
		SET_NULL_TO_BUFFER(project_);
		void* the_project = &project_;
		//
		ASSERT_TRUE(project_new(the_project)) << project_free(the_project);
		ASSERT_NE(nullptr, the_project) << project_free(the_project);
		//
		ASSERT_NE(code.empty(), project_load_from_content(
					  code_in_a_range.start, code_in_a_range.finish, the_project, 0, verbose)) << project_free(the_project);

		for (const auto& the_property_ : node.node().select_nodes("property"))
		{
			void* the_property = nullptr;
			//
			std::string property_name;
			std::string property_value;
			uint8_t dynamic = 0;
			uint8_t over_write = 0;
			uint8_t read_only = 0;
			uint8_t fail_on_error = 0;
			uint8_t property_verbose = 0;
			//
			const auto property_node = the_property_.node();
			//
			property_load_from_node(property_node, property_name, property_value, dynamic,
									over_write, read_only, fail_on_error, property_verbose);
			//
			property_verbose = MAX(verbose, property_verbose);
			//
			const std::string return_str(property_node.select_node("return").node().child_value());
			auto input_in_a_range = string_to_range(return_str);
			const auto expected_return =
				static_cast<uint8_t>(
					int_parse(input_in_a_range.start, input_in_a_range.finish));
			//
			auto returned = project_property_set_value(the_project,
							reinterpret_cast<const uint8_t*>(property_name.c_str()),
							static_cast<uint8_t>(property_name.size()),
							reinterpret_cast<const uint8_t*>(property_value.c_str()),
							property_value.size(),
							dynamic, over_write, read_only, property_verbose);
			//
			ASSERT_EQ(expected_return, returned) << project_free(the_project);
			//
			returned = project_property_exists(
						   the_project,
						   reinterpret_cast<const uint8_t*>(property_name.c_str()),
						   static_cast<uint8_t>(property_name.size()), &the_property,
						   property_verbose);
			//
			ASSERT_EQ(expected_return, returned) << project_free(the_project);

			if (expected_return)
			{
				uint8_t returned_dynamic = 0;
				uint8_t returned_read_only = 0;
				//
				//ASSERT_TRUE(property_get_by_pointer(the_property, returned_value));
				ASSERT_TRUE(property_is_dynamic(the_property, &returned_dynamic)) << project_free(the_project);
				ASSERT_TRUE(property_is_readonly(the_property, &returned_read_only)) << project_free(the_project);
				//
				ASSERT_EQ(dynamic, returned_dynamic) << project_free(the_project);
				ASSERT_EQ(read_only, returned_read_only) << project_free(the_project);
			}
		}

		project_unload(the_project);
		//
		--node_count;
	}
}

TEST_F(TestProject, project_load_from_content)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		std::cout << "[ RUN      ]" << std::endl;
		//
		const auto the_node = node.node();
		//
		const std::string content(the_node.select_node("content").node().child_value());
		//
		const std::string project_help_str(
			the_node.select_node("project_help").node().child_value());
		auto input_in_a_range = string_to_range(project_help_str);
		const auto project_help =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string return_str(the_node.select_node("return").node().child_value());
		input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		auto expected_target_return = expected_return;
		const auto target_return_node = the_node.select_node("target_return").node();

		if (!target_return_node.empty())
		{
			const std::string child_value(target_return_node.child_value());
			input_in_a_range = string_to_range(child_value);
			expected_target_return =
				static_cast<uint8_t>(
					int_parse(input_in_a_range.start, input_in_a_range.finish));
		}

		const std::string expected_name(the_node.select_node("name").node().child_value());
		const std::string expected_default_target(the_node.select_node("default").node().child_value());
		const std::string target_to_run(the_node.select_node("target_to_run").node().child_value());
		const auto expected_default_target_in_a_range(string_to_range(expected_default_target));
		//
		std::string expected_base_directory(the_node.select_node("base_directory").node().child_value());

		if (!expected_base_directory.empty())
		{
			ASSERT_TRUE(buffer_resize(&output, 0))
					<< buffer_free(&output);
			input_in_a_range = string_to_range(expected_base_directory);
			ASSERT_TRUE(interpreter_evaluate_code(
							nullptr, nullptr, nullptr, &input_in_a_range, &output, verbose))
					<< buffer_free(&output);
			expected_base_directory = buffer_to_string(&output);
		}

		const auto properties = the_node.select_nodes("property");
		const auto targets = the_node.select_nodes("target");
		//
		input_in_a_range = string_to_range(content);
		ASSERT_EQ(content.empty(), range_is_null_or_empty(&input_in_a_range))
				<< buffer_free(&output);
		//
		buffer project_;
		SET_NULL_TO_BUFFER(project_);
		void* the_project = &project_;
		//
		ASSERT_TRUE(project_new(the_project))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned = project_load_from_content(input_in_a_range.start, input_in_a_range.finish,
							  the_project, project_help, verbose);
		ASSERT_EQ(expected_return, returned)
				<< content << std::endl << buffer_free(&output) << project_free(the_project);

		if (target_to_run.empty())
		{
			ASSERT_EQ(expected_target_return, project_evaluate_default_target(the_project, verbose))
					<< content << std::endl << buffer_free(&output) << project_free(the_project);
		}
		else
		{
			const auto target_to_run_in_a_range(string_to_range(target_to_run));
			ASSERT_EQ(expected_target_return, target_evaluate_by_name(
						  the_project,
						  target_to_run_in_a_range.start,
						  target_to_run_in_a_range.finish,
						  verbose))
					<< content << std::endl << buffer_free(&output) << project_free(the_project);
		}

		const void* the_property = nullptr;

		if (!expected_base_directory.empty())
		{
			ASSERT_NE(content.empty(), project_get_base_directory(the_project, &the_property, verbose))
					<< buffer_free(&output) << project_free(the_project);
			//
			const auto returned_base_directory(property_to_string(the_property, &output));
			ASSERT_EQ(expected_base_directory, returned_base_directory)
					<< buffer_free(&output) << project_free(the_project);
		}

		the_property = nullptr;
		ASSERT_EQ(!expected_name.empty(), project_get_name(the_project, &the_property, verbose))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned_name(property_to_string(the_property, &output));
		ASSERT_EQ(expected_name, returned_name)
				<< buffer_free(&output) << project_free(the_project);
		//
		the_property = nullptr;
		ASSERT_EQ(!expected_default_target.empty(), project_get_default_target(the_project, &the_property, verbose))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned_default_target(property_to_string(the_property, &output));
		ASSERT_EQ(expected_default_target, returned_default_target)
				<< buffer_free(&output) << project_free(the_project);
		//
		static const uint8_t asterisk = '*';

		if (!project_target_exists(the_project, &asterisk, &asterisk + 1, verbose))
		{
			ASSERT_NE(expected_default_target.empty(),
					  project_target_exists(
						  the_project,
						  expected_default_target_in_a_range.start,
						  expected_default_target_in_a_range.finish, verbose))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< expected_default_target;
		}

		for (const auto& the_property_ : properties)
		{
			void* the_project_property = nullptr;
			const auto property_node = the_property_.node();
			const std::string property_name(property_node.attribute("name").as_string());
			//
			ASSERT_TRUE(project_property_exists(
							the_project,
							reinterpret_cast<const uint8_t*>(property_name.c_str()),
							static_cast<uint8_t>(property_name.size()),
							&the_project_property, verbose))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			const uint8_t expected_is_dynamic = property_node.attribute("dynamic").as_bool();
			const uint8_t expected_is_read_only = property_node.attribute("readonly").as_bool();
			const std::string expected_property_value(property_node.attribute("value").as_string());
			//
			uint8_t is_dynamic = 0;
			uint8_t is_read_only = 0;
			//
			ASSERT_TRUE(property_is_dynamic(the_project_property, &is_dynamic))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_is_dynamic, is_dynamic)
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_TRUE(property_is_readonly(the_project_property, &is_read_only))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_is_read_only, is_read_only)
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_TRUE(buffer_resize(&output, 0))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_TRUE(property_get_by_pointer(the_project_property, &output))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_property_value, buffer_to_string(&output))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
		}

		for (const auto& target_name : targets)
		{
			std::string target_name_str(target_name.node().child_value());
			input_in_a_range = string_to_range(target_name_str);
			//
			ASSERT_TRUE(project_target_exists(
							the_project,
							input_in_a_range.start,
							input_in_a_range.finish,
							verbose))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< "Target name - '" << target_name_str << "'." << std::endl;
			//
			void* target = nullptr;
			ASSERT_TRUE(project_target_get(
							the_project,
							input_in_a_range.start,
							input_in_a_range.finish,
							&target,
							verbose))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< "Target name - '" << target_name_str << "'." << std::endl;
			//
			uint16_t index = 0;
			const range* depend_target_name = nullptr;

			while (nullptr != (depend_target_name = target_get_depend(target, index++)))
			{
				target_name_str = range_to_string(depend_target_name);
				input_in_a_range = string_to_range(target_name_str);
				//
				ASSERT_TRUE(project_target_exists(
								the_project,
								input_in_a_range.start,
								input_in_a_range.finish,
								verbose))
						<< buffer_free(&output) << project_free(the_project) << std::endl
						<< "Target name - '" << target_name_str << "'." << std::endl;
			}
		}

		project_unload(the_project);
		the_project = nullptr;
		//
		std::cout << "[       OK ]" << std::endl;
		//
		--node_count;
	}

	buffer_release(&output);
}

TEST_F(TestProject, project_load_from_build_file)
{
	static const std::string hashes[] =
	{
		/*"sha3", "blake3",*/
		"blake2b", "crc32"
	};
	//
	static const std::string tests_base_property("tests_base_directory");
	const auto current_path_in_a_range(string_to_range(tests_base_directory));
	//
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		const auto file_node = the_node.select_node("file").node();

		if (file_node.empty())
		{
			std::cerr << "[Warning]: skip test case, no input data." << std::endl;
			//
			--node_count;
			continue;
		}

		std::string path(file_node.attribute("path").as_string());
		auto path_in_a_range = string_to_range(path);
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0))
				<< buffer_free(&tmp);
		ASSERT_TRUE(path_combine(
						current_path_in_a_range.start,
						current_path_in_a_range.finish,
						path_in_a_range.start,
						path_in_a_range.finish,
						&tmp))
				<< buffer_free(&tmp);
		//
		path = buffer_to_string(&tmp);

		if (!file_exists(reinterpret_cast<const uint8_t*>(path.c_str())))
		{
			std::cerr << "[Warning]: skip test case, file '"
					  << path << "' not exists." << std::endl;
			//
			--node_count;
			continue;
		}

		auto hash_exists = false;

		for (const auto& hash : hashes)
		{
			const std::string expected_hash_value(
				file_node.attribute(hash.c_str()).as_string());

			if (expected_hash_value.empty())
			{
				continue;
			}

			const auto algorithm = string_to_range(hash);
			const std::string algorithm_parameter_str(file_node.attribute("algorithm_parameter").as_string());
			const auto algorithm_parameter = string_to_range(algorithm_parameter_str);
			//
			ASSERT_TRUE(buffer_resize(&tmp, 0))
					<< buffer_free(&tmp);
			ASSERT_TRUE(file_get_checksum(
							reinterpret_cast<const uint8_t*>(path.c_str()),
							&algorithm,
							&algorithm_parameter,
							&tmp))
					<< path << std::endl << buffer_free(&tmp);
			//
			const std::string returned_hash(buffer_to_string(&tmp));
			ASSERT_EQ(expected_hash_value, returned_hash)
					<< path << std::endl << buffer_free(&tmp);
			//
			hash_exists = true;
			break;
		}

		if (!hash_exists)
		{
			std::cerr << "[Warning]: skip test case,"
					  << " no hash sum specific for the file '"
					  << path << "'." << std::endl;
			//
			--node_count;
			continue;
		}

		const auto content = the_node.select_nodes("content");

		if (!content.empty())
		{
			std::map<uint16_t, std::string> new_content;

			for (const auto& line : content)
			{
				const auto line_node = line.node();
				const auto line_number = static_cast<uint16_t>(line_node.attribute("line").as_uint());

				if (0 == line_number)
				{
					continue;
				}

				if (new_content.count(line_number))
				{
					new_content[line_number] = line_node.child_value();
				}
				else
				{
					new_content.emplace(std::make_pair(line_number, std::string(line_node.child_value())));
				}
			}

			ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
			ASSERT_TRUE(file_read_lines(reinterpret_cast<const uint8_t*>(path.c_str()), &tmp)) << buffer_free(&tmp);
			//
			path.clear();
			uint16_t i = 0;
			range* ptr = nullptr;

			while (nullptr != (ptr = buffer_range_data(&tmp, i++)))
			{
				if (new_content.count(i))
				{
					path += new_content[i];
					new_content.erase(i);
				}
				else
				{
					path.append(reinterpret_cast<const char*>(ptr->start), range_size(ptr));
				}

				path += '\n';
			}

			ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
			ASSERT_TRUE(path_get_temp_file_name(&tmp)) << buffer_free(&tmp);
			ASSERT_TRUE(buffer_push_back(&tmp, 0)) << buffer_free(&tmp);
			//
			ASSERT_TRUE(echo(0, Default, buffer_data(&tmp, 0), Info,
							 reinterpret_cast<const uint8_t*>(path.c_str()),
							 static_cast<ptrdiff_t>(path.size()),
							 0, verbose)) << buffer_free(&tmp);
			//
			path = buffer_to_string(&tmp);
		}

		const std::string return_str(the_node.select_node("return").node().child_value());
		path_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(path_in_a_range.start, path_in_a_range.finish));
		//
		buffer project_;
		SET_NULL_TO_BUFFER(project_);
		void* the_project = &project_;
		//
		ASSERT_TRUE(project_new(the_project)) << buffer_free(&tmp) << project_free(the_project);
		//
		ASSERT_TRUE(project_property_set_value(
						the_project,
						reinterpret_cast<const uint8_t*>(tests_base_property.c_str()),
						static_cast<uint8_t>(tests_base_property.size()),
						reinterpret_cast<const uint8_t*>(tests_base_directory.c_str()),
						static_cast<ptrdiff_t>(tests_base_directory.size()),
						0, 0, 1, verbose))
				<< path << std::endl
				<< buffer_free(&tmp) << project_free(the_project);
		//
		const auto properties = the_node.select_nodes("property");

		for (const auto& the_property : properties)
		{
			const auto property_node = the_property.node();
			//
			const std::string property_name(property_node.attribute("name").as_string());
			const std::string property_value(property_node.attribute("value").as_string());
			//
			ASSERT_TRUE(project_property_set_value(
							the_project,
							reinterpret_cast<const uint8_t*>(property_name.c_str()),
							static_cast<uint8_t>(property_name.size()),
							reinterpret_cast<const uint8_t*>(property_value.c_str()),
							static_cast<ptrdiff_t>(property_value.size()),
							0, 0, 1, verbose))
					<< path << std::endl
					<< property_name << std::endl
					<< property_value << std::endl
					<< buffer_free(&tmp) << project_free(the_project);
		}

		const std::string project_help_str(the_node.select_node("project_help").node().child_value());
		path_in_a_range = string_to_range(project_help_str);
		const auto project_help =
			static_cast<uint8_t>(
				int_parse(path_in_a_range.start, path_in_a_range.finish));
		//
		std::cout << "[ RUN      ]" << std::endl;
		//
		path_in_a_range = string_to_range(path);
		auto returned = project_load_from_build_file(
							&path_in_a_range, &current_path_in_a_range,
							Default, the_project, project_help, verbose);
		ASSERT_EQ(expected_return, returned)
				<< path << std::endl << buffer_free(&tmp) << project_free(the_project);
		//
		const void* the_property = nullptr;
		ASSERT_TRUE(project_get_buildfile_path(the_project, &the_property, verbose))
				<< path << buffer_free(&tmp) << project_free(the_project);
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << path << buffer_free(&tmp) << project_free(the_project);
		ASSERT_TRUE(project_get_buildfile_uri(the_property, &tmp, verbose))
				<< path << buffer_free(&tmp) << project_free(the_project);
		ASSERT_TRUE(buffer_size(&tmp)) << path << buffer_free(&tmp) << project_free(the_project);
		//
		auto expected_target_return = expected_return;
		const auto target_return_node = the_node.select_node("target_return").node();

		if (!target_return_node.empty())
		{
			const std::string child_value(target_return_node.child_value());
			path_in_a_range = string_to_range(child_value);
			expected_target_return =
				static_cast<uint8_t>(
					int_parse(path_in_a_range.start, path_in_a_range.finish));
		}

		const std::string target_to_run(the_node.select_node("target_to_run").node().child_value());

		if (target_to_run.empty())
		{
			returned = project_evaluate_default_target(the_project, verbose);
			ASSERT_EQ(expected_target_return, returned)
					<< path << std::endl << buffer_free(&tmp) << project_free(the_project);
		}
		else
		{
			const auto target_to_run_in_a_range(string_to_range(target_to_run));
			returned = target_evaluate_by_name(
						   the_project,
						   target_to_run_in_a_range.start,
						   target_to_run_in_a_range.finish,
						   verbose);
			ASSERT_EQ(expected_target_return, returned)
					<< path << std::endl << buffer_free(&tmp) << project_free(the_project);
		}

		project_unload(the_project);
		std::cout << "[       OK ]" << std::endl;
		//
		--node_count;
	}

	buffer_release(&tmp);
}

TEST(TestProject_, project_get_build_files_from_directory)
{
	static const uint8_t verbose = 0;
	//
	buffer directory;
	SET_NULL_TO_BUFFER(directory);
	//
	ASSERT_TRUE(path_get_temp_path(&directory)) <<
			buffer_free(&directory);
	ASSERT_TRUE(buffer_push_back(&directory, PATH_DELIMITER)) <<
			buffer_free(&directory);
	//
	const auto now = datetime_now();
	uint8_t hash[sizeof(uint64_t)];
	//
	ASSERT_TRUE(hash_algorithm_crc32(
					reinterpret_cast<const uint8_t*>(&now),
					reinterpret_cast<const uint8_t*>(&now) + sizeof(now), hash, 1)) <<
							buffer_free(&directory);
	ASSERT_TRUE(hash_algorithm_bytes_to_string(hash, hash + sizeof(hash), &directory)) <<
			buffer_free(&directory);
	//
	const auto size = buffer_size(&directory);
	const auto expected_current_directory(buffer_to_string(&directory));
	//
	ASSERT_TRUE(buffer_push_back(&directory, 0)) <<
			buffer_free(&directory);
	//
	ASSERT_TRUE(directory_create(buffer_data(&directory, 0))) <<
			buffer_free(&directory);
	//
	int i = 0;
	//
#define COUNT_OF_FILE 5
	std::vector<std::string> expected_files(COUNT_OF_FILE);
	expected_files.clear();

	for (i = 0; i < COUNT_OF_FILE; ++i)
	{
		const auto start = buffer_data(&directory, 0);
		const auto finish = start + buffer_size(&directory);
		//
		ASSERT_TRUE(hash_algorithm_crc32(start, finish, hash, 1));
		//
		ASSERT_TRUE(buffer_resize(&directory, size)) <<
				buffer_free(&directory);
		ASSERT_TRUE(buffer_push_back(&directory, PATH_DELIMITER)) <<
				buffer_free(&directory);
		//
		const auto minimal_size = buffer_size(&directory);
		//
		ASSERT_TRUE(hash_algorithm_bytes_to_string(hash, hash + sizeof(hash), &directory)) <<
				buffer_free(&directory);
		//
		const auto new_size = buffer_size(&directory) - i;
		//
		ASSERT_LT(minimal_size, new_size) <<
										  buffer_free(&directory);
		//
		ASSERT_TRUE(buffer_resize(&directory, new_size)) <<
				buffer_free(&directory);
		ASSERT_TRUE(buffer_append_char(&directory, ".build\0", 7)) <<
				buffer_free(&directory);
		//
		const auto path = buffer_data(&directory, 0);
		//
		ASSERT_TRUE(file_create(path)) <<
									   buffer_free(&directory);
		//
		expected_files.emplace_back(std::string(reinterpret_cast<const char*>(path), buffer_size(&directory) - 1));
	}

	ASSERT_TRUE(buffer_resize(&directory, size)) <<
			buffer_free(&directory);
	//
	ASSERT_TRUE(argument_parser_init()) <<
										buffer_free(&directory) << argument_parser_free();
	//
	ASSERT_TRUE(project_get_build_files_from_directory(&directory, verbose)) <<
			buffer_free(&directory) << argument_parser_free();
	//
	ASSERT_EQ(size + 1, buffer_size(&directory)) <<
			buffer_free(&directory) << argument_parser_free();
	//
	ASSERT_EQ(0, *buffer_data(&directory, size)) <<
			buffer_free(&directory) << argument_parser_free();
	//
	ASSERT_TRUE(buffer_resize(&directory, size)) <<
			buffer_free(&directory) << argument_parser_free();
	//
	ASSERT_EQ(expected_current_directory, buffer_to_string(&directory)) <<
			buffer_free(&directory) << argument_parser_free();
	//
	i = 0;
	const uint8_t* build_file;

	while (nullptr != (build_file = argument_parser_get_build_file(i++)))
	{
		const std::string build_file_str(reinterpret_cast<const char*>(build_file));
		const auto fonded_path = std::find_if(expected_files.cbegin(),
											  expected_files.cend(), [&build_file_str](const std::string & expected_path)
		{
			return build_file_str == expected_path;
		});
		//
		ASSERT_NE(fonded_path, expected_files.cend()) <<
				build_file_str << std::endl <<
				buffer_free(&directory) <<
				argument_parser_free();
#if defined(__GNUC__) && __GNUC__ <= 5
		auto fonded_path_ = expected_files.begin();
		std::advance(fonded_path_, std::distance(expected_files.cbegin(), fonded_path));
		expected_files.erase(fonded_path_);
#else
		expected_files.erase(fonded_path);
#endif
	}

	ASSERT_TRUE(expected_files.empty()) <<
										std::to_string(expected_files.size()) << std::endl <<
										i << std::endl <<
										buffer_free(&directory) <<
										argument_parser_free();
	//
	ASSERT_EQ(nullptr, argument_parser_get_build_file(i - 1)) <<
			i << std::endl <<
			buffer_free(&directory) <<
			argument_parser_free();
	ASSERT_EQ(nullptr, argument_parser_get_build_file(i)) <<
			i << std::endl <<
			buffer_free(&directory) <<
			argument_parser_free();
	//
	buffer_release(&directory);
	argument_parser_release();
	//
	ASSERT_TRUE(directory_delete(reinterpret_cast<const uint8_t*>(expected_current_directory.c_str()))) <<
			expected_current_directory;
}

class TestProgram : public TestsBaseXml
{
};

TEST_F(TestProgram, program_get_properties)
{
	buffer properties_elements;
	SET_NULL_TO_BUFFER(properties_elements);
	//
	buffer properties;
	SET_NULL_TO_BUFFER(properties);
	//
	buffer project_;
	SET_NULL_TO_BUFFER(project_);
	void* the_project = &project_;
	//
	ASSERT_TRUE(project_new(the_project))
			<< buffer_free(&properties_elements)
			<< properties_free(&properties)
			<< project_free(the_project);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const std::string input(the_node.select_node("input").node().child_value());
		const auto input_in_a_range(string_to_range(input));
		//
		ASSERT_TRUE(xml_get_sub_nodes_elements(
						input_in_a_range.start, input_in_a_range.finish, nullptr, &properties_elements))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);
		//
		ASSERT_TRUE(program_get_properties(the_project, nullptr, &properties_elements, &properties, 1, 0))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);
		//
		ASSERT_TRUE(buffer_resize(&properties_elements, 0))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);

		for (const auto& the_property_ : the_node.select_nodes("properties/property"))
		{
			void* the_property = nullptr;
			const auto property_node = the_property_.node();
			const std::string property_name(property_node.attribute("name").as_string());
			//
			ASSERT_TRUE(property_exists(
							&properties,
							reinterpret_cast<const uint8_t*>(property_name.c_str()),
							static_cast<uint8_t>(property_name.size()),
							&the_property))
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			const uint8_t expected_is_dynamic = property_node.attribute("dynamic").as_bool();
			const uint8_t expected_is_read_only = property_node.attribute("readonly").as_bool();
			//
			uint8_t is_dynamic = 0;
			uint8_t is_read_only = 0;
			//
			ASSERT_TRUE(property_is_dynamic(the_property, &is_dynamic))
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_is_dynamic, is_dynamic)
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			ASSERT_TRUE(property_is_readonly(the_property, &is_read_only))
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			ASSERT_EQ(expected_is_read_only, is_read_only)
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
		}

		property_release_inner(&properties);
		project_clear(the_project);
		//
		--node_count;
	}

	buffer_release(&properties_elements);
	property_release(&properties);
	project_unload(the_project);
}
