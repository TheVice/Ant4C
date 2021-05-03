/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 TheVice
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

protected:
	TestProject() :
		TestsBaseXml()
	{
		predefine_arguments.insert(std::make_pair("--tests_base_directory=", &tests_base_directory));
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
		const auto code_in_range = string_to_range(code);
		//
		buffer project_;
		SET_NULL_TO_BUFFER(project_);
		void* the_project = &project_;
		//
		ASSERT_TRUE(project_new(the_project)) << project_free(the_project);
		ASSERT_NE(nullptr, the_project) << project_free(the_project);
		//
		ASSERT_NE(code.empty(), project_load_from_content(
					  code_in_range.start, code_in_range.finish, the_project, 0, verbose)) << project_free(the_project);

		for (const auto& property : node.node().select_nodes("property"))
		{
			std::string property_name;
			std::string property_value;
			uint8_t dynamic = 0;
			uint8_t over_write = 0;
			uint8_t read_only = 0;
			uint8_t fail_on_error = 0;
			uint8_t property_verbose = 0;
			//
			property_load_from_node(property.node(), property_name, property_value, dynamic,
									over_write, read_only, fail_on_error, property_verbose);
			//
			property_verbose = MAX(verbose, property_verbose);
			//
			const auto expected_return = (uint8_t)INT_PARSE(property.node().select_node("return").node().child_value());
			//
			auto returned = project_property_set_value(the_project,
							(const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(),
							(const uint8_t*)property_value.c_str(), property_value.size(),
							dynamic, over_write, read_only, property_verbose);
			//
			ASSERT_EQ(expected_return, returned) << project_free(the_project);
			//
			void* the_property = NULL;
			returned = project_property_exists(
						   the_project, (const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(), &the_property,
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
		const std::string content(node.node().select_node("content").node().child_value());
		range content_in_range;
		const auto project_help = (uint8_t)INT_PARSE(
									  node.node().select_node("project_help").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		uint8_t expected_target_return = expected_return;
		const auto target_return_node = node.node().select_node("target_return").node();

		if (!target_return_node.empty())
		{
			expected_target_return = (uint8_t)INT_PARSE(target_return_node.child_value());
		}

		const std::string expected_name(node.node().select_node("name").node().child_value());
		const std::string expected_default_target(node.node().select_node("default").node().child_value());
		const std::string target_to_run(node.node().select_node("target_to_run").node().child_value());
		//
		std::string expected_base_directory(node.node().select_node("base_directory").node().child_value());

		if (!expected_base_directory.empty())
		{
			ASSERT_TRUE(buffer_resize(&output, 0))
					<< buffer_free(&output);
			content_in_range = string_to_range(expected_base_directory);
			ASSERT_TRUE(interpreter_evaluate_code(NULL, NULL, &content_in_range, &output, verbose))
					<< buffer_free(&output);
			expected_base_directory = buffer_to_string(&output);
		}

		const auto properties = node.node().select_nodes("property");
		const auto targets = node.node().select_nodes("target");
		//
		content_in_range = string_to_range(content);
		ASSERT_EQ(content.empty(), range_is_null_or_empty(&content_in_range))
				<< buffer_free(&output);
		//
		buffer project_;
		SET_NULL_TO_BUFFER(project_);
		void* the_project = &project_;
		//
		ASSERT_TRUE(project_new(the_project))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned = project_load_from_content(content_in_range.start, content_in_range.finish,
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
			const auto target_to_run_in_range(string_to_range(target_to_run));
			ASSERT_EQ(expected_target_return, target_evaluate_by_name(the_project, &target_to_run_in_range, verbose))
					<< content << std::endl << buffer_free(&output) << project_free(the_project);
		}

		const void* the_property = NULL;

		if (!expected_base_directory.empty())
		{
			ASSERT_NE(content.empty(), project_get_base_directory(the_project, &the_property, verbose))
					<< buffer_free(&output) << project_free(the_project);
			//
			const auto returned_base_directory(property_to_string(the_property, &output));
			ASSERT_EQ(expected_base_directory, returned_base_directory)
					<< buffer_free(&output) << project_free(the_project);
		}

		the_property = NULL;
		ASSERT_EQ(!expected_name.empty(), project_get_name(the_project, &the_property, verbose))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned_name(property_to_string(the_property, &output));
		ASSERT_EQ(expected_name, returned_name)
				<< buffer_free(&output) << project_free(the_project);
		//
		the_property = NULL;
		ASSERT_EQ(!expected_default_target.empty(), project_get_default_target(the_project, &the_property, verbose))
				<< buffer_free(&output) << project_free(the_project);
		//
		const auto returned_default_target(property_to_string(the_property, &output));
		ASSERT_EQ(expected_default_target, returned_default_target)
				<< buffer_free(&output) << project_free(the_project);
		//
		static const uint8_t asterisk = '*';

		if (!project_target_exists(the_project, &asterisk, 1))
		{
			ASSERT_NE(expected_default_target.empty(), project_target_exists(the_project,
					  (const uint8_t*)expected_default_target.c_str(), (uint8_t)expected_default_target.size()))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< expected_default_target;
		}

		for (const auto& property_node : properties)
		{
			void* the_project_property = NULL;
			const std::string property_name(property_node.node().attribute("name").as_string());
			//
			ASSERT_TRUE(project_property_exists(the_project,
												(const uint8_t*)property_name.c_str(),
												(uint8_t)property_name.size(),
												&the_project_property, verbose))
					<< property_name << std::endl
					<< buffer_free(&output)
					<< project_free(the_project);
			//
			const uint8_t expected_is_dynamic = property_node.node().attribute("dynamic").as_bool();
			const uint8_t expected_is_read_only = property_node.node().attribute("readonly").as_bool();
			const std::string expected_property_value(property_node.node().attribute("value").as_string());
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
			ASSERT_TRUE(project_target_exists(the_project, (const uint8_t*)target_name_str.c_str(),
											  (uint8_t)target_name_str.size()))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< "Target name - '" << target_name_str << "'." << std::endl;
			//
			void* target = NULL;
			ASSERT_TRUE(project_target_get(the_project, (const uint8_t*)target_name_str.c_str(),
										   (uint8_t)target_name_str.size(), &target, verbose))
					<< buffer_free(&output) << project_free(the_project) << std::endl
					<< "Target name - '" << target_name_str << "'." << std::endl;
			//
			uint16_t index = 0;
			const range* depend_target_name = NULL;

			while (NULL != (depend_target_name = target_get_depend(target, index++)))
			{
				target_name_str = range_to_string(depend_target_name);
				ASSERT_TRUE(project_target_exists(the_project, (const uint8_t*)target_name_str.c_str(),
												  (uint8_t)target_name_str.size()))
						<< buffer_free(&output) << project_free(the_project) << std::endl
						<< "Target name - '" << target_name_str << "'." << std::endl;
			}
		}

		project_unload(the_project);
		the_project = NULL;
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
	const auto current_path_in_range(string_to_range(tests_base_directory));
	//
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		const auto file_node = node.node().select_node("file");

		if (file_node.node().empty())
		{
			std::cerr << "[Warning]: skip test case, no input data." << std::endl;
			//
			--node_count;
			continue;
		}

		std::string path(file_node.node().attribute("path").as_string());
		auto path_in_range = string_to_range(path);
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0))
				<< buffer_free(&tmp);
		ASSERT_TRUE(path_combine(
						current_path_in_range.start,
						current_path_in_range.finish,
						path_in_range.start,
						path_in_range.finish,
						&tmp))
				<< buffer_free(&tmp);
		//
		path = buffer_to_string(&tmp);

		if (!file_exists((const uint8_t*)path.c_str()))
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
				file_node.node().attribute(hash.c_str()).as_string());

			if (expected_hash_value.empty())
			{
				continue;
			}

			const auto algorithm = string_to_range(hash);
			const std::string algorithm_parameter_str(file_node.node().attribute("algorithm_parameter").as_string());
			const auto algorithm_parameter = string_to_range(algorithm_parameter_str);
			//
			ASSERT_TRUE(buffer_resize(&tmp, 0))
					<< buffer_free(&tmp);
			ASSERT_TRUE(file_get_checksum((const uint8_t*)path.c_str(),
										  &algorithm, &algorithm_parameter, &tmp))
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

		const auto content = node.node().select_nodes("content");

		if (!content.empty())
		{
			std::map<uint16_t, std::string> new_content;

			for (const auto& line : content)
			{
				const auto line_number = (uint16_t)line.node().attribute("line").as_uint();

				if (0 == line_number)
				{
					continue;
				}

				if (new_content.count(line_number))
				{
					new_content[line_number] = line.node().child_value();
				}
				else
				{
					new_content.insert(std::make_pair(line_number, std::string(line.node().child_value())));
				}
			}

			ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
			ASSERT_TRUE(file_read_lines((const uint8_t*)path.c_str(), &tmp)) << buffer_free(&tmp);
			//
			path.clear();
			uint16_t i = 0;
			range* ptr = NULL;

			while (NULL != (ptr = buffer_range_data(&tmp, i++)))
			{
				if (new_content.count(i))
				{
					path += new_content[i];
					new_content.erase(i);
				}
				else
				{
					path.append((const char*)ptr->start, range_size(ptr));
				}

				path.push_back('\n');
			}

			ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
			ASSERT_TRUE(path_get_temp_file_name(&tmp)) << buffer_free(&tmp);
			ASSERT_TRUE(buffer_push_back(&tmp, 0)) << buffer_free(&tmp);
			//
			ASSERT_TRUE(echo(0, Default, buffer_data(&tmp, 0), Info,
							 (const uint8_t*)path.c_str(), (ptrdiff_t)path.size(),
							 0, verbose)) << buffer_free(&tmp);
			//
			path = buffer_to_string(&tmp);
		}

		const auto expected_return = (uint8_t)INT_PARSE(
										 node.node().select_node("return").node().child_value());
		//
		buffer project_;
		SET_NULL_TO_BUFFER(project_);
		void* the_project = &project_;
		//
		ASSERT_TRUE(project_new(the_project)) << buffer_free(&tmp) << project_free(the_project);
		//
		ASSERT_TRUE(project_property_set_value(
						the_project,
						(const uint8_t*)tests_base_property.c_str(),
						(uint8_t)tests_base_property.size(),
						(const uint8_t*)tests_base_directory.c_str(),
						(ptrdiff_t)tests_base_directory.size(),
						0, 0, 1,
						verbose))
				<< path << std::endl
				<< buffer_free(&tmp) << project_free(the_project);
		//
		const auto properties = node.node().select_nodes("property");

		for (const auto& property_node : properties)
		{
			const std::string property_name(property_node.node().attribute("name").as_string());
			const std::string property_value(property_node.node().attribute("value").as_string());
			//
			ASSERT_TRUE(project_property_set_value(
							the_project,
							(const uint8_t*)property_name.c_str(),
							(uint8_t)property_name.size(),
							(const uint8_t*)property_value.c_str(),
							(ptrdiff_t)property_value.size(),
							0, 0, 1,
							verbose))
					<< path << std::endl
					<< property_name << std::endl
					<< property_value << std::endl
					<< buffer_free(&tmp) << project_free(the_project);
		}

		const auto project_help = (uint8_t)INT_PARSE(
									  node.node().select_node("project_help").node().child_value());
		//
		std::cout << "[ RUN      ]" << std::endl;
		//
		path_in_range = string_to_range(path);
		auto returned = project_load_from_build_file(
							&path_in_range, &current_path_in_range,
							Default, the_project, project_help, verbose);
		ASSERT_EQ(expected_return, returned)
				<< path << std::endl << buffer_free(&tmp) << project_free(the_project);
		//
		const void* the_property = NULL;
		ASSERT_TRUE(project_get_buildfile_path(the_project, &the_property, verbose))
				<< path << buffer_free(&tmp) << project_free(the_project);
		//
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << path << buffer_free(&tmp) << project_free(the_project);
		ASSERT_TRUE(project_get_buildfile_uri(the_property, &tmp, verbose))
				<< path << buffer_free(&tmp) << project_free(the_project);
		ASSERT_TRUE(buffer_size(&tmp)) << path << buffer_free(&tmp) << project_free(the_project);
		//
		uint8_t expected_target_return = expected_return;
		const auto target_return_node = node.node().select_node("target_return").node();

		if (!target_return_node.empty())
		{
			expected_target_return = (uint8_t)INT_PARSE(target_return_node.child_value());
		}

		const std::string target_to_run(node.node().select_node("target_to_run").node().child_value());

		if (target_to_run.empty())
		{
			returned = project_evaluate_default_target(the_project, verbose);
			ASSERT_EQ(expected_target_return, returned)
					<< path << std::endl << buffer_free(&tmp) << project_free(the_project);
		}
		else
		{
			const auto target_to_run_in_range(string_to_range(target_to_run));
			returned = target_evaluate_by_name(the_project, &target_to_run_in_range, verbose);
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
	uint32_t hash = 0;
	//
	ASSERT_TRUE(hash_algorithm_crc32((const uint8_t*)&now, (const uint8_t*)&now + sizeof(now), &hash, 1));
	ASSERT_TRUE(hash_algorithm_bytes_to_string(
					(const uint8_t*)&hash, (const uint8_t*)&hash + sizeof(hash), &directory)) <<
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
	std::vector<std::string> expected_files(5);
	expected_files.clear();

	for (i = 0; i < 5; ++i)
	{
		const auto start = buffer_data(&directory, 0);
		const auto finish = start + buffer_size(&directory);
		//
		ASSERT_TRUE(hash_algorithm_crc32(start, finish, &hash, 1));
		//
		ASSERT_TRUE(buffer_resize(&directory, size)) <<
				buffer_free(&directory);
		ASSERT_TRUE(buffer_push_back(&directory, PATH_DELIMITER)) <<
				buffer_free(&directory);
		//
		const auto minimal_size = buffer_size(&directory);
		//
		ASSERT_TRUE(hash_algorithm_bytes_to_string(
						(const uint8_t*)&hash, (const uint8_t*)&hash + sizeof(hash), &directory)) <<
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
		expected_files.push_back(std::string((const char*)path, buffer_size(&directory) - 1));
	}

	ASSERT_TRUE(buffer_resize(&directory, size)) <<
			buffer_free(&directory);
	//
	buffer argument_value;
	SET_NULL_TO_BUFFER(argument_value);
	//
	buffer command_arguments;
	SET_NULL_TO_BUFFER(command_arguments);
	//
	ASSERT_TRUE(project_get_build_files_from_directory(
					&command_arguments, &argument_value, &directory, verbose)) <<
							buffer_free(&directory) << buffer_free(&argument_value) <<
							properties_free(&command_arguments);
	//
	ASSERT_EQ(size + 1, buffer_size(&directory)) <<
			buffer_free(&directory) << buffer_free(&argument_value) <<
			properties_free(&command_arguments);
	//
	ASSERT_EQ(0, *buffer_data(&directory, size)) <<
			buffer_free(&directory) << buffer_free(&argument_value) <<
			properties_free(&command_arguments);
	//
	ASSERT_TRUE(buffer_resize(&directory, size)) <<
			buffer_free(&directory) << buffer_free(&argument_value) <<
			properties_free(&command_arguments);
	//
	ASSERT_EQ(expected_current_directory, buffer_to_string(&directory)) <<
			buffer_free(&directory) << buffer_free(&argument_value) <<
			properties_free(&command_arguments);
	//
	i = 0;
	std::string build_file;

	while (!(build_file = range_to_string(argument_parser_get_build_file(&command_arguments, &argument_value,
										  i++))).empty())
	{
#if defined(__GNUC__) && (__GNUC__ < 5)
		const auto fonded_path = std::find_if(expected_files.begin(),
											  expected_files.end(), [&build_file](const std::string & expected_path)
#else
		const auto fonded_path = std::find_if(expected_files.cbegin(),
											  expected_files.cend(), [&build_file](const std::string & expected_path)
#endif
		{
			return build_file == expected_path;
		});
		//
		ASSERT_NE(fonded_path, expected_files.end()) <<
				build_file << std::endl <<
				buffer_free(&directory) << buffer_free(&argument_value) <<
				properties_free(&command_arguments);
		//
		expected_files.erase(fonded_path);
	}

	ASSERT_TRUE(expected_files.empty()) << expected_files.size() << std::endl << i << std::endl <<
										buffer_free(&directory) << buffer_free(&argument_value) <<
										properties_free(&command_arguments);
	//
	ASSERT_EQ(nullptr, argument_parser_get_build_file(&command_arguments, &argument_value, i - 1)) <<
			i << std::endl <<
			buffer_free(&directory) << buffer_free(&argument_value) <<
			properties_free(&command_arguments);
	ASSERT_EQ(nullptr, argument_parser_get_build_file(&command_arguments, &argument_value, i)) <<
			i << std::endl <<
			buffer_free(&directory) << buffer_free(&argument_value) <<
			properties_free(&command_arguments);
	//
	buffer_release(&directory);
	buffer_release(&argument_value);
	property_release(&command_arguments);
	//
	ASSERT_TRUE(directory_delete((const uint8_t*)expected_current_directory.c_str())) <<
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
		const std::string input(node.node().select_node("input").node().child_value());
		const auto input_in_range(string_to_range(input));
		//
		ASSERT_TRUE(xml_get_sub_nodes_elements(
						input_in_range.start, input_in_range.finish, NULL, &properties_elements))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);
		//
		ASSERT_TRUE(program_get_properties(the_project, NULL, &properties_elements, &properties, 1, 0))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);
		//
		ASSERT_TRUE(buffer_resize(&properties_elements, 0))
				<< buffer_free(&properties_elements)
				<< properties_free(&properties)
				<< project_free(the_project);

		for (const auto& property_node : node.node().select_nodes("properties/property"))
		{
			void* the_property = NULL;
			const std::string property_name(property_node.node().attribute("name").as_string());
			//
			ASSERT_TRUE(property_exists(
							&properties,
							(const uint8_t*)property_name.c_str(), (uint8_t)property_name.size(), &the_property))
					<< property_name << std::endl
					<< buffer_free(&properties_elements)
					<< properties_free(&properties)
					<< project_free(the_project);
			//
			const uint8_t expected_is_dynamic = property_node.node().attribute("dynamic").as_bool();
			const uint8_t expected_is_read_only = property_node.node().attribute("readonly").as_bool();
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
