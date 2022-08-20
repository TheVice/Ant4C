/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#ifndef _TESTS_BASE_XML_H_
#define _TESTS_BASE_XML_H_

extern "C" {
#include "range.h"
};

#include <pugixml.hpp>
#include <gtest/gtest.h>

#include <map>
#include <list>
#include <cwchar>
#include <string>
#include <cstdint>

#define DOUBLE_PARSE(A) double_parse(reinterpret_cast<const uint8_t*>(A))

std::string buffer_to_string(const void* input);
range buffer_to_range(const void* input);

uint8_t string_to_buffer(const std::string& input, void* output);
range string_to_range(const std::string& input);

std::string range_to_string(
	const uint8_t* start_of_range, const uint8_t* finish_of_range);
std::string range_to_string(const range& input);
std::string range_to_string(const range* input);

void null_range_to_empty(range& input);

uint8_t buffer_free(void* input);
uint8_t buffer_resize_and_free_inner_buffers(void* storage);
uint8_t buffer_free_with_inner_buffers(void* storage);

std::string get_data_from_nodes(
	const pugi::xml_node& parent_node, const std::string& name_of_nodes);

uint8_t project_free(void* the_project);

std::string property_to_string(const void* the_property, void* value);

void property_load_from_node(
	const pugi::xml_node& property_in_a_xml,
	std::string& name, std::string& value,
	uint8_t& dynamic, uint8_t& over_write,
	uint8_t& read_only, uint8_t& fail_on_error,
	uint8_t& verbose);
uint8_t properties_load_from_node(
	const pugi::xpath_node& node, const char* path, void* properties);
uint8_t properties_free(void* properties);
uint8_t argument_parser_free();

void add_slash(std::string& path);
std::string get_directory_for_current_process(void* tmp, uint8_t* result);
/*uint8_t select_nodes_by_condition(
	const void* the_project, const pugi::xpath_node_set& all_nodes,
	std::list<pugi::xpath_node>& nodes, void* tmp);*/
std::string get_path_to_directory_with_source(uint8_t* result);
std::string join_path(const std::string& path, const std::string& child_path);
bool starts_with_(const std::string& input, const std::string& value);

extern std::wstring char_to_wchar_t(const std::string& input);

class TestsBase
{
protected:
	static std::map<std::string, std::string*> predefine_arguments;

protected:
	static bool parse_input_arguments();

protected:
	TestsBase();

	virtual void SetUp();
	virtual void TearDown();

	virtual ~TestsBase();
};

class TestsBaseXml : public TestsBase, public testing::Test
{
private:
	static std::string tests_xml;
	static pugi::xml_document document;
	static bool loaded;

protected:
	std::list<pugi::xpath_node> nodes;
	std::size_t node_count;

protected:
	static bool load_document(
		pugi::xml_document& doc, const std::string& xml_file,
		unsigned int options = pugi::parse_default | pugi::parse_ws_pcdata_single);

protected:
	void load_nodes(const void* the_project);

protected:
	TestsBaseXml();

	virtual void SetUp() override;
	virtual void TearDown() override;

	virtual ~TestsBaseXml();
};

#endif
