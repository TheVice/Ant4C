/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "file_system.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "hash.h"
#include "path.h"
#include "range.h"

enum file_function
{
	file_exists_,
	file_get_checksum_,
	file_get_creation_time_,
	file_get_creation_time_utc_,
	file_get_last_access_time_,
	file_get_last_access_time_utc_,
	file_get_last_write_time_,
	file_get_last_write_time_utc_,
	file_get_length_,
	file_up_to_date_,
	file_replace_,
	UNKNOWN_FILE_FUNCTION,
	file_set_creation_time_,
	file_set_creation_time_utc_,
	file_set_last_access_time_,
	file_set_last_access_time_utc_,
	file_set_last_write_time_,
	file_set_last_write_time_utc_
};

enum dir_function
{
	enumerate_file_system_entries,
	dir_exists,
	dir_get_creation_time,
	dir_get_creation_time_utc,
	get_current_directory,
	get_directory_root,
	dir_get_last_access_time,
	dir_get_last_access_time_utc,
	dir_get_last_write_time,
	dir_get_last_write_time_utc,
	get_logical_drives,
	get_parent_directory,
	UNKNOWN_DIR_FUNCTION
};

enum enumerate_entry_types
{
	directory_entry,
	file_entry,
	all_entries,
	UNKNOWN_ENTRY_TYPE
};

uint8_t file_get_id_of_file_set_creation_time_function()
{
	return file_set_creation_time_;
}

uint8_t file_get_id_of_file_set_creation_time_utc_function()
{
	return file_set_creation_time_utc_;
}

uint8_t file_get_id_of_file_set_last_access_time_function()
{
	return file_set_last_access_time_;
}

uint8_t file_get_id_of_file_set_last_access_time_utc_function()
{
	return file_set_last_access_time_utc_;
}

uint8_t file_get_id_of_file_set_last_write_time_function()
{
	return file_set_last_write_time_;
}

uint8_t file_get_id_of_file_set_last_write_time_utc_function()
{
	return file_set_last_write_time_utc_;
}

uint8_t dir_get_id_of_get_current_directory_function()
{
	return get_current_directory;
}

uint8_t file_system_get_id_of_directory_entry()
{
	return directory_entry;
}

uint8_t file_system_get_id_of_file_entry()
{
	return file_entry;
}

uint8_t file_system_get_id_of_all_entries()
{
	return all_entries;
}

uint8_t dir_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* dir_function_str[] =
	{
		(const uint8_t*)"enumerate-file-system-entries",
		(const uint8_t*)"exists",
		(const uint8_t*)"get-creation-time",
		(const uint8_t*)"get-creation-time-utc",
		(const uint8_t*)"get-current-directory",
		(const uint8_t*)"get-directory-root",
		(const uint8_t*)"get-last-access-time",
		(const uint8_t*)"get-last-access-time-utc",
		(const uint8_t*)"get-last-write-time",
		(const uint8_t*)"get-last-write-time-utc",
		(const uint8_t*)"get-logical-drives",
		(const uint8_t*)"get-parent-directory"
	};
	/**/
	return common_string_to_enum(name_start, name_finish, dir_function_str, UNKNOWN_DIR_FUNCTION);
}

uint8_t dir_exec_function(uint8_t function, const void* arguments, uint8_t arguments_count,
						  void* output)
{
	if (UNKNOWN_DIR_FUNCTION <= function ||
		get_current_directory == function ||
		NULL == arguments ||
		3 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range values[3];

	if (arguments_count && !common_get_arguments(arguments, arguments_count, values, 1))
	{
		return 0;
	}

	switch (function)
	{
		case enumerate_file_system_entries:
		{
			if (3 != arguments_count &&
				2 != arguments_count)
			{
				break;
			}

			static const uint8_t* entry_types_str[] =
			{
				(const uint8_t*)"directory",
				(const uint8_t*)"file",
				(const uint8_t*)"all"
			};
			/**/
			const uint8_t entry_type = common_string_to_enum(
										   values[1].start, values[1].finish, entry_types_str, UNKNOWN_ENTRY_TYPE);

			if (UNKNOWN_ENTRY_TYPE == entry_type)
			{
				break;
			}

			uint8_t recurse = 0;

			if (3 == arguments_count &&
				!bool_parse(values[2].start, values[2].finish, &recurse))
			{
				break;
			}

			return directory_enumerate_file_system_entries(
					   buffer_buffer_data(arguments, 0), entry_type, recurse, output, 1);
		}

		case dir_exists:
			return 1 == arguments_count && bool_to_string(directory_exists(values[0].start), output);

		case dir_get_creation_time:
			return 1 == arguments_count && int64_to_string(directory_get_creation_time(values[0].start), output);

		case dir_get_creation_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_creation_time_utc(values[0].start), output);

		case get_directory_root:
			return 1 == arguments_count &&
				   directory_get_directory_root(values[0].start, &values[0].finish) &&
				   buffer_append_data_from_range(output, &values[0]);

		case dir_get_last_access_time:
			return 1 == arguments_count && int64_to_string(directory_get_last_access_time(values[0].start), output);

		case dir_get_last_access_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_last_access_time_utc(values[0].start), output);

		case dir_get_last_write_time:
			return 1 == arguments_count && int64_to_string(directory_get_last_write_time(values[0].start), output);

		case dir_get_last_write_time_utc:
			return 1 == arguments_count && int64_to_string(directory_get_last_write_time_utc(values[0].start), output);

		case get_logical_drives:
			return 0 == arguments_count && directory_get_logical_drives(output);

		case get_parent_directory:
			return 1 == arguments_count &&
				   directory_get_parent_directory(values[0].start, &values[0].finish) &&
				   buffer_append_data_from_range(output, &values[0]);

		case UNKNOWN_DIR_FUNCTION:
		default:
			break;
	}

	return 0;
}

uint8_t file_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* file_function_str[] =
	{
		(const uint8_t*)"exists",
		(const uint8_t*)"get-checksum",
		(const uint8_t*)"get-creation-time",
		(const uint8_t*)"get-creation-time-utc",
		(const uint8_t*)"get-last-access-time",
		(const uint8_t*)"get-last-access-time-utc",
		(const uint8_t*)"get-last-write-time",
		(const uint8_t*)"get-last-write-time-utc",
		(const uint8_t*)"get-length",
		(const uint8_t*)"up-to-date",
		(const uint8_t*)"replace"
	};
	/**/
	return common_string_to_enum(name_start, name_finish, file_function_str, UNKNOWN_FILE_FUNCTION);
}

uint8_t file_exec_function(uint8_t function, const void* arguments, uint8_t arguments_count,
						   void* output)
{
	if (UNKNOWN_FILE_FUNCTION <= function ||
		NULL == arguments ||
		!arguments_count ||
		3 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range values[3];

	if (!common_get_arguments(arguments, arguments_count, values, 1))
	{
		return 0;
	}

	for (uint8_t i = arguments_count, count = COUNT_OF(values); i < count; ++i)
	{
		values[i].start = values[i].finish = NULL;
	}

	switch (function)
	{
		case file_exists_:
			return 1 == arguments_count &&
				   bool_to_string(file_exists(values[0].start), output);
#if 0

		case file_get_checksum_:
			return (2 == arguments_count || 3 == arguments_count) &&
				   file_get_checksum(values[0].start, &values[1], &values[2], output);
#endif

		case file_get_creation_time_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_creation_time(values[0].start), output);

		case file_get_creation_time_utc_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_creation_time_utc(values[0].start), output);

		case file_get_last_access_time_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_last_access_time(values[0].start), output);

		case file_get_last_access_time_utc_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_last_access_time_utc(values[0].start), output);

		case file_get_last_write_time_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_last_write_time(values[0].start), output);

		case file_get_last_write_time_utc_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_last_write_time_utc(values[0].start), output);

		case file_get_length_:
			return 1 == arguments_count &&
				   int64_to_string(file_get_length(values[0].start), output);

		case file_up_to_date_:
			return 2 == arguments_count &&
				   bool_to_string(file_up_to_date(values[0].start, values[1].start), output);

		case file_replace_:
			return 3 == arguments_count &&
				   bool_to_string(
					   file_replace(values[0].start,
									values[1].start, values[1].finish,
									values[2].start, values[2].finish), output);

		case UNKNOWN_FILE_FUNCTION:
		default:
			break;
	}

	return 0;
}

#define ATTRIB_ARCHIVE_POSITION		0
#define ATTRIB_FILE_POSITION		1
#define ATTRIB_HIDDEN_POSITION		2
#define ATTRIB_NORMAL_POSITION		3
#define ATTRIB_READ_ONLY_POSITION	4
#define ATTRIB_SYSTEM_POSITION		5

#define ATTRIB_MAX_POSITION			(ATTRIB_SYSTEM_POSITION + 1)

uint8_t attrib_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	static const uint8_t* attrib_attributes[] =
	{
		(const uint8_t*)"archive",
		(const uint8_t*)"file",
		(const uint8_t*)"hidden",
		(const uint8_t*)"normal",
		(const uint8_t*)"readonly",
		(const uint8_t*)"system"
	};
	/**/
	static const uint8_t attrib_attributes_lengths[] = { 7, 4, 6, 6, 8, 6 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   attrib_attributes, attrib_attributes_lengths,
			   COUNT_OF(attrib_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t attrib_evaluate_task(void* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == task_arguments)
	{
		return 0;
	}

	void* file_path_in_a_buffer = buffer_buffer_data(task_arguments, ATTRIB_FILE_POSITION);

	if (buffer_size(file_path_in_a_buffer))
	{
		if (!buffer_push_back(file_path_in_a_buffer, 0))
		{
			return 0;
		}
	}
	else
	{
		return 1;
	}

	uint8_t attributes[ATTRIB_MAX_POSITION];

	for (uint8_t i = 0, count = ATTRIB_MAX_POSITION; i < count; ++i)
	{
		if (ATTRIB_FILE_POSITION == i)
		{
			continue;
		}

		const void* data = buffer_buffer_data(task_arguments, i);

		if (NULL == data)
		{
			return 0;
		}

		attributes[ATTRIB_FILE_POSITION] = (uint8_t)buffer_size(data);

		if (!attributes[ATTRIB_FILE_POSITION])
		{
			attributes[i] = 0;
			continue;
		}

		const uint8_t* value = buffer_uint8_t_data(data, 0);

		if (!bool_parse(value, value + attributes[ATTRIB_FILE_POSITION], &(attributes[i])))
		{
			return 0;
		}
	}

	return file_set_attributes(buffer_uint8_t_data(file_path_in_a_buffer, 0),
							   attributes[ATTRIB_ARCHIVE_POSITION], attributes[ATTRIB_HIDDEN_POSITION],
							   attributes[ATTRIB_NORMAL_POSITION], attributes[ATTRIB_READ_ONLY_POSITION],
							   attributes[ATTRIB_SYSTEM_POSITION]);
}

#define DELETE_DIR_POSITION		0
#define DELETE_FILE_POSITION	1

static const uint8_t* delete_attributes[] =
{
	(const uint8_t*)"dir",
	(const uint8_t*)"file"
};

static const uint8_t delete_attributes_lengths[] = { 3, 4 };

uint8_t delete_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   delete_attributes, delete_attributes_lengths,
			   COUNT_OF(delete_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t delete_evaluate_task(void* task_arguments, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	void* dir_path_in_buffer = buffer_buffer_data(task_arguments, DELETE_DIR_POSITION);
	void* file_path_in_buffer = buffer_buffer_data(task_arguments, DELETE_FILE_POSITION);
	const uint8_t* dir_ = NULL;

	if (buffer_size(dir_path_in_buffer))
	{
		if (!buffer_push_back(dir_path_in_buffer, 0))
		{
			return 0;
		}

		dir_ = buffer_uint8_t_data(dir_path_in_buffer, 0);

		if (file_exists(dir_))
		{
			return 0;
		}
	}

	const uint8_t* file = NULL;

	if (buffer_size(file_path_in_buffer))
	{
		if (!buffer_push_back(file_path_in_buffer, 0))
		{
			return 0;
		}

		file = buffer_uint8_t_data(file_path_in_buffer, 0);

		if (directory_exists(file))
		{
			return 0;
		}
	}

	if (NULL == dir_ && NULL == file)
	{
		return 0;
	}

	if (file && file_exists(file))
	{
		verbose = file_delete(file);
	}
	else
	{
		verbose = 1;
	}

	if (dir_ && directory_exists(dir_))
	{
		verbose = directory_delete(dir_) && verbose;
	}

	return verbose;
}

#define MKDIR_DIR_POSITION		0

uint8_t mkdir_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   delete_attributes, delete_attributes_lengths, 1,
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t mkdir_evaluate_task(void* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == task_arguments)
	{
		return 0;
	}

	void* dir_path_in_buffer = buffer_buffer_data(task_arguments, MKDIR_DIR_POSITION);

	if (!buffer_size(dir_path_in_buffer))
	{
		return 0;
	}

	if (!buffer_push_back(dir_path_in_buffer, 0))
	{
		return 0;
	}

	const uint8_t* dir_ = buffer_uint8_t_data(dir_path_in_buffer, 0);

	if (file_exists(dir_))
	{
		return 0;
	}

	return directory_create(dir_);
}

#define TOUCH_DATE_TIME_POSITION	0
#define TOUCH_FILE_POSITION			1
#define TOUCH_MILLIS_POSITION		2

uint8_t touch_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	static const uint8_t* touch_attributes[] =
	{
		(const uint8_t*)"datetime",
		(const uint8_t*)"file",
		(const uint8_t*)"millis"
	};
	/**/
	static const uint8_t touch_attributes_lengths[] = { 8, 4, 6 };
	/**/
	return common_get_attributes_and_arguments_for_task(
			   touch_attributes, touch_attributes_lengths,
			   COUNT_OF(touch_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t touch_evaluate_task(void* task_arguments, uint8_t verbose)
{
	(void)verbose;

	if (NULL == task_arguments)
	{
		return 0;
	}

	void* file_path_in_buffer = buffer_buffer_data(task_arguments, TOUCH_FILE_POSITION);

	if (!buffer_size(file_path_in_buffer))
	{
		return 1;
	}

	if (!buffer_push_back(file_path_in_buffer, 0) ||
		directory_exists(buffer_uint8_t_data(file_path_in_buffer, 0)))
	{
		return 0;
	}

	void* date_time_in_buffer = buffer_buffer_data(task_arguments, TOUCH_DATE_TIME_POSITION);
	void* millis_in_buffer = buffer_buffer_data(task_arguments, TOUCH_MILLIS_POSITION);

	if (!file_exists(buffer_uint8_t_data(file_path_in_buffer, 0)))
	{
		if (!file_create(buffer_uint8_t_data(file_path_in_buffer, 0)))
		{
			return 0;
		}

		if (!buffer_size(date_time_in_buffer) &&
			!buffer_size(millis_in_buffer))
		{
			return 1;
		}
	}

	int64_t seconds = 0;

	if (buffer_size(date_time_in_buffer))
	{
		struct range date_time_in_a_range;
		date_time_in_a_range.start = buffer_uint8_t_data(date_time_in_buffer, 0);
		date_time_in_a_range.finish = date_time_in_a_range.start + buffer_size(date_time_in_buffer);

		if (!datetime_parse_range(&date_time_in_a_range, &seconds))
		{
			return 0;
		}
	}
	else if (buffer_size(millis_in_buffer))
	{
		const uint8_t* start = buffer_uint8_t_data(millis_in_buffer, 0);
		const uint8_t* finish = start + buffer_size(millis_in_buffer);
		seconds = int64_parse(start, finish);
		seconds = date_time_millisecond_to_second(seconds);
	}
	else
	{
		seconds = datetime_now();
	}

	return file_set_last_write_time(buffer_uint8_t_data(file_path_in_buffer, 0), seconds);
}
