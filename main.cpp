
/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

#include <string>
#include <cstdlib>

int main(int argc, char** argv)
{
	(void)argc;
	std::string argument(argv[0]);
	return 0 == *(argument.data() + argument.size()) ? EXIT_SUCCESS : EXIT_FAILURE;
}
