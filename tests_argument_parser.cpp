/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"
};

#include <string>
#include <ostream>
#include <cstdint>

class TestArgumentParser : public TestsBaseXml
{
};

uint8_t string_to_command_arguments(const std::string& input, buffer* output, int* argc, char*** argv)
{
	if (NULL == output ||
		NULL == argc ||
		NULL == argv)
	{
		return 0;
	}

	if (input.empty())
	{
		(*argc) = 0;
		(*argv) = NULL;
		return 1;
	}

	range input_in_range = string_to_range(input);

	if (!string_trim(&input_in_range))
	{
		return 0;
	}

	return argument_from_char((const char*)input_in_range.start, (const char*)input_in_range.finish, output, argc,
							  argv);
}
#if defined(_WIN32)
uint8_t string_to_command_arguments(const std::wstring& input, buffer* output, int* argc, wchar_t*** argv)
{
	if (NULL == output ||
		NULL == argc ||
		NULL == argv)
	{
		return 0;
	}

	if (input.empty())
	{
		(*argc) = 0;
		(*argv) = NULL;
		return 1;
	}

	//TODO: range input_in_range = string_to_range(input);
	const wchar_t* start = input.empty() ? NULL : input.data();
	const wchar_t* finish = input.empty() ? NULL : input.data() + input.size();
	/*TODO: if (!string_trim(&input_in_range))
	{
		return 0;
	}*/
	return argument_from_wchar_t(start, finish, output, argc, argv);
}
#endif
uint8_t argument_parser_free()
{
	argument_parser_release();
	return 0;
}

#define ARGUMENT_PARSER_AT_ALL(INPUT, ARGUMENT_PARSER)											\
	ASSERT_TRUE(string_to_command_arguments((INPUT), &command_arguments, &argc, &argv)) <<		\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	const uint8_t returned = (ARGUMENT_PARSER)(0, argc, argv);									\
	ASSERT_EQ(expected_return, returned) <<														\
										 (INPUT) << std::endl <<								\
										 buffer_free(&property_value) <<						\
										 buffer_free(&command_arguments) <<						\
										 argument_parser_free();								\
	/**/																						\
	int i = 0;																					\
	\
	ASSERT_EQ(build_files.empty(), NULL == argument_parser_get_build_file(i)) <<				\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	\
	for (const auto& build_file : build_files)													\
	{																							\
		const range* returned_build_file =  argument_parser_get_build_file(i++);				\
		ASSERT_EQ(std::string(build_file.node().child_value()),									\
				  range_to_string(returned_build_file)) <<										\
						  (INPUT) << std::endl <<												\
						  buffer_free(&property_value) <<										\
						  buffer_free(&command_arguments) <<									\
						  argument_parser_free();												\
	}																							\
	/**/																						\
	ASSERT_EQ(expected_pause, argument_parser_get_pause())  <<									\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(expected_verbose, argument_parser_get_verbose())  <<								\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(expected_debug, argument_parser_get_debug())  <<									\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(expected_quiet, argument_parser_get_quiet())  <<									\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(expected_indent, argument_parser_get_indent())  <<								\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(properties.empty(), 0 == buffer_size(argument_parser_get_properties()))  <<		\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	for (const auto& property : properties)														\
	{																							\
		const std::string name(property.node().attribute("name").as_string());					\
		const std::string value(property.node().attribute("value").as_string());				\
		const uint8_t expected_dynamic = property.node().attribute("dynamic").as_bool();		\
		const uint8_t expected_readonly = property.node().attribute("readonly").as_bool();		\
		/**/																					\
		void* the_property = NULL;																\
		ASSERT_TRUE(property_exists(argument_parser_get_properties(),							\
									(const uint8_t*)name.data(),								\
									(uint8_t)name.size(), &the_property)) <<					\
											(INPUT) << std::endl <<								\
											buffer_free(&property_value) <<						\
											buffer_free(&command_arguments) <<					\
											argument_parser_free();								\
		/**/																					\
		ASSERT_TRUE(property_get_by_pointer(the_property, &property_value)) <<					\
				(INPUT) << std::endl <<															\
				buffer_free(&property_value) <<													\
				buffer_free(&command_arguments) <<												\
				argument_parser_free();															\
		/**/																					\
		ASSERT_EQ(value, buffer_to_string(&property_value)) <<									\
				(INPUT) << std::endl <<															\
				buffer_free(&property_value) <<													\
				buffer_free(&command_arguments) <<												\
				argument_parser_free();															\
		/**/																					\
		uint8_t returned_dynamic = 0;															\
		ASSERT_TRUE(property_is_dynamic(the_property, &returned_dynamic)) <<					\
				(INPUT) << std::endl <<															\
				buffer_free(&property_value) <<													\
				buffer_free(&command_arguments) <<												\
				argument_parser_free();															\
		ASSERT_EQ(expected_dynamic, returned_dynamic) <<										\
				(INPUT) << std::endl <<															\
				buffer_free(&property_value) <<													\
				buffer_free(&command_arguments) <<												\
				argument_parser_free();															\
		/**/																					\
		uint8_t returned_readonly = 0;															\
		ASSERT_TRUE(property_is_readonly(the_property, &returned_readonly)) <<					\
				(INPUT) << std::endl <<															\
				buffer_free(&property_value) <<													\
				buffer_free(&command_arguments) <<												\
				argument_parser_free();															\
		ASSERT_EQ(expected_readonly, returned_readonly) <<										\
				(INPUT) << std::endl <<															\
				buffer_free(&property_value) <<													\
				buffer_free(&command_arguments) <<												\
				argument_parser_free();															\
	}																							\
	/**/																						\
	const range* log_file = argument_parser_get_log_file();										\
	ASSERT_EQ(expected_log_file.empty(), NULL == log_file)  <<									\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(expected_log_file, range_to_string(log_file))  <<									\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(expected_project_help, argument_parser_get_project_help())  <<					\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(expected_no_logo, argument_parser_get_no_logo())  <<								\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();																\
	/**/																						\
	ASSERT_EQ(expected_help, argument_parser_get_help())  <<									\
			(INPUT) << std::endl <<																\
			buffer_free(&property_value) <<														\
			buffer_free(&command_arguments) <<													\
			argument_parser_free();

TEST_F(TestArgumentParser, argument_parser_at_all)
{
	buffer command_arguments;
	SET_NULL_TO_BUFFER(command_arguments);
	//
	buffer property_value;
	SET_NULL_TO_BUFFER(property_value);

	for (const auto& node : nodes)
	{
		const auto input(get_data_from_nodes(node, "input"));
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		const auto build_files = node.node().select_nodes("build_file");
		const uint8_t expected_pause = (uint8_t)INT_PARSE(node.node().select_node("pause").node().child_value());
		const uint8_t expected_verbose = (uint8_t)INT_PARSE(node.node().select_node("verbose").node().child_value());
		const uint8_t expected_debug = (uint8_t)INT_PARSE(node.node().select_node("debug").node().child_value());
		const uint8_t expected_quiet = (uint8_t)INT_PARSE(node.node().select_node("quiet").node().child_value());
		const uint8_t expected_indent = (uint8_t)INT_PARSE(node.node().select_node("indent").node().child_value());
		const auto properties = node.node().select_nodes("properties/property");
		const std::string expected_log_file(node.node().select_node("log_file").node().child_value());
		const uint8_t expected_project_help = (uint8_t)INT_PARSE(
				node.node().select_node("project_help").node().child_value());
		const uint8_t expected_no_logo = (uint8_t)INT_PARSE(node.node().select_node("no_logo").node().child_value());
		const uint8_t expected_help = (uint8_t)INT_PARSE(node.node().select_node("help").node().child_value());
#if defined(_WIN32)
		ASSERT_TRUE(buffer_resize(&property_value, 0)) <<
				buffer_free(&property_value) <<
				buffer_free(&command_arguments) <<
				argument_parser_free();
		std::wstring inputW = u8string_to_u16string(input, &property_value);
#endif

		for (uint8_t step = 0; step < 2; ++step)
		{
			argument_parser_release();
			//
			ASSERT_TRUE(buffer_resize(&command_arguments, 0)) <<
					buffer_free(&property_value) <<
					buffer_free(&command_arguments) <<
					argument_parser_free();
			//
			ASSERT_TRUE(buffer_resize(&property_value, 0)) <<
					buffer_free(&property_value) <<
					buffer_free(&command_arguments) <<
					argument_parser_free();

			if (!step)
			{
				int argc = 0;
				char** argv = NULL;
				ARGUMENT_PARSER_AT_ALL(input, argument_parser_char);
			}

#if defined(_WIN32)
			else
			{
				int argc = 0;
				wchar_t** argv = NULL;
				ARGUMENT_PARSER_AT_ALL(inputW, argument_parser_wchar_t);
			}

#else
			break;
#endif
		}

		--node_count;
	}

	argument_parser_release();
	buffer_release(&property_value);
	buffer_release(&command_arguments);
}

TEST_F(TestArgumentParser, argument_append_arguments)
{
	buffer command_arguments;
	SET_NULL_TO_BUFFER(command_arguments);

	for (const auto& node : nodes)
	{
		ASSERT_TRUE(buffer_resize(&command_arguments, 0)) << buffer_free(&command_arguments);
		//
		const auto arguments = node.node().select_nodes("argument");
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		std::string expected_output(node.node().select_node("output").node().child_value());
		auto pos = std::string::npos;

		while (std::string::npos != (pos = expected_output.find("0x0")))
		{
			expected_output = expected_output.replace(pos, 3, "\0", 1);
		}

		for (const auto& argument : arguments)
		{
			const std::string argument_str(argument.node().child_value());
			const range argument_in_range = string_to_range(argument_str);
			const uint8_t returned = argument_append_arguments(
										 argument_in_range.start, argument_in_range.finish, &command_arguments);
			//
			ASSERT_EQ(expected_return, returned) << buffer_free(&command_arguments);
		}

		ASSERT_EQ(expected_output, buffer_to_string(&command_arguments)) << buffer_free(&command_arguments);
		//
		int argc = 0;
		char** argv = NULL;
		ASSERT_TRUE(argument_create_arguments(&command_arguments, &argc, &argv)) << buffer_free(&command_arguments);

		if (argv)
		{
			int i = 0;

			while (NULL != argv[i])
			{
				++i;
				--argc;
			}

			ASSERT_EQ(0, argc) << buffer_free(&command_arguments);
			i = 0;

			for (const auto& argument : arguments)
			{
				std::string argument_str(argument.node().child_value());

				if (!argument_str.empty())
				{
					if ('\"' == argument_str[0])
					{
						argument_str = argument_str.substr(1);
					}

					if (!argument_str.empty() && '\"' == argument_str[argument_str.size() - 1])
					{
						argument_str = argument_str.substr(0, argument_str.size() - 1);
					}
				}

				ASSERT_STREQ(argument_str.empty() ? NULL : argument_str.c_str(),
							 argv[i++]) << buffer_free(&command_arguments);
			}
		}

		--node_count;
	}

	buffer_release(&command_arguments);
}

#define GET_CHAR_ARGV(ARGUMENT, INPUT)																						\
	\
	ASSERT_TRUE(buffer_resize(&(ARGUMENT), 0)) << (INPUT) << std::endl << buffer_free(&(ARGUMENT));							\
	ASSERT_TRUE(common_append_string_to_buffer((INPUT), &(ARGUMENT))) << (INPUT) << std::endl << buffer_free(&(ARGUMENT));	\
	ASSERT_TRUE(buffer_push_back(&(ARGUMENT), 0)) << (INPUT) << std::endl << buffer_free(&(ARGUMENT));						\
	\
	char* argv[1];																											\
	argv[0] = buffer_char_data(&(ARGUMENT), 0);

#define GET_WCHAR_T_ARGVW(ARGUMENT, INPUT)																					\
	ASSERT_TRUE(buffer_resize(&(ARGUMENT), 0)) << (INPUT) << std::endl << buffer_free(&(ARGUMENT));							\
	ASSERT_TRUE(text_encoding_UTF8_to_UTF16LE(																				\
				(INPUT), (INPUT) + common_count_bytes_until((INPUT), 0),													\
				&(ARGUMENT))) << (INPUT) << std::endl << buffer_free(&(ARGUMENT));											\
	ASSERT_TRUE(buffer_push_back_uint16(&(ARGUMENT), 0)) << (INPUT) << std::endl << buffer_free(&(ARGUMENT));				\
	\
	wchar_t* argvW[1];																										\
	argvW[0] = buffer_wchar_t_data(&(ARGUMENT), 0);

TEST_F(TestArgumentParser, argument_parser_get_verbose)
{
	buffer argument;
	SET_NULL_TO_BUFFER(argument);

	for (const auto& node : nodes)
	{
		const uint8_t* input = (const uint8_t*)node.node().select_node("argument").node().child_value();
		const uint8_t expected_output = (uint8_t)INT_PARSE(node.node().select_node("output").node().child_value());
		//
		argument_parser_release();
		GET_CHAR_ARGV(argument, input);
		//
		ASSERT_TRUE(argument_parser_char(0, 1, argv)) << input << std::endl << buffer_free(&argument);
		ASSERT_EQ(expected_output, argument_parser_get_verbose()) << input << std::endl << buffer_free(&argument);
#if defined(_WIN32)
		argument_parser_release();
		GET_WCHAR_T_ARGVW(argument, input);
		//
		ASSERT_TRUE(argument_parser_wchar_t(0, 1, argvW)) << input << std::endl << buffer_free(&argument);
		ASSERT_EQ(expected_output, argument_parser_get_verbose()) << input << std::endl << buffer_free(&argument);
#endif
		--node_count;
	}

	buffer_release(&argument);
	argument_parser_release();
}

TEST_F(TestArgumentParser, argument_parser_get_build_file)
{
	buffer argument;
	SET_NULL_TO_BUFFER(argument);

	for (const auto& node : nodes)
	{
		const uint8_t* input = (const uint8_t*)node.node().select_node("argument").node().child_value();
		const auto expected_output = node.node().select_nodes("output");
		//
		argument_parser_release();
		GET_CHAR_ARGV(argument, input);
		//
		ASSERT_TRUE(argument_parser_char(0, 1, argv)) << input << std::endl << buffer_free(&argument);
		int i = 0;

		for (const auto& out : expected_output)
		{
			ASSERT_STREQ(out.node().child_value(),
						 range_to_string(argument_parser_get_build_file(i++)).c_str())
					<< input << std::endl << buffer_free(&argument);
		}

#if defined(_WIN32)
		argument_parser_release();
		GET_WCHAR_T_ARGVW(argument, input);
		//
		ASSERT_TRUE(argument_parser_wchar_t(0, 1, argvW)) << input << std::endl << buffer_free(&argument);
		i = 0;

		for (const auto& out : expected_output)
		{
			ASSERT_STREQ(out.node().child_value(),
						 range_to_string(argument_parser_get_build_file(i++)).c_str())
					<< input << std::endl << buffer_free(&argument);
		}

#endif
		--node_count;
	}

	buffer_release(&argument);
	argument_parser_release();
}

TEST_F(TestArgumentParser, argument_parser_get_log_file)
{
	buffer argument;
	SET_NULL_TO_BUFFER(argument);

	for (const auto& node : nodes)
	{
		const uint8_t* input = (const uint8_t*)node.node().select_node("argument").node().child_value();
		const char* expected_output = node.node().select_node("output").node().child_value();
		const uint8_t expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		argument_parser_release();
		GET_CHAR_ARGV(argument, input);
		//
		uint8_t returned = argument_parser_char(0, 1, argv);
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&argument);
		ASSERT_STREQ(expected_output, range_to_string(argument_parser_get_log_file()).c_str()) << input << std::endl
				<< buffer_free(&argument);
#if defined(_WIN32)
		argument_parser_release();
		GET_WCHAR_T_ARGVW(argument, input);
		//
		returned = argument_parser_wchar_t(0, 1, argvW);
		ASSERT_EQ(expected_return, returned) << input << std::endl << buffer_free(&argument);
		ASSERT_STREQ(expected_output, range_to_string(argument_parser_get_log_file()).c_str()) << input << std::endl
				<< buffer_free(&argument);
#endif
		--node_count;
	}

	buffer_release(&argument);
	argument_parser_release();
}
