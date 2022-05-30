/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#ifndef _GLOBAL_ARGUMENT_PARSER_H_
#define _GLOBAL_ARGUMENT_PARSER_H_

#include <string>
#include <vector>
#include <cstdint>

class GlobalArgumentParser
{
	static int argc;
	static std::string empty;
	static std::vector<char*> argv;
	static std::vector<std::string> args;

public:
	static uint8_t get_properties(void* the_project, uint8_t verbose);
};

#endif
