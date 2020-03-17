/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#if defined(_WIN32)
#include <wchar.h>
#endif

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct range;

#if defined(_WIN32)
uint8_t file_system_append_pre_root(const struct range* path, struct buffer* output);
uint8_t file_system_get_position_after_pre_root(struct range* path);
void file_system_set_position_after_pre_root_wchar_t(const wchar_t** path);
#endif

uint8_t directory_create(const uint8_t* path);
uint8_t directory_delete(const uint8_t* path);
uint8_t directory_enumerate_file_system_entries(
	struct buffer* path, const uint8_t entry_type, const uint8_t recurse, struct buffer* output);
#if defined(_WIN32)
uint8_t directory_exists_wchar_t(const wchar_t* path);
#endif
uint8_t directory_exists(const uint8_t* path);
int64_t directory_get_creation_time(const uint8_t* path);
int64_t directory_get_creation_time_utc(const uint8_t* path);
uint8_t directory_get_current_directory(
	const void* project, const void** the_property, struct buffer* output);
uint8_t directory_get_directory_root(const uint8_t* path, struct range* root);
int64_t directory_get_last_access_time(const uint8_t* path);
int64_t directory_get_last_access_time_utc(const uint8_t* path);
int64_t directory_get_last_write_time(const uint8_t* path);
int64_t directory_get_last_write_time_utc(const uint8_t* path);
uint8_t directory_get_logical_drives(struct buffer* drives);
uint8_t directory_get_parent_directory(
	const uint8_t* path_start, const uint8_t* path_finish, struct range* parent);
uint8_t directory_move(const uint8_t* current_path, const uint8_t* new_path);
uint8_t directory_set_current_directory(const uint8_t* path);

#if defined(_WIN32)
int32_t _file_fileno(void* stream);
#endif
uint8_t file_close(void* stream);
uint8_t file_copy(const uint8_t* exists_file, const uint8_t* new_file);
uint8_t file_create(const uint8_t* path);
uint8_t file_delete(const uint8_t* path);
#if defined(_WIN32)
uint8_t file_exists_wchar_t(const wchar_t* path);
#endif
uint8_t file_exists(const uint8_t* path);
uint8_t file_flush(void* stream);
uint8_t file_seek(void* stream, long offset, int32_t origin);
long file_tell(void* stream);
size_t file_write(const void* content, const size_t size_of_content_element,
				  const size_t count_of_elements, void* stream);
int64_t file_get_creation_time(const uint8_t* path);
int64_t file_get_creation_time_utc(const uint8_t* path);
int64_t file_get_last_access_time(const uint8_t* path);
int64_t file_get_last_access_time_utc(const uint8_t* path);
int64_t file_get_last_write_time(const uint8_t* path);
int64_t file_get_last_write_time_utc(const uint8_t* path);
uint64_t file_get_length(const uint8_t* path);
uint8_t file_move(const uint8_t* current_path, const uint8_t* new_path);
uint8_t file_open(const uint8_t* path, const uint8_t* mode, void** output);
size_t file_read(void* content, const size_t size_of_content_element,
				 const size_t count_of_elements, void* stream);
uint8_t file_read_all_bytes(const uint8_t* path, struct buffer* output);
uint8_t file_up_to_date(const uint8_t* src_file, const uint8_t* target_file);
uint8_t file_set_creation_time(const uint8_t* path, int64_t time);
uint8_t file_set_creation_time_utc(const uint8_t* path, int64_t time);
uint8_t file_set_last_access_time(const uint8_t* path, int64_t time);
uint8_t file_set_last_access_time_utc(const uint8_t* path, int64_t time);
uint8_t file_set_last_write_time(const uint8_t* path, int64_t time);
uint8_t file_set_last_write_time_utc(const uint8_t* path, int64_t time);
uint8_t file_write_all_bytes(const uint8_t* path, const struct buffer* output);

uint8_t dir_get_id_of_get_current_directory_function();
uint8_t dir_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t dir_exec_function(
	uint8_t function, const struct buffer* arguments, uint8_t arguments_count, struct buffer* output);

uint8_t delete_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t delete_evaluate_task(struct buffer* task_arguments, uint8_t verbose);

uint8_t mkdir_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t mkdir_evaluate_task(struct buffer* task_arguments, uint8_t verbose);

uint8_t touch_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t touch_evaluate_task(struct buffer* task_arguments, uint8_t verbose);

uint8_t copy_move_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments);
uint8_t copy_evaluate_task(struct buffer* task_arguments, uint8_t verbose);
uint8_t move_evaluate_task(struct buffer* task_arguments, uint8_t verbose);

#endif
