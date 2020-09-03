/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "load_tasks.h"
#include "buffer.h"
#include "conversion.h"
#include "project.h"
}

#include <string>
#include <ostream>

class TestLoadTasks : public TestsBaseXml
{
};

TEST_F(TestLoadTasks, load_tasks_evaluate_task)
{
	buffer the_project;
	SET_NULL_TO_BUFFER(the_project);
	//
	ASSERT_TRUE(project_new(&the_project)) << project_free(&the_project);
	//
	buffer task_arguments;
	SET_NULL_TO_BUFFER(task_arguments);

	for (const auto& node : nodes)
	{
		const uint8_t** task_attributes = NULL;
		const uint8_t* task_attributes_lengths = NULL;
		uint8_t task_attributes_count = 0;
		//
		ASSERT_TRUE(load_tasks_get_attributes_and_arguments_for_task(
						&task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments))
				<< buffer_free_with_inner_buffers(&task_arguments) << std::endl << project_free(&the_project);

		for (uint8_t i = 0; i < task_attributes_count; ++i)
		{
			const std::string attribute((const char*)task_attributes[i], task_attributes_lengths[i]);
			std::string attribute_value(node.node().select_node(attribute.c_str()).node().child_value());

			if (attribute_value.empty())
			{
				continue;
			}

#if defined(_WIN32) && (defined(__MINGW32__) || defined(__MINGW64__))
			static const std::string module_str("module");

			if (module_str == attribute)
			{
				attribute_value = "lib" + attribute_value;
			}

#elif !defined(_WIN32)
			static const std::string module_str("module");

			if (module_str == attribute)
			{
				const auto pos = attribute_value.rfind(".dll");
				attribute_value = attribute_value.replace(pos, 4, ".so", 3);
				attribute_value = "lib" + attribute_value;
			}

#endif
			auto attribute_value_in_buffer = buffer_buffer_data(&task_arguments, i);
			ASSERT_TRUE(buffer_append_char(attribute_value_in_buffer,
										   attribute_value.c_str(), (ptrdiff_t)attribute_value.size()))
					<< buffer_free_with_inner_buffers(&task_arguments) << std::endl << project_free(&the_project);
		}

		const auto expected_return = (uint8_t)INT_PARSE(node.node().select_node("return").node().child_value());
		//
		static const void* the_target = NULL;
		const auto returned = load_tasks_evaluate_task(&the_project, the_target, &task_arguments, verbose);
		//
		ASSERT_EQ(expected_return, returned)
				<< buffer_free_with_inner_buffers(&task_arguments) << std::endl << project_free(&the_project);
		//
		buffer_release_inner_buffers(&task_arguments);
		ASSERT_TRUE(buffer_resize(&task_arguments, 0))
				<< buffer_free_with_inner_buffers(&task_arguments) << std::endl << project_free(&the_project);
		//
		project_clear(&the_project);
		//
		--node_count;
	}

	project_unload(&the_project);
	buffer_release_with_inner_buffers(&task_arguments);
}
