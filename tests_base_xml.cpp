/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "conversion.h"
#include "echo.h"
#include "interpreter.h"
#include "property.h"
};

#include <cassert>
#include <utility>
#include <ostream>
#include <iostream>

#ifdef _WIN32
#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

GTEST_API_ int main(int argc, char** argv)
{
	std::cout << "Running main() from ";
	std::cout << __FILE__ << std::endl;
#ifdef _WIN32
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(NUMBER);
#endif
#endif
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

std::string buffer_to_string(const buffer* input)
{
	std::string output((NULL != input && 0 < buffer_size(input)) ? buffer_size(input) : 0, '\0');
	output.clear();

	if (NULL != input && NULL != buffer_char_data(input, 0))
	{
		output.append(buffer_char_data(input, 0), buffer_size(input));
	}

	return output;
}

std::wstring buffer_to_u16string(const buffer* input)
{
	std::wstring output((NULL != input &&
						 0 < buffer_size(input)) ? buffer_size(input) / sizeof(uint16_t) : 0, L'\0');
	output.clear();
	const uint16_t* ptr = NULL;

	if (NULL != input && NULL != (ptr = buffer_uint16_data(input, 0)))
	{
		switch (sizeof(wchar_t))
		{
			case 2:
				output.append((const wchar_t*)ptr, buffer_size(input) / sizeof(uint16_t));
				break;

			case 4:
			{
				ptrdiff_t i = 0;

				while (NULL != (ptr = buffer_uint16_data(input, i++)))
				{
					uint16_t value = *ptr;
					output.push_back(value);
				}
			}
			break;

			default:
				break;
		}
	}

	return output;
}

range buffer_to_range(const buffer* input)
{
	range output;
	output.start = output.finish = NULL;

	if (NULL == input ||
		NULL == (output.start = buffer_char_data(input, 0)))
	{
		return output;
	}

	output.finish = output.start + buffer_size(input);
	return output;
}

uint8_t string_to_buffer(const std::string& input, buffer* output)
{
	return NULL != output && buffer_append_char(output, input.empty() ? NULL : input.data(), input.size());
}

range string_to_range(const std::string& input)
{
	range output;
	output.start = input.empty() ? NULL : input.data();
	output.finish = input.empty() ? NULL : input.data() + input.size();
	return output;
}

std::string range_to_string(const char* start_of_range, const char* finish_of_range)
{
	static const std::string empty;

	if (NULL == start_of_range ||
		NULL == finish_of_range ||
		finish_of_range <= start_of_range)
	{
		return empty;
	}

	return std::string(start_of_range, finish_of_range - start_of_range);
}

std::string range_to_string(const range& input)
{
	return range_to_string(input.start, input.finish);
}

std::string range_to_string(const range* input)
{
	return (NULL == input) ? range_to_string(NULL, NULL) : range_to_string(input->start, input->finish);
}

uint8_t buffer_free(buffer* input)
{
	buffer_release(input);
	return 0;
}

uint8_t buffer_resize_and_free_inner_buffers(buffer* storage)
{
	ptrdiff_t i = 0;
	buffer* ptr = NULL;

	while (NULL != (ptr = buffer_buffer_data(storage, i++)))
	{
		buffer_release(ptr);
	}

	return buffer_resize(storage, 0);
}

uint8_t buffer_free_with_inner_buffers(buffer* storage)
{
	buffer_release_with_inner_buffers(storage);
	return 0;
}

uint8_t is_this_node_pass_by_if_condition(const pugi::xpath_node& node, buffer* tmp, uint8_t* condition)
{
	if (NULL == tmp || NULL == condition)
	{
		return 0;
	}

	const std::string if_(node.node().attribute("if").as_string());

	if (!if_.empty())
	{
		if (!buffer_resize(tmp, 0))
		{
			return 0;
		}

		range code = string_to_range(if_);

		if (!interpreter_evaluate_code(NULL, NULL, &code, tmp))
		{
			return 0;
		}

		code = buffer_to_range(tmp);
		return bool_parse(code.start, code.finish, condition);
	}

	(*condition) = 1;
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

uint8_t properties_load_from_node(const pugi::xpath_node& node, const char* path, buffer* properties)
{
	for (const auto& property : node.node().select_nodes(path))
	{
		const std::string name(property.node().attribute("name").as_string());

		if (name.empty())
		{
			return 0;
		}

		const std::string value(property.node().attribute("value").as_string());
		const uint8_t dynamic = property.node().attribute("dynamic").as_bool();
		const auto overwrite_attribute = property.node().attribute("overwrite");
		const uint8_t overwrite = NULL == overwrite_attribute ? 1 : overwrite_attribute.as_bool();
		const uint8_t readonly = property.node().attribute("readonly").as_bool();
		const auto failonerror_attribute = property.node().attribute("failonerror");
		const uint8_t failonerror = NULL == failonerror_attribute ? 1 : failonerror_attribute.as_bool();
		const uint8_t verbose = property.node().attribute("verbose").as_bool();
		/**/
		const uint8_t returned = property_set_by_name(
									 NULL, NULL, properties,
									 name.c_str(), (uint8_t)name.size(),
									 value.c_str(), value.size(),
									 property_value_is_char_array,
									 dynamic, overwrite, readonly, verbose);

		if (!returned && failonerror)
		{
			return 0;
		}

		if (!failonerror && !returned)
		{
			static const std::string
			warn_about_failonerror("Failed to set property. Continue as demanded at 'fail on error' option.");

			if (!echo(0, Default, NULL, Warning, warn_about_failonerror.c_str(), warn_about_failonerror.size(), 1, 0))
			{
				return 0;
			}
		}
	}

	return 1;
}

uint8_t properties_free(buffer* properties)
{
	property_clear(properties);
	return 0;
}

std::string TestsBaseXml::tests_xml;
pugi::xml_document TestsBaseXml::document;
std::map<std::string, std::string*> TestsBaseXml::predefine_arguments;

static bool starts_with_(const std::string& input, const std::string& value)
{
	return (value.size() <= input.size() && value == input.substr(0, value.size()));
}

bool TestsBaseXml::parse_input_arguments()
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

void TestsBaseXml::load_nodes()
{
	const auto* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	const auto test_path = std::string("Tests") + "/" + test_info->test_case_name() + "/" + test_info->name();
	nodes = document.select_nodes(pugi::xpath_query(test_path.c_str()));
}

bool TestsBaseXml::load_document(pugi::xml_document& doc, const std::string& xml_file, unsigned int options)
{
	const auto result = doc.load_file(xml_file.c_str(), options);
	return pugi::xml_parse_status::status_ok == result.status;
}

TestsBaseXml::TestsBaseXml() : nodes(), node_count(0)
{
	predefine_arguments.insert(std::make_pair("--tests_xml=", &tests_xml));
}

void TestsBaseXml::SetUp()
{
	if (tests_xml.empty())
	{
		auto result = parse_input_arguments();
		assert(result);
		ASSERT_TRUE(result);
		//
		result = tests_xml.empty();
		assert(!result);
		ASSERT_FALSE(result);
		//
		result = load_document(document, tests_xml);
		assert(result);
		ASSERT_TRUE(result) << tests_xml << std::endl;
	}

	load_nodes();
	node_count = nodes.size();
	ASSERT_NE(0u, node_count) << "Test has no any case(s)." << std::endl;
}

void TestsBaseXml::TearDown()
{
	ASSERT_EQ(0u, node_count);
}

TestsBaseXml::~TestsBaseXml()
{
}
