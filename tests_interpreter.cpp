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
#include "interpreter.h"
#include "range.h"
};

class TestInterpreter : public TestsBaseXml
{
};

TEST_F(TestInterpreter, interpreter_disassemble_function)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_name_space(node.node().select_node("name_space").node().child_value());
		const std::string expected_name(node.node().select_node("name").node().child_value());
		const std::string expected_arguments_area(node.node().select_node("arguments_area").node().child_value());
		const uint8_t expected_return = (uint8_t)int_parse(node.node().select_node("return").node().child_value());
		//
		const range function_in_range = string_to_range(input);
		//
		range name_space;
		name_space.start = name_space.finish = NULL;
		range name;
		name.start = name.finish = NULL;
		range arguments_area;
		arguments_area.start = arguments_area.finish = NULL;
		//
		const uint8_t returned = interpreter_disassemble_function(
									 &function_in_range, &name_space, &name, &arguments_area);
		ASSERT_EQ(expected_return, returned);
		//
		const std::string returned_name_space(range_to_string(name_space));
		ASSERT_EQ(expected_name_space, returned_name_space);
		const std::string returned_name(range_to_string(name));
		ASSERT_EQ(expected_name, returned_name);
		const std::string returned_arguments_area(range_to_string(arguments_area));
		ASSERT_EQ(expected_arguments_area, returned_arguments_area);
		//
		--node_count;
	}
}

TEST_F(TestInterpreter, interpreter_get_values_for_arguments)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const std::string arguments(node.node().select_node("arguments").node().child_value());
		pugi::xpath_node_set expected_outputs = node.node().select_nodes("output");
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const range arguments_in_range = string_to_range(arguments);
		ASSERT_TRUE(buffer_resize_and_free_inner_buffers(&output)) << buffer_free_with_inner_buffers(&output);
		//
		const uint8_t returned = interpreter_get_values_for_arguments(NULL, NULL, &arguments_in_range, &output);
		ASSERT_EQ(expected_return, returned) << buffer_free_with_inner_buffers(&output) << std::endl
											 << arguments;
		ptrdiff_t i = 0;
		buffer* current_output = NULL;

		for (const auto& expected_output : expected_outputs)
		{
			const std::string str_expected_output(expected_output.node().child_value());
			current_output = buffer_buffer_data(&output, i++);
			//
			ASSERT_NE(nullptr, current_output) << buffer_free_with_inner_buffers(&output) << std::endl
											   << arguments;
			//
			const std::string str_current_output(buffer_to_string(current_output));
			ASSERT_EQ(str_expected_output, str_current_output) << buffer_free_with_inner_buffers(&output) << std::endl
					<< arguments;
		}

		current_output = buffer_buffer_data(&output, i);
		ASSERT_EQ(nullptr, current_output) << buffer_free_with_inner_buffers(&output) << std::endl
										   << arguments;
		--node_count;
	}

	buffer_release_with_inner_buffers(&output);
}

TEST_F(TestInterpreter, interpreter_evaluate_code)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		uint8_t condition = 0;
		ASSERT_TRUE(is_this_node_pass_by_if_condition(node, &output, &condition)) << buffer_free(&output);

		if (!condition)
		{
			--node_count;
			continue;
		}

		const std::string code(get_data_from_nodes(node, "code"));
		const std::string expected_output(get_data_from_nodes(node, ("output")));
		const uint8_t expected_return = (uint8_t)int_parse(
											node.node().select_node("return").node().child_value());
		//
		const range code_in_range = string_to_range(code);
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		const uint8_t returned = interpreter_evaluate_code(NULL, NULL, &code_in_range, &output);
		ASSERT_EQ(expected_return, returned) << "'" << code << "'" << std::endl << buffer_free(&output);
		//
		const std::string current_output(buffer_to_string(&output));
		ASSERT_EQ(expected_output, current_output) << "'" << code << "'" << std::endl << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}
