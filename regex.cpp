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
#if !defined(__MINGW32__) && !defined(__MINGW64__) && defined(__GNUC__) && (__cplusplus < 200000L)
	output.append(tmp.begin(), tmp.end());
	output.append(property_name.begin(), property_name.end());
	tmp = "\" value=\"";
	output.append(tmp.begin(), tmp.end());
	output.append(property_value.begin(), property_value.end());
	tmp = "\" />";
	output.append(tmp.begin(), tmp.end());
#else
	output.append(tmp.cbegin(), tmp.cend());
	output.append(property_name.cbegin(), property_name.cend());
	tmp = "\" value=\"";
	output.append(tmp.cbegin(), tmp.cend());
	output.append(property_value.cbegin(), property_value.cend());
	tmp = "\" />";
	output.append(tmp.cbegin(), tmp.cend());
#endif
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
				const auto match = what.named_subexpression(&expression[i], &expression[j]);

				if (match.length())
				{
					output += make_output_code(expression.substr(i, j - i), match.str());
				}
			}
		}
	}

	return output;
}

extern std::wstring char_to_wchar_t(const uint8_t* input_start, const uint8_t* input_finish);
extern std::string wchar_t_to_char(const std::wstring& input);

static const uint8_t* task_name = reinterpret_cast<const uint8_t*>("regex");

static const uint8_t* tasks_attributes[][3] =
{
	{
		reinterpret_cast<const uint8_t*>("input"),
		reinterpret_cast<const uint8_t*>("pattern"),
		nullptr
	}
};

static const uint8_t tasks_attributes_lengths[][3] =
{
	{ 5, 7, 0 }
};

const uint8_t* enumerate_tasks(ptrdiff_t index)
{
	if (0 != index)
	{
		return nullptr;
	}

	return task_name;
}

uint8_t get_attributes_and_arguments_for_task(const uint8_t* task,
		const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
		uint8_t* task_attributes_count)
{
	if (task_name != task ||
		nullptr == task_attributes ||
		nullptr == task_attributes_lengths ||
		nullptr == task_attributes_count)
	{
		return 0;
	}

	*task_attributes = tasks_attributes[0];
	*task_attributes_lengths = tasks_attributes_lengths[0];
	*task_attributes_count = 2;
	//
	return 1;
}

uint8_t evaluate_task(const uint8_t* task,
					  const uint8_t** arguments, const uint16_t* arguments_lengths, uint8_t arguments_count,
					  const uint8_t** output, uint16_t* output_length,
					  uint8_t verbose)
{
	if (task_name != task ||
		nullptr == arguments ||
		nullptr == arguments_lengths ||
		arguments_count < 2 ||
		nullptr == output ||
		nullptr == output_length ||
		nullptr == arguments[0] ||
		nullptr == arguments[1] ||
		!arguments_lengths[0] ||
		!arguments_lengths[1])
	{
		return 0;
	}

	(void)verbose;
	uint8_t use_wchar = 0;
	static const uint8_t max_ASCII_char = 0x7f;

	for (uint16_t i = 0, count = arguments_lengths[0]; i < count; ++i)
	{
		if (max_ASCII_char < arguments[0][i])
		{
			use_wchar = 1;
			break;
		}
	}

	if (!use_wchar)
	{
		for (uint16_t i = 0, count = arguments_lengths[1]; i < count; ++i)
		{
			if (max_ASCII_char < arguments[1][i])
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
		const auto expression_w = char_to_wchar_t(arguments[1], arguments[1] + arguments_lengths[1]);
		str_output = wchar_t_to_char(regex(input_w, expression_w));
	}
	else
	{
		const auto input = std::string(reinterpret_cast<const char*>(arguments[0]), arguments_lengths[0]);
		const auto expression = std::string(reinterpret_cast<const char*>(arguments[1]), arguments_lengths[1]);
		str_output = regex(input, expression);
	}

	*output = reinterpret_cast<const uint8_t*>(str_output.c_str());
	*output_length = static_cast<uint16_t>(str_output.size());
	//
	return !str_output.empty();
}
