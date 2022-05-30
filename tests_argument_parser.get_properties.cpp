/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "tests_argument_parser.h"

extern "C" {
#include "argument_parser.h"
#include "property.h"
};

#include <gtest/gtest.h>

int GlobalArgumentParser::argc = 0;
std::string GlobalArgumentParser::empty("");
std::vector<char*> GlobalArgumentParser::argv;
std::vector<std::string> GlobalArgumentParser::args;

uint8_t GlobalArgumentParser::get_properties(void* the_project, uint8_t verbose)
{
	if (!the_project)
	{
		return 0;
	}

	if (argc < 1)
	{
		args = ::testing::internal::GetArgvs();
		argc = static_cast<int>(args.size());
		argv.resize(argc);

		for (auto i = 0; i < argc; ++i)
		{
			argv[i] = args[i].empty() ? &empty[0] : &(args[i][0]);
		}
	}

	if (!argument_parser_init() ||
		!argument_parser_char(0, argc, argv.data()))
	{
		argument_parser_release();
		return 0;
	}

	const auto* properties = argument_parser_get_properties();

	if (!property_add_at_project(the_project, properties, verbose))
	{
		argument_parser_release();
		return 0;
	}

	argument_parser_release();
	return 1;
}
