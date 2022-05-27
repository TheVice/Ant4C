/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "conversion.h"
#include "interpreter.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"
};

class TestInterpreter : public TestsBaseXml
{
protected:
	uint8_t verbose;

	TestInterpreter();
};

TestInterpreter::TestInterpreter()
	: TestsBaseXml(),
	  verbose()
{
}

TEST_F(TestInterpreter, interpreter_disassemble_function)
{
	for (const auto& node : nodes)
	{
		const std::string input(node.node().select_node("input").node().child_value());
		const std::string expected_name_space(node.node().select_node("name_space").node().child_value());
		const std::string expected_name(node.node().select_node("name").node().child_value());
		const std::string expected_arguments_area(node.node().select_node("arguments_area").node().child_value());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto input_in_a_range = string_to_range(return_str);
		//
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		input_in_a_range = string_to_range(input);
		//
		range name_space;
		name_space.start = name_space.finish = nullptr;
		range name;
		name.start = name.finish = nullptr;
		range arguments_area;
		arguments_area.start = arguments_area.finish = nullptr;
		//
		const auto returned = interpreter_disassemble_function(
								  &input_in_a_range, &name_space, &name, &arguments_area);
		ASSERT_EQ(expected_return, returned);
		//
		const auto returned_name_space(range_to_string(name_space));
		ASSERT_EQ(expected_name_space, returned_name_space);
		const auto returned_name(range_to_string(name));
		ASSERT_EQ(expected_name, returned_name);
		const auto returned_arguments_area(range_to_string(arguments_area));
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
		auto expected_outputs = node.node().select_nodes("output");
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto arguments_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(arguments_in_a_range.start, arguments_in_a_range.finish));
		//
		arguments_in_a_range = string_to_range(arguments);
		ASSERT_TRUE(buffer_resize_and_free_inner_buffers(&output)) << buffer_free_with_inner_buffers(&output);
		//
		uint8_t values_count;
		ASSERT_TRUE(interpreter_get_values_for_arguments(
						nullptr, nullptr, &arguments_in_a_range,
						&output, &values_count, verbose)) << arguments;
		ASSERT_EQ(expected_return, values_count) << buffer_free_with_inner_buffers(&output) << std::endl
				<< arguments;
		ptrdiff_t i = 0;
		buffer* current_output = nullptr;

		for (const auto& expected_output : expected_outputs)
		{
			const std::string str_expected_output(expected_output.node().child_value());
			current_output = buffer_buffer_data(&output, i++);
			//
			ASSERT_NE(nullptr, current_output) << buffer_free_with_inner_buffers(&output) << std::endl
											   << arguments;
			//
			const auto str_current_output(buffer_to_string(current_output));
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
//interpreter_evaluate_function
TEST_F(TestInterpreter, interpreter_evaluate_code)
{
	buffer output;
	SET_NULL_TO_BUFFER(output);

	for (const auto& node : nodes)
	{
		const std::string code(get_data_from_nodes(node, "code"));
		const auto expected_output(get_data_from_nodes(node, ("output")));
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto code_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(code_in_a_range.start, code_in_a_range.finish));
		//
		code_in_a_range = string_to_range(code);
		ASSERT_TRUE(buffer_resize(&output, 0)) << buffer_free(&output);
		//
		const auto returned = interpreter_evaluate_code(
								  nullptr, nullptr, nullptr, &code_in_a_range, &output, verbose);
		ASSERT_EQ(expected_return, returned) << "'" << code << "'" << std::endl << buffer_free(&output);
		//
		const auto current_output(buffer_to_string(&output));
		ASSERT_EQ(expected_output, current_output) << "'" << code << "'" << std::endl << buffer_free(&output);
		//
		--node_count;
	}

	buffer_release(&output);
}
/*interpreter_is_xml_tag_should_be_skip_by_if_or_unless
interpreter_get_arguments_from_xml_tag_record
interpreter_get_task*/
TEST_F(TestInterpreter, interpreter_evaluate_task)
{
	for (const auto& node : nodes)
	{
		const std::string code(node.node().select_node("code").node().child_value());
		ASSERT_FALSE(code.empty());
		//
		auto doc = pugi::xml_document();
		const auto result = doc.load_string(code.c_str());
		ASSERT_EQ(pugi::xml_parse_status::status_ok, result.status) << code;
		//
		const std::string task_name(doc.first_child().name());
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		auto code_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(code_in_a_range.start, code_in_a_range.finish));
		//
		buffer the_project;
		SET_NULL_TO_BUFFER(the_project);

		if (node.node().attribute("project").as_bool())
		{
			ASSERT_TRUE(project_new(&the_project)) << project_free(&the_project);
		}

		code_in_a_range = string_to_range(code);
		//
		auto task_name_in_a_range = code_in_a_range;
		task_name_in_a_range.start =
			string_enumerate(task_name_in_a_range.start, task_name_in_a_range.finish, nullptr);
		task_name_in_a_range.finish = task_name_in_a_range.start + task_name.size();
		//
		code_in_a_range.start = string_enumerate(code_in_a_range.start, code_in_a_range.finish, nullptr);
		code_in_a_range.start += task_name.size();
		const auto returned = interpreter_evaluate_task(
								  &the_project, nullptr, &task_name_in_a_range, code_in_a_range.finish, nullptr, 0, verbose);
		//
		ASSERT_EQ(expected_return, returned) << code << std::endl << project_free(&the_project);
		project_unload(&the_project);
		//
		--node_count;
	}
}
//interpreter_evaluate_tasks
