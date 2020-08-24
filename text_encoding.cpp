/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include <cwchar>
#include <string>
#include <cstdint>

extern "C" {
	extern uint8_t text_encoding_decode_UTF8_single(const uint8_t* input_start, const uint8_t* input_finish,
			uint32_t* output);
	extern uint8_t text_encoding_encode_UTF8_single(uint32_t input, uint8_t* output);

	extern uint8_t text_encoding_decode_UTF16LE_single(const uint16_t* input_start, const uint16_t* input_finish,
			uint32_t* output);
	extern uint8_t text_encoding_encode_UTF16LE_single(uint32_t input, uint16_t* output);
};

std::wstring char_to_wchar_t(const uint8_t* input_start, const uint8_t* input_finish)
{
	if (input_finish < input_start)
	{
		return std::wstring();
	}

	std::wstring output(input_finish - input_start, 0);
	output.clear();
	/**/
	uint32_t out = 0;
	uint8_t bytes = 0;

	switch (sizeof(wchar_t))
	{
		case sizeof(uint16_t) :
			while (input_start < input_finish &&
				   0 < (bytes = text_encoding_decode_UTF8_single(input_start, input_finish, &out)))
			{
				input_start += bytes;
				uint16_t* ptr = (uint16_t*)&out;
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

			break;

		case sizeof(uint32_t) :
			while (input_start < input_finish &&
				   0 < (bytes = text_encoding_decode_UTF8_single(input_start, input_finish, &out)))
			{
				input_start += bytes;
				output.push_back((wchar_t)out);
			}

			break;

		default:
			break;
	}

	return output;
}

std::wstring char_to_wchar_t(const std::string& input)
{
	const uint8_t* input_start = (const uint8_t*)input.c_str();
	const uint8_t* input_finish = input_start + input.size();
	return char_to_wchar_t(input_start, input_finish);
}

std::string wchar_t_to_char(const std::wstring& input)
{
	std::string output(2 * input.size(), 0);
	output.clear();
	/**/
	const wchar_t* input_start = input.c_str();
	const wchar_t* input_finish = input_start + input.size();
	/**/
	uint8_t out[4];
	uint8_t bytes = 0;

	switch (sizeof(wchar_t))
	{
		case sizeof(uint16_t) :
		{
			uint32_t out_ = 0;

			while (input_start < input_finish &&
				   0 < (bytes = text_encoding_decode_UTF16LE_single((const uint16_t*)input_start, (const uint16_t*)input_finish,
								&out_)))
			{
				input_start += bytes;
				bytes = text_encoding_encode_UTF8_single(out_, out);

				if (!bytes)
				{
					break;
				}

				output.append((const char*)out, bytes);
			}
		}
		break;

		case sizeof(uint32_t) :
			while (input_start < input_finish && 0 < (bytes = text_encoding_encode_UTF8_single(*input_start, out)))
			{
				++input_start;
				output.append((const char*)out, bytes);
			}

			break;

		default:
			break;
	}

	return output;
}
