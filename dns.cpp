/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "dns.h"

#include <boost/asio.hpp>

#include <string>

static const auto dns_name_space = reinterpret_cast<const uint8_t*>("dns");
static const auto get_host_name = reinterpret_cast<const uint8_t*>("get-host-name");

const uint8_t* enumerate_name_spaces(ptrdiff_t index)
{
	if (0 != index)
	{
		return nullptr;
	}

	return dns_name_space;
}

const uint8_t* enumerate_functions(const uint8_t* name_space, ptrdiff_t index)
{
	if (dns_name_space != name_space || 0 != index)
	{
		return nullptr;
	}

	return get_host_name;
}

uint8_t evaluate_function(const uint8_t* function,
						  const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
						  const uint8_t** output, uint16_t* output_length)
{
	if (get_host_name != function ||
		nullptr == values ||
		nullptr == values_lengths ||
		0 != values_count ||
		nullptr == output ||
		nullptr == output_length)
	{
		return 0;
	}

	static const auto str_output = boost::asio::ip::host_name();
	*output = reinterpret_cast<const uint8_t*>(str_output.c_str());
	*output_length = static_cast<uint16_t>(str_output.size());
	//
	return !str_output.empty();
}
