/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _TESTS_BASE_XML_H_
#define _TESTS_BASE_XML_H_

extern "C" {
#include "range.h"
};

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

#define _POSIXSOURCE 1

#include <sys/param.h>
#endif
#if !defined(BSD)
#define PUGIXML_HEADER_ONLY
#endif

#include <pugixml.hpp>
#include <gtest/gtest.h>

#include <map>
#include <string>
#include <cstdint>

#define DOUBLE_PARSE(A) double_parse((const uint8_t*)(A))
#define INT_PARSE(A) int_parse((const uint8_t*)(A))

struct buffer;

std::string buffer_to_string(const buffer* input);
std::wstring buffer_to_u16string(const buffer* input);
range buffer_to_range(const buffer* input);

uint8_t string_to_buffer(const std::string& input, buffer* output);
range string_to_range(const std::string& input);

std::string range_to_string(const uint8_t* start_of_range, const uint8_t* finish_of_range);
std::string range_to_string(const range& input);
std::string range_to_string(const range* input);

uint8_t buffer_free(buffer* input);
uint8_t buffer_resize_and_free_inner_buffers(buffer* storage);
uint8_t buffer_free_with_inner_buffers(buffer* storage);

uint8_t is_this_node_pass_by_if_condition(const pugi::xpath_node& node, buffer* tmp, uint8_t* condition);
std::string get_data_from_nodes(const pugi::xpath_node& parent_node, const std::string& name_of_nodes);

uint8_t properties_load_from_node(const pugi::xpath_node& node, const char* path, buffer* properties);
uint8_t properties_free(buffer* properties);

class TestsBaseXml : public testing::Test
{
protected:
	pugi::xpath_node_set nodes;
	std::size_t node_count;

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
