/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "conversion.h"
#include "math_unit.h"
};

#include <cfloat>

class TestMathUnit : public TestsBaseXml
{
};

TEST_F(TestMathUnit, math_abs)
{
	for (const auto& node : nodes)
	{
		const double input = double_parse(node.node().select_node("input").node().child_value());
		const double expected_output = double_parse(node.node().select_node("output").node().child_value());
		const double returned = math_abs(input);
		//
		ASSERT_NEAR(expected_output, returned, 50 * DBL_EPSILON) << input;
		//
		--node_count;
	}
}

TEST_F(TestMathUnit, math_ceiling)
{
	for (const auto& node : nodes)
	{
		const double input = double_parse(node.node().select_node("input").node().child_value());
		const double expected_output = double_parse(node.node().select_node("output").node().child_value());
		const double returned = math_ceiling(input);
		//
		ASSERT_NEAR(expected_output, returned, 50 * DBL_EPSILON) << input;
		//
		--node_count;
	}
}

TEST_F(TestMathUnit, math_floor)
{
	for (const auto& node : nodes)
	{
		const double input = double_parse(node.node().select_node("input").node().child_value());
		const double expected_output = double_parse(node.node().select_node("output").node().child_value());
		const double returned = math_floor(input);
		//
		ASSERT_NEAR(expected_output, returned, 50 * DBL_EPSILON) << input;
		//
		--node_count;
	}
}

TEST_F(TestMathUnit, math_round)
{
	for (const auto& node : nodes)
	{
		const double input = double_parse(node.node().select_node("input").node().child_value());
		const double expected_output = double_parse(node.node().select_node("output").node().child_value());
		const double returned = math_round(input);
		//
		ASSERT_NEAR(expected_output, returned, 50 * DBL_EPSILON) << input;
		//
		--node_count;
	}
}
