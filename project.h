/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <stddef.h>
#include <stdint.h>

struct range;

uint8_t project_property_exists(
	const void* the_project,
	const uint8_t* property_name, uint8_t property_name_length,
	void** the_property, uint8_t verbose);
uint8_t project_property_set_value(
	void* the_project,
	const uint8_t* property_name, uint8_t property_name_length,
	const uint8_t* property_value, ptrdiff_t property_value_length,
	uint8_t dynamic, uint8_t over_write,
	uint8_t read_only, uint8_t verbose);
uint8_t project_property_get_by_name(
	const void* the_project,
	const uint8_t* property_name, uint8_t property_name_length,
	void* output, uint8_t verbose);

uint8_t project_target_new(
	void* the_project,
	const uint8_t* target_name_start, const uint8_t* target_name_finish,
	const uint8_t* target_depends_start, const uint8_t* target_depends_finish,
	void* description,
	const uint8_t* attributes_start, const uint8_t* attributes_finish,
	const uint8_t* element_finish,
	const struct range* sub_nodes_names, uint8_t verbose);
uint8_t project_target_get(
	const void* the_project,
	const uint8_t* name_start, const uint8_t* name_finish,
	void** the_target, uint8_t verbose);
uint8_t project_target_exists(
	const void* the_project,
	const uint8_t* name_start, const uint8_t* name_finish, uint8_t verbose);
uint8_t project_target_has_executed(
	const void* the_project,
	const uint8_t* name_start, const uint8_t* name_finish, uint8_t verbose);

uint8_t project_add_module(
	void* the_project, const void* the_module, uint8_t length);
const uint8_t* project_get_task_from_module(
	const void* the_project, const struct range* task_name,
	void** the_module_of_task, ptrdiff_t* task_id);
const uint8_t* project_get_function_from_module(
	const void* the_project,
	const struct range* name_space, const struct range* function_name,
	void** the_module_of_task, const uint8_t** name_space_at_module);

uint8_t project_get_base_directory(
	const void* the_project, const void** the_property, uint8_t verbose);
uint8_t project_get_buildfile_path(
	const void* the_project, const void** the_property, uint8_t verbose);
uint8_t project_get_buildfile_uri(
	const void* the_property, void* build_file_uri, uint8_t verbose);
uint8_t project_get_default_target(
	const void* the_project, const void** the_property, uint8_t verbose);
uint8_t project_get_name(
	const void* the_project, const void** the_property, uint8_t verbose);

uint8_t project_get_current_directory(
	const void* the_project, const void* the_target,
	void* output, ptrdiff_t size, uint8_t verbose);

uint8_t project_new(void* the_project);
uint8_t project_load_from_content(
	const uint8_t* content_start, const uint8_t* content_finish,
	void* the_project, uint8_t project_help, uint8_t verbose);
uint8_t project_load_from_build_file(
	const struct range* path_to_build_file,
	const struct range* current_directory,
	uint16_t encoding, void* the_project,
	uint8_t project_help, uint8_t verbose);
uint8_t project_evaluate_default_target(void* the_project, uint8_t verbose);
uint8_t project_load_and_evaluate_target(
	void* the_project, const struct range* build_file,
	const struct range* current_directory,
	uint8_t project_help,
	uint16_t encoding, uint8_t verbose);
void project_clear(void* the_project);
void project_unload(void* the_project);
ptrdiff_t project_get_source_offset(
	const void* the_project, const uint8_t* cursor);
uint8_t project_evaluate_target_by_name_from_property(
	void* the_project, const void* the_target,
	const uint8_t* property_name, uint8_t property_name_length,
	void* argument_value, uint8_t verbose);
uint8_t project_on_success(
	void* the_project, const void* the_target, void* argument_value,
	uint8_t verbose);
uint8_t project_on_failure(
	void* the_project, const void* the_target, void* argument_value,
	uint8_t verbose);
uint8_t project_get_build_files_from_directory(
	void* directory, uint8_t verbose);

uint8_t project_set_listener_project_name(
	const void* the_project, uint8_t verbose);
uint8_t project_set_listener_task(
	const void* the_project, const struct range* task_name,
	ptrdiff_t task_id, const uint8_t* the_module);
const uint8_t* project_get_listener_project_name(const void* the_project);
const uint8_t* project_get_listener_task_name(const void* the_project);

uint8_t project_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t project_evaluate_task(
	void* the_project,
	const uint8_t* attributes_finish, const uint8_t* element_finish,
	const struct range* sub_nodes_names, uint8_t project_help,
	const void* task_arguments, uint8_t verbose);

uint8_t project_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t project_exec_function(
	const void* the_project,
	uint8_t function, const void* arguments, uint8_t arguments_count,
	const void** the_property, void* output, uint8_t verbose);

uint8_t program_exec_function(
	uint8_t function, const void* arguments, uint8_t arguments_count,
	void* output);

uint8_t program_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t program_evaluate_task(
	const void* the_project, const void* the_target,
	void* task_arguments, const uint8_t* attributes_finish,
	const uint8_t* element_finish, uint8_t verbose);

uint8_t program_set_log_file(
	const struct range* path_to_log_file,
	const struct range* current_directory,
	void* tmp, void** file_stream);
uint8_t program_set_listener(
	const struct range* path_to_listener,
	const struct range* current_directory,
	void* tmp, void** listener_object);

#endif
