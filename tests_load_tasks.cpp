/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2021 https://github.com/TheVice/
 *
 */

#include "tests_exec.h"

extern "C" {
#include "load_tasks.h"
#include "buffer.h"
#include "conversion.h"
#include "file_system.h"
#include "path.h"
#include "project.h"
}

#include <string>
#include <ostream>
#include <iostream>

class TestLoadTasks : public TestExec
{
};

TEST_F(TestLoadTasks, load_tasks_evaluate_task)
{
	buffer task_arguments;
	SET_NULL_TO_BUFFER(task_arguments);
	//
	uint8_t task_attributes_count = 0;
	//
	const auto path_to_directory_with_image = get_path_to_directory_with_image(
				&task_arguments, &task_attributes_count);
	ASSERT_TRUE(task_attributes_count) << buffer_free(&task_arguments);
	ASSERT_TRUE(buffer_resize(&task_arguments, 0)) << buffer_free(&task_arguments);
	//
	const auto current_directory(get_directory_for_current_process(&task_arguments, &task_attributes_count));
	ASSERT_TRUE(task_attributes_count) << buffer_free(&task_arguments);
	ASSERT_TRUE(buffer_resize(&task_arguments, 0)) << buffer_free(&task_arguments);
	//
	const uint8_t** task_attributes = nullptr;
	const uint8_t* task_attributes_lengths = nullptr;
	//
	ASSERT_TRUE(load_tasks_get_attributes_and_arguments_for_task(
					&task_attributes, &task_attributes_lengths, &task_attributes_count, &task_arguments))
			<< buffer_free_with_inner_buffers(&task_arguments);
	//
	buffer the_project;
	SET_NULL_TO_BUFFER(the_project);
	//
	ASSERT_TRUE(project_new(&the_project)) << project_free(&the_project);

	for (const auto& node : nodes)
	{
		expected_return = static_cast<uint8_t>(INT_PARSE(
				node.node().select_node("return").node().child_value()));
		const uint8_t* path = nullptr;

		for (uint8_t i = 0; i < task_attributes_count; ++i)
		{
			auto attribute_value_in_buffer = buffer_buffer_data(&task_arguments, i);
			//
			ASSERT_TRUE(buffer_resize(attribute_value_in_buffer, 0))
					<< buffer_free_with_inner_buffers(&task_arguments) << std::endl << project_free(&the_project);
			//
			const std::string attribute(reinterpret_cast<const char*>(task_attributes[i]), task_attributes_lengths[i]);
			std::string attribute_value(node.node().select_node(attribute.c_str()).node().child_value());

			if (attribute_value.empty())
			{
				continue;
			}

			static const std::string module_str("module");

			if (module_str == attribute)
			{
				const auto path_in_range(string_to_range(attribute_value));

				if (!path_is_path_rooted(path_in_range.start, path_in_range.finish))
				{
					attribute_value = current_directory + attribute_value;

					if (!file_exists(reinterpret_cast<const uint8_t*>(attribute_value.c_str())) &&
						!path_to_directory_with_image.empty() &&
						path_to_directory_with_image != current_directory)
					{
						attribute_value = path_to_directory_with_image + attribute_value.substr(current_directory.size());
					}
				}
			}

			ASSERT_TRUE(buffer_append_char(attribute_value_in_buffer,
										   attribute_value.c_str(), static_cast<ptrdiff_t>(attribute_value.size())))
					<< buffer_free_with_inner_buffers(&task_arguments) << std::endl << project_free(&the_project);

			if (module_str == attribute)
			{
				ASSERT_TRUE(buffer_push_back(attribute_value_in_buffer, 0))
						<< buffer_free_with_inner_buffers(&task_arguments) << std::endl << project_free(&the_project);
				path = buffer_data(attribute_value_in_buffer, 0);
			}
		}

		if (expected_return && !file_exists(path))
		{
			--node_count;
			continue;
		}

		static const void* the_target = nullptr;
		const auto returned = load_tasks_evaluate_task(&the_project, the_target, &task_arguments, verbose);
		//
		ASSERT_EQ(expected_return, returned)
				<< buffer_free_with_inner_buffers(&task_arguments) << std::endl << project_free(&the_project);
		//
		project_clear(&the_project);
		//
		--node_count;
	}

	project_unload(&the_project);
	buffer_release_with_inner_buffers(&task_arguments);
}
