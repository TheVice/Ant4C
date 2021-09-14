/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "tests_exec.h"

extern "C" {
#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "exec.h"
#include "file_system.h"
#include "interpreter.h"
#include "path.h"
#include "project.h"
#include "property.h"
};

#include <string>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <utility>

extern "C" {
	extern uint8_t argument_get_key_and_value(
		const uint8_t* input_start, const uint8_t* input_finish,
		range* key, range* value);
	extern uint8_t exec_get_program_full_path(
		const void* the_project, const void* the_target,
		buffer* path_to_the_program, uint8_t is_path_rooted,
		const range* base_dir, buffer* tmp, uint8_t verbose);
};

std::string TestExec::tests_exec_app;

std::string TestExec::get_path_to_directory_with_image(buffer* tmp, uint8_t* result)
{
	static std::string path_to_directory_with_image;

	if (!tmp ||
		!result)
	{
		if (result)
		{
			*result = 0;
		}

		return path_to_directory_with_image;
	}

	if (path_to_directory_with_image.empty())
	{
		if (!path_get_directory_for_current_image(tmp))
		{
			std::cerr << "[Warning]: unable to get path to directory with image." << std::endl;
			std::cerr <<
					  "[Warning]: function 'path_get_directory_for_current_image' probably not implemented for used operation system."
					  << std::endl;

			if (!tests_exec_app.empty())
			{
				auto working_dir = string_to_range(tests_exec_app);

				if (!path_get_directory_name(
						working_dir.start, working_dir.finish, &working_dir))
				{
					*result = 0;
					return path_to_directory_with_image;
				}

				path_to_directory_with_image = range_to_string(working_dir);
			}
		}
		else
		{
			path_to_directory_with_image = buffer_to_string(tmp);
		}

		add_slash(path_to_directory_with_image);
	}

	*result = 1;
	return path_to_directory_with_image;
}

TestExec::TestExec() :
	TestsBaseXml(),
	append(),
	program_str(),
	program(),
	base_dir_str(),
	base_dir(),
	command_line_str(),
	command_line(),
	pid_property_str(),
	pid_property(),
	result_property_str(),
	result_property(),
	working_dir_str(),
	working_dir(),
	environment_variables_str(),
	environment_variables(),
	spawn(),
	time_out(),
	expected_return(),
	result_property_value(),
	allow_output_to_console()
{
	predefine_arguments.insert(std::make_pair("--tests_exec_app=", &tests_exec_app));
}

void TestExec::SetUp()
{
	auto init_required = tests_exec_app.empty();
	//
	TestsBaseXml::SetUp();

	if (tests_exec_app.empty())
	{
		init_required = parse_input_arguments();
		assert(init_required);
		ASSERT_TRUE(init_required);
		//
		init_required = tests_exec_app.empty();
		assert(!init_required);
		ASSERT_FALSE(init_required);
		//
		init_required = true;
	}

	if (init_required)
	{
		buffer tmp;
		SET_NULL_TO_BUFFER(tmp);
		//
		const auto path(string_to_range(tests_exec_app));
		ASSERT_TRUE(path_combine(path.start, path.finish, nullptr, nullptr, &tmp)) << buffer_free(&tmp);
		tests_exec_app = buffer_to_string(&tmp);
		//
		buffer_release(&tmp);
	}

	allow_output_to_console =
		nodes.cbegin()->parent().attribute("allow_output_to_console").as_bool();
}

void TestExec::load_input_data(const pugi::xpath_node& node)
{
	append = static_cast<uint8_t>(INT_PARSE(node.node().select_node("append").node().child_value()));
	//
	program_str = std::string(node.node().select_node("program").node().child_value());
	program = program_str.empty() ? string_to_range(tests_exec_app) : string_to_range(program_str);
	//
	base_dir_str = std::string(node.node().select_node("base_dir").node().child_value());
	base_dir = string_to_range(base_dir_str);
	//
	command_line_str = std::string(node.node().select_node("command_line").node().child_value());
	command_line = string_to_range(command_line_str);
	//
	pid_property_str = std::string(node.node().select_node("pid_property").node().child_value());
	pid_property = nullptr;
	//
	result_property_str = std::string(node.node().select_node("result_property").node().child_value());
	result_property = nullptr;
	//
	working_dir_str = std::string(node.node().select_node("working_dir").node().child_value());
	working_dir = string_to_range(working_dir_str);
	//
	environment_variables_str = std::string(
									node.node().select_node("environment_variables").node().child_value());
	environment_variables = string_to_range(environment_variables_str);
	//
	spawn = static_cast<uint8_t>(INT_PARSE(node.node().select_node("spawn").node().child_value()));
	time_out = INT_PARSE(node.node().select_node("time_out").node().child_value());
	verbose = static_cast<uint8_t>(INT_PARSE(node.node().select_node("verbose").node().child_value()));
	//
	expected_return = static_cast<uint8_t>(INT_PARSE(node.node().select_node("return").node().child_value()));
	//
	result_property_value = INT_PARSE(node.node().select_node("result_property_value").node().child_value());
}

static const uint8_t zero_symbol = '\0';
static const uint8_t quote_symbol = '"';
static const uint8_t equal_symbol = '=';
static const uint8_t space_symbol = ' ';

static uint8_t argument_get_keys_and_values(
	const uint8_t* input_start, const uint8_t* input_finish, buffer* output)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		nullptr == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, nullptr, input_finish - input_start + 1) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	const uint8_t* previous = input_start;

	while (input_finish != (input_start = find_any_symbol_like_or_not_like_that(input_start, input_finish,
										  &equal_symbol, 1, 1, 1)))
	{
		input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, &equal_symbol, 1, 0, 1);

		if (quote_symbol == *input_start)
		{
			input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, &quote_symbol, 1, 1, 1);
			input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, &quote_symbol, 1, 0, 1);
		}
		else
		{
			input_start = find_any_symbol_like_or_not_like_that(input_start, input_finish, &space_symbol, 1, 1, 1);
		}

		range key;
		range value;

		if (!argument_get_key_and_value(previous, input_start, &key, &value))
		{
			return 0;
		}

		if (!buffer_append_data_from_range(output, &key) ||
			!buffer_push_back(output, equal_symbol) ||
			!buffer_append_data_from_range(output, &value) ||
			!buffer_push_back(output, 0))
		{
			return 0;
		}

		input_start = find_any_symbol_like_or_not_like_that(input_start, input_finish, &space_symbol, 1, 1, 1);
		input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, &space_symbol, 1, 0, 1);
		previous = input_start;
	}

#if defined(_WIN32)

	if (size < buffer_size(output))
	{
		if (!buffer_push_back(output, 0))
		{
			return 0;
		}
	}

#endif
	return 1;
}

uint8_t compare_paths_with_delimiter_independ(const uint8_t* path_1, const uint8_t* path_2)
{
	if (nullptr == path_1 ||
		nullptr == path_2)
	{
		return 0;
	}

	const ptrdiff_t size = common_count_bytes_until(path_1, 0);

	if (size != common_count_bytes_until(path_2, 0))
	{
		return 0;
	}

	for (ptrdiff_t i = 0; i < size; ++i, ++path_1, ++path_2)
	{
		const uint8_t a = *path_1;
		const uint8_t b = *path_2;

		if ((path_posix_delimiter == a || path_windows_delimiter == a) &&
			(path_posix_delimiter == b || path_windows_delimiter == b))
		{
			continue;
		}

		if (a != b)
		{
			return 0;
		}
	}

	return 1;
}

uint8_t compare_paths_with_delimiter_independ(const char* path_1, const char* path_2)
{
	return compare_paths_with_delimiter_independ(
			   reinterpret_cast<const uint8_t*>(path_1),
			   reinterpret_cast<const uint8_t*>(path_2));
}

TEST_F(TestExec, exec_with_redirect_to_tmp_file)
{
	static const auto exec_str(std::string("exec"));
	//
	std::vector<std::pair<std::string*, void**>> input_properties;
	input_properties.push_back(std::make_pair(&pid_property_str, &pid_property));
	input_properties.push_back(std::make_pair(&result_property_str, &result_property));
	//
	buffer the_project;
	SET_NULL_TO_BUFFER(the_project);
	//
	ASSERT_TRUE(project_new(&the_project)) << project_free(&the_project);
	//
	buffer temp_file_name;
	SET_NULL_TO_BUFFER(temp_file_name);
	//
	pugi::xml_document output_file_content;

	for (const auto& node : nodes)
	{
		load_input_data(node);

		if (!range_is_null_or_empty(&working_dir) &&
			!directory_exists(working_dir.start))
		{
			--node_count;
			continue;
		}

		for (auto& the_property : input_properties)
		{
			if (std::get<0>(the_property)->empty())
			{
				continue;
			}

			ASSERT_TRUE(project_property_set_value(
							&the_project,
							reinterpret_cast<const uint8_t*>(std::get<0>(the_property)->data()),
							static_cast<uint8_t>(std::get<0>(the_property)->size()),
							reinterpret_cast<const uint8_t*>(&the_property), 0, 0, 0, 0, verbose)) <<
									std::get<0>(the_property)->data() << std::endl <<
									buffer_free(&temp_file_name) << project_free(&the_project);
			//
			ASSERT_TRUE(project_property_exists(
							&the_project,
							reinterpret_cast<const uint8_t*>(std::get<0>(the_property)->data()),
							static_cast<uint8_t>(std::get<0>(the_property)->size()),
							std::get<1>(the_property), verbose)) <<
									std::get<0>(the_property)->data() << std::endl <<
									buffer_free(&temp_file_name) << project_free(&the_project);
			//
			ASSERT_NE(nullptr, *(std::get<1>(the_property))) <<
					std::get<0>(the_property)->data() << std::endl <<
					buffer_free(&temp_file_name) << project_free(&the_project);
		}

		if (!range_is_null_or_empty(&environment_variables))
		{
			ASSERT_TRUE(buffer_resize(&temp_file_name, 0))
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			ASSERT_TRUE(
				argument_get_keys_and_values(
					environment_variables.start, environment_variables.finish, &temp_file_name))
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			environment_variables_str = buffer_to_string(&temp_file_name);
			environment_variables = string_to_range(environment_variables_str);
		}

		ASSERT_TRUE(program_str.empty()) << buffer_free(&temp_file_name) << project_free(&the_project);
		//
		uint8_t returned = 0;
		//
		const uint8_t* start;
		const uint8_t* finish;

		if (allow_output_to_console)
		{
			ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&temp_file_name) << project_free(&the_project);
			ASSERT_TRUE(string_to_buffer(range_to_string(&program), &temp_file_name))
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			//
			returned = exec(&the_project, NULL, append,
							&temp_file_name, &base_dir, &command_line, NULL,
							pid_property, result_property,
							&working_dir, &environment_variables, spawn,
							static_cast<uint32_t>(date_time_millisecond_to_second(time_out)), verbose);
			//
			ASSERT_EQ(expected_return, returned)
					<< "tests_exec_app - '" << tests_exec_app << "'" << std::endl
					<< "append - '" << static_cast<int>(append) << "'" << std::endl
					<< "program - '" << range_to_string(&program) << "'" << std::endl
					<< "base directory - '" << base_dir_str << "'" << std::endl
					<< "command line - '" << command_line_str << "'" << std::endl
					<< "pid property - '" << pid_property_str << "'" << std::endl
					<< "result property - '" << result_property_str << "'" << std::endl
					<< "working directory - '" << working_dir_str << "'" << std::endl
					<< "environment variables - '" << environment_variables_str << "'" << std::endl
					<< "spawn - '" << static_cast<int>(spawn) << "'" << std::endl
					<< "time out - '" << static_cast<uint32_t>(date_time_millisecond_to_second(time_out)) << "'" << std::endl
					<< buffer_free(&temp_file_name) << project_free(&the_project);

			for (auto& the_property : input_properties)
			{
				if (std::get<0>(the_property)->empty())
				{
					continue;
				}

				ASSERT_TRUE(buffer_resize(&temp_file_name, 0))
						<< buffer_free(&temp_file_name) << project_free(&the_project);
				ASSERT_TRUE(property_get_by_pointer(*std::get<1>(the_property), &temp_file_name))
						<< buffer_free(&temp_file_name) << project_free(&the_project);

				if (buffer_size(&temp_file_name))
				{
					start = buffer_data(&temp_file_name, 0);
					finish = start + buffer_size(&temp_file_name);
				}
				else
				{
					start = finish = nullptr;
				}

				if (result_property == *std::get<1>(the_property))
				{
					ASSERT_EQ(result_property_value, int64_parse(start, finish))
							<< buffer_free(&temp_file_name) << project_free(&the_project);
				}
				else if (buffer_size(&temp_file_name))
				{
					ASSERT_NE(0, int64_parse(start, finish))
							<< buffer_free(&temp_file_name) << project_free(&the_project);
				}
			}

			project_clear(&the_project);
			//
			pugi::xml_document exec_document;
			auto exec_node = exec_document.append_child(exec_str.c_str());
			//
			ASSERT_TRUE(exec_node.append_attribute("program").set_value(reinterpret_cast<const char*>(program.start)))
					<< program.start << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);
			ASSERT_TRUE(exec_node.append_attribute("append").set_value(append ? true : false))
					<< static_cast<int>(append) << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);

			if (!command_line_str.empty())
			{
				ASSERT_TRUE(
					exec_node.append_attribute("commandline").set_value(command_line_str.data()))
						<< command_line_str << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);
			}

			ASSERT_TRUE(exec_node.append_attribute("spawn").set_value(spawn ? true : false))
					<< static_cast<int>(spawn) << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);

			if (!working_dir_str.empty())
			{
				ASSERT_TRUE(
					exec_node.append_attribute("workingdir").set_value(working_dir_str.data()))
						<< working_dir_str << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);
			}

			ASSERT_TRUE(exec_node.append_attribute("failonerror").set_value(true))
					<< 1 << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);
			//
			ASSERT_TRUE(exec_node.append_attribute("timeout").set_value(time_out))
					<< time_out << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);

			if (!pid_property_str.empty())
			{
				ASSERT_TRUE(exec_node.append_attribute("pidproperty").set_value(pid_property_str.data()))
						<< pid_property_str << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);
			}

			if (!result_property_str.empty())
			{
				ASSERT_TRUE(exec_node.append_attribute("resultproperty").set_value(result_property_str.data()))
						<< result_property_str << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);
			}

			ASSERT_TRUE(exec_node.append_attribute("verbose").set_value(verbose ? true : false))
					<< static_cast<int>(verbose) << std::endl << buffer_free(&temp_file_name) << project_free(&the_project);
			//
			std::ostringstream string_stream;
			exec_document.print(string_stream);
			const auto exec_code = string_stream.str();
			const auto exec_code_in_range = string_to_range(exec_code);
			//
			range exec_in_range;
			exec_in_range.start = exec_code_in_range.start + 1;
			exec_in_range.finish = exec_in_range.start + exec_str.size();
			//
			returned = interpreter_evaluate_task(
						   &the_project, NULL,
						   &exec_in_range, exec_code_in_range.finish,
						   NULL, 0, verbose);
			//
			ASSERT_EQ(expected_return, returned)
					<< exec_code << std::endl
					<< buffer_free(&temp_file_name) << project_free(&the_project);

			for (auto& the_property : input_properties)
			{
				if (std::get<0>(the_property)->empty())
				{
					continue;
				}

				ASSERT_TRUE(project_property_exists(
								&the_project,
								reinterpret_cast<const uint8_t*>(std::get<0>(the_property)->data()),
								static_cast<uint8_t>(std::get<0>(the_property)->size()),
								std::get<1>(the_property), verbose)) <<
										std::get<0>(the_property)->data() << std::endl <<
										buffer_free(&temp_file_name) << project_free(&the_project);
				//
				ASSERT_NE(nullptr, *(std::get<1>(the_property))) <<
						std::get<0>(the_property)->data() << std::endl <<
						buffer_free(&temp_file_name) << project_free(&the_project);
				//
				ASSERT_TRUE(buffer_resize(&temp_file_name, 0))
						<< buffer_free(&temp_file_name) << project_free(&the_project);
				ASSERT_TRUE(property_get_by_pointer(*std::get<1>(the_property), &temp_file_name))
						<< buffer_free(&temp_file_name) << project_free(&the_project);

				if (buffer_size(&temp_file_name))
				{
					start = buffer_data(&temp_file_name, 0);
					finish = start + buffer_size(&temp_file_name);
				}
				else
				{
					start = finish = nullptr;
				}

				if (result_property == *std::get<1>(the_property))
				{
					ASSERT_EQ(result_property_value, int64_parse(start, finish))
							<< buffer_free(&temp_file_name) << project_free(&the_project);
				}
				else if (buffer_size(&temp_file_name))
				{
					ASSERT_NE(0, int64_parse(start, finish))
							<< buffer_free(&temp_file_name) << project_free(&the_project);
				}
			}
		}

		ASSERT_TRUE(buffer_resize(&temp_file_name, 0))
				<< buffer_free(&temp_file_name) << project_free(&the_project);
		ASSERT_TRUE(path_get_temp_file_name(&temp_file_name))
				<< buffer_free(&temp_file_name) << project_free(&the_project);
		ASSERT_TRUE(buffer_push_back(&temp_file_name, 0))
				<< buffer_free(&temp_file_name) << project_free(&the_project);
		//
		const auto output_file_str = buffer_to_string(&temp_file_name);
		const auto output_file = string_to_range(output_file_str);
		//
		ASSERT_TRUE(buffer_resize(&temp_file_name, 0))
				<< buffer_free(&temp_file_name) << project_free(&the_project);
		ASSERT_TRUE(string_to_buffer(range_to_string(&program), &temp_file_name))
				<< buffer_free(&temp_file_name) << project_free(&the_project);
		//
		returned =
			exec(NULL, NULL, append, &temp_file_name, &base_dir, &command_line, &output_file,
				 NULL, NULL, &working_dir, &environment_variables,
				 spawn, time_out, verbose);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&temp_file_name) << project_free(&the_project);
		//
		const auto result = output_file_content.load_file(reinterpret_cast<const char*>(output_file.start));
		ASSERT_EQ(pugi::xml_parse_status::status_ok, result.status)
				<< buffer_free(&temp_file_name) << project_free(&the_project);
		//
		const auto arguments = output_file_content.select_nodes(pugi::xpath_query("App4ExecTest/arguments/argument"));
		ASSERT_FALSE(arguments.empty()) << buffer_free(&temp_file_name) << project_free(&the_project);
		ASSERT_EQ(command_line_str.empty(), 1 == arguments.size())
				<< buffer_free(&temp_file_name) << project_free(&the_project);
		ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&temp_file_name) << project_free(&the_project);

		if (!base_dir_str.empty() && !path_is_path_rooted(program.start, program.finish))
		{
			ASSERT_TRUE(path_combine(base_dir.start, base_dir.finish,
									 program.start, program.finish, &temp_file_name))
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			base_dir_str = buffer_to_string(&temp_file_name);
		}

		if (base_dir_str.empty())
		{
			base_dir_str = tests_exec_app;
		}

		if (std::string::npos != base_dir_str.find(' ', 0))
		{
			base_dir_str = '"' + base_dir_str + '"';
		}

		base_dir = string_to_range(base_dir_str);
		//
		ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&temp_file_name) << project_free(&the_project);
		ASSERT_TRUE(argument_append_arguments(base_dir.start, base_dir.finish, &temp_file_name))
				<< buffer_free(&temp_file_name) << project_free(&the_project);

		if (command_line_str.empty())
		{
			int argc = 0;
			char** argv = nullptr;
			//
			ASSERT_TRUE(argument_create_arguments(&temp_file_name, &argc, &argv))
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			//
			ASSERT_EQ(1, argc) << buffer_free(&temp_file_name) << project_free(&the_project);
			ASSERT_NE(nullptr, argv) << buffer_free(&temp_file_name) << project_free(&the_project);
			ASSERT_NE(nullptr, argv[0]) << buffer_free(&temp_file_name) << project_free(&the_project);
			ASSERT_EQ(nullptr, argv[1]) << buffer_free(&temp_file_name) << project_free(&the_project);
			//
			ASSERT_TRUE(compare_paths_with_delimiter_independ(arguments[0].node().child_value(), argv[0]))
					<< "Path 1 - \"" << arguments[0].node().child_value() << "\"" << std::endl
					<< "Path 2 - \"" << argv[0] << "\"" << std::endl
					<< buffer_free(&temp_file_name) << project_free(&the_project);
		}
		else
		{
			ASSERT_TRUE(argument_append_arguments(command_line.start, command_line.finish, &temp_file_name))
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			//
			int argc = 0;
			char** argv = nullptr;
			//
			ASSERT_TRUE(argument_create_arguments(&temp_file_name, &argc, &argv))
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			//
			ASSERT_EQ(static_cast<int>(arguments.size()), argc)
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			ASSERT_NE(nullptr, argv) << buffer_free(&temp_file_name) << project_free(&the_project);
			argc = 0;

			for (const auto& argument : arguments)
			{
				ASSERT_NE(nullptr, argv[argc]) << buffer_free(&temp_file_name) << project_free(&the_project);

				if (!argc)
				{
					ASSERT_TRUE(compare_paths_with_delimiter_independ(argument.node().child_value(), argv[argc]))
							<< "Path 1 - \"" << argument.node().child_value() << "\"" << std::endl
							<< "Path 2 - \"" << argv[argc] << "\"" << std::endl
							<< buffer_free(&temp_file_name) << project_free(&the_project);
				}
				else
				{
					ASSERT_STREQ(argument.node().child_value(), argv[argc])
							<< buffer_free(&temp_file_name) << project_free(&the_project);
				}

				argc++;
			}

			ASSERT_EQ(static_cast<int>(arguments.size()), argc)
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			ASSERT_EQ(nullptr, argv[argc]) << buffer_free(&temp_file_name) << project_free(&the_project);
		}

		/*TODO:
		const auto working_directory = output_file_content.select_node(
			pugi::xpath_query("App4ExecTest/working_directory"));
		*/
		const auto environments = output_file_content.select_nodes(
									  pugi::xpath_query("App4ExecTest/environments/environment"));

		if (range_is_null_or_empty(&environment_variables))
		{
			ASSERT_FALSE(environments.empty()) << buffer_free(&temp_file_name) << project_free(&the_project);
		}
		else
		{
			ASSERT_EQ(environment_variables_str.empty(), environments.empty())
					<< buffer_free(&temp_file_name) << project_free(&the_project);
			const uint8_t* previous = environment_variables.start;
			start = environment_variables.start;
			finish = environment_variables.finish;

			for (const auto& environment : environments)
			{
				start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 1, 1);
				ASSERT_STREQ(range_to_string(previous, start).c_str(), environment.node().child_value())
						<< buffer_free(&temp_file_name) << project_free(&the_project);
				start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 0, 1);
				previous = start;
			}

			ASSERT_EQ(start, finish) << buffer_free(&temp_file_name) << project_free(&the_project);
		}

		project_clear(&the_project);
		--node_count;
	}

	buffer_release(&temp_file_name);
	project_unload(&the_project);
}

TEST_F(TestExec, exec_get_program_full_path)
{
	buffer path_to_the_program;
	SET_NULL_TO_BUFFER(path_to_the_program);
	//
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);

	for (const auto& node : nodes)
	{
		ASSERT_TRUE(buffer_resize(&path_to_the_program, 0)) << buffer_free(&path_to_the_program) << buffer_free(&tmp);
		ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&path_to_the_program) << buffer_free(&tmp);
		//
		program_str = node.node().select_node("program").node().child_value();
		base_dir_str = node.node().select_node("base_dir").node().child_value();
		base_dir = string_to_range(base_dir_str);
		expected_return = static_cast<uint8_t>(INT_PARSE(node.node().select_node("return").node().child_value()));
		//
		ASSERT_TRUE(string_to_buffer(program_str, &path_to_the_program))
				<< buffer_free(&path_to_the_program) << buffer_free(&tmp);
		//
		const auto program_in_the_range = string_to_range(program_str);
		const auto is_path_rooted = path_is_path_rooted(program_in_the_range.start, program_in_the_range.finish);
		//
		const auto returned = exec_get_program_full_path(nullptr, nullptr,
							  &path_to_the_program, is_path_rooted, &base_dir, &tmp, verbose);
		ASSERT_EQ(expected_return, returned) <<
											 program_str << std::endl << base_dir_str << std::endl <<
											 buffer_free(&path_to_the_program) << buffer_free(&tmp);
		//
		const std::string expected_path_to_the_program(node.node().select_node("output").node().child_value());
		std::string returned_path_to_the_program(buffer_to_string(&path_to_the_program));
		//
		const auto pos = returned_path_to_the_program.find('\0');

		if (std::string::npos != pos)
		{
			returned_path_to_the_program = returned_path_to_the_program.substr(0, pos);
		}

		ASSERT_EQ(returned_path_to_the_program, expected_path_to_the_program)
				<< buffer_free(&path_to_the_program) << buffer_free(&tmp);
		//
		--node_count;
	}

	buffer_release(&tmp);
	buffer_release(&path_to_the_program);
}
