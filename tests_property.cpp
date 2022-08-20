/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "interpreter.h"
#include "project.h"
#include "property.h"
#include "string_unit.h"
};

#include <cfloat>
#include <string>
#include <cstddef>
#include <cstdint>
#include <ostream>

class TestProperty : public TestsBaseXml
{
};

TEST(TestProperty_, property_at_all)
{
	std::string properties_buffer(buffer_size_of(), 0);
	auto properties = reinterpret_cast<void*>(&properties_buffer[0]);
	ASSERT_TRUE(buffer_init(properties, buffer_size_of()));
	//
	std::string output_buffer(buffer_size_of(), 0);
	auto output = reinterpret_cast<void*>(&output_buffer[0]);
	ASSERT_TRUE(buffer_init(output, buffer_size_of())) << buffer_free(properties);
	//
	const auto property_name = reinterpret_cast<const uint8_t*>("My_property");
	const auto property_name_length = static_cast<uint8_t>(common_count_bytes_until(property_name, 0));
	//
	void* the_property = nullptr;
	ASSERT_FALSE(property_exists(properties, property_name, property_name_length, &the_property))
			<< buffer_free(output) << properties_free(properties);
	//
	ASSERT_FALSE(property_get_by_pointer(the_property, output))
			<< buffer_free(output) << properties_free(properties);
	//
	uint8_t dynamic = 0;
	ASSERT_FALSE(property_is_dynamic(the_property, &dynamic))
			<< buffer_free(output) << properties_free(properties);
	//
	uint8_t read_only = 0;
	ASSERT_FALSE(property_is_readonly(the_property, &read_only))
			<< buffer_free(output) << properties_free(properties);
	//
	const auto property_value = reinterpret_cast<const uint8_t*>("Property value");
	const auto property_value_length = static_cast<uint8_t>(common_count_bytes_until(property_value, 0));
	ASSERT_TRUE(
		property_set_by_name(properties, property_name, property_name_length,
							 property_value, property_value_length,
							 property_value_is_byte_array,
							 0, 0, 1, 0))
			<< buffer_free(output) << properties_free(properties);
	//
	ASSERT_TRUE(property_exists(properties, property_name, property_name_length, &the_property))
			<< buffer_free(output) << properties_free(properties);
	//
	ASSERT_TRUE(property_get_by_pointer(the_property, output))
			<< buffer_free(output) << properties_free(properties);
	//
	ASSERT_EQ(property_value_length, buffer_size(output))
			<< buffer_free(output) << properties_free(properties);
	//
	ASSERT_TRUE(string_equal(property_value, property_value + property_value_length,
							 buffer_uint8_t_data(output, 0),
							 buffer_uint8_t_data(output, 0) + property_value_length))
			<< buffer_free(output) << properties_free(properties);
	//
	ASSERT_TRUE(property_is_dynamic(the_property, &dynamic))
			<< buffer_free(output) << properties_free(properties);
	ASSERT_EQ(0, dynamic)
			<< buffer_free(output) << properties_free(properties);
	//
	ASSERT_TRUE(property_is_readonly(the_property, &read_only))
			<< buffer_free(output) << properties_free(properties);
	ASSERT_EQ(1, read_only)
			<< buffer_free(output) << properties_free(properties);
	//
	ASSERT_EQ(ATTEMPT_TO_WRITE_READ_ONLY_PROPERTY,
			  property_set_by_pointer(the_property, the_property, 0, property_value_is_byte_array, 0, 1, 0))
			<< buffer_free(output) << properties_free(properties);
	//
	buffer_release(output);
	property_release(properties);
}
#if 0
TEST(TestProperty_, property_get)
{
	std::string code("<property name=\"my\" value=\"${my}\" failonerror=\"false\" />");
	auto code_in_a_range(string_to_range(code));
	//
	std::string the_project_buffer(buffer_size_of(), 0);
	auto the_project = reinterpret_cast<void*>(&the_project_buffer[0]);
	ASSERT_TRUE(buffer_init(the_project, buffer_size_of()));
	//
	ASSERT_TRUE(project_new(the_project));
	//
	ASSERT_EQ(FAIL_WITH_OUT_ERROR, project_load_from_content(code_in_a_range.start, code_in_a_range.finish,
			  the_project, 0, 0)) << project_free(the_project);
	//
	project_clear(the_project);
	//
	code = "<project><property name=\"my\" value=\"${my}\" failonerror=\"false\" /></project>";
	code_in_a_range = string_to_range(code);
	//
	ASSERT_EQ(FAIL_WITH_OUT_ERROR, project_load_from_content(code_in_a_range.start, code_in_a_range.finish,
			  the_project, 0, 0)) << project_free(the_project);
	//
	project_unload(the_project);
}
#endif
TEST(TestProperty_, property_set_by_pointer)
{
	static const auto property_name = reinterpret_cast<const uint8_t*>("A");
	static const uint8_t property_name_length = 1;
	//
	static const std::vector<int64_t> int64_values({ INT64_MAX, INT32_MAX, INT16_MIN, INT8_MIN, -1, 0, 1, INT8_MAX, INT16_MAX, INT32_MAX, INT64_MAX });
	static const std::vector<int32_t> int32_values({ INT32_MAX, INT16_MIN, INT8_MIN, -1, 0, 1, INT8_MAX, INT16_MAX, INT32_MAX });
	static const std::vector<int16_t> int16_values({ INT16_MIN, INT8_MIN, -1, 0, 1, INT8_MAX, INT16_MAX });
	static const std::vector<int8_t> int8_values({ INT8_MIN, -1, 0, 1, INT8_MAX });
	//
	static const std::vector<double> double_values({ -FLT_MIN, 0.0, FLT_MIN, FLT_MAX });
	static const std::vector<float> float_values({ -FLT_MIN, 0.0, FLT_MIN, FLT_MAX });
	//
	std::string properties_buffer(buffer_size_of(), 0);
	auto properties = reinterpret_cast<void*>(&properties_buffer[0]);
	ASSERT_TRUE(buffer_init(properties, buffer_size_of()));
	//
	std::string property_value_buffer(buffer_size_of(), 0);
	auto property_value = reinterpret_cast<void*>(&property_value_buffer[0]);
	ASSERT_TRUE(buffer_init(property_value, buffer_size_of())) << buffer_free(properties);
	//
	const uint8_t* start;
	const uint8_t* finish;
	//
	ASSERT_TRUE(property_set_by_name(
					properties, property_name, property_name_length,
					reinterpret_cast<const uint8_t*>(properties), 0,
					property_value_is_integer, 0, 0, 0, 0)) <<
							properties_free(properties) << buffer_free(property_value);
	//
	void* the_property = nullptr;
	ASSERT_TRUE(property_exists(properties, property_name, property_name_length, &the_property)) <<
			properties_free(properties) << buffer_free(property_value);

	for (const auto& int64_value : int64_values)
	{
		ASSERT_TRUE(property_set_by_pointer(
						the_property, reinterpret_cast<const uint8_t*>(&int64_value), sizeof(int64_t),
						property_value_is_integer, 0, 0, 0)) << properties_free(properties) <<
								properties_free(properties) << buffer_free(property_value);
		//
		ASSERT_TRUE(buffer_resize(property_value, 0)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(property_get_by_pointer(the_property, property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(buffer_size(property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		//
		start = buffer_uint8_t_data(property_value, 0);
		finish = start + buffer_size(property_value);
		//
		ASSERT_EQ(int64_value, int64_parse(start, finish)) <<
				properties_free(properties) << buffer_free(property_value);
	}

	for (const auto& int32_value : int32_values)
	{
		ASSERT_TRUE(property_set_by_pointer(
						the_property, reinterpret_cast<const uint8_t*>(&int32_value), sizeof(int32_t),
						property_value_is_integer, 0, 0, 0)) << properties_free(properties) <<
								properties_free(properties) << buffer_free(property_value);
		//
		ASSERT_TRUE(buffer_resize(property_value, 0)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(property_get_by_pointer(the_property, property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(buffer_size(property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		//
		start = buffer_uint8_t_data(property_value, 0);
		finish = start + buffer_size(property_value);
		//
		ASSERT_EQ(int32_value, int_parse(start, finish)) <<
				properties_free(properties) << buffer_free(property_value);
	}

	for (const auto& int16_value : int16_values)
	{
		ASSERT_TRUE(property_set_by_pointer(
						the_property, reinterpret_cast<const uint8_t*>(&int16_value), sizeof(int16_t),
						property_value_is_integer, 0, 0, 0)) << properties_free(properties) <<
								properties_free(properties) << buffer_free(property_value);
		//
		ASSERT_TRUE(buffer_resize(property_value, 0)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(property_get_by_pointer(the_property, property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(buffer_size(property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		//
		start = buffer_uint8_t_data(property_value, 0);
		finish = start + buffer_size(property_value);
		//
		ASSERT_EQ(int16_value, int_parse(start, finish)) <<
				properties_free(properties) << buffer_free(property_value);
	}

	for (const auto& int8_value : int8_values)
	{
		ASSERT_TRUE(property_set_by_pointer(
						the_property, reinterpret_cast<const uint8_t*>(&int8_value), sizeof(int8_t),
						property_value_is_integer, 0, 0, 0)) << properties_free(properties) <<
								properties_free(properties) << buffer_free(property_value);
		//
		ASSERT_TRUE(buffer_resize(property_value, 0)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(property_get_by_pointer(the_property, property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(buffer_size(property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		//
		start = buffer_uint8_t_data(property_value, 0);
		finish = start + buffer_size(property_value);
		//
		ASSERT_EQ(int8_value, int_parse(start, finish)) <<
				properties_free(properties) << buffer_free(property_value);
	}

	for (const auto& double_value : double_values)
	{
		ASSERT_TRUE(property_set_by_pointer(
						the_property, reinterpret_cast<const uint8_t*>(&double_value), sizeof(double),
						property_value_is_double, 0, 0, 0)) << properties_free(properties) <<
								properties_free(properties) << buffer_free(property_value);
		//
		ASSERT_TRUE(buffer_resize(property_value, 0)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(property_get_by_pointer(the_property, property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(buffer_size(property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(buffer_push_back(property_value, 0)) <<
				properties_free(properties) << buffer_free(property_value);
		//
		const auto returned_value = double_parse(buffer_uint8_t_data(property_value, 0));
		ASSERT_NEAR(double_value, returned_value, 50 * DBL_EPSILON) <<
				properties_free(properties) << buffer_free(property_value);
	}

	for (const auto& float_value : float_values)
	{
		ASSERT_TRUE(property_set_by_pointer(
						the_property, reinterpret_cast<const uint8_t*>(&float_value), sizeof(float),
						property_value_is_double, 0, 0, 0)) << properties_free(properties) <<
								properties_free(properties) << buffer_free(property_value);
		//
		ASSERT_TRUE(buffer_resize(property_value, 0)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(property_get_by_pointer(the_property, property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(buffer_size(property_value)) <<
				properties_free(properties) << buffer_free(property_value);
		ASSERT_TRUE(buffer_push_back(property_value, 0)) <<
				properties_free(properties) << buffer_free(property_value);
		//
		const auto returned_value = static_cast<float>(double_parse(buffer_uint8_t_data(property_value, 0)));
		ASSERT_NEAR(float_value, returned_value, 50 * FLT_EPSILON) <<
				properties_free(properties) << buffer_free(property_value);
	}

	buffer_release(property_value);
	property_release(properties);
}
#if 0
TEST_F(TestProperty, property_task)
{
	static const std::string property_str("property");
	//
	std::string properties_buffer(buffer_size_of(), 0);
	auto properties = reinterpret_cast<void*>(&properties_buffer[0]);
	ASSERT_TRUE(buffer_init(properties, buffer_size_of()));
	//
	uint8_t verbose = 0;

	for (const auto& node : nodes)
	{
		property_release(properties);
		ASSERT_TRUE(properties_load_from_node(node, "properties/property", properties))
				<< properties_free(properties);
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		const auto return_in_a_range(string_to_range(return_str));
		const auto expected_return =
			static_cast<uint8_t>(int_parse(return_in_a_range.start, return_in_a_range.finish));
		const auto output_properties = node.node().select_nodes("output_properties/property");
		//
		buffer the_project;
		SET_NULL_TO_BUFFER(the_project);
		//
		ASSERT_TRUE(project_new(&the_project)) << properties_free(properties) << project_free(&the_project);
		//
		ASSERT_TRUE(property_add_at_project(&the_project, properties, verbose))
				<< properties_free(properties) << project_free(&the_project);
		//
		property_release(properties);

		for (const auto& record_node : node.node().select_nodes("record"))
		{
			const auto record = property_str + " " + record_node.node().child_value();
			const auto record_in_a_range(string_to_range(record));
			//
			range task_in_a_range;
			task_in_a_range.start = reinterpret_cast<const uint8_t*>(record.c_str());
			task_in_a_range.finish = task_in_a_range.start + property_str.size();
			//
			const auto returned = interpreter_evaluate_task(
									  &the_project, nullptr,
									  &task_in_a_range, record_in_a_range.finish,
									  nullptr, 0, verbose);
			//
			ASSERT_EQ(expected_return, returned)
					<< record << std::endl
					<< properties_free(properties) << project_free(&the_project);
		}

		for (const auto& property : output_properties)
		{
			std::string name;
			std::string expected_value;
			uint8_t expected_dynamic;
			uint8_t over_write;
			uint8_t expected_read_only;
			uint8_t fail_on_error;
			uint8_t local_verbose;
			//
			property_load_from_node(property.node(), name, expected_value, expected_dynamic,
									over_write, expected_read_only, fail_on_error, local_verbose);
			//
			const auto property_name = reinterpret_cast<const uint8_t*>(name.c_str());
			const auto property_name_length = static_cast<uint8_t>(name.size());
			//
			ASSERT_TRUE(buffer_resize(properties, 0)) << buffer_free(properties) << project_free(&the_project);
			ASSERT_TRUE(project_property_get_by_name(
							&the_project,
							property_name, property_name_length,
							properties, verbose)) << name << std::endl <<
												  buffer_free(properties) << project_free(&the_project);
			ASSERT_EQ(expected_value, buffer_to_string(properties)) << buffer_free(properties);
			ASSERT_TRUE(buffer_resize(properties, 0)) << buffer_free(properties) << project_free(&the_project);
			//
			void* the_property = nullptr;
			ASSERT_TRUE(project_property_exists(
							&the_project,
							property_name, property_name_length,
							&the_property, verbose)) << name << std::endl <<
									buffer_free(properties) << project_free(&the_project);
			//
			uint8_t returned_dynamic = 0;
			ASSERT_TRUE(property_is_dynamic(the_property, &returned_dynamic)) << name << std::endl <<
					buffer_free(properties) << project_free(&the_project);
			ASSERT_EQ(expected_dynamic, returned_dynamic) << name << std::endl <<
					buffer_free(properties) << project_free(&the_project);
			//
			uint8_t returned_read_only = 0;
			ASSERT_TRUE(property_is_readonly(the_property, &returned_read_only)) << name << std::endl <<
					buffer_free(properties) << project_free(&the_project);
			ASSERT_EQ(expected_read_only, returned_read_only) << name << std::endl <<
					buffer_free(properties) << project_free(&the_project);
		}

		project_unload(&the_project);
		--node_count;
	}

	property_release(properties);
}
#endif