/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#ifndef _GTEST_ARGUMENT_PARSER_H__
#define _GTEST_ARGUMENT_PARSER_H__

#include <string>
#include <vector>
#include <cstdint>

class GTestArgumentParser
{
	static int argc;
	static std::string empty;
	static std::vector<char*> argv;
	static std::vector<std::string> args;

public:
	static uint8_t set_to_project_properties(void* the_project, uint8_t verbose);
};

#endif
