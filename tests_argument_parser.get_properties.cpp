/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "tests_argument_parser.h"

extern "C" {
#include "argument_parser.h"
#include "buffer.h"
#include "property.h"
};

#include <vector>
#include <string>

uint8_t TestArgumentParser::get_properties(buffer* properties, uint8_t verbose)
{
	static std::string empty("");

	if (!properties)
	{
		return 0;
	}

	auto args = ::testing::internal::GetArgvs();
	const auto argc = static_cast<int>(args.size());
	std::vector<char*> argv(argc);

	for (auto i = 0; i < argc; ++i)
	{
		argv[i] = args[i].empty() ? &empty[0] : &(args[i][0]);
	}

	buffer arguments;
	SET_NULL_TO_BUFFER(arguments);

	if (!argument_parser_char(0, argc, argv.data(), &arguments, verbose))
	{
		property_release(&arguments);
		return 0;
	}

	if (!argument_parser_get_properties(&arguments, properties, verbose))
	{
		property_release(properties);
	}

	property_release(&arguments);
	return 1;
}
