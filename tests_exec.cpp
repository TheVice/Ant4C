/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"
#if 0
extern "C" {
#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "exec.h"
#include "path.h"
#include "range.h"
};

#include <string>
#include <cassert>
#include <cstdint>
#include <utility>

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
	uint8_t verbose;
	uint8_t expected_return;

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
		verbose(),
		expected_return()
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
	}

	void load_input_data(const pugi::xpath_node& node);
};

std::string TestExec::tests_exec_app;

void TestExec::load_input_data(const pugi::xpath_node& node)
{
	append = (uint8_t)int_parse(node.node().select_node("append").node().child_value());
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
	spawn = (uint8_t)int_parse(node.node().select_node("spawn").node().child_value());
	time_out = (uint8_t)int_parse(node.node().select_node("time_out").node().child_value());
	verbose = (uint8_t)int_parse(node.node().select_node("verbose").node().child_value());
	//
	expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
}

static uint8_t argument_get_keys_and_values(
	const char* input_start, const char* input_finish,
	struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == output)
	{
		return 0;
	}

	while (input_start < input_finish)
	{
		struct range key;
		struct range value;
		const char* start = input_start;

		if ('"' == *input_start)
		{
			input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, "\"", 1, 1, 1);
			input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, "=", 1, 1, 1);
			input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, "=", 1, 0, 1);

			if ('"' == *input_start)
			{
				input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, "\"", 1, 1, 1);

				if (input_finish != input_start)
				{
					++input_start;
				}
			}
			else
			{
				input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, " ", 1, 1, 1);
			}
		}
		else
		{
			input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, " ", 1, 1, 1);
		}

		if (!argument_get_key_and_value(start, input_start, &key, &value))
		{
			return 0;
		}

		if (!buffer_append_data_from_range(output, &key) ||
			!buffer_push_back(output, '=') ||
			!buffer_append_data_from_range(output, &value) ||
			!buffer_push_back(output, '\0'))
		{
			return 0;
		}

		++input_start;
	}

	return 1;
}

TEST_F(TestExec, exec_at_all)
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

		load_input_data(node);

		if (!range_is_null_or_empty(&environment_variables))
		{
			ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp);
			ASSERT_TRUE(argument_get_keys_and_values(environment_variables.start, environment_variables.finish,
						&tmp)) << buffer_free(&tmp);
			environment_variables = buffer_to_range(&tmp);
		}

		const uint8_t returned =
			exec(append, &program, &base_dir, &command_line, NULL,
				 NULL, NULL, &working_dir, &environment_variables,
				 spawn, time_out, verbose);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&tmp);
		//
		--node_count;
	}

	buffer_release(&tmp);
}

TEST_F(TestExec, exec_with_redirect_to_tmp_file)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);
	//
	buffer temp_file_name;
	SET_NULL_TO_BUFFER(temp_file_name);
	//
	pugi::xml_document output_file_content;

	for (const auto& node : nodes)
	{
		load_input_data(node);

		if (!range_is_null_or_empty(&environment_variables))
		{
			ASSERT_TRUE(buffer_resize(&tmp, 0)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			ASSERT_TRUE(argument_get_keys_and_values(environment_variables.start, environment_variables.finish,
						&tmp)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			environment_variables = buffer_to_range(&tmp);
		}

		ASSERT_TRUE(program_str.empty()) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		//
		ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		ASSERT_TRUE(path_get_temp_file_name(&temp_file_name)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		ASSERT_TRUE(buffer_push_back(&temp_file_name, '\0')) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		//
		const range output_file = buffer_to_range(&temp_file_name);
		//
		const uint8_t returned =
			exec(append, &program, &base_dir, &command_line, &output_file,
				 NULL, NULL, &working_dir, &environment_variables,
				 spawn, time_out, verbose);
		//
		ASSERT_EQ(expected_return, returned) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		//
		const auto result = output_file_content.load_file((const char*)output_file.start);
		ASSERT_EQ(pugi::xml_parse_status::status_ok,
				  result.status) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		//
		const auto arguments = output_file_content.select_nodes(pugi::xpath_query("App4ExecTest/arguments/argument"));
		ASSERT_FALSE(arguments.empty()) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		ASSERT_EQ(command_line_str.empty(),
				  1 == arguments.size()) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&tmp) << buffer_free(&temp_file_name);

		if (!base_dir_str.empty() && !path_is_path_rooted(program.start, program.finish))
		{
			ASSERT_TRUE(path_combine(base_dir.start, base_dir.finish,
									 program.start, program.finish, &temp_file_name)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
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
		ASSERT_TRUE(buffer_resize(&temp_file_name, 0)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		ASSERT_TRUE(argument_append_arguments(base_dir.start, base_dir.finish,
											  &temp_file_name)) << buffer_free(&tmp) << buffer_free(&temp_file_name);

		if (command_line_str.empty())
		{
			int argc = 0;
			char** argv = NULL;
			//
			ASSERT_TRUE(argument_create_arguments(&temp_file_name, &argc,
												  &argv)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			//
			ASSERT_EQ(1, argc) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			ASSERT_NE(nullptr, argv) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			ASSERT_NE(nullptr, argv[0]) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			ASSERT_EQ(nullptr, argv[1]) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			//
			ASSERT_STREQ(arguments[0].node().child_value(),
						 argv[0]) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		}
		else
		{
			ASSERT_TRUE(argument_append_arguments(command_line.start, command_line.finish,
												  &temp_file_name)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			//
			int argc = 0;
			char** argv = NULL;
			//
			ASSERT_TRUE(argument_create_arguments(&temp_file_name, &argc,
												  &argv)) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			//
			ASSERT_EQ((int)arguments.size(), argc) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			ASSERT_NE(nullptr, argv) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			argc = 0;

			for (const auto& argument : arguments)
			{
				ASSERT_NE(nullptr, argv[argc]) << buffer_free(&tmp) << buffer_free(&temp_file_name);
				ASSERT_STREQ(argument.node().child_value(),
							 argv[argc++]) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			}

			ASSERT_EQ((int)arguments.size(), argc) << buffer_free(&tmp) << buffer_free(&temp_file_name);
			ASSERT_EQ(nullptr, argv[argc]) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		}

		const auto environments = output_file_content.select_nodes(
									  pugi::xpath_query("App4ExecTest/environments/environment"));

		if (range_is_null_or_empty(&environment_variables))
		{
			ASSERT_FALSE(environments.empty()) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		}

		/*TODO: else
		{
			ASSERT_EQ(environment_variables_str.empty(), environments.empty()) << buffer_free(&tmp) << buffer_free(&temp_file_name);
		}*/
		--node_count;
	}

	buffer_release(&temp_file_name);
}
#endif
