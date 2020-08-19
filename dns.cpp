/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "dns.h"

#include <boost/asio.hpp>

#include <string>

static const uint8_t* dns = (const uint8_t*)"dns";
static const uint8_t* get_host_name = (const uint8_t*)"get-host-name";

const uint8_t* enumerate_name_spaces(uint8_t index)
{
	if (0 != index)
	{
		return NULL;
	}

	return dns;
}

const uint8_t* enumerate_functions(const uint8_t* name_space, uint8_t index)
{
	if (dns != name_space || 0 != index)
	{
		return NULL;
	}

	return get_host_name;
}

uint8_t evaluate_function(const uint8_t* function,
						  const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
						  const uint8_t** output, uint16_t* output_length)
{
	if (get_host_name != function ||
		NULL == values ||
		NULL == values_lengths ||
		0 != values_count ||
		NULL == output ||
		NULL == output_length)
	{
		return 0;
	}

	static std::string out = boost::asio::ip::host_name();
	*output = (const uint8_t*)out.c_str();
	*output_length = (uint16_t)out.size();
	/**/
	return !out.empty();
}
