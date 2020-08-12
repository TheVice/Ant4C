/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "regex.h"

#include <boost/regex.hpp>

#include <string>

template<typename TYPE>
std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>> make_output_code(
			const std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>>& property_name,
			const std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>>& property_value)
{
	std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>> output;
	std::string tmp = "<property name=\"";
	output.append(tmp.cbegin(), tmp.cend());
	output.append(property_name.cbegin(), property_name.cend());
	tmp = "\" value=\"";
	output.append(tmp.cbegin(), tmp.cend());
	output.append(property_value.cbegin(), property_value.cend());
	tmp = "\" />";
	output.append(tmp.cbegin(), tmp.cend());
	return output;
}

template<typename TYPE>
std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>> regex(const
		std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>>& input,
		const std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>>& expression)
{
	const boost::basic_regex<TYPE, boost::regex_traits<TYPE>> regex_expression(expression);
	boost::match_results<typename std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>>::const_iterator>
			what;
	std::basic_string<TYPE, std::char_traits<TYPE>, std::allocator<TYPE>> output;

	if (boost::regex_search(input, what, regex_expression))
	{
		for (size_t i = 0, count = expression.size(); i < count; ++i)
		{
			for (auto j = i + 1; j < count; ++j)
			{
				auto match = what.named_subexpression(&expression[i], &expression[j]);

				if (match.length())
				{
					output += make_output_code(expression.substr(i, j - i), match.str());
				}
			}
		}
	}

	return output;
}

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
#if 0
std::wstring char_to_wchar_t(const std::string& input)
{
	const uint8_t* input_start = (const uint8_t*)input.c_str();
	const uint8_t* input_finish = input_start + input.size();
	return char_to_wchar_t(input_start, input_finish);
}
#endif
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

static const uint8_t* task_name = (const uint8_t*)"regex";

static const uint8_t* tasks_attributes[][3] =
{
	{ (const uint8_t*)"input", (const uint8_t*)"pattern", NULL }
};

static const uint8_t tasks_attributes_lengths[][3] =
{
	{ 5, 7, 0 }
};

const uint8_t* enumerate_tasks(uint8_t index)
{
	if (0 != index)
	{
		return NULL;
	}

	return task_name;
}

uint8_t get_attributes_and_arguments_for_task(const uint8_t* task,
		const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
		uint8_t* task_attributes_count)
{
	if (task_name != task ||
		NULL == task_attributes ||
		NULL == task_attributes_lengths ||
		NULL == task_attributes_count)
	{
		return 0;
	}

	*task_attributes = tasks_attributes[0];
	*task_attributes_lengths = tasks_attributes_lengths[0];
	*task_attributes_count = 2;
	/**/
	return 1;
}

uint8_t evaluate_task(const uint8_t* task,
					  const uint8_t** arguments, const uint16_t* arguments_lengths, uint8_t arguments_count,
					  const uint8_t** output, uint16_t* output_length,
					  uint8_t verbose)
{
	if (task_name != task ||
		NULL == arguments ||
		NULL == arguments_lengths ||
		arguments_count < 2 ||
		NULL == output ||
		NULL == output_length ||
		NULL == arguments[0] ||
		NULL == arguments[1] ||
		!arguments_lengths[0] ||
		!arguments_lengths[1])
	{
		return 0;
	}

	(void)verbose;
	uint8_t use_wchar = 0;

	for (uint16_t i = 0, count = arguments_lengths[0]; i < count; ++i)
	{
		if ((uint8_t)0x7f < arguments[0][i])
		{
			use_wchar = 1;
			break;
		}
	}

	if (!use_wchar)
	{
		for (uint16_t i = 0, count = arguments_lengths[1]; i < count; ++i)
		{
			if ((uint8_t)0x7f < arguments[1][i])
			{
				use_wchar = 1;
				break;
			}
		}
	}

	static std::string str_output;

	if (use_wchar)
	{
		const auto input_w = char_to_wchar_t(arguments[0], arguments[0] + arguments_lengths[0]);
		const auto pattern_w = char_to_wchar_t(arguments[1], arguments[1] + arguments_lengths[1]);
		str_output = wchar_t_to_char(regex(input_w, pattern_w));
	}
	else
	{
		str_output = regex(std::string((const char*)arguments[0], arguments_lengths[0]),
						   std::string((const char*)arguments[1], arguments_lengths[1]));
	}

	*output = (const uint8_t*)str_output.c_str();
	*output_length = (uint16_t)str_output.size();
	/**/
	return !str_output.empty();
}
