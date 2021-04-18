/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2021 https://github.com/TheVice/
 *
 */

extern "C" {
#include "text_encoding.h"
};

#include <cwchar>
#include <string>
#include <cstdint>

#if defined(_WIN32)
extern "C" {
	extern uint8_t text_encoding_decode_UTF16LE_single(
		const uint16_t* input_start, const uint16_t* input_finish, uint32_t* output);
	extern uint8_t text_encoding_encode_UTF16LE_single(uint32_t input, uint16_t* output);
};
#endif

std::wstring char_to_wchar_t(const uint8_t* input_start, const uint8_t* input_finish)
{
	if (input_finish < input_start)
	{
		return std::wstring();
	}

	std::wstring output(input_finish - input_start, 0);
	output.clear();
	//
	uint32_t out = 0;
	uint8_t bytes = 0;
#if defined(_WIN32)

	while (input_start < input_finish &&
		   0 < (bytes = text_encoding_decode_UTF8_single(input_start, input_finish, &out)))
	{
		input_start += bytes;
		uint16_t* ptr = reinterpret_cast<uint16_t*>(&out);
		bytes = text_encoding_encode_UTF16LE_single(out, ptr);

		if (!bytes || 2 < bytes)
		{
			break;
		}

		while (bytes)
		{
			output.push_back(*ptr);
			++ptr;
			--bytes;
		}
	}

#else

	while (input_start < input_finish &&
		   0 < (bytes = text_encoding_decode_UTF8_single(input_start, input_finish, &out)))
	{
		input_start += bytes;
		output.push_back(static_cast<wchar_t>(out));
	}

#endif
	return output;
}

std::wstring char_to_wchar_t(const std::string& input)
{
	const auto input_start = reinterpret_cast<const uint8_t*>(input.c_str());
	const auto input_finish = input_start + input.size();
	return char_to_wchar_t(input_start, input_finish);
}

std::string wchar_t_to_char(const wchar_t* input_start, const wchar_t* input_finish)
{
	if (input_finish < input_start)
	{
		return std::string();
	}

	std::string output(2 * (input_finish - input_start), 0);
	output.clear();
	//
	uint8_t out[4];
	uint8_t bytes = 0;
#if defined(_WIN32)
	uint32_t char_code = 0;

	while (input_start < input_finish &&
		   0 < (bytes = text_encoding_decode_UTF16LE_single(
							reinterpret_cast<const uint16_t*>(input_start),
							reinterpret_cast<const uint16_t*>(input_finish),
							&char_code)))
	{
		input_start += bytes;
		bytes = text_encoding_encode_UTF8_single(char_code, out);

		if (!bytes)
		{
			break;
		}

		output.append(reinterpret_cast<const char*>(out), bytes);
	}

#else

	while (input_start < input_finish && 0 < (bytes = text_encoding_encode_UTF8_single(*input_start, out)))
	{
		++input_start;
		output.append(reinterpret_cast<const char*>(out), bytes);
	}

#endif
	return output;
}

std::string wchar_t_to_char(const std::wstring& input)
{
	const auto input_start = input.c_str();
	const auto input_finish = input_start + input.size();
	return wchar_t_to_char(input_start, input_finish);
}
