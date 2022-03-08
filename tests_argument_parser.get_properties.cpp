/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
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
	(void)verbose;
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

	if (!argument_parser_init() ||
		!argument_parser_char(0, argc, argv.data()))
	{
		argument_parser_release();
		return 0;
	}

	const auto* properties_ = argument_parser_get_properties();

	if (!buffer_append_data_from_buffer(properties, properties_))
	{
		argument_parser_release();
	}

	argument_parser_release();
	return 1;
}
