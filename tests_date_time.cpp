/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019, 2021 - 2022, 2024 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "range.h"
};

#include <string>
#include <cstdint>

int INT_PARSE(const char* input)
{
	const std::string i(input);
	const auto r(string_to_range(i));
	return int_parse(r.start, r.finish);
}

class TestDateTime : public TestsBaseXml
{
};

TEST_F(TestDateTime, datetime_format_to_string)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		//
		const auto year = INT_PARSE(the_node.select_node("year").node().child_value());
		const auto month = static_cast<uint8_t>(INT_PARSE(the_node.select_node("month").node().child_value()));
		const auto day = static_cast<uint8_t>(INT_PARSE(the_node.select_node("day").node().child_value()));
		const auto hour = static_cast<uint8_t>(INT_PARSE(the_node.select_node("hour").node().child_value()));
		const auto minute = static_cast<uint8_t>(INT_PARSE(the_node.select_node("minute").node().child_value()));
		const auto second = static_cast<uint8_t>(INT_PARSE(the_node.select_node("second").node().child_value()));
		//
		const auto expected_return = static_cast<uint8_t>(INT_PARSE(
										 the_node.select_node("return").node().child_value()));
		const std::string format(the_node.select_node("format").node().child_value());
		const std::string expected_output(the_node.select_node("output").node().child_value());
		const auto date_time = datetime_encode(year, month, day, hour, minute, second);
		//
		const auto returned = datetime_format_to_string(
								  date_time, reinterpret_cast<const uint8_t*>(format.c_str()), output);
		ASSERT_EQ(expected_return, returned) <<
											 year << " " <<
											 static_cast<int>(month) << " " <<
											 static_cast<int>(day) << " " <<
											 static_cast<int>(hour) << " " <<
											 static_cast<int>(minute) << " " <<
											 static_cast<int>(second) << std::endl <<
											 format << std::endl << buffer_free(output);
		//
		ASSERT_EQ(expected_output, buffer_to_string(output)) <<
				year << " " <<
				static_cast<int>(month) << " " <<
				static_cast<int>(day) << " " <<
				static_cast<int>(hour) << " " <<
				static_cast<int>(minute) << " " <<
				static_cast<int>(second) << std::endl <<
				format << std::endl << buffer_free(output);
		//
		--node_count;
	}

	buffer_release(output);
}

TEST_F(TestDateTime, datetime_parse)
{
	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const std::string input(the_node.select_node("input").node().child_value());
		const auto expected_return = static_cast<uint8_t>(INT_PARSE(
										 the_node.select_node("return").node().child_value()));
		//
		const auto year = static_cast<uint32_t>(INT_PARSE(the_node.select_node("year").node().child_value()));
		const auto month = static_cast<uint8_t>(INT_PARSE(the_node.select_node("month").node().child_value()));
		const auto day = static_cast<uint8_t>(INT_PARSE(the_node.select_node("day").node().child_value()));
		const auto hour = static_cast<uint8_t>(INT_PARSE(the_node.select_node("hour").node().child_value()));
		const auto minute = static_cast<uint8_t>(INT_PARSE(the_node.select_node("minute").node().child_value()));
		const auto second = static_cast<uint8_t>(INT_PARSE(the_node.select_node("second").node().child_value()));
		//
		const auto input_in_range = string_to_range(input);
		//
		uint32_t returned_year = 0;
		uint8_t returned_month = 0;
		uint8_t returned_day = 0;
		uint8_t returned_hour = 24;
		uint8_t returned_minute = 60;
		uint8_t returned_second = 60;
		//
		auto returned = datetime_parse(input_in_range.start, input_in_range.finish,
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
		int64_t returned_total_second = 0;
		returned = datetime_parse_range(&input_in_range, &returned_total_second);
		//
		ASSERT_EQ(expected_return, returned) << input;
		//
		uint16_t year_day;
		//
		ASSERT_TRUE(datetime_decode(
						returned_total_second, &returned_year, &returned_month, &returned_day,
						&returned_hour, &returned_minute, &returned_second, &year_day)) << input;

		if (returned)
		{
			ASSERT_EQ(year, returned_year) << input;
			ASSERT_EQ(month, returned_month) << input;
			ASSERT_EQ(day, returned_day) << input;
			ASSERT_EQ(hour, returned_hour) << input;
			ASSERT_EQ(minute, returned_minute) << input;
			ASSERT_EQ(second, returned_second) << input;
		}

		--node_count;
	}
}

TEST_F(TestDateTime, datetime_to_string)
{
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of()));

	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		ASSERT_TRUE(buffer_resize(output, 0)) << buffer_free(output);
		//
		const auto year = INT_PARSE(the_node.select_node("year").node().child_value());
		const auto month = static_cast<uint8_t>(INT_PARSE(the_node.select_node("month").node().child_value()));
		const auto day = static_cast<uint8_t>(INT_PARSE(the_node.select_node("day").node().child_value()));
		const auto hour = static_cast<uint8_t>(INT_PARSE(the_node.select_node("hour").node().child_value()));
		const auto minute = static_cast<uint8_t>(INT_PARSE(the_node.select_node("minute").node().child_value()));
		const auto second = static_cast<uint8_t>(INT_PARSE(the_node.select_node("second").node().child_value()));
		//
		const std::string expected_output(the_node.select_node("output").node().child_value());
		//
		ASSERT_TRUE(datetime_to_string(year, month, day, hour, minute, second, output)) <<
				year << " " <<
				static_cast<int>(month) << " " <<
				static_cast<int>(day) << " " <<
				static_cast<int>(hour) << " " <<
				static_cast<int>(minute) << " " <<
				static_cast<int>(second) << std::endl << buffer_free(output);
		ASSERT_EQ(expected_output, buffer_to_string(output)) <<
				year << " " <<
				static_cast<int>(month) << " " <<
				static_cast<int>(day) << " " <<
				static_cast<int>(hour) << " " <<
				static_cast<int>(minute) << " " <<
				static_cast<int>(second) << std::endl << buffer_free(output);
		//
		--node_count;
	}

	buffer_release(output);
}

TEST_F(TestDateTime, datetime_get_day)
{
	for (const auto& node : nodes)
	{
		const auto input = static_cast<uint8_t>(INT_PARSE(node.node().select_node("input").node().child_value()));
		//
		const auto day = datetime_encode(2019, 1, input, 0, 0, 0);
		ASSERT_EQ(input, datetime_get_day(day));
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_get_day_of_week)
{
	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const auto year = INT_PARSE(the_node.select_node("year").node().child_value());
		const auto month = static_cast<uint8_t>(INT_PARSE(the_node.select_node("month").node().child_value()));
		const auto day = static_cast<uint8_t>(INT_PARSE(the_node.select_node("day").node().child_value()));
		const auto expected_return = static_cast<uint8_t>(INT_PARSE(
										 the_node.select_node("return").node().child_value()));
		//
		const auto input = datetime_encode(year, month, day, 0, 0, 0);
		//
		const auto returned = datetime_get_day_of_week(input);
		//
		ASSERT_EQ(expected_return, returned) <<
											 year << " " <<
											 static_cast<int>(month) << " " <<
											 static_cast<int>(day);
		//
		--node_count;
	}
}
/*datetime_get_day_of_year*/
TEST_F(TestDateTime, datetime_get_days_in_month)
{
	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const auto year = INT_PARSE(the_node.select_node("year").node().child_value());
		const auto month = static_cast<uint8_t>(INT_PARSE(the_node.select_node("month").node().child_value()));
		const auto expected_return = static_cast<uint8_t>(INT_PARSE(
										 the_node.select_node("return").node().child_value()));
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
		const auto input = static_cast<uint8_t>(INT_PARSE(node.node().select_node("input").node().child_value()));
		//
		const auto hour = datetime_encode(2019, 1, 1, input, 0, 0);
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
		const auto input = static_cast<uint8_t>(INT_PARSE(node.node().select_node("input").node().child_value()));
		//
		const auto minute = datetime_encode(2019, 1, 1, 0, input, 0);
		ASSERT_EQ(input, datetime_get_minute(minute));
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_get_month)
{
	for (const auto& node : nodes)
	{
		const auto input = static_cast<uint8_t>(INT_PARSE(node.node().select_node("input").node().child_value()));
		//
		const auto month = datetime_encode(2019, input, 1, 0, 0, 0);
		ASSERT_EQ(input, datetime_get_month(month));
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_get_second)
{
	for (const auto& node : nodes)
	{
		const auto input = static_cast<uint8_t>(INT_PARSE(node.node().select_node("input").node().child_value()));
		//
		const auto second = datetime_encode(2019, 1, 1, 0, 0, input);
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
		const auto input = static_cast<uint32_t>(INT_PARSE(node.node().select_node("input").node().child_value()));
		//
		const auto year = datetime_encode(input, 1, 1, 0, 0, 0);
		ASSERT_EQ(input, datetime_get_year(year));
		//
		--node_count;
	}
}

TEST_F(TestDateTime, datetime_is_leap_year)
{
	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const auto input = INT_PARSE(the_node.select_node("input").node().child_value());
		const auto expected_return = static_cast<uint8_t>(INT_PARSE(
										 the_node.select_node("return").node().child_value()));
		//
		const auto returned = datetime_is_leap_year(input);
		ASSERT_EQ(expected_return, returned) << input;
		//
		--node_count;
	}
}

TEST(TestDateTime_, datetime_now)
{
	const auto now = datetime_now();
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
	const auto returned_now = datetime_encode(returned_year, returned_month, returned_day, returned_hour,
							  returned_minute, returned_second);
	//
	ASSERT_EQ(now, returned_now) <<
								 returned_year << " " <<
								 static_cast<int>(returned_month) << " " <<
								 static_cast<int>(returned_day) << " " <<
								 static_cast<int>(returned_hour) << " " <<
								 static_cast<int>(returned_minute) << " " <<
								 static_cast<int>(returned_second);
}

TEST_F(TestDateTime, datetime_decode)
{
	for (const auto& node : nodes)
	{
		const auto the_node = node.node();
		//
		const std::string input_str(the_node.select_node("input").node().child_value());
		const auto input_in_range(string_to_range(input_str));
		const auto input = int64_parse(input_in_range.start, input_in_range.finish);
		const auto expected_return = static_cast<uint8_t>(INT_PARSE(
										 the_node.select_node("return").node().child_value()));
		//
		const auto expected_year = static_cast<uint32_t>(INT_PARSE(
									   the_node.select_node("year").node().child_value()));
		const auto expected_month = static_cast<uint8_t>(INT_PARSE(
										the_node.select_node("month").node().child_value()));
		const auto expected_day = static_cast<uint8_t>(INT_PARSE(
									  the_node.select_node("day").node().child_value()));
		const auto expected_hour = static_cast<uint8_t>(INT_PARSE(
									   the_node.select_node("hour").node().child_value()));
		const auto expected_minute = static_cast<uint8_t>(INT_PARSE(
										 the_node.select_node("minute").node().child_value()));
		const auto expected_second = static_cast<uint8_t>(INT_PARSE(
										 the_node.select_node("second").node().child_value()));
		const auto expected_year_day = static_cast<uint16_t>(INT_PARSE(
										   the_node.select_node("year_day").node().child_value()));
		//
		uint32_t returned_year = 0;
		uint8_t returned_month = 0;
		uint8_t returned_day = 0;
		uint8_t returned_hour = 0;
		uint8_t returned_minute = 0;
		uint8_t returned_second = 0;
		uint16_t returned_year_day = 0;
		//
		const auto returned = datetime_decode(input, &returned_year, &returned_month, &returned_day,
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
