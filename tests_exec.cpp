/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "exec.h"
#include "interpreter.h"
#include "path.h"
#include "range.h"
};

#include <string>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <utility>

extern "C" {
	extern uint8_t argument_get_key_and_value(
		const uint8_t* input_start, const uint8_t* input_finish,
		struct range* key, struct range* value);
};

class TestExec : public TestsBaseXml
{
protected:
	static std::string tests_exec_app;

protected:
	uint8_t append;

	std::string program_str;
	range program;

	std::string base_dir_str;
	range base_dir;

	std::string command_line_str;
	range command_line;

	std::string pid_property_str;
	std::string result_property_str;

	std::string working_dir_str;
	range working_dir;

	std::string environment_variables_str;
	range environment_variables;

	uint8_t spawn;
	uint32_t time_out;
	uint8_t verbose_;
	uint8_t expected_return;

	uint8_t allow_output_to_console;

protected:
	TestExec() :
		TestsBaseXml(),
		append(),
		program_str(),
		program(),
		base_dir_str(),
		base_dir(),
		command_line_str(),
		command_line(),
		pid_property_str(),
		result_property_str(),
		working_dir_str(),
		working_dir(),
		environment_variables_str(),
		environment_variables(),
		spawn(),
		time_out(),
		verbose_(),
		expected_return(),
		allow_output_to_console()
	{
		predefine_arguments.insert(std::make_pair("--tests_exec_app=", &tests_exec_app));
	}

	virtual void SetUp() override
	{
		TestsBaseXml::SetUp();

		if (tests_exec_app.empty())
		{
			auto result = parse_input_arguments();
			assert(result);
			ASSERT_TRUE(result);
			//
			result = tests_exec_app.empty();
			assert(!result);
			ASSERT_FALSE(result);
		}

		allow_output_to_console = nodes.first().parent().attribute("allow_output_to_console").as_bool();
	}

	void load_input_data(const pugi::xpath_node& node);
};

std::string TestExec::tests_exec_app;

void TestExec::load_input_data(const pugi::xpath_node& node)
{
	append = (uint8_t)INT_PARSE(node.node().select_node("append").node().child_value());
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
	result_property_str = std::string(node.node().select_node("result_property").node().child_value());
	//
	working_dir_str = std::string(node.node().select_node("working_dir").node().child_value());
	working_dir = string_to_range(working_dir_str);
	//
	environment_variables_str = std::string(
									node.node().select_node("environment_variables").node().child_value());
	environment_variables = string_to_range(environment_variables_str);
	//
	spawn = (uint8_t)INT_PARSE(node.node().select_node("spawn").node().child_value());
	time_out = (uint8_t)INT_PARSE(node.node().select_node("time_out").node().child_value());
	verbose_ = (uint8_t)INT_PARSE(node.node().select_node("verbose").node().child_value());
	//
	expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
}

static const uint8_t zero_symbol = '\0';
static const uint8_t quote_symbol = '"';
static const uint8_t equal_symbol = '=';
static const uint8_t space_symbol = ' ';

static uint8_t argument_get_keys_and_values(
	const uint8_t* input_start, const uint8_t* input_finish,
	struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, input_finish - input_start + 1) ||
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

		struct range key;

		struct range value;

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
	if (NULL == path_1 ||
		NULL == path_2)
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
	return compare_paths_with_delimiter_independ((const uint8_t*)path_1, (const uint8_t*)path_2);
}

TEST_F(TestExec, exec_with_redirect_to_tmp_file)
{
	static const auto exec_str(std::string("exec"));
	static const auto exec_in_range = string_to_range(exec_str);
	static const uint8_t exec_task_id = interpreter_get_task(exec_in_range.start, exec_in_range.finish);
	//
	buffer temp_file_name;
	SET_NULL_TO_BUFFER(temp_file_name);
	//
	pugi::xml_document output_file_content;

	for (const auto& node : nodes)
	{
		uint8_t condition = 0;
		ASSERT_TRUE(is_this_node_pass_by_if_condition(node, &temp_file_name,
					&condition, verbose)) << buffer_free(&temp_file_name);

		if (!condition)
		{
			--node_count;
			continue;
		}

		load_input_data(node);

		if (!range_is_null_or_empty(&environment_variables))
		{
			ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&temp_file_name);
			ASSERT_TRUE(argument_get_keys_and_values(environment_variables.start, environment_variables.finish,
						&temp_file_name))
					<< buffer_free(&temp_file_name);
			environment_variables_str = buffer_to_string(&temp_file_name);
			environment_variables = string_to_range(environment_variables_str);
		}

		ASSERT_TRUE(program_str.empty()) << buffer_free(&temp_file_name);
		//
		uint8_t returned = 0;

		if (allow_output_to_console)
		{
			returned = exec(append, &program, &base_dir, &command_line, NULL,
							NULL, NULL, &working_dir, &environment_variables,
							spawn, time_out, verbose_);
			//
			ASSERT_EQ(expected_return, returned)
					<< "program - '" << range_to_string(&program) << "'" << std::endl
					<< "base directory - '" << range_to_string(base_dir) << "'" << std::endl
					<< "command line - '" << range_to_string(command_line) << "'" << std::endl
					<< "working directory - '" << range_to_string(working_dir) << "'" << std::endl
					<< "environment variables - '" << range_to_string(environment_variables) << "'" << std::endl
					<< buffer_free(&temp_file_name);
			//
			pugi::xml_document exec_document;
			auto exec_node = exec_document.append_child(exec_str.c_str());
			//
			ASSERT_TRUE(exec_node.append_attribute("program").set_value((const char*)program.start))
					<< program.start << std::endl << buffer_free(&temp_file_name);
			ASSERT_TRUE(exec_node.append_attribute("append").set_value(append ? true : false))
					<< (int)append << std::endl << buffer_free(&temp_file_name);

			if (!range_is_null_or_empty(&command_line))
			{
				ASSERT_TRUE(exec_node.append_attribute("commandline").set_value((const char*)command_line.start))
						<< program.start << std::endl << buffer_free(&temp_file_name);
			}

			ASSERT_TRUE(exec_node.append_attribute("spawn").set_value(spawn ? true : false))
					<< (int)spawn << std::endl << buffer_free(&temp_file_name);

			if (!range_is_null_or_empty(&working_dir))
			{
				ASSERT_TRUE(exec_node.append_attribute("workingdir").set_value((const char*)working_dir.start))
						<< working_dir.start << std::endl << buffer_free(&temp_file_name);
			}

			ASSERT_TRUE(exec_node.append_attribute("failonerror").set_value(true))
					<< 1 << std::endl << buffer_free(&temp_file_name);
			//
			ASSERT_TRUE(exec_node.append_attribute("timeout").set_value(time_out))
					<< time_out << std::endl << buffer_free(&temp_file_name);
			//
			ASSERT_TRUE(exec_node.append_attribute("verbose").set_value(verbose_ ? true : false))
					<< (int)verbose_ << std::endl << buffer_free(&temp_file_name);
			//
			std::ostringstream string_stream;
			exec_document.print(string_stream);
			const auto exec_code = string_stream.str();
			const auto exec_code_in_range = string_to_range(exec_code);
			//
			struct range exec_in_range_;
			exec_in_range_.start = exec_code_in_range.start + 1;
			exec_in_range_.finish = exec_in_range_.start + exec_str.size();
			//
			returned = interpreter_evaluate_task(NULL, NULL, exec_task_id,
												 &exec_in_range_,
												 exec_code_in_range.finish, NULL, 0, verbose_);
			ASSERT_EQ(expected_return, returned)
					<< exec_code << std::endl
					<< buffer_free(&temp_file_name);
		}

		ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&temp_file_name);
		ASSERT_TRUE(path_get_temp_file_name(&temp_file_name)) << buffer_free(&temp_file_name);
		ASSERT_TRUE(buffer_push_back(&temp_file_name, 0)) << buffer_free(&temp_file_name);
		//
		const range output_file = buffer_to_range(&temp_file_name);
		//
		returned =
			exec(append, &program, &base_dir, &command_line, &output_file,
				 NULL, NULL, &working_dir, &environment_variables,
				 spawn, time_out, verbose_);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&temp_file_name);
		//
		const auto result = output_file_content.load_file((const char*)output_file.start);
		ASSERT_EQ(pugi::xml_parse_status::status_ok, result.status)
				<< buffer_free(&temp_file_name);
		//
		const auto arguments = output_file_content.select_nodes(pugi::xpath_query("App4ExecTest/arguments/argument"));
		ASSERT_FALSE(arguments.empty()) << buffer_free(&temp_file_name);
		ASSERT_EQ(command_line_str.empty(), 1 == arguments.size())
				<< buffer_free(&temp_file_name);
		ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&temp_file_name);

		if (!base_dir_str.empty() && !path_is_path_rooted(program.start, program.finish))
		{
			ASSERT_TRUE(path_combine(base_dir.start, base_dir.finish,
									 program.start, program.finish, &temp_file_name))
					<< buffer_free(&temp_file_name);
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
		ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&temp_file_name);
		ASSERT_TRUE(argument_append_arguments(base_dir.start, base_dir.finish, &temp_file_name))
				<< buffer_free(&temp_file_name);

		if (command_line_str.empty())
		{
			int argc = 0;
			char** argv = NULL;
			//
			ASSERT_TRUE(argument_create_arguments(&temp_file_name, &argc, &argv))
					<< buffer_free(&temp_file_name);
			//
			ASSERT_EQ(1, argc) << buffer_free(&temp_file_name);
			ASSERT_NE(nullptr, argv) << buffer_free(&temp_file_name);
			ASSERT_NE(nullptr, argv[0]) << buffer_free(&temp_file_name);
			ASSERT_EQ(nullptr, argv[1]) << buffer_free(&temp_file_name);
			//
			ASSERT_TRUE(compare_paths_with_delimiter_independ(arguments[0].node().child_value(), argv[0]))
					<< arguments[0].node().child_value()
					<< argv[0]
					<< buffer_free(&temp_file_name);
		}
		else
		{
			ASSERT_TRUE(argument_append_arguments(command_line.start, command_line.finish, &temp_file_name))
					<< buffer_free(&temp_file_name);
			//
			int argc = 0;
			char** argv = NULL;
			//
			ASSERT_TRUE(argument_create_arguments(&temp_file_name, &argc, &argv))
					<< buffer_free(&temp_file_name);
			//
			ASSERT_EQ((int)arguments.size(), argc) << buffer_free(&temp_file_name);
			ASSERT_NE(nullptr, argv) << buffer_free(&temp_file_name);
			argc = 0;

			for (const auto& argument : arguments)
			{
				ASSERT_NE(nullptr, argv[argc]) << buffer_free(&temp_file_name);

				if (!argc)
				{
					ASSERT_TRUE(compare_paths_with_delimiter_independ(argument.node().child_value(), argv[argc]))
							<< argument.node().child_value()
							<< argv[argc]
							<< buffer_free(&temp_file_name);
				}
				else
				{
					ASSERT_STREQ(argument.node().child_value(), argv[argc])
							<< buffer_free(&temp_file_name);
				}

				argc++;
			}

			ASSERT_EQ((int)arguments.size(), argc) << buffer_free(&temp_file_name);
			ASSERT_EQ(nullptr, argv[argc]) << buffer_free(&temp_file_name);
		}

		/*TODO:
		const auto working_directory = output_file_content.select_node(
			pugi::xpath_query("App4ExecTest/working_directory"));
		*/
		const auto environments = output_file_content.select_nodes(
									  pugi::xpath_query("App4ExecTest/environments/environment"));

		if (range_is_null_or_empty(&environment_variables))
		{
			ASSERT_FALSE(environments.empty()) << buffer_free(&temp_file_name);
		}
		else
		{
			ASSERT_EQ(environment_variables_str.empty(), environments.empty()) << buffer_free(&temp_file_name);
			const uint8_t* previous = environment_variables.start;
			const uint8_t* start = environment_variables.start;
			const uint8_t* finish = environment_variables.finish;

			for (const auto& environment : environments)
			{
				start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 1, 1);
				ASSERT_STREQ(range_to_string(previous, start).c_str(), environment.node().child_value())
						<< buffer_free(&temp_file_name);
				start = find_any_symbol_like_or_not_like_that(start, finish, &zero_symbol, 1, 0, 1);
				previous = start;
			}

			ASSERT_EQ(start, finish) << buffer_free(&temp_file_name);
		}

		--node_count;
	}

	buffer_release(&temp_file_name);
}
