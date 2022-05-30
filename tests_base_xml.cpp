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
#include "conversion.h"
#include "echo.h"
#include "interpreter.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "string_unit.h"
#include "text_encoding.h"
};

#include <cassert>
#include <cstring>
#include <ostream>
#include <utility>
#include <iostream>

#ifdef _WIN32
#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#if defined(_MSC_VER)
int main_(int argc, char** argv)
{
#else
GTEST_API_ int main(int argc, char** argv)
{
	std::cout << "Running main() from ";
	std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif
#ifdef _WIN32
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(NUMBER);
#endif
#endif
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
#if defined(_MSC_VER)

extern std::string wchar_t_to_char(const wchar_t* input_start, const wchar_t* input_finish);

GTEST_API_ int wmain(int argc, wchar_t** argv)
{
	std::wcout << L"Running wmain() from ";
	std::wcout << __FILEW__ << L" " << __LINE__ << std::endl;
	//
	std::string arguments(0 < argc ? 512 : 0, 0);
	arguments.clear();

	for (int i = 0; i < argc; ++i)
	{
		const auto* finish = argv[i];

		while (0 != *finish)
		{
			++finish;
		}

		arguments += wchar_t_to_char(argv[i], finish);
		arguments.push_back(0);
	}

	char** argvA = nullptr;

	if (argc)
	{
		const auto size = arguments.size();
		arguments.append(argc * sizeof(char**), 0);
		argvA = reinterpret_cast<char**>(&arguments[size]);
		//
		const auto start = reinterpret_cast<const uint8_t*>(&arguments[0]);
		const auto finish = reinterpret_cast<const uint8_t*>(argvA);
		//
		int i = 0;
		const auto* pos = start;
		static const uint8_t zero = 0;

		do
		{
			pos = string_find_any_symbol_like_or_not_like_that(pos, finish, &zero, &zero + 1, 0, 1);
			argvA[i++] = &arguments[pos - start];
		}
		while (i < argc &&
			   finish != (pos = string_find_any_symbol_like_or_not_like_that(pos, finish, &zero, &zero + 1, 1, 1)));
	}

	return main_(argc, argvA);
}
#endif
std::string buffer_to_string(const buffer* input)
{
	std::string output((nullptr != input && 0 < buffer_size(input)) ? buffer_size(input) : 0, '\0');
	output.clear();

	if (nullptr != input && nullptr != buffer_char_data(input, 0))
	{
		output.append(buffer_char_data(input, 0), buffer_size(input));
	}

	return output;
}

range buffer_to_range(const buffer* input)
{
	range output;
	output.start = buffer_data(input, 0);
	output.finish = output.start + buffer_size(input);
	return output;
}

uint8_t string_to_buffer(const std::string& input, buffer* output)
{
	return nullptr != output && buffer_append_char(output, input.empty() ? nullptr : input.c_str(), input.size());
}

range string_to_range(const std::string& input)
{
	range output;
	output.start = input.empty() ? nullptr : reinterpret_cast<const uint8_t*>(input.c_str());
	output.finish = input.empty() ? nullptr : output.start + input.size();
	return output;
}

std::string range_to_string(const uint8_t* start_of_range, const uint8_t* finish_of_range)
{
	static const std::string empty;

	if (range_in_parts_is_null_or_empty(start_of_range, finish_of_range))
	{
		return empty;
	}

	return std::string(reinterpret_cast<const char*>(start_of_range), finish_of_range - start_of_range);
}

std::string range_to_string(const range& input)
{
	return range_to_string(input.start, input.finish);
}

std::string range_to_string(const range* input)
{
	return (nullptr == input) ? range_to_string(nullptr, nullptr) : range_to_string(input->start, input->finish);
}

void null_range_to_empty(range& input)
{
	if (nullptr == input.start ||
		nullptr == input.finish)
	{
		input.start = input.finish = reinterpret_cast<const uint8_t*>(&input);
	}
}

uint8_t buffer_free(buffer* input)
{
	buffer_release(input);
	return 0;
}

uint8_t buffer_resize_and_free_inner_buffers(buffer* storage)
{
	buffer_release_inner_buffers(storage);
	return buffer_resize(storage, 0);
}

uint8_t buffer_free_with_inner_buffers(buffer* storage)
{
	buffer_release_with_inner_buffers(storage);
	return 0;
}

uint8_t is_this_node_pass_by_if_condition(
	const pugi::xpath_node& node, buffer* tmp, uint8_t* condition, uint8_t verbose)
{
	if (nullptr == tmp || nullptr == condition)
	{
		return 0;
	}

	static const char* attributes[2] = { "if", "unless" };
	std::string node_attributes;

	for (const auto& attribute : attributes)
	{
		const auto attribute_value = node.node().attribute(attribute).as_string();

		if (!std::strlen(attribute_value))
		{
			continue;
		}

		node_attributes += " ";
		node_attributes.append(attribute);
		node_attributes += "=\"";
		node_attributes.append(attribute_value);
		node_attributes += "\"";
	}

	if (!node_attributes.empty())
	{
		if (!buffer_resize(tmp, 0))
		{
			return 0;
		}

		const auto code = string_to_range(node_attributes);
		verbose = interpreter_is_xml_tag_should_be_skip_by_if_or_unless(
					  nullptr, nullptr, code.start, code.finish, condition, tmp, verbose);
		buffer_release_inner_buffers(tmp);

		if (!verbose)
		{
			return 0;
		}

		*condition = !(*condition);
	}
	else
	{
		*condition = 1;
	}

	return 1;
}

std::string get_data_from_nodes(const pugi::xpath_node& parent_node, const std::string& name_of_nodes)
{
	std::string output;

	for (const auto& node : parent_node.node().select_nodes(name_of_nodes.c_str()))
	{
		output.append(node.node().child_value());
	}

	return output;
}

uint8_t project_free(void* the_project)
{
	project_unload(the_project);
	return 0;
}

std::string property_to_string(const void* the_property, buffer* value)
{
	return buffer_resize(value, 0) && property_get_by_pointer(the_property, value) ?
		   buffer_to_string(value) : std::string();
}

void property_load_from_node(const pugi::xml_node& property,
							 std::string& name, std::string& value,
							 uint8_t& dynamic, uint8_t& over_write,
							 uint8_t& read_only, uint8_t& fail_on_error,
							 uint8_t& verbose)
{
	name = property.attribute("name").as_string();
	value = property.attribute("value").as_string();
	dynamic = property.attribute("dynamic").as_bool();
	const auto over_write_attribute = property.attribute("overwrite");
	over_write = nullptr == over_write_attribute ? 1 : over_write_attribute.as_bool();
	read_only = property.attribute("readonly").as_bool();
	const auto fail_on_error_attribute = property.attribute("failonerror");
	fail_on_error = nullptr == fail_on_error_attribute ? 1 : fail_on_error_attribute.as_bool();
	verbose = property.attribute("verbose").as_bool();
}

uint8_t properties_load_from_node(const pugi::xpath_node& node, const char* path, buffer* properties)
{
	for (const auto& property : node.node().select_nodes(path))
	{
		std::string name;
		std::string value;
		uint8_t dynamic = 0;
		uint8_t over_write = 0;
		uint8_t read_only = 0;
		uint8_t fail_on_error = 0;
		uint8_t verbose = 0;
		//
		property_load_from_node(property.node(), name, value, dynamic,
								over_write, read_only, fail_on_error, verbose);

		if (name.empty())
		{
			return 0;
		}

		const auto returned = property_set_by_name(
								  properties,
								  reinterpret_cast<const uint8_t*>(name.c_str()), static_cast<uint8_t>(name.size()),
								  reinterpret_cast<const uint8_t*>(value.c_str()), value.size(),
								  property_value_is_byte_array,
								  dynamic, over_write, read_only, verbose);

		if (!returned && fail_on_error)
		{
			return 0;
		}

		if (!fail_on_error && !returned)
		{
			static const std::string
			warn_about_failonerror("Failed to set property. Continue as demanded at 'fail on error' option.");

			if (!echo(0, Default, nullptr, Warning, reinterpret_cast<const uint8_t*>(warn_about_failonerror.c_str()),
					  warn_about_failonerror.size(), 1, 0))
			{
				return 0;
			}
		}
	}

	return 1;
}

uint8_t properties_free(buffer* properties)
{
	property_release(properties);
	return 0;
}

uint8_t argument_parser_free()
{
	argument_parser_release();
	return 0;
}

void add_slash(std::string& path)
{
	if (path.empty())
	{
		return;
	}

#if defined(_WIN32)

	if ('\\' != *(path.rbegin()))
	{
		path += '\\';
	}

#else

	if ('/' != *(path.rbegin()))
	{
		path += '/';
	}

#endif
}

std::string get_directory_for_current_process(buffer* tmp, uint8_t* result)
{
	static std::string current_directory;

	if (!tmp ||
		!result)
	{
		if (result)
		{
			*result = 0;
		}

		return current_directory;
	}

	if (current_directory.empty())
	{
		if (!path_get_directory_for_current_process(tmp))
		{
			*result = 0;
			return current_directory;
		}

		current_directory = buffer_to_string(tmp);
		add_slash(current_directory);
	}

	*result = 1;
	return current_directory;
}

uint8_t select_nodes_by_condition(
	const pugi::xpath_node_set& all_nodes,
	std::list<pugi::xpath_node>& nodes, buffer* tmp)
{
	if (!tmp)
	{
		return 0;
	}

	uint8_t condition = 0;

	for (const auto& node : all_nodes)
	{
		const uint8_t verbose = node.node().attribute("verbose").as_bool();

		if (!is_this_node_pass_by_if_condition(node, tmp, &condition, verbose))
		{
			return 0;
		}

		if (!condition)
		{
			continue;
		}

		nodes.push_back(node);
	}

	return !nodes.empty();
}

std::string get_path_to_directory_with_source(uint8_t* result)
{
	static std::string path_to_source;

	if (!result)
	{
		return path_to_source;
	}

	if (path_to_source.empty())
	{
		path_to_source = __FILE__;
		auto path_in_range(string_to_range(path_to_source));

		if (!path_get_directory_name(path_in_range.start, &path_in_range.finish))
		{
			*result = 0;
			return path_to_source;
		}

		path_to_source = range_to_string(path_in_range);
		add_slash(path_to_source);
	}

	*result = 1;
	return path_to_source;
}

std::string join_path(const std::string& path, const std::string& child_path)
{
	buffer tmp;
	SET_NULL_TO_BUFFER(tmp);
	//
	const auto path_(string_to_range(path));
	const auto child_path_(string_to_range(child_path));

	if (!path_combine(
			path_.start, path_.finish,
			child_path_.start, child_path_.finish, &tmp))
	{
		buffer_release(&tmp);
		return "";
	}

	const auto output(buffer_to_string(&tmp));
	buffer_release(&tmp);
	//
	return output;
}

bool starts_with_(const std::string& input, const std::string& value)
{
	return value.size() <= input.size() && value == input.substr(0, value.size());
}

std::string TestsBaseXml::tests_xml;
pugi::xml_document TestsBaseXml::document;
bool TestsBaseXml::loaded = false;
std::map<std::string, std::string*> TestsBase::predefine_arguments;

bool TestsBase::parse_input_arguments()
{
	const auto args = ::testing::internal::GetArgvs();

	for (const auto& arg : args)
	{
		for (const auto& pre_arg : predefine_arguments)
		{
			if (std::get<1>(pre_arg)->empty() && starts_with_(arg, std::get<0>(pre_arg)))
			{
				(*std::get<1>(pre_arg)) = arg.substr(std::get<0>(pre_arg).size());
				break;
			}
		}
	}

	for (const auto& pre_arg : predefine_arguments)
	{
		if (!std::get<1>(pre_arg)->empty())
		{
			continue;
		}

		std::string content;
		content += "[Error]: Argument ";
		content += std::get<0>(pre_arg);
		content += " is not defined.\n";
		content += "[Error]: Please set all required arguments.\n";

		for (const auto& pre_arg_ : predefine_arguments)
		{
			if (!std::get<1>(pre_arg_)->empty() ||
				std::get<0>(pre_arg) == std::get<0>(pre_arg_))
			{
				continue;
			}

			content += "[Error]: ";
			content += std::get<0>(pre_arg_);
			content.push_back('\n');
		}

		std::cerr << content;
		return false;
	}

	return true;
}

TestsBase::TestsBase()
{
}

void TestsBase::SetUp()
{
	const auto result = parse_input_arguments();
	assert(result);
	ASSERT_TRUE(result);
}

void TestsBase::TearDown()
{
}

TestsBase::~TestsBase()
{
}

void TestsBaseXml::load_nodes()
{
	const auto* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	const auto test_path = std::string("Tests") + "/" + test_info->test_case_name() + "/" + test_info->name();
	const auto all_nodes = document.select_nodes(pugi::xpath_query(test_path.c_str()));
	nodes.clear();
	//
	struct buffer tmp;
	SET_NULL_TO_BUFFER(tmp);
	//
	ASSERT_TRUE(select_nodes_by_condition(all_nodes, nodes, &tmp))
			<< test_path << std::endl << buffer_free(&tmp);
	//
	buffer_release(&tmp);
}

bool TestsBaseXml::load_document(pugi::xml_document& doc, const std::string& xml_file, unsigned int options)
{
	const auto result = doc.load_file(xml_file.c_str(), options);
	return pugi::xml_parse_status::status_ok == result.status;
}

TestsBaseXml::TestsBaseXml() :
	TestsBase(),
	nodes(),
	node_count(0)
{
	predefine_arguments.insert(std::make_pair("--tests_xml=", &tests_xml));
}

void TestsBaseXml::SetUp()
{
	TestsBase::SetUp();

	if (!loaded)
	{
		loaded = tests_xml.empty();
		assert(!loaded);
		ASSERT_FALSE(loaded);
		//
		loaded = load_document(document, tests_xml);
		assert(loaded);
		ASSERT_TRUE(loaded) << tests_xml << std::endl;
	}

	load_nodes();
	node_count = nodes.size();
	ASSERT_NE(0u, node_count) << "Test has no any case(s)." << std::endl;
}

void TestsBaseXml::TearDown()
{
	nodes.clear();
	ASSERT_EQ(0u, node_count);
}

TestsBaseXml::~TestsBaseXml()
{
}
