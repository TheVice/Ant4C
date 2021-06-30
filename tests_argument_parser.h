/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef _TESTS_ARGUMENT_PARSER_H_
#define _TESTS_ARGUMENT_PARSER_H_

#include "tests_base_xml.h"

#include <cstdint>

struct buffer;

class TestArgumentParser : public TestsBaseXml
{
public:
	static uint8_t get_properties(buffer* properties, uint8_t verbose);
};

#endif
