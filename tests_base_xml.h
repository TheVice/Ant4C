/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
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
#include <string>
#include <cstdint>

#define DOUBLE_PARSE(A) double_parse(reinterpret_cast<const uint8_t*>(A))
#define INT_PARSE(A) int_parse(reinterpret_cast<const uint8_t*>(A))

struct buffer;

std::string buffer_to_string(const buffer* input);
std::wstring buffer_to_u16string(const buffer* input);
range buffer_to_range(const buffer* input);

uint8_t string_to_buffer(const std::string& input, buffer* output);
range string_to_range(const std::string& input);

std::string range_to_string(const uint8_t* start_of_range, const uint8_t* finish_of_range);
std::string range_to_string(const range& input);
std::string range_to_string(const range* input);

void null_range_to_empty(range& input);

uint8_t buffer_free(buffer* input);
uint8_t buffer_resize_and_free_inner_buffers(buffer* storage);
uint8_t buffer_free_with_inner_buffers(buffer* storage);

uint8_t is_this_node_pass_by_if_condition(const pugi::xpath_node& node, buffer* tmp, uint8_t* condition,
		uint8_t verbose);
std::string get_data_from_nodes(const pugi::xpath_node& parent_node, const std::string& name_of_nodes);

uint8_t project_free(void* the_project);

std::string property_to_string(const void* the_property, buffer* value);

void property_load_from_node(const pugi::xml_node& property,
							 std::string& name, std::string& value,
							 uint8_t& dynamic, uint8_t& over_write,
							 uint8_t& read_only, uint8_t& fail_on_error,
							 uint8_t& verbose);
uint8_t properties_load_from_node(const pugi::xpath_node& node, const char* path, buffer* properties);
uint8_t properties_free(buffer* properties);

std::wstring u8string_to_u16string(const std::string& input);

class TestsBaseXml : public testing::Test
{
protected:
	pugi::xpath_node_set nodes;
	std::size_t node_count;
	uint8_t verbose;

private:
	static std::string tests_xml;
	static pugi::xml_document document;

private:
	void load_nodes();

protected:
	static std::map<std::string, std::string*> predefine_arguments;

protected:
	static bool parse_input_arguments();
	static bool load_document(pugi::xml_document& doc, const std::string& xml_file,
							  unsigned int options = pugi::parse_default | pugi::parse_ws_pcdata_single);

protected:
	TestsBaseXml();

	virtual void SetUp() override;
	virtual void TearDown() override;

	virtual ~TestsBaseXml();
};

#endif
