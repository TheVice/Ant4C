/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "conversion.h"
#include "date_time.h"
#include "range.h"
};

#include <cstdint>

class TestDateTime : public TestsBaseXml
{
};

TEST_F(TestDateTime, datetime_format_to_string)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		const uint32_t year = int_parse(node.node().select_node("year").node().child_value());
		const uint8_t month = (uint8_t)int_parse(node.node().select_node("month").node().child_value());
		const uint8_t day = (uint8_t)int_parse(node.node().select_node("day").node().child_value());
		const uint8_t hour = (uint8_t)int_parse(node.node().select_node("hour").node().child_value());
		const uint8_t minute = (uint8_t)int_parse(node.node().select_node("minute").node().child_value());
		const uint8_t second = (uint8_t)int_parse(node.node().select_node("second").node().child_value());
		//
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		const std::string format(node.node().select_node("format").node().child_value());
#if (defined(_MSC_VER) && (1900 < _MSC_VER) || !defined(_WIN32))
		const std::string expected_output(node.node().select_node("output").node().child_value());
#else
		const std::string expected_output(node.node().select_node("mingw_output").node().child_value());
#endif
		//
		const int64_t date_time = datetime_encode(year, month, day, hour, minute, second);
		//
		const uint8_t returned = datetime_format_to_string(date_time, format.c_str(), &output);
		ASSERT_EQ(expected_return, returned) << year << " " << (int)month << " " << (int)day << " " <<
											 (int)hour << " " << (int)minute << " " << (int)second << std::endl <<
											 format << std::endl << buffer_free(&output);
		//
		ASSERT_EQ(expected_output, buffer_to_string(&output)) <<
				year << " " << (int)month << " " << (int)day << " " <<
				(int)hour << " " << (int)minute << " " << (int)second << std::endl <<
				format << std::endl << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}

TEST_F(TestDateTime, datetime_parse)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		//
		const uint32_t year = int_parse(node.node().select_node("year").node().child_value());
		const uint8_t month = (uint8_t)int_parse(node.node().select_node("month").node().child_value());
		const uint8_t day = (uint8_t)int_parse(node.node().select_node("day").node().child_value());
		const uint8_t hour = (uint8_t)int_parse(node.node().select_node("hour").node().child_value());
		const uint8_t minute = (uint8_t)int_parse(node.node().select_node("minute").node().child_value());
		const uint8_t second = (uint8_t)int_parse(node.node().select_node("second").node().child_value());
		//
		const range input_in_range = string_to_range(input);
		//
		uint32_t returned_year = 0;
		uint8_t returned_month = 0;
		uint8_t returned_day = 0;
		uint8_t returned_hour = 24;
		uint8_t returned_minute = 60;
		uint8_t returned_second = 60;
		//
		const uint8_t returned = datetime_parse(input_in_range.start, input_in_range.finish,
												&returned_year, &returned_month, &returned_day,
												&returned_hour, &returned_minute, &returned_second);
		ASSERT_EQ(expected_return, returned) << input;

		if (!returned)
		{
			returned_hour = 0;
			returned_minute = 0;
			returned_second = 0;
		}

		ASSERT_EQ(year, returned_year) << input;
		ASSERT_EQ(month, returned_month) << input;
		ASSERT_EQ(day, returned_day) << input;
		ASSERT_EQ(hour, returned_hour) << input;
		ASSERT_EQ(minute, returned_minute) << input;
		ASSERT_EQ(second, returned_second) << input;
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_to_string)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		const uint32_t year = int_parse(node.node().select_node("year").node().child_value());
		const uint8_t month = (uint8_t)int_parse(node.node().select_node("month").node().child_value());
		const uint8_t day = (uint8_t)int_parse(node.node().select_node("day").node().child_value());
		const uint8_t hour = (uint8_t)int_parse(node.node().select_node("hour").node().child_value());
		const uint8_t minute = (uint8_t)int_parse(node.node().select_node("minute").node().child_value());
		const uint8_t second = (uint8_t)int_parse(node.node().select_node("second").node().child_value());
		//
		const std::string expected_output(node.node().select_node("output").node().child_value());
		//
		ASSERT_TRUE(datetime_to_string(year, month, day, hour, minute, second, &output)) <<
				year << " " << (int)month << " " << (int)day << " " <<
				(int)hour << " " << (int)minute << " " << (int)second
				<< std::endl << buffer_free(&output);
		ASSERT_EQ(expected_output, buffer_to_string(&output)) <<
				year << " " << (int)month << " " << (int)day << " " <<
				(int)hour << " " << (int)minute << " " << (int)second
				<< std::endl << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}

TEST_F(TestDateTime, datetime_get_day)
{
	for (const auto& node : nodes)
	{
		const uint8_t input = (uint8_t)int_parse(node.node().select_node("input").node().child_value());
		//
		const int64_t day = datetime_encode(2019, 1, input, 0, 0, 0);
		ASSERT_EQ(input, datetime_get_day(day));
		//
		--node_count;
	}
}
/*datetime_get_day_of_week
datetime_get_day_of_year*/
TEST_F(TestDateTime, datetime_get_days_in_month)
{
	for (const auto& node : nodes)
	{
		const uint32_t year = int_parse(node.node().select_node("year").node().child_value());
		const uint8_t month = (uint8_t)int_parse(node.node().select_node("month").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		//
		ASSERT_EQ(expected_return, datetime_get_days_in_month(year, month)) << year << " " << (int)month;
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_get_hour)
{
	for (const auto& node : nodes)
	{
		const uint8_t input = (uint8_t)int_parse(node.node().select_node("input").node().child_value());
		//
		const int64_t hour = datetime_encode(2019, 1, 1, input, 0, 0);
		ASSERT_EQ(input, datetime_get_hour(hour));
		//
		--node_count;
	}
}
/*datetime_get_millisecond*/
TEST_F(TestDateTime, datetime_get_minute)
{
	for (const auto& node : nodes)
	{
		const uint8_t input = (uint8_t)int_parse(node.node().select_node("input").node().child_value());
		//
		const int64_t minute = datetime_encode(2019, 1, 1, 0, input, 0);
		ASSERT_EQ(input, datetime_get_minute(minute));
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_get_month)
{
	for (const auto& node : nodes)
	{
		const uint8_t input = (uint8_t)int_parse(node.node().select_node("input").node().child_value());
		//
		const int64_t month = datetime_encode(2019, input, 1, 0, 0, 0);
		ASSERT_EQ(input, datetime_get_month(month));
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_get_second)
{
	for (const auto& node : nodes)
	{
		const uint8_t input = (uint8_t)int_parse(node.node().select_node("input").node().child_value());
		//
		const int64_t second = datetime_encode(2019, 1, 1, 0, 0, input);
		ASSERT_EQ(input, datetime_get_second(second));
		//
		--node_count;
	}
}
/*datetime_get_ticks*/
TEST_F(TestDateTime, datetime_get_year)
{
	for (const auto& node : nodes)
	{
		const uint32_t input = int_parse(node.node().select_node("input").node().child_value());
		//
		const int64_t year = datetime_encode(input, 1, 1, 0, 0, 0);
		ASSERT_EQ(input, datetime_get_year(year));
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_is_leap_year)
{
	for (const auto& node : nodes)
	{
		const uint32_t input = int_parse(node.node().select_node("input").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const uint8_t returned = datetime_is_leap_year(input);
		ASSERT_EQ(expected_return, returned) << input;
		//
		--node_count;
	}
}

TEST(TestDateTime_, datetime_now)
{
	const int64_t now = datetime_now();
	ASSERT_LT(0, now);
	//
	uint32_t returned_year = 0;
	uint8_t returned_month = 0;
	uint8_t returned_day = 0;
	uint8_t returned_hour = 0;
	uint8_t returned_minute = 0;
	uint8_t returned_second = 0;
	uint16_t returned_year_day = 0;
	//
	ASSERT_TRUE(datetime_decode(now, &returned_year, &returned_month, &returned_day,
								&returned_hour, &returned_minute, &returned_second,
								&returned_year_day));
	//
	const int64_t returned_now = datetime_encode(returned_year, returned_month, returned_day, returned_hour,
								 returned_minute, returned_second);
	//
	ASSERT_EQ(now, returned_now) <<
								 returned_year << " " << (int)returned_month << " " << (int)returned_day << " " <<
								 (int)returned_hour << " " << (int)returned_minute << " " << (int)returned_second;
}

TEST_F(TestDateTime, datetime_decode)
{
	for (const auto& node : nodes)
	{
		const int64_t input = int64_parse(node.node().select_node("input").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		//
		const uint32_t expected_year = int_parse(node.node().select_node("year").node().child_value());
		const uint8_t expected_month = (uint8_t)int_parse(node.node().select_node("month").node().child_value());
		const uint8_t expected_day = (uint8_t)int_parse(node.node().select_node("day").node().child_value());
		const uint8_t expected_hour = (uint8_t)int_parse(node.node().select_node("hour").node().child_value());
		const uint8_t expected_minute = (uint8_t)int_parse(node.node().select_node("minute").node().child_value());
		const uint8_t expected_second = (uint8_t)int_parse(node.node().select_node("second").node().child_value());
		const uint16_t expected_year_day = (uint16_t)int_parse(
											   node.node().select_node("year_day").node().child_value());
		//
		uint32_t returned_year = 0;
		uint8_t returned_month = 0;
		uint8_t returned_day = 0;
		uint8_t returned_hour = 0;
		uint8_t returned_minute = 0;
		uint8_t returned_second = 0;
		uint16_t returned_year_day = 0;
		//
		const uint8_t returned = datetime_decode(input, &returned_year, &returned_month, &returned_day,
								 &returned_hour, &returned_minute, &returned_second,
								 &returned_year_day);
		ASSERT_EQ(expected_return, returned) << input;
		//
		ASSERT_EQ(expected_year, returned_year) << input;
		ASSERT_EQ(expected_month, returned_month) << input;
		ASSERT_EQ(expected_day, returned_day) << input;
		ASSERT_EQ(expected_hour, returned_hour) << input;
		ASSERT_EQ(expected_minute, returned_minute) << input;
		ASSERT_EQ(expected_second, returned_second) << input;
		ASSERT_EQ(returned_year_day, expected_year_day) << input;
		//
		--node_count;
	}
}
