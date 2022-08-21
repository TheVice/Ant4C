/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 - 2022 TheVice
 *
 */

#include "copy_move.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "file_system.h"
#include "interpreter.load_file.h"
#include "load_file.h"
#include "path.h"
#include "project.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#include <stddef.h>

#define COPY_MOVE_DIR					0
#define COPY_MOVE_FILE					1
#define COPY_MOVE_TO_DIR				2
#define COPY_MOVE_TO_FILE				3
#define COPY_MOVE_FLATTEN				4
#define COPY_MOVE_OVER_WRITE			5
#define COPY_MOVE_INPUT_ENCODING		6
#define COPY_MOVE_OUTPUT_ENCODING		7
#define COPY_MOVE_INCLUDE_EMPTY_DIRS	8

static const uint8_t* copy_move_attributes[] =
{
	(const uint8_t*)"dir",
	(const uint8_t*)"file",
	(const uint8_t*)"todir",
	(const uint8_t*)"tofile",
	(const uint8_t*)"flatten",
	(const uint8_t*)"overwrite",
	(const uint8_t*)"inputencoding",
	(const uint8_t*)"outputencoding",
	(const uint8_t*)"includeemptydirs"
};

static const uint8_t copy_move_attributes_lengths[] = { 3, 4, 5, 6, 7, 9, 13, 14, 16 };

uint8_t copy_move_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, void* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   copy_move_attributes, copy_move_attributes_lengths,
			   COUNT_OF(copy_move_attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

enum file_system_tasks { file_system_copy_task, file_system_move_task };

uint8_t copy_move_file_with_encoding(
	uint8_t task_id, const uint8_t* source, const uint8_t* target,
	uint16_t input_encoding, uint8_t output_encoding, uint8_t verbose)
{
	(void)task_id;
	(void)source;
	(void)target;
	(void)input_encoding;
	(void)output_encoding;
	(void)verbose;
	/*TODO:*/
	return 0;
}

uint8_t copy_move_file(uint8_t task_id, const uint8_t* source,
					   const uint8_t** out_files, uint8_t count,
					   uint16_t input_encoding, uint8_t output_encoding,
					   uint8_t over_write, uint8_t verbose)
{
	if (NULL == source ||
		NULL == out_files)
	{
		return 0;
	}

	for (uint8_t i = 0; i < count; ++i)
	{
		if (NULL == out_files[i])
		{
			continue;
		}

		if (over_write)
		{
			if (file_exists(out_files[i]) &&
				!file_delete(out_files[i]))
			{
				return 0;
			}
		}
		else
		{
			if (file_exists(out_files[i]))
			{
				continue;
			}
		}

		uint8_t result = 0;

		if (input_encoding == output_encoding ||
			Default == input_encoding ||
			Default == output_encoding)
		{
			result = file_system_copy_task == task_id ?
					 file_copy(source, out_files[i]) :
					 file_move(source, out_files[i]);
		}
		else
		{
			result = copy_move_file_with_encoding(
						 task_id, source, out_files[i],
						 input_encoding, output_encoding, verbose);
		}

		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

uint8_t copy_move_read_over_write(
	void* task_arguments, uint8_t* over_write, uint8_t verbose)
{
	void* over_write_in_a_buffer = buffer_buffer_data(
									   task_arguments, COPY_MOVE_OVER_WRITE);
	const uint8_t size = (uint8_t)buffer_size(over_write_in_a_buffer);

	if (size)
	{
		const uint8_t* value = buffer_uint8_t_data(over_write_in_a_buffer, 0);

		if (!bool_parse(value, value + size, over_write) ||
			!buffer_resize(over_write_in_a_buffer, 0))
		{
			return 0;
		}
	}

	(void)verbose;
	return 1;
}

uint8_t copy_move_read_encodings(
	void* task_arguments,
	uint16_t* input_encoding, uint8_t* output_encoding,
	uint8_t verbose)
{
	if (NULL == task_arguments ||
		NULL == input_encoding ||
		NULL == output_encoding)
	{
		return 0;
	}

	void* input_encoding_in_a_buffer = buffer_buffer_data(
										   task_arguments, COPY_MOVE_INPUT_ENCODING);
	*input_encoding = (uint16_t)buffer_size(input_encoding_in_a_buffer);

	if (*input_encoding)
	{
		*input_encoding = load_file_get_encoding(input_encoding_in_a_buffer);

		if (FILE_ENCODING_UNKNOWN == *input_encoding)
		{
			return 0;
		}
	}
	else
	{
		*input_encoding = Default;
	}

	const void* output_encoding_in_a_buffer = buffer_buffer_data(
				task_arguments, COPY_MOVE_OUTPUT_ENCODING);
	*output_encoding = (uint8_t)buffer_size(output_encoding_in_a_buffer);

	if (*output_encoding)
	{
		const uint8_t* value = buffer_uint8_t_data(output_encoding_in_a_buffer, 0);
		*output_encoding = text_encoding_get_one(
							   value, value + *output_encoding);

		if (TEXT_ENCODING_UNKNOWN == *output_encoding)
		{
			return 0;
		}
	}
	else
	{
		*output_encoding = Default;
	}

	(void)verbose;
	return 1;
}

uint8_t copy_move_file_evaluate_task(
	const struct range* source_file, uint8_t task_id,
	uint16_t input_encoding, uint8_t output_encoding, uint8_t over_write,
	void* task_arguments, uint8_t verbose)
{
	if (range_is_null_or_empty(source_file) ||
		NULL == task_arguments)
	{
		return 0;
	}

	const uint8_t* out_files[2];
	out_files[0] = out_files[1] = NULL;
	/**/
	void* to_dir_in_a_buffer = buffer_buffer_data(task_arguments, COPY_MOVE_TO_DIR);

	if (buffer_size(to_dir_in_a_buffer))
	{
		const uint8_t* target_file_start = source_file->start;

		if (!path_get_file_name(&target_file_start, source_file->finish))
		{
			return 0;
		}

		if (!path_combine_in_place(to_dir_in_a_buffer, 0, target_file_start, source_file->finish) ||
			!buffer_push_back(to_dir_in_a_buffer, 0))
		{
			return 0;
		}

		out_files[0] = buffer_uint8_t_data(to_dir_in_a_buffer, 0);

		if (string_equal(source_file->start, source_file->finish,
						 out_files[0], out_files[0] + buffer_size(to_dir_in_a_buffer)))
		{
			out_files[0] = NULL;
		}
	}

	void* to_file_in_a_buffer = buffer_buffer_data(task_arguments, COPY_MOVE_TO_FILE);

	if (buffer_size(to_file_in_a_buffer))
	{
		if (!buffer_push_back(to_file_in_a_buffer, 0))
		{
			return 0;
		}

		out_files[1] = buffer_uint8_t_data(to_file_in_a_buffer, 0);

		if (string_equal(source_file->start, source_file->finish,
						 out_files[1], out_files[1] + buffer_size(to_file_in_a_buffer)))
		{
			out_files[1] = NULL;
		}
	}

	if (NULL != out_files[0] &&
		NULL != out_files[1])
	{
		if (string_equal(
				out_files[0], out_files[0] + buffer_size(to_dir_in_a_buffer),
				out_files[1], out_files[1] + buffer_size(to_file_in_a_buffer)))
		{
			out_files[1] = NULL;
		}
	}

	return copy_move_file(task_id, source_file->start,
						  out_files, COUNT_OF(out_files),
						  input_encoding, output_encoding,
						  over_write, verbose);
}

uint8_t copy_move_dir_evaluate_task(
	void* dir_in_a_buffer, uint8_t task_id,
	uint16_t input_encoding, uint8_t output_encoding, uint8_t over_write,
	void* task_arguments, uint8_t verbose)
{
	if (NULL == dir_in_a_buffer ||
		NULL == task_arguments)
	{
		return 0;
	}

	void* flatten_in_a_buffer = buffer_buffer_data(task_arguments, COPY_MOVE_FLATTEN);
	uint8_t flatten = (uint8_t)buffer_size(flatten_in_a_buffer);

	if (flatten)
	{
		const uint8_t* value = buffer_uint8_t_data(flatten_in_a_buffer, 0);

		if (!bool_parse(value, value + flatten, &flatten))
		{
			return 0;
		}
	}

	const ptrdiff_t dir_in_a_buffer_size = buffer_size(dir_in_a_buffer);
#ifdef _WIN32
	static const uint8_t* wild_card = (const uint8_t*)"*\0";

	if (!path_combine_in_place(dir_in_a_buffer, 0, wild_card, wild_card + 2))
	{
		return 0;
	}

#else

	if (!buffer_push_back(dir_in_a_buffer, 0))
	{
		return 0;
	}

#endif

	if (!buffer_resize(flatten_in_a_buffer, 0) ||
		!directory_enumerate_file_system_entries(dir_in_a_buffer, 1, 1, flatten_in_a_buffer, 1))
	{
		return 0;
	}

	const uint8_t* start = buffer_uint8_t_data(flatten_in_a_buffer, 0);
	const uint8_t* finish = start + buffer_size(flatten_in_a_buffer);
	/**/
	static const uint8_t zero = '\0';
	/**/
	void* to_dir_in_a_buffer = buffer_buffer_data(task_arguments, COPY_MOVE_TO_DIR);
	const ptrdiff_t to_dir_path_size = buffer_size(to_dir_in_a_buffer);

	if (flatten)
	{
		while (start < finish)
		{
			const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
									 start, finish, &zero, &zero + 1, 1, 1);
			const uint8_t* file_name_start = start;

			if (!path_get_file_name(&file_name_start, pos))
			{
				return 0;
			}

			if (!path_combine_in_place(to_dir_in_a_buffer, 0, file_name_start, pos) ||
				!buffer_push_back(to_dir_in_a_buffer, 0))
			{
				return 0;
			}

			file_name_start = buffer_uint8_t_data(to_dir_in_a_buffer, 0);

			if (!copy_move_file(
					task_id, start, &file_name_start, 1,
					input_encoding, output_encoding,
					over_write, verbose))
			{
				return 0;
			}

			if (!buffer_resize(to_dir_in_a_buffer, to_dir_path_size))
			{
				return 0;
			}

			start = string_enumerate(pos, finish, NULL);
		}

		return 1;
	}

	while (start < finish)
	{
		const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
								 start, finish, &zero, &zero + 1, 1, 1);
		start += dir_in_a_buffer_size;

		if (range_in_parts_is_null_or_empty(start, pos))
		{
			return 0;
		}

		if (!path_combine_in_place(to_dir_in_a_buffer, 0, start, pos) ||
			!buffer_push_back(to_dir_in_a_buffer, 0))
		{
			return 0;
		}

		struct range directory_name;

		BUFFER_TO_RANGE(directory_name, to_dir_in_a_buffer);

		if (!path_get_directory_name(
				directory_name.start, &directory_name.finish))
		{
			return 0;
		}

		const uint8_t delimiter = *directory_name.finish;
		uint8_t* ptr = buffer_uint8_t_data(to_dir_in_a_buffer, directory_name.finish - directory_name.start);
		*ptr = zero;

		if (!directory_exists(directory_name.start) &&
			!directory_create(directory_name.start))
		{
			return 0;
		}

		*ptr = delimiter;
		start -= dir_in_a_buffer_size;

		if (!copy_move_file(
				task_id, start, &(directory_name.start), 1,
				input_encoding, output_encoding,
				over_write, verbose))
		{
			return 0;
		}

		if (!buffer_resize(to_dir_in_a_buffer, to_dir_path_size))
		{
			return 0;
		}

		start = string_enumerate(pos, finish, NULL);
	}

	void* include_empty_dirs_in_a_buffer = buffer_buffer_data(
			task_arguments, COPY_MOVE_INCLUDE_EMPTY_DIRS);
	uint8_t include_empty_dirs = (uint8_t)buffer_size(include_empty_dirs_in_a_buffer);

	if (include_empty_dirs)
	{
		const uint8_t* value = buffer_uint8_t_data(include_empty_dirs_in_a_buffer, 0);

		if (!bool_parse(value, value + include_empty_dirs, &include_empty_dirs))
		{
			return 0;
		}
	}
	else
	{
		include_empty_dirs = 1;
	}

	if (include_empty_dirs)
	{
		if (!buffer_resize(flatten_in_a_buffer, 0) ||
			!directory_enumerate_file_system_entries(dir_in_a_buffer, 0, 1, flatten_in_a_buffer, 1))
		{
			return 0;
		}

		start = buffer_uint8_t_data(flatten_in_a_buffer, 0);
		finish = start + buffer_size(flatten_in_a_buffer);

		while (start < finish)
		{
			const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
									 start, finish, &zero, &zero + 1, 1, 1);
			start += dir_in_a_buffer_size;

			if (range_in_parts_is_null_or_empty(start, pos))
			{
				return 0;
			}

			if (!path_combine_in_place(to_dir_in_a_buffer, 0, start, pos) ||
				!buffer_push_back(to_dir_in_a_buffer, 0))
			{
				return 0;
			}

			start = buffer_uint8_t_data(to_dir_in_a_buffer, 0);

			if (!directory_exists(start) &&
				!directory_create(start))
			{
				return 0;
			}

			if (!buffer_resize(to_dir_in_a_buffer, to_dir_path_size))
			{
				return 0;
			}

			start = string_enumerate(pos, finish, NULL);
		}
	}

	return 1;
}

uint8_t copy_move_evaluate_task(
	const void* the_project, const void* the_target,
	uint8_t task_id, void* task_arguments, uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	uint16_t input_encoding = Default;
	uint8_t output_encoding = Default;

	if (!copy_move_read_encodings(
			task_arguments, &input_encoding, &output_encoding, verbose))
	{
		return 0;
	}

	uint8_t over_write = 0;

	if (!copy_move_read_over_write(task_arguments, &over_write, verbose))
	{
		return 0;
	}

	void* to_dir_in_a_buffer = buffer_buffer_data(task_arguments, COPY_MOVE_TO_DIR);
	const ptrdiff_t to_dir_path_size = buffer_size(to_dir_in_a_buffer);

	if (to_dir_path_size)
	{
		if (!buffer_push_back(to_dir_in_a_buffer, 0))
		{
			return 0;
		}

		const uint8_t* to_dir_path = buffer_uint8_t_data(to_dir_in_a_buffer, 0);

		if (!path_combine_in_place(to_dir_in_a_buffer, 0, NULL, NULL) ||
			file_exists(to_dir_path))
		{
			return 0;
		}

		if (!directory_exists(to_dir_path) &&
			!directory_create(to_dir_path))
		{
			return 0;
		}

		if (!buffer_resize(to_dir_in_a_buffer, to_dir_path_size))
		{
			return 0;
		}
	}

	void* dir_in_a_buffer = buffer_buffer_data(task_arguments, COPY_MOVE_DIR);
	const ptrdiff_t dir_in_a_buffer_size = buffer_size(dir_in_a_buffer);

	if (dir_in_a_buffer_size)
	{
		if (!buffer_push_back(dir_in_a_buffer, 0) ||
			!directory_exists(buffer_uint8_t_data(dir_in_a_buffer, 0)))
		{
			return 0;
		}

		if (!to_dir_path_size)
		{
			if (!project_get_current_directory(the_project, the_target, to_dir_in_a_buffer, 0, verbose))
			{
				return 0;
			}
		}

		if (!buffer_resize(dir_in_a_buffer, dir_in_a_buffer_size))
		{
			return 0;
		}

		struct range in;

		BUFFER_TO_RANGE(in, dir_in_a_buffer);

		struct range out;

		BUFFER_TO_RANGE(out, to_dir_in_a_buffer);

		if (string_equal(in.start, in.finish, out.start, out.finish))
		{
			return 1;
		}

		if (!copy_move_dir_evaluate_task(
				dir_in_a_buffer, task_id, input_encoding, output_encoding, over_write, task_arguments, verbose))
		{
			return 0;
		}
	}

	void* file_in_a_buffer = buffer_buffer_data(task_arguments, COPY_MOVE_FILE);

	if (buffer_size(file_in_a_buffer))
	{
		if (!buffer_push_back(file_in_a_buffer, 0) ||
			!file_exists(buffer_uint8_t_data(file_in_a_buffer, 0)))
		{
			return 0;
		}

		const void* to_file_in_a_buffer = buffer_buffer_data(task_arguments, COPY_MOVE_TO_FILE);
		const ptrdiff_t to_file_path_size = buffer_size(to_file_in_a_buffer);

		if (!to_dir_path_size &&
			!to_file_path_size)
		{
			if (!project_get_current_directory(the_project, the_target, to_dir_in_a_buffer, 0, verbose))
			{
				return 0;
			}
		}

		struct range source;

		BUFFER_TO_RANGE(source, file_in_a_buffer);

		if (!copy_move_file_evaluate_task(
				&source, task_id, input_encoding, output_encoding, over_write, task_arguments, verbose))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t copy_evaluate_task(
	const void* the_project, const void* the_target,
	void* task_arguments, uint8_t verbose)
{
	return copy_move_evaluate_task(
			   the_project, the_target,
			   file_system_copy_task, task_arguments, verbose);
}

uint8_t move_evaluate_task(
	const void* the_project, const void* the_target,
	void* task_arguments, uint8_t verbose)
{
	return copy_move_evaluate_task(
			   the_project, the_target,
			   file_system_move_task, task_arguments, verbose);
}
