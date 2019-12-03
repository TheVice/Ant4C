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
#include "echo.h"
#include "interpreter.h"
};

#include <string>
#include <cstdint>

class TestEcho : public TestsBaseXml
{
};

TEST_F(TestEcho, echo_evaluate_task)
{
	static const auto echo_str(std::string("echo"));
	static const auto echo_task_id = interpreter_get_task(
										 (const uint8_t*)echo_str.c_str(),
										 (const uint8_t*)echo_str.c_str() + echo_str.size());

	for (const auto& node : nodes)
	{
		const auto echo_code(std::string(node.node().select_node("code").node().child_value()));
		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		auto verbose_attribute = node.node().attribute("verbose");
		auto verbose = (uint8_t)0;

		if (verbose_attribute.empty())
		{
			auto doc = pugi::xml_document();
			const auto result = doc.load_string(echo_code.c_str());
			ASSERT_EQ(pugi::xml_parse_status::status_ok, result.status);
			ASSERT_STREQ(echo_str.c_str(), doc.first_child().name());
			verbose = (uint8_t)doc.first_child().attribute("verbose").as_bool();
		}
		else
		{
			verbose = (uint8_t)verbose_attribute.as_bool();
		}

		const auto code_in_range = string_to_range(echo_code);
		const auto returned = interpreter_evaluate_task(
								  NULL, NULL, echo_task_id,
								  code_in_range.start + 1 + echo_str.size(), code_in_range.finish,
								  verbose);
		//
		ASSERT_EQ(expected_return, returned) << echo_code;
		//
		--node_count;
	}
}

TEST(TestEcho_, echo_get_attributes_and_arguments_for_task)
{
	buffer task_arguments;
	SET_NULL_TO_BUFFER(task_arguments);
	/**/
	const uint8_t** task_attributes = NULL;
	const uint8_t* task_attributes_lengths = NULL;
	uint8_t task_attributes_count = 0;
	/**/
	const auto returned = echo_get_attributes_and_arguments_for_task(
							  &task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments);
	/**/
	ASSERT_TRUE(returned) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_NE(nullptr, task_attributes) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_NE(nullptr, task_attributes_lengths) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_EQ(7, task_attributes_count) << buffer_free_with_inner_buffers(&task_arguments);
	ASSERT_LT(0, buffer_size(&task_arguments)) << buffer_free_with_inner_buffers(&task_arguments);
	/**/
	task_attributes_count = 0;
	buffer* argument = NULL;

	while (NULL != (argument = buffer_buffer_data(&task_arguments, task_attributes_count++)))
	{
		ASSERT_FALSE(buffer_size(argument)) << buffer_free_with_inner_buffers(&task_arguments);
	}

	ASSERT_EQ(8, task_attributes_count) << buffer_free_with_inner_buffers(&task_arguments);
	buffer_release_with_inner_buffers(&task_arguments);
}
