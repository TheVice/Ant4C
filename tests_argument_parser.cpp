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
#include "load_file.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"
};

#include <vector>
#include <string>
#include <ostream>
#include <utility>
#include <algorithm>

class TestArgumentParser : public TestsBaseXml
{
};

uint8_t string_to_command_arguments(const std::string& input, buffer* output, int* argc, char*** argv)
{
	if (nullptr == output ||
		nullptr == argc ||
		nullptr == argv)
	{
		return 0;
	}

	if (input.empty())
	{
		*argc = 0;
		*argv = nullptr;
		return 1;
	}

	auto input_in_a_range = string_to_range(input);

	if (!string_trim(&input_in_a_range))
	{
		return 0;
	}

	return argument_from_char(
			   reinterpret_cast<const char*>(input_in_a_range.start),
			   reinterpret_cast<const char*>(input_in_a_range.finish),
			   output, argc, argv);
}
#if defined(_WIN32)
uint8_t string_to_command_arguments(const std::string& input, buffer* output, int* argc, wchar_t*** argv)
{
	if (nullptr == output ||
		nullptr == argc ||
		nullptr == argv)
	{
		return 0;
	}

	if (input.empty())
	{
		*argc = 0;
		*argv = nullptr;
		return 1;
	}

	char** argvA;

	if (!string_to_command_arguments(input, output, argc, &argvA))
	{
		return 0;
	}

	const auto size = reinterpret_cast<uint8_t*>(argvA) - buffer_data(output, 0);

	if (!buffer_append(output, nullptr, 4 * (size + 1) + sizeof(uint32_t)))
	{
		return 0;
	}

	const auto* start = buffer_data(output, 0);
	const auto* finish = buffer_data(output, size);

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	if (!text_encoding_UTF8_to_UTF16LE(start, finish, output))
	{
		return 0;
	}

	const auto new_size = buffer_size(output);

	if (!buffer_append(output, nullptr, static_cast<ptrdiff_t>(1) + (*argc) * sizeof(wchar_t*)) ||
		!buffer_resize(output, new_size))
	{
		return 0;
	}

	int i = 0;
	static const auto zero_symbol = L'\0';
	const auto* startW = reinterpret_cast<const wchar_t*>(buffer_data(output, size));
	const auto* finishW = reinterpret_cast<const wchar_t*>(buffer_data(output, 0) + new_size);

	while ((startW = find_any_symbol_like_or_not_like_that_wchar_t(startW, finishW, &zero_symbol, 1, 0,
					 1)) < finishW && i < (*argc))
	{
		if (!buffer_append(output, reinterpret_cast<const uint8_t*>(&startW), sizeof(const wchar_t*)))
		{
			return 0;
		}

		startW = find_any_symbol_like_or_not_like_that_wchar_t(startW, finishW, &zero_symbol, 1, 1, 1);
		++i;
	}

	startW = nullptr;

	if (!buffer_append(output, reinterpret_cast<const uint8_t*>(&startW), sizeof(const wchar_t*)))
	{
		return 0;
	}

	*argv = reinterpret_cast<wchar_t**>(buffer_data(output, 0) + new_size);
	return 1;
}
#endif
#define ARGUMENT_PARSER_AT_ALL(INPUT, ARGUMENT_PARSER, I)																			\
	/**/																															\
	ASSERT_TRUE(argument_parser_init()) <<																							\
										(INPUT) << std::endl <<																		\
										argument_parser_free() <<																	\
										buffer_free(&property_value);																\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&property_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(string_to_command_arguments((INPUT), &property_value, &argc, &argv)) <<												\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	const auto returned = (ARGUMENT_PARSER)(0, argc, argv);																			\
	ASSERT_EQ(expected_return, returned) <<																							\
										 (INPUT) << std::endl <<																	\
										 argument_parser_free() <<																	\
										 buffer_free(&property_value);																\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&property_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	const auto verbose = argument_parser_get_verbose();																				\
	(I) = 0;																														\
	/**/																															\
	ASSERT_EQ(expected_build_files.empty(), nullptr == argument_parser_get_build_file((I))) <<										\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	for (const auto& build_file : expected_build_files)																				\
	{																																\
		const auto returned_build_file = argument_parser_get_build_file((I)++);														\
		const std::string returned_build_file_str(																					\
				nullptr == returned_build_file ? "" : reinterpret_cast<const char*>(returned_build_file));							\
		ASSERT_EQ(std::string(build_file.node().child_value()), returned_build_file_str) <<											\
				(INPUT) << std::endl << (I) - 1 << std::endl <<																		\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
	}																																\
	/**/																															\
	(I) = 0;																														\
	/**/																															\
	ASSERT_EQ(expected_targets.empty(), nullptr == argument_parser_get_target(I)) <<												\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	for (const auto& target : expected_targets)																						\
	{																																\
		const auto returned_target = argument_parser_get_target((I)++);																\
		const std::string returned_target_str(nullptr == returned_target ? "" : reinterpret_cast<const char*>(returned_target));	\
		ASSERT_EQ(std::string(target.node().child_value()), returned_target_str) <<													\
				(INPUT) << std::endl << (I) - 1 << std::endl <<																		\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
	}																																\
	/**/																															\
	ASSERT_EQ(expected_pause, argument_parser_get_pause()) <<																		\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_verbose, verbose) <<																							\
										 (INPUT) << std::endl <<																	\
										 argument_parser_free() <<																	\
										 buffer_free(&property_value);																\
	/**/																															\
	ASSERT_EQ(expected_debug, argument_parser_get_debug()) <<																		\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_quiet, argument_parser_get_quiet()) <<																		\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_indent, argument_parser_get_indent()) <<																		\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	const struct buffer* properties = argument_parser_get_properties();																\
	ASSERT_EQ(expected_properties.empty(), 0 == buffer_size(properties)) <<															\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	for (const auto& expected_property : expected_properties)																		\
	{																																\
		const auto the_expected_property = expected_property.node();																\
		const std::string name(the_expected_property.attribute("name").as_string());												\
		const std::string value(the_expected_property.attribute("value").as_string());												\
		const uint8_t expected_dynamic = the_expected_property.attribute("dynamic").as_bool();										\
		const uint8_t expected_readonly = the_expected_property.attribute("readonly").as_bool();									\
		/**/																														\
		void* the_property;																											\
		ASSERT_TRUE(property_exists(properties,																						\
									reinterpret_cast<const uint8_t*>(name.c_str()),													\
									static_cast<uint8_t>(name.size()), &the_property)) <<											\
											(INPUT) << std::endl <<																	\
											argument_parser_free() <<																\
											buffer_free(&property_value);															\
		/**/																														\
		ASSERT_TRUE(buffer_resize(&property_value, 0)) <<																			\
				(INPUT) << std::endl <<																								\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
		/**/																														\
		ASSERT_TRUE(property_get_by_pointer(the_property, &property_value)) <<														\
				(INPUT) << std::endl <<																								\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
		ASSERT_EQ(value, buffer_to_string(&property_value)) <<																		\
				(INPUT) << std::endl <<																								\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
		/**/																														\
		uint8_t returned_dynamic;																									\
		ASSERT_TRUE(property_is_dynamic(the_property, &returned_dynamic)) <<														\
				(INPUT) << std::endl <<																								\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
		ASSERT_EQ(expected_dynamic, returned_dynamic) <<																			\
				(INPUT) << std::endl <<																								\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
		/**/																														\
		uint8_t returned_readonly;																									\
		ASSERT_TRUE(property_is_readonly(the_property, &returned_readonly)) <<														\
				(INPUT) << std::endl <<																								\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
		ASSERT_EQ(expected_readonly, returned_readonly) <<																			\
				(INPUT) << std::endl <<																								\
				argument_parser_free() <<																							\
				buffer_free(&property_value);																						\
	}																																\
	/**/																															\
	const auto log_file = argument_parser_get_log_file();																			\
	const std::string log_file_str(nullptr == log_file ? "" : reinterpret_cast<const char*>(log_file));								\
	ASSERT_EQ(expected_log_file.empty(), nullptr == log_file) <<																	\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_log_file, log_file_str) <<																					\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_project_help, argument_parser_get_project_help()) <<															\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_no_logo, argument_parser_get_no_logo()) <<																	\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_help, argument_parser_get_program_help()) <<																	\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_encoding, argument_parser_get_encoding()) <<																	\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	const auto path_to_listener = argument_parser_get_listener();																	\
	const std::string path_to_listener_str(nullptr == path_to_listener ? "" : reinterpret_cast<const char*>(path_to_listener));		\
	ASSERT_EQ(expected_listener, path_to_listener_str) <<																			\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_module_priority, argument_parser_get_module_priority()) <<													\
			(INPUT) << std::endl <<																									\
			argument_parser_free() <<																								\
			buffer_free(&property_value);

static const std::map<std::string, std::uint16_t> encodings =
{
	std::make_pair("ASCII", static_cast<std::uint16_t>(ASCII)),
	std::make_pair("UTF8", static_cast<std::uint16_t>(UTF8)),
	std::make_pair("UTF16BE", static_cast<std::uint16_t>(UTF16BE)),
	std::make_pair("UTF16LE", static_cast<std::uint16_t>(UTF16LE)),
	std::make_pair("UTF32BE", static_cast<std::uint16_t>(UTF32BE)),
	std::make_pair("UTF32LE", static_cast<std::uint16_t>(UTF32LE)),
	std::make_pair("BigEndianUnicode", static_cast<std::uint16_t>(BigEndianUnicode)),
	std::make_pair("Unicode", static_cast<std::uint16_t>(Unicode)),
	std::make_pair("UTF32", static_cast<std::uint16_t>(UTF32)),
	std::make_pair("Default", static_cast<std::uint16_t>(Default)),
	std::make_pair("Windows_1250", static_cast<std::uint16_t>(Windows_1250)),
	std::make_pair("Windows_1251", static_cast<std::uint16_t>(Windows_1251)),
	std::make_pair("Windows_1252", static_cast<std::uint16_t>(Windows_1252)),
	std::make_pair("Windows_1253", static_cast<std::uint16_t>(Windows_1253)),
	std::make_pair("Windows_1254", static_cast<std::uint16_t>(Windows_1254)),
	std::make_pair("Windows_1255", static_cast<std::uint16_t>(Windows_1255)),
	std::make_pair("Windows_1256", static_cast<std::uint16_t>(Windows_1256)),
	std::make_pair("Windows_1257", static_cast<std::uint16_t>(Windows_1257)),
	std::make_pair("Windows_1258", static_cast<std::uint16_t>(Windows_1258)),
	std::make_pair("ISO_8859_1", static_cast<std::uint16_t>(ISO_8859_1)),
	std::make_pair("ISO_8859_2", static_cast<std::uint16_t>(ISO_8859_2)),
	std::make_pair("ISO_8859_7", static_cast<std::uint16_t>(ISO_8859_7)),
	std::make_pair("ISO_8859_9", static_cast<std::uint16_t>(ISO_8859_9)),
	std::make_pair("ISO_8859_11", static_cast<std::uint16_t>(ISO_8859_11)),
	std::make_pair("ISO_8859_13", static_cast<std::uint16_t>(ISO_8859_13))
};

template<typename TYPE>
std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>>
		all_arguments(
			const std::vector<std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>>>& arguments)
{
	std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>>
			output;

	for (const auto& argument : arguments)
	{
		output += argument;
		output.push_back('\n');
	}

	return output;
}

TEST_F(TestArgumentParser, argument_parser_get_verbose)
{
	std::vector<std::string> arguments;
	std::vector<char*> argv;
#if defined(_WIN32)
	std::vector<std::wstring> argumentsW;
	std::vector<wchar_t*> argvW;
#endif

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		const std::string i_str(
			the_node.select_node("i").node().child_value());
		const auto i_in_a_range(string_to_range(i_str));
		const auto i = int_parse(i_in_a_range.start, i_in_a_range.finish);
		//
		const std::string return_str(
			the_node.select_node("return").node().child_value());
		const auto return_in_a_range(string_to_range(return_str));
		const auto expected_return =
			static_cast<uint8_t>(
				int_parse(return_in_a_range.start, return_in_a_range.finish));
		//
		arguments.clear();
		//
		const auto arguments_ = the_node.select_nodes("argument");
		std::transform(
			arguments_.begin(), arguments_.end(),
			std::back_inserter(arguments),
			[](const pugi::xpath_node & argument)
		{
			return argument.node().child_value();
		});
		//
		argv.clear();
		const auto argc = static_cast<int>(arguments.size());

		for (auto& argument : arguments)
		{
			if (argument.empty())
			{
				argv.push_back(nullptr);
				continue;
			}

			argv.push_back(&argument[0]);
		}

		ASSERT_TRUE(argument_parser_init()) << argument_parser_free();
		ASSERT_TRUE(argument_parser_char(i, argc, argv.data())) << argument_parser_free();
		auto returned = argument_parser_get_verbose();
		ASSERT_EQ(expected_return, returned)
				<< i << std::endl
				<< static_cast<int>(expected_return) << std::endl
				<< all_arguments(arguments) << std::endl
				<< argument_parser_free() << std::endl;
#if defined(_WIN32)
		argumentsW.clear();
		//
		std::transform(
			arguments.begin(), arguments.end(),
			std::back_inserter(argumentsW),
			[](const std::string & argument)
		{
			return char_to_wchar_t(argument);
		});
		//
		argvW.clear();

		for (auto& argument : argumentsW)
		{
			if (argument.empty())
			{
				argvW.push_back(nullptr);
				continue;
			}

			argvW.push_back(&argument[0]);
		}

		ASSERT_TRUE(argument_parser_init()) << argument_parser_free();
		ASSERT_TRUE(argument_parser_wchar_t(i, argc, argvW.data())) << argument_parser_free();
		returned = argument_parser_get_verbose();
		ASSERT_EQ(expected_return, returned)
				<< i << std::endl
				<< static_cast<int>(expected_return) << std::endl
				<< all_arguments(argumentsW) << std::endl
				<< argument_parser_free() << std::endl;
#endif
		argument_parser_release();
		--node_count;
	}
}

TEST_F(TestArgumentParser, argument_parser_at_all)
{
	buffer property_value;
	SET_NULL_TO_BUFFER(property_value);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		const auto input(get_data_from_nodes(the_node, "input"));
		const auto expected_build_files =
			the_node.select_nodes("build_file");
		//
		const std::string debug_str(the_node.select_node("debug").node().child_value());
		auto input_in_a_range = string_to_range(debug_str);
		const auto expected_debug =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string help_str(the_node.select_node("help").node().child_value());
		input_in_a_range = string_to_range(help_str);
		const auto expected_help =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string indent_str(the_node.select_node("indent").node().child_value());
		input_in_a_range = string_to_range(indent_str);
		const auto expected_indent =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string no_logo_str(the_node.select_node("no_logo").node().child_value());
		input_in_a_range = string_to_range(no_logo_str);
		const auto expected_no_logo =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string pause_str(the_node.select_node("pause").node().child_value());
		input_in_a_range = string_to_range(pause_str);
		const auto expected_pause =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string project_help_str(the_node.select_node("project_help").node().child_value());
		input_in_a_range = string_to_range(project_help_str);
		const auto expected_project_help =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string quiet_str(the_node.select_node("quiet").node().child_value());
		input_in_a_range = string_to_range(quiet_str);
		const auto expected_quiet =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string return_str(the_node.select_node("return").node().child_value());
		input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string verbose_str(the_node.select_node("verbose").node().child_value());
		input_in_a_range = string_to_range(verbose_str);
		const auto expected_verbose =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const auto expected_properties = the_node.select_nodes("properties/property");
		const auto expected_targets = the_node.select_nodes("target");
		const std::string expected_log_file(the_node.select_node("log_file").node().child_value());
		const std::string expected_encoding_str(the_node.select_node("encoding").node().child_value());
		const auto expected_encoding =
			encodings.count(expected_encoding_str) ?
			encodings.at(expected_encoding_str) : (expected_encoding_str.empty() ? UTF8 : FILE_ENCODING_UNKNOWN);
		const std::string expected_listener(the_node.select_node("listener").node().child_value());
		//
		const std::string module_priority_str(the_node.select_node("module_priority").node().child_value());
		input_in_a_range = string_to_range(module_priority_str);
		const auto expected_module_priority =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		int i = 0;
		int argc = 0;
		{
			char** argv = nullptr;
			ARGUMENT_PARSER_AT_ALL(input, argument_parser_char, i);
		}
#if defined(_WIN32)
		i = 0;
		argc = 0;
		{
			wchar_t** argv = nullptr;
			ARGUMENT_PARSER_AT_ALL(input, argument_parser_wchar_t, i);
		}
#endif
		--node_count;
	}

	buffer_release(&property_value);
	argument_parser_release();
}

TEST_F(TestArgumentParser, argument_append_arguments)
{
	buffer command_arguments;
	SET_NULL_TO_BUFFER(command_arguments);

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		ASSERT_TRUE(buffer_resize(&command_arguments, 0)) << buffer_free(&command_arguments);
		//
		const auto arguments = the_node.select_nodes("input");
		const std::string return_str(the_node.select_node("return").node().child_value());
		const auto return_in_a_range(string_to_range(return_str));
		const auto expected_return = static_cast<uint8_t>(
										 int_parse(return_in_a_range.start, return_in_a_range.finish));
		std::string expected_output(the_node.select_node("output").node().child_value());
		size_t pos;

		while (std::string::npos != (pos = expected_output.find("0x0")))
		{
			expected_output = expected_output.replace(pos, 3, "\0", 1);
		}

		for (const auto& argument : arguments)
		{
			const std::string argument_str(argument.node().child_value());
			const auto argument_in_a_range = string_to_range(argument_str);
			const auto returned = argument_append_arguments(
									  argument_in_a_range.start,
									  argument_in_a_range.finish,
									  &command_arguments);
			//
			ASSERT_EQ(expected_return, returned) << buffer_free(&command_arguments);
		}

		ASSERT_EQ(expected_output, buffer_to_string(&command_arguments)) << buffer_free(&command_arguments);
		//
		--node_count;
	}

	buffer_release(&command_arguments);
}
