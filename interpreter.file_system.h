/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#ifndef _INTERPRETER_FILE_SYSTEM_H_
#define _INTERPRETER_FILE_SYSTEM_H_

#include <stdint.h>

uint8_t file_get_id_of_file_set_creation_time_function();
uint8_t file_get_id_of_file_set_creation_time_utc_function();
uint8_t file_get_id_of_file_set_last_access_time_function();
uint8_t file_get_id_of_file_set_last_access_time_utc_function();
uint8_t file_get_id_of_file_set_last_write_time_function();
uint8_t file_get_id_of_file_set_last_write_time_utc_function();

uint8_t dir_get_id_of_get_current_directory_function();

uint8_t file_system_get_id_of_directory_entry();
uint8_t file_system_get_id_of_file_entry();
uint8_t file_system_get_id_of_all_entries();

uint8_t dir_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t dir_exec_function(
	uint8_t function, const void* arguments, uint8_t arguments_count, void* output);

uint8_t file_get_function(
	const uint8_t* name_start, const uint8_t* name_finish);
uint8_t file_exec_function(
	uint8_t function, const void* arguments, uint8_t arguments_count, void* output);

uint8_t attrib_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t attrib_evaluate_task(void* task_arguments, uint8_t verbose);

uint8_t delete_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t delete_evaluate_task(void* task_arguments, uint8_t verbose);

uint8_t mkdir_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t mkdir_evaluate_task(void* task_arguments, uint8_t verbose);

uint8_t touch_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments);
uint8_t touch_evaluate_task(void* task_arguments, uint8_t verbose);

#endif
