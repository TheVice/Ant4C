/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "path.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#if defined(_WIN32)
#include "file_system.h"
#endif
#include "range.h"

enum path_function
{
	path_change_extension_function,
	path_combine_function,
	path_get_directory_name_function,
	path_get_extension_function,
	path_get_file_name_function,
	path_get_file_name_without_extension_function,
	path_get_full_path_function,
	path_get_path_root_function,
	path_get_temp_file_name_function,
	path_get_temp_path_function,
	path_glob_function,
	path_has_extension_function,
	path_is_path_rooted_function,
	cygpath_get_dos_path_function,
	cygpath_get_unix_path_function,
	cygpath_get_windows_path_function,
	UNKNOWN_PATH_FUNCTION
};

uint8_t path_get_id_of_get_full_path_function()
{
	return path_get_full_path_function;
}

uint8_t path_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* path_function_str[] =
	{
		(const uint8_t*)"change-extension",
		(const uint8_t*)"combine",
		(const uint8_t*)"get-directory-name",
		(const uint8_t*)"get-extension",
		(const uint8_t*)"get-file-name",
		(const uint8_t*)"get-file-name-without-extension",
		(const uint8_t*)"get-full-path",
		(const uint8_t*)"get-path-root",
		(const uint8_t*)"get-temp-file-name",
		(const uint8_t*)"get-temp-path",
		(const uint8_t*)"glob",
		(const uint8_t*)"has-extension",
		(const uint8_t*)"is-path-rooted",
		(const uint8_t*)"get-dos-path",
		(const uint8_t*)"get-unix-path",
		(const uint8_t*)"get-windows-path"
	};
	/**/
	return common_string_to_enum(name_start, name_finish, path_function_str, UNKNOWN_PATH_FUNCTION);
}

uint8_t path_exec_function(const void* project, uint8_t function, const struct buffer* arguments,
						   uint8_t arguments_count,
						   struct buffer* output)
{
	(void)project;

	if (UNKNOWN_PATH_FUNCTION <= function ||
		NULL == arguments ||
		2 < arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range values[2];

	if (!common_get_arguments(arguments, arguments_count, values, 0))
	{
		return 0;
	}

	switch (function)
	{
		case path_change_extension_function:
			return (2 == arguments_count) &&
				   path_change_extension(values[0].start, values[0].finish, values[1].start, values[1].finish, output);

		case path_combine_function:
			return (2 == arguments_count) &&
				   path_combine(values[0].start, values[0].finish, values[1].start, values[1].finish, output);

		case path_get_directory_name_function:
			return (1 == arguments_count) &&
				   path_get_directory_name(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_extension_function:
			return (1 == arguments_count) &&
				   path_get_extension(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_file_name_function:
			return (1 == arguments_count) &&
				   path_get_file_name(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_file_name_without_extension_function:
			return (1 == arguments_count) &&
				   path_get_file_name_without_extension(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_path_root_function:
			return (1 == arguments_count) &&
				   path_get_path_root(values[0].start, values[0].finish, &values[1]) &&
				   buffer_append_data_from_range(output, &values[1]);

		case path_get_temp_file_name_function:
#if defined(_WIN32)
			return !arguments_count && path_get_temp_file_name(output) && file_create(buffer_data(output, 0));
#else
			return !arguments_count && path_get_temp_file_name(output);
#endif

		case path_get_temp_path_function:
			return !arguments_count && path_get_temp_path(output);

		case path_glob_function:
			return 2 == arguments_count &&
				   bool_to_string(path_glob(values[0].start, values[0].finish,
											values[1].start, values[1].finish), output);

		case path_has_extension_function:
			return 1 == arguments_count &&
				   bool_to_string(path_has_extension(values[0].start, values[0].finish), output);

		case path_is_path_rooted_function:
			return 1 == arguments_count &&
				   bool_to_string(path_is_path_rooted(values[0].start, values[0].finish), output);

		case path_get_full_path_function:
		case UNKNOWN_PATH_FUNCTION:
		default:
			break;
	}

	return 0;
}

uint8_t cygpath_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
							  struct buffer* output)
{
	if (UNKNOWN_PATH_FUNCTION <= function ||
		NULL == arguments ||
		1 != arguments_count ||
		NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!common_get_arguments(arguments, arguments_count, &argument, 0))
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	switch (function)
	{
		case cygpath_get_dos_path_function:
			return cygpath_get_dos_path(argument.start, argument.finish, output);

		case cygpath_get_unix_path_function:
			return buffer_append_data_from_range(output, &argument) &&
				   cygpath_get_unix_path(buffer_data(output, size),
										 buffer_data(output, size) + range_size(&argument));

		case cygpath_get_windows_path_function:
			return buffer_append_data_from_range(output, &argument) &&
				   cygpath_get_windows_path(buffer_data(output, size),
											buffer_data(output, size) + range_size(&argument));

		case UNKNOWN_PATH_FUNCTION:
		default:
			break;
	}

	return 0;
}
