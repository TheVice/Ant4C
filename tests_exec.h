/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "range.h"
};

#include <string>
#include <cstdint>

std::string get_directory_for_current_process(buffer* tmp, uint8_t* result);

class TestExec : public TestsBaseXml
{
protected:
	static std::string tests_exec_app;

protected:
	uint8_t append;

	std::string program_str;
	range program;

	std::string base_dir_str;
	range base_dir;

	std::string command_line_str;
	range command_line;

	std::string pid_property_str;
	void* pid_property;

	std::string result_property_str;
	void* result_property;

	std::string working_dir_str;
	range working_dir;

	std::string environment_variables_str;
	range environment_variables;

	uint8_t spawn;
	uint32_t time_out;
	uint8_t expected_return;
	int32_t result_property_value;

	uint8_t allow_output_to_console;

protected:
	static std::string get_path_to_directory_with_image(buffer* tmp, uint8_t* result);

protected:
	TestExec();

	virtual void SetUp() override;

	void load_input_data(const pugi::xpath_node& node);
};
