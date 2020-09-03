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
#include "load_file.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"
};

#include <vector>
#include <string>
#include <cstdint>
#include <ostream>
#include <utility>
#include <algorithm>

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

	return argument_from_char(
			   (const char*)input_in_range.start,
			   (const char*)input_in_range.finish,
			   output, argc, argv);
}
#if defined(_WIN32)
uint8_t string_to_command_arguments(const std::string& input, buffer* output, int* argc, wchar_t*** argv)
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

	char** argvA = NULL;

	if (!string_to_command_arguments(input, output, argc, &argvA))
	{
		return 0;
	}

	const ptrdiff_t size = (uint8_t*)argvA - buffer_data(output, 0);

	if (!buffer_append(output, NULL, 4 * (size + 1) + sizeof(uint32_t)))
	{
		return 0;
	}

	const uint8_t* start = buffer_data(output, 0);
	const uint8_t* finish = buffer_data(output, size);

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	if (!text_encoding_UTF8_to_UTF16LE(start, finish, output))
	{
		return 0;
	}

	const ptrdiff_t new_size = buffer_size(output);

	if (!buffer_append(output, NULL, (ptrdiff_t)1 + (*argc) * sizeof(wchar_t*)) ||
		!buffer_resize(output, new_size))
	{
		return 0;
	}

	int i = 0;
	static const wchar_t zero_symbol = L'\0';
	const wchar_t* startW = (const wchar_t*)buffer_data(output, size);
	const wchar_t* finishW = (const wchar_t*)(buffer_data(output, 0) + new_size);

	while ((startW = find_any_symbol_like_or_not_like_that_wchar_t(startW, finishW, &zero_symbol, 1, 0,
					 1)) < finishW && i < (*argc))
	{
		if (!buffer_append(output, (const uint8_t*)&startW, sizeof(const wchar_t*)))
		{
			return 0;
		}

		startW = find_any_symbol_like_or_not_like_that_wchar_t(startW, finishW, &zero_symbol, 1, 1, 1);
		++i;
	}

	startW = NULL;

	if (!buffer_append(output, (const uint8_t*)&startW, sizeof(const wchar_t*)))
	{
		return 0;
	}

	(*argv) = (wchar_t**)(buffer_data(output, 0) + new_size);
	return 1;
}
#endif
#define ARGUMENT_PARSER_AT_ALL(INPUT, VERBOSE_PARSER, ARGUMENT_PARSER, I)															\
	\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(string_to_command_arguments((INPUT), &argument_value, &argc, &argv)) <<												\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	verbose = (VERBOSE_PARSER)(0, argc, argv);																						\
	/**/																															\
	const auto returned = (ARGUMENT_PARSER)(0, argc, argv, &command_arguments, verbose);											\
	ASSERT_EQ(expected_return, returned) <<																							\
										 (INPUT) << std::endl <<																	\
										 buffer_free(&argument_value) <<															\
										 properties_free(&command_arguments) <<														\
										 buffer_free(&property_value);																\
	/**/																															\
	(I) = 0;																														\
	\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_build_files.empty(), NULL == argument_parser_get_build_file(&command_arguments, &argument_value, (I))) <<	\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	\
	for (const auto& build_file : expected_build_files)																				\
	{																																\
		ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																			\
				(INPUT) << std::endl <<																								\
				buffer_free(&argument_value) <<																						\
				properties_free(&command_arguments) <<																				\
				buffer_free(&property_value);																						\
		/**/																														\
		const auto returned_build_file =  argument_parser_get_build_file(&command_arguments, &argument_value, (I)++);				\
		ASSERT_EQ(std::string(build_file.node().child_value()),																		\
				  range_to_string(returned_build_file)) <<																			\
						  (INPUT) << std::endl << (I) - 1 << std::endl <<															\
						  buffer_free(&argument_value) <<																			\
						  properties_free(&command_arguments) <<																	\
						  buffer_free(&property_value);																				\
	}																																\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	(I) = 0;																														\
	\
	ASSERT_EQ(expected_targets.empty(), NULL == argument_parser_get_target(&command_arguments, &argument_value, I)) <<				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	\
	for (const auto& target : expected_targets)																						\
	{																																\
		const auto returned_target = argument_parser_get_target(&command_arguments, &argument_value, (I)++);						\
		ASSERT_EQ(std::string(target.node().child_value()),																			\
				  range_to_string(returned_target)) <<																				\
						  (INPUT) << std::endl << (I) - 1 << std::endl <<															\
						  buffer_free(&argument_value) <<																			\
						  properties_free(&command_arguments) <<																	\
						  buffer_free(&property_value);																				\
	}																																\
	\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_pause, argument_parser_get_pause(&command_arguments, &argument_value))  <<									\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_verbose, verbose) <<																							\
										 (INPUT) << std::endl <<																	\
										 buffer_free(&argument_value) <<															\
										 properties_free(&command_arguments) <<														\
										 buffer_free(&property_value);																\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_debug, argument_parser_get_debug(&command_arguments, &argument_value))  <<									\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_quiet, argument_parser_get_quiet(&command_arguments, &argument_value))  <<									\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_indent, argument_parser_get_indent(&command_arguments, &argument_value)) <<									\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_properties.empty(), 0 == argument_parser_get_properties(&command_arguments, &argument_value, verbose)) <<	\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	for (const auto& expected_property : expected_properties)																		\
	{																																\
		const std::string name(expected_property.node().attribute("name").as_string());												\
		const std::string value(expected_property.node().attribute("value").as_string());											\
		const uint8_t expected_dynamic = expected_property.node().attribute("dynamic").as_bool();									\
		const uint8_t expected_readonly = expected_property.node().attribute("readonly").as_bool();									\
		/**/																														\
		void* the_property = NULL;																									\
		ASSERT_TRUE(property_exists(&argument_value,																				\
									(const uint8_t*)name.data(),																	\
									(uint8_t)name.size(), &the_property)) <<														\
											(INPUT) << std::endl <<																	\
											properties_free(&argument_value) <<														\
											properties_free(&command_arguments) <<													\
											buffer_free(&property_value);															\
		/**/																														\
		ASSERT_TRUE(buffer_resize(&property_value, 0)) <<																			\
				(INPUT) << std::endl <<																								\
				properties_free(&argument_value) <<																					\
				properties_free(&command_arguments) <<																				\
				buffer_free(&property_value);																						\
		/**/																														\
		ASSERT_TRUE(property_get_by_pointer(the_property, &property_value)) <<														\
				(INPUT) << std::endl <<																								\
				properties_free(&argument_value) <<																					\
				properties_free(&command_arguments) <<																				\
				buffer_free(&property_value);																						\
		ASSERT_EQ(value, buffer_to_string(&property_value)) <<																		\
				(INPUT) << std::endl <<																								\
				properties_free(&argument_value) <<																					\
				properties_free(&command_arguments) <<																				\
				buffer_free(&property_value);																						\
		/**/																														\
		uint8_t returned_dynamic = 0;																								\
		ASSERT_TRUE(property_is_dynamic(the_property, &returned_dynamic)) <<														\
				(INPUT) << std::endl <<																								\
				properties_free(&argument_value) <<																					\
				properties_free(&command_arguments) <<																				\
				buffer_free(&property_value);																						\
		ASSERT_EQ(expected_dynamic, returned_dynamic) <<																			\
				(INPUT) << std::endl <<																								\
				properties_free(&argument_value) <<																					\
				properties_free(&command_arguments) <<																				\
				buffer_free(&property_value);																						\
		/**/																														\
		uint8_t returned_readonly = 0;																								\
		ASSERT_TRUE(property_is_readonly(the_property, &returned_readonly)) <<														\
				(INPUT) << std::endl <<																								\
				properties_free(&argument_value) <<																					\
				properties_free(&command_arguments) <<																				\
				buffer_free(&property_value);																						\
		ASSERT_EQ(expected_readonly, returned_readonly) <<																			\
				(INPUT) << std::endl <<																								\
				properties_free(&argument_value) <<																					\
				properties_free(&command_arguments) <<																				\
				buffer_free(&property_value);																						\
	}																																\
	/**/																															\
	property_release_inner(&argument_value);																						\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	const auto log_file = argument_parser_get_log_file(&command_arguments, &argument_value);										\
	ASSERT_EQ(expected_log_file.empty(), NULL == log_file)  <<																		\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_log_file, range_to_string(log_file))  <<																		\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_project_help, argument_parser_get_project_help(&command_arguments, &argument_value))  <<						\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_no_logo, argument_parser_get_no_logo(&command_arguments, &argument_value))  <<								\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_help, argument_parser_get_program_help(&command_arguments, &argument_value))  <<								\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_encoding, argument_parser_get_encoding(&command_arguments, &argument_value))  <<								\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_listener, range_to_string(argument_parser_get_listener(&command_arguments, &argument_value))) <<				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_TRUE(buffer_resize(&argument_value, 0)) <<																				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);																							\
	/**/																															\
	ASSERT_EQ(expected_module_priority, argument_parser_get_module_priority(&command_arguments, &argument_value)) <<				\
			(INPUT) << std::endl <<																									\
			buffer_free(&argument_value) <<																							\
			properties_free(&command_arguments) <<																					\
			buffer_free(&property_value);

static const std::map<std::string, std::uint16_t> encodings =
{
	std::make_pair("ASCII", (std::uint16_t)ASCII),
	std::make_pair("UTF8", (std::uint16_t)UTF8),
	std::make_pair("UTF16BE", (std::uint16_t)UTF16BE),
	std::make_pair("UTF16LE", (std::uint16_t)UTF16LE),
	std::make_pair("UTF32BE", (std::uint16_t)UTF32BE),
	std::make_pair("UTF32LE", (std::uint16_t)UTF32LE),
	std::make_pair("BigEndianUnicode", (std::uint16_t)BigEndianUnicode),
	std::make_pair("Unicode", (std::uint16_t)Unicode),
	std::make_pair("UTF32", (std::uint16_t)UTF32),
	std::make_pair("Default", (std::uint16_t)Default),
	std::make_pair("Windows_1250", (std::uint16_t)Windows_1250),
	std::make_pair("Windows_1251", (std::uint16_t)Windows_1251),
	std::make_pair("Windows_1252", (std::uint16_t)Windows_1252),
	std::make_pair("Windows_1253", (std::uint16_t)Windows_1253),
	std::make_pair("Windows_1254", (std::uint16_t)Windows_1254),
	std::make_pair("Windows_1255", (std::uint16_t)Windows_1255),
	std::make_pair("Windows_1256", (std::uint16_t)Windows_1256),
	std::make_pair("Windows_1257", (std::uint16_t)Windows_1257),
	std::make_pair("Windows_1258", (std::uint16_t)Windows_1258),
	std::make_pair("ISO_8859_1", (std::uint16_t)ISO_8859_1),
	std::make_pair("ISO_8859_2", (std::uint16_t)ISO_8859_2),
	std::make_pair("ISO_8859_7", (std::uint16_t)ISO_8859_7),
	std::make_pair("ISO_8859_9", (std::uint16_t)ISO_8859_9),
	std::make_pair("ISO_8859_11", (std::uint16_t)ISO_8859_11),
	std::make_pair("ISO_8859_13", (std::uint16_t)ISO_8859_13)
};

TEST_F(TestArgumentParser, argument_parser_at_all)
{
	buffer command_arguments;
	SET_NULL_TO_BUFFER(command_arguments);
	//
	buffer argument_value;
	SET_NULL_TO_BUFFER(argument_value);
	//
	buffer property_value;
	SET_NULL_TO_BUFFER(property_value);

	for (const auto& node : nodes)
	{
		const auto input(get_data_from_nodes(node, "input"));
		//
		const auto expected_build_files = node.node().select_nodes("build_file");
		const auto expected_debug = (uint8_t)INT_PARSE(node.node().select_node("debug").node().child_value());
		const auto expected_help = (uint8_t)INT_PARSE(node.node().select_node("help").node().child_value());
		const auto expected_indent = (uint8_t)INT_PARSE(node.node().select_node("indent").node().child_value());
		const auto expected_no_logo = (uint8_t)INT_PARSE(node.node().select_node("no_logo").node().child_value());
		const auto expected_pause = (uint8_t)INT_PARSE(node.node().select_node("pause").node().child_value());
		const auto expected_project_help =
			(uint8_t)INT_PARSE(node.node().select_node("project_help").node().child_value());
		const auto expected_quiet = (uint8_t)INT_PARSE(node.node().select_node("quiet").node().child_value());
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		const auto expected_verbose = (uint8_t)INT_PARSE(node.node().select_node("verbose").node().child_value());
		const auto expected_properties = node.node().select_nodes("properties/property");
		const auto expected_targets = node.node().select_nodes("target");
		const std::string expected_log_file(node.node().select_node("log_file").node().child_value());
		const std::string expected_encoding_str(node.node().select_node("encoding").node().child_value());
		const auto expected_encoding = encodings.count(expected_encoding_str) ? encodings.at(expected_encoding_str) :
									   (expected_encoding_str.empty() ? UTF8 : FILE_ENCODING_UNKNOWN);
		const std::string expected_listener(node.node().select_node("listener").node().child_value());
		const auto expected_module_priority = (uint8_t)INT_PARSE(
				node.node().select_node("module_priority").node().child_value());

		for (uint8_t step = 0; step < 2; ++step)
		{
			if (!step)
			{
				int i = 0;
				int argc = 0;
				char** argv = NULL;
				ARGUMENT_PARSER_AT_ALL(input, argument_parser_get_verbose_char, argument_parser_char, i);
			}

#if defined(_WIN32)
			else
			{
				int i = 0;
				int argc = 0;
				wchar_t** argv = NULL;
				ARGUMENT_PARSER_AT_ALL(input, argument_parser_get_verbose_wchar_t, argument_parser_wchar_t, i);
			}

#endif
			property_release_inner(&command_arguments);
			ASSERT_TRUE(buffer_resize(&command_arguments, 0)) <<
					buffer_free(&argument_value) <<
					buffer_free(&command_arguments) <<
					buffer_free(&property_value);
		}

		--node_count;
	}

	buffer_release(&argument_value);
	buffer_release(&command_arguments);
	buffer_release(&property_value);
}

TEST_F(TestArgumentParser, argument_append_arguments)
{
	buffer command_arguments;
	SET_NULL_TO_BUFFER(command_arguments);

	for (const auto& node : nodes)
	{
		ASSERT_TRUE(buffer_resize(&command_arguments, 0)) << buffer_free(&command_arguments);
		//
		const auto arguments = node.node().select_nodes("input");
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		std::string expected_output(node.node().select_node("output").node().child_value());
		size_t pos = 0;

		while (std::string::npos != (pos = expected_output.find("0x0")))
		{
			expected_output = expected_output.replace(pos, 3, "\0", 1);
		}

		for (const auto& argument : arguments)
		{
			const std::string argument_str(argument.node().child_value());
			const auto argument_in_range = string_to_range(argument_str);
			const auto returned = argument_append_arguments(
									  argument_in_range.start, argument_in_range.finish, &command_arguments);
			//
			ASSERT_EQ(expected_return, returned) << buffer_free(&command_arguments);
		}

		ASSERT_EQ(expected_output, buffer_to_string(&command_arguments)) << buffer_free(&command_arguments);
		//
		--node_count;
	}

	buffer_release(&command_arguments);
}
