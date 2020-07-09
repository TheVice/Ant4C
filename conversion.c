/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "conversion.h"

#include <stdlib.h>

int32_t int_parse(const uint8_t* value)
{
	return atoi((const char*)value);
}
