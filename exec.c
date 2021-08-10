/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "exec.h"
#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "echo.h"
#include "file_system.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"
#include "xml.h"

#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>

static const uint8_t space_symbol = ' ';
static const wchar_t zero_symbol_w = L'\0';

#else
#define _POSIXSOURCE 1

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

static const uint8_t zero_symbol = '\0';

uint8_t exec_get_program_full_path(
	const void* the_project, const void* the_target,
	struct buffer* path_to_the_program, uint8_t is_path_rooted,
	const struct range* base_dir, struct buffer* tmp, uint8_t verbose)
{
	if (!buffer_size(path_to_the_program) ||
		!buffer_resize(tmp, 0))
	{
		return 0;
	}

	struct range program_in_the_range;

	BUFFER_TO_RANGE(program_in_the_range, path_to_the_program);

	if (!range_is_null_or_empty(base_dir) && !is_path_rooted)
	{
		if (!path_combine(base_dir->start, base_dir->finish,
						  program_in_the_range.start, program_in_the_range.finish, tmp))
		{
			return 0;
		}

		if (!buffer_resize(path_to_the_program, 0) ||
			!buffer_append_data_from_buffer(path_to_the_program, tmp))
		{
			return 0;
		}
	}
	else
	{
		const uint8_t* path = path_try_to_get_absolute_path(
								  the_project, the_target, path_to_the_program, tmp, verbose);

		if (buffer_data(path_to_the_program, 0) != path)
		{
			if (!buffer_resize(path_to_the_program, 0))
			{
				return 0;
			}

			if (!buffer_append(path_to_the_program, path, common_count_bytes_until(path, zero_symbol)) ||
				!buffer_push_back(path_to_the_program, 0))
			{
				return 0;
			}
		}
	}

#if defined(_WIN32)
	const ptrdiff_t size = buffer_size(path_to_the_program);

	if (FILENAME_MAX < size &&
		!file_system_append_pre_root(path_to_the_program))
	{
		return 0;
	}

	if (!path_combine_in_place(path_to_the_program,
							   buffer_size(path_to_the_program) - size, &zero_symbol, &zero_symbol))
#else
	if (!path_combine_in_place(path_to_the_program, 0, &zero_symbol, &zero_symbol))
#endif
	{
		return 0;
	}

	return buffer_push_back(path_to_the_program, zero_symbol);
}

#if defined(_WIN32)

uint8_t exec_win32_append_command_line(const struct range* command_line, struct buffer* output)
{
	if (!output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);
	struct range path_in_range;
	path_in_range.start = buffer_data(output, 0);
	path_in_range.finish = path_in_range.start + size;
	const uint8_t contains = string_contains(path_in_range.start, path_in_range.finish,
							 &space_symbol, &space_symbol + 1);

	if (!buffer_append(output, NULL, size + 3) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	path_in_range.start = buffer_data(output, 0);
	path_in_range.finish = path_in_range.start + size;

	if (!file_system_get_position_after_pre_root(&path_in_range))
	{
		return 0;
	}

	path_in_range.finish = 1 + find_any_symbol_like_or_not_like_that(
							   path_in_range.finish - 1, path_in_range.start, &zero_symbol, 1, 0, -1);

	if (contains)
	{
		if (!string_quote(path_in_range.start, path_in_range.finish, output))
		{
			return 0;
		}
	}
	else
	{
		if (!buffer_append_data_from_range(output, &path_in_range))
		{
			return 0;
		}
	}

	if (!range_is_null_or_empty(command_line))
	{
		if (!buffer_push_back(output, space_symbol) ||
			!buffer_append_data_from_range(output, command_line))
		{
			return 0;
		}
	}

	return buffer_push_back(output, zero_symbol);
}

uint8_t exec_win32(const wchar_t* program, wchar_t* cmd,
				   wchar_t* env, const wchar_t* working_dir,
				   HANDLE std_output, HANDLE std_error,
				   void* pid_property, HANDLE* process_handle,
				   uint8_t spawn, uint8_t verbose)
{
	if (NULL == program)
	{
		return 0;
	}

	STARTUPINFOW start_up_info;
	memset(&start_up_info, 0, sizeof(STARTUPINFOW));
	start_up_info.cb = sizeof(STARTUPINFO);
	start_up_info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	start_up_info.wShowWindow = SW_HIDE;
	start_up_info.hStdOutput = std_output;
	start_up_info.hStdError = std_error;
	/**/
	PROCESS_INFORMATION process_information;
	memset(&process_information, 0, sizeof(PROCESS_INFORMATION));
	process_information.hProcess = INVALID_HANDLE_VALUE;
	process_information.hThread = INVALID_HANDLE_VALUE;
	process_information.dwProcessId = process_information.dwThreadId = 0;

	if (!CreateProcessW(program,
						cmd,
						NULL,
						NULL,
						TRUE,
						NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
						env,
						working_dir,
						&start_up_info,
						&process_information))
	{
		return 0;
	}

	if (spawn &&
		NULL != pid_property &&
		!property_set_by_pointer(pid_property, &process_information.dwProcessId, sizeof(HANDLE),
								 property_value_is_integer, 0, 0, verbose))
	{
		CloseHandle(process_information.hProcess);
		CloseHandle(process_information.hThread);
		return 0;
	}

	if (NULL == process_handle)
	{
		CloseHandle(process_information.hProcess);
	}
	else
	{
		*process_handle = process_information.hProcess;
	}

	CloseHandle(process_information.hThread);
	return 1;
}

uint8_t exec_win32_with_redirect(
	const wchar_t* program, wchar_t* cmd, wchar_t* env, const wchar_t* working_dir,
	void* pid_property, void* result_property, const uint8_t* file, struct buffer* tmp,
	uint32_t time_out, uint8_t verbose)
{
	if (NULL == tmp)
	{
		return 0;
	}

	HANDLE hReadPipe = INVALID_HANDLE_VALUE;
	HANDLE hWritePipe = INVALID_HANDLE_VALUE;
	/**/
	SECURITY_ATTRIBUTES security_attributes;
	memset(&security_attributes, 0, sizeof(SECURITY_ATTRIBUTES));
	security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	security_attributes.lpSecurityDescriptor = NULL;
	security_attributes.bInheritHandle = TRUE;

	if (!CreatePipe(&hReadPipe, &hWritePipe, &security_attributes, 0))
	{
		return 0;
	}

	HANDLE process_handle = INVALID_HANDLE_VALUE;
	const int64_t time_span_start = 10 < time_out ? timespan_from_seconds((double)datetime_now()) : time_out;

	if (!exec_win32(
			program, cmd, env, working_dir, hWritePipe, hWritePipe,
			pid_property, result_property ? &process_handle : NULL, 0, verbose))
	{
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
		return 0;
	}

	CloseHandle(hWritePipe);

	if (!buffer_resize(tmp, 4096))
	{
		CloseHandle(hReadPipe);
		return 0;
	}

	security_attributes.nLength = (DWORD)(buffer_size(tmp) - 1);
	security_attributes.lpSecurityDescriptor = buffer_data(tmp, 0);
	DWORD numberOfBytesRead = 0;

	while (ReadFile(hReadPipe,
					security_attributes.lpSecurityDescriptor,
					security_attributes.nLength,
					&numberOfBytesRead, 0) &&
		   0 < numberOfBytesRead)
	{
		if (!echo(1, Default, file, Info, security_attributes.lpSecurityDescriptor, numberOfBytesRead, 0, verbose))
		{
			CloseHandle(hReadPipe);
			return 0;
		}
	}

	CloseHandle(hReadPipe);
	numberOfBytesRead = 1;

	if (result_property)
	{
		numberOfBytesRead = (WAIT_FAILED != WaitForSingleObject(process_handle, INFINITE));

		if (!numberOfBytesRead)
		{
			CloseHandle(process_handle);
			return 0;
		}

		security_attributes.nLength = 0;
		numberOfBytesRead = (0 != GetExitCodeProcess(process_handle, &security_attributes.nLength));
		CloseHandle(process_handle);

		if (numberOfBytesRead &&
			!property_set_by_pointer(result_property,
									 (const void*)&security_attributes.nLength, sizeof(DWORD),
									 property_value_is_integer, 0, 0, verbose))
		{
			return 0;
		}
	}

	if (!numberOfBytesRead)
	{
		return 0;
	}

	const int64_t time_span_finish = 10 < time_out ? timespan_from_seconds((double)datetime_now()) : time_out;
	return time_span_start < time_span_finish ? time_span_finish - time_span_start <= time_out : 1;
}

uint8_t exec(
	const void* the_project,
	const void* the_target,
	uint8_t append,
	struct buffer* program,
	const struct range* base_dir,
	const struct range* command_line,
	const struct range* output_file,
	void* pid_property,
	void* result_property,
	const struct range* working_dir,
	const struct range* environment_variables,
	uint8_t spawn,
	uint32_t time_out,
	uint8_t verbose)
{
	if (!buffer_size(program))
	{
		return 0;
	}

	const uint8_t* file = range_is_null_or_empty(output_file) ? NULL : output_file->start;

	if (!spawn && !append && NULL != file)
	{
		if (!echo(0, Default, file, Info, NULL, 0, 1, verbose))
		{
			return 0;
		}
	}

	struct range program_in_the_range;

	BUFFER_TO_RANGE(program_in_the_range, program);

	const uint8_t is_path_rooted = path_is_path_rooted(program_in_the_range.start, program_in_the_range.finish);

	struct buffer application;

	SET_NULL_TO_BUFFER(application);

	if (!buffer_resize(&application, spawn ? 1024 : 4096))
	{
		buffer_release(&application);
		return 0;
	}

	if (!exec_get_program_full_path(
			the_project, the_target, program,
			is_path_rooted, base_dir, &application, verbose) ||
		!buffer_resize(&application, 0) ||
		!buffer_append_data_from_buffer(&application, program))
	{
		buffer_release(&application);
		return 0;
	}

	if (!exec_win32_append_command_line(command_line, &application))
	{
		buffer_release(&application);
		return 0;
	}

	if (!range_is_null_or_empty(working_dir))
	{
		if (!path_combine(NULL, NULL, working_dir->start, working_dir->finish, &application))
		{
			buffer_release(&application);
			return 0;
		}

		if (!buffer_push_back(&application, zero_symbol))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (!range_is_null_or_empty(environment_variables))
	{
		if (!buffer_append_data_from_range(&application, environment_variables) ||
			!buffer_push_back(&application, zero_symbol))
		{
			buffer_release(&application);
			return 0;
		}
	}

	const ptrdiff_t size = buffer_size(&application);

	if (!buffer_append(&application, NULL, 4 * (size + 1) + sizeof(uint32_t)))
	{
		buffer_release(&application);
		return 0;
	}

	if (!buffer_resize(&application, size))
	{
		buffer_release(&application);
		return 0;
	}

	if (!text_encoding_UTF8_to_UTF16LE(buffer_data(&application, 0),
									   buffer_data(&application, 0) + size,
									   &application))
	{
		buffer_release(&application);
		return 0;
	}

	const wchar_t* programW = (const wchar_t*)buffer_data(&application, size);

	if (!range_is_null_or_empty(base_dir) && path_is_path_rooted(base_dir->start, base_dir->finish))
	{
		if (!file_exists_wchar_t(programW))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (is_path_rooted)
	{
		if (!file_exists_wchar_t(programW))
		{
			buffer_release(&application);
			return 0;
		}
	}

	const wchar_t* start = programW;
	const wchar_t* finish = (const wchar_t*)(buffer_data(&application, 0) + buffer_size(&application));
	/**/
	ptrdiff_t indexes[3];
	memset(indexes, 0, sizeof(indexes));
	uint8_t count = 0;

	while (finish != (start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, &zero_symbol_w, 1, 1,
							  1)) &&
		   count < COUNT_OF(indexes))
	{
		if (finish == (start = find_any_symbol_like_or_not_like_that_wchar_t(start + 1, finish, &zero_symbol_w, 1, 0,
							   1)))
		{
			break;
		}

		indexes[count++] = start - programW;
	}

	wchar_t* ptr = (wchar_t*)buffer_data(&application, size);
	wchar_t* command_lineW = ptr + indexes[0];
	const wchar_t* working_dirW = NULL;
	wchar_t* environment_variablesW = NULL;

	if (1 < count)
	{
		for (uint8_t i = 1; i < count; ++i)
		{
			if (NULL == working_dirW && !range_is_null_or_empty(working_dir))
			{
				working_dirW = programW + indexes[i];
				continue;
			}

			if (NULL == environment_variablesW && !range_is_null_or_empty(environment_variables))
			{
				environment_variablesW = ptr + indexes[i];
				continue;
			}
		}
	}

	if (spawn)
	{
		spawn = exec_win32(programW, command_lineW, environment_variablesW, working_dirW,
						   NULL, NULL, pid_property, NULL, spawn, verbose);
	}
	else
	{
		spawn = exec_win32_with_redirect(programW, command_lineW, environment_variablesW, working_dirW,
										 pid_property, result_property, file, &application, time_out, verbose);
	}

	buffer_release(&application);
	return spawn;
}

#else

uint8_t exec_posix_no_redirect(
	const char* program, char** cmd, char** env, const char* working_dir,
	void* pid_property, void* result_property, uint8_t verbose)
{
	if (NULL == program ||
		NULL == cmd ||
		NULL == pid_property)
	{
		return 0;
	}

	const pid_t pid = fork();

	if (!property_set_by_pointer(
			pid_property, (const void*)&pid, sizeof(pid_t), property_value_is_integer, 0, 0, verbose))
	{
		return 0;
	}

	if (-1 == pid)
	{
		return 0;
	}
	else if (0 == pid)
	{
		if (NULL != working_dir && -1 == chdir(working_dir))
		{
			exit(EXIT_FAILURE);
		}

		int status = EXIT_SUCCESS;

		if (NULL == env)
		{
			status = execv(program, cmd);
		}
		else
		{
			status = execve(program, cmd, env);
		}

		if (NULL != result_property)
		{
			property_set_by_pointer(result_property, (const void*)&status, sizeof(status),
									property_value_is_integer, 0, 0, verbose);
		}

		exit(status);
	}

	return 1;
}

uint8_t exec_posix_with_redirect(
	const char* program, char** cmd, char** env, const char* working_dir,
	const uint8_t* file, struct buffer* tmp, uint32_t time_out, void* result_property, uint8_t verbose)
{
	if (NULL == program ||
		NULL == cmd ||
		NULL == tmp)
	{
		return 0;
	}

	int file_des[2];
	const int64_t time_span_start = 10 < time_out ? timespan_from_seconds((double)datetime_now()) : time_out;

	if (pipe(file_des) == -1)
	{
		return 0;
	}

	const pid_t pid = fork();

	if (-1 == pid)
	{
		close(file_des[1]);
		close(file_des[0]);
		return 0;
	}
	else if (0 == pid)
	{
		while ((dup2(file_des[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}

		close(file_des[1]);
		close(file_des[0]);

		if (NULL != working_dir && -1 == chdir(working_dir))
		{
			exit(EXIT_FAILURE);
		}

		int status = EXIT_SUCCESS;

		if (NULL == env)
		{
			status = execv(program, cmd);
		}
		else
		{
			status = execve(program, cmd, env);
		}

		exit(status);
	}

	close(file_des[1]);

	if (!buffer_resize(tmp, 4096))
	{
		close(file_des[0]);
		return 0;
	}

	uint8_t* ptr = buffer_data(tmp, 0);
	const ptrdiff_t size = buffer_size(tmp) - 1;
	ssize_t count = 0;

	while (0 != (count = read(file_des[0], ptr, size)))
	{
		if (-1 == count)
		{
			if (EINTR == errno)
			{
				continue;
			}

			close(file_des[0]);
			return 0;
		}

		if (!echo(1, Default, file, Info, ptr, count, 0, verbose))
		{
			close(file_des[0]);
			return 0;
		}
	}

	close(file_des[0]);
	count = 0;

	if (-1 == wait((int*)&count))
	{
		return 0;
	}

	if (NULL != result_property)
	{
		return property_set_by_pointer(
				   result_property, (const void*)&count, sizeof(int),
				   property_value_is_integer, 0, 0, verbose);
	}

	const int64_t time_span_finish = 10 < time_out ? timespan_from_seconds((double)datetime_now()) : time_out;
	return time_span_start < time_span_finish ? time_span_finish - time_span_start <= time_out : 1;
}

uint8_t exec(
	const void* the_project,
	const void* the_target,
	uint8_t append,
	struct buffer* program,
	const struct range* base_dir,
	const struct range* command_line,
	const struct range* output_file,
	void* pid_property,
	void* result_property,
	const struct range* working_dir,
	const struct range* environment_variables,
	uint8_t spawn,
	uint32_t time_out,
	uint8_t verbose)
{
	if (!buffer_size(program))
	{
		return 0;
	}

	const uint8_t* file = range_is_null_or_empty(output_file) ? NULL : output_file->start;

	if (!spawn && !append && NULL != file)
	{
		if (!echo(0, Default, file, Info, NULL, 0, 1, verbose))
		{
			return 0;
		}
	}

	ptrdiff_t required_size = buffer_size(program);
	required_size += range_size(base_dir);
	required_size += range_size(command_line);
	required_size += range_size(working_dir);
	required_size += sizeof(const uint8_t*) * required_size;
	required_size += 1024;
	required_size = spawn ? required_size : MAX(required_size, 4096);
	/**/
	struct buffer application;
	SET_NULL_TO_BUFFER(application);

	if (!buffer_append(&application, NULL, required_size))
	{
		buffer_release(&application);
		return 0;
	}

	struct range program_in_the_range;

	BUFFER_TO_RANGE(program_in_the_range, program);

	const uint8_t is_path_rooted = path_is_path_rooted(program_in_the_range.start, program_in_the_range.finish);

	if (!exec_get_program_full_path(
			the_project, the_target, program,
			is_path_rooted, base_dir, &application, verbose) ||
		!buffer_resize(&application, 0) ||
		!buffer_append_data_from_buffer(&application, program))
	{
		buffer_release(&application);
		return 0;
	}

	if (!range_is_null_or_empty(command_line))
	{
		if (!argument_append_arguments(command_line->start, command_line->finish, &application))
		{
			buffer_release(&application);
			return 0;
		}
	}

	int argc = 0;
	char** cmd = NULL;

	if (!argument_create_arguments(&application, &argc, &cmd))
	{
		buffer_release(&application);
		return 0;
	}

	const ptrdiff_t cmd_index = (uint8_t*)cmd - buffer_data(&application, 0);
	ptrdiff_t working_dir_index = -1;

	if (!range_is_null_or_empty(working_dir))
	{
		working_dir_index = buffer_size(&application);

		if (!path_combine(NULL, NULL, working_dir->start, working_dir->finish, &application))
		{
			buffer_release(&application);
			return 0;
		}

		if (!buffer_push_back(&application, zero_symbol))
		{
			buffer_release(&application);
			return 0;
		}

		struct range work_dir_path;

		work_dir_path.start = buffer_data(&application, working_dir_index);

		work_dir_path.finish = buffer_data(&application, 0) + buffer_size(&application);

		if (path_is_path_rooted(work_dir_path.start, work_dir_path.finish) && !directory_exists(work_dir_path.start))
		{
			buffer_release(&application);
			return 0;
		}
	}

	char** env = NULL;
	ptrdiff_t env_index = -1;

	if (!range_is_null_or_empty(environment_variables))
	{
		env_index = buffer_size(&application);

		if (!buffer_append_data_from_range(&application, environment_variables))
		{
			buffer_release(&application);
			return 0;
		}

		argc = (int)env_index;

		if (!argument_create_arguments(&application, &argc, &env))
		{
			buffer_release(&application);
			return 0;
		}

		env_index = (uint8_t*)env - buffer_data(&application, 0);
	}

	cmd = (char**)buffer_data(&application, cmd_index);
	env = 0 < env_index ? (char**)buffer_data(&application, env_index) : NULL;
	const char* work = 0 < working_dir_index ?
					   (const char*)buffer_data(&application, working_dir_index) : NULL;

	if (spawn)
	{
		spawn = exec_posix_no_redirect((const char*)buffer_data(&application, 0), cmd, env, work,
									   pid_property, result_property, verbose);
	}
	else
	{
		spawn = exec_posix_with_redirect((const char*)buffer_data(&application, 0), cmd, env, work,
										 file, &application, time_out, result_property, verbose);
	}

	buffer_release(&application);
	return spawn;
}

#endif

static const uint8_t* exec_attributes[] =
{
	(const uint8_t*)"program",
	(const uint8_t*)"append",
	(const uint8_t*)"basedir",
	(const uint8_t*)"commandline",
	(const uint8_t*)"output",
	(const uint8_t*)"pidproperty",
	(const uint8_t*)"resultproperty",
	(const uint8_t*)"spawn",
	(const uint8_t*)"workingdir",
	(const uint8_t*)"timeout"
};

static const uint8_t exec_attributes_lengths[] =
{
	7, 6, 7, 11, 6, 11, 14, 5, 10, 7
};

#define PROGRAM_POSITION			0
#define APPEND_POSITION				1
#define BASE_DIR_POSITION			2
#define COMMAND_LINE_POSITION		3
#define OUTPUT_POSITION				4
#define PID_PROPERTY_POSITION		5
#define RESULT_PROPERTY_POSITION	6
#define SPAWN_POSITION				7
#define WORKING_DIR_POSITION		8
#define TIME_OUT_POSITION			9
#define ENVIRONMENT_POSITION		10

#define ATTRIBUTES_COUNT	(ENVIRONMENT_POSITION + 1)

uint8_t exec_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	if (!common_get_attributes_and_arguments_for_task(
			exec_attributes, exec_attributes_lengths,
			COUNT_OF(exec_attributes_lengths),
			task_attributes, task_attributes_lengths,
			task_attributes_count, task_arguments))
	{
		return 0;
	}

	buffer_release_inner_buffers(task_arguments);

	if (!buffer_resize(task_arguments, 0) ||
		!buffer_append_buffer(task_arguments, NULL, ATTRIBUTES_COUNT))
	{
		return 0;
	}

	for (uint8_t i = 0, attributes_count = ATTRIBUTES_COUNT; i < attributes_count; ++i)
	{
		struct buffer* attribute = buffer_buffer_data(task_arguments, i);
		SET_NULL_TO_BUFFER(*attribute);
	}

	return 1;
}

uint8_t exec_evaluate_task(void* the_project, const void* the_target, const struct buffer* task_arguments,
						   uint8_t verbose)
{
	if (NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* path_to_the_program = buffer_buffer_data(task_arguments, PROGRAM_POSITION);

	if (!buffer_size(path_to_the_program))
	{
		return 0;
	}

	const struct buffer* append_in_a_buffer = buffer_buffer_data(task_arguments, APPEND_POSITION);
	uint8_t append = 0;

	if (buffer_size(append_in_a_buffer) &&
		!bool_parse(buffer_data(append_in_a_buffer, 0), buffer_size(append_in_a_buffer), &append))
	{
		return 0;
	}

	const struct buffer* base_dir_in_a_buffer = buffer_buffer_data(task_arguments, BASE_DIR_POSITION);
	struct range base_directory;
	BUFFER_TO_RANGE(base_directory, base_dir_in_a_buffer);
	const struct buffer* command_line_in_a_buffer = buffer_buffer_data(task_arguments, COMMAND_LINE_POSITION);
	struct range command_line;
	BUFFER_TO_RANGE(command_line, command_line_in_a_buffer);
	struct buffer* output_path_in_a_buffer = buffer_buffer_data(task_arguments, OUTPUT_POSITION);
	struct range output_file;

	if (buffer_size(output_path_in_a_buffer))
	{
		if (!buffer_push_back(output_path_in_a_buffer, 0))
		{
			return 0;
		}

		output_file.start = buffer_data(output_path_in_a_buffer, 0);
		output_file.finish = output_file.start + buffer_size(output_path_in_a_buffer);
	}
	else
	{
		output_file.start = output_file.finish = NULL;
	}

	void* pid_property = NULL;
	void* result_property = NULL;

	for (uint8_t index = PID_PROPERTY_POSITION; ; index = RESULT_PROPERTY_POSITION)
	{
		const struct buffer* property_in_a_buffer = buffer_buffer_data(task_arguments, index);

		if (!buffer_size(property_in_a_buffer))
		{
			if (RESULT_PROPERTY_POSITION == index)
			{
				break;
			}

			continue;
		}

		if (NULL == the_project)
		{
			return 0;
		}

		void** the_property = (PID_PROPERTY_POSITION == index ? &pid_property : &result_property);

		if (!project_property_set_value(the_project, buffer_data(property_in_a_buffer, 0),
										(uint8_t)buffer_size(property_in_a_buffer),
										(const uint8_t*)the_property, 0, 0, 1, 0, verbose) ||
			!project_property_exists(the_project, buffer_data(property_in_a_buffer, 0),
									 (uint8_t)buffer_size(property_in_a_buffer), the_property, verbose))
		{
			return 0;
		}

		if (RESULT_PROPERTY_POSITION == index)
		{
			break;
		}
	}

	const struct buffer* spawn_in_a_buffer = buffer_buffer_data(task_arguments, SPAWN_POSITION);
	uint8_t spawn = 0;

	if (buffer_size(spawn_in_a_buffer) &&
		!bool_parse(buffer_data(spawn_in_a_buffer, 0), buffer_size(spawn_in_a_buffer), &spawn))
	{
		return 0;
	}

	struct buffer* working_dir_in_a_buffer = buffer_buffer_data(task_arguments, WORKING_DIR_POSITION);

	struct range working_directory;

	if (buffer_size(working_dir_in_a_buffer))
	{
		if (!buffer_push_back(working_dir_in_a_buffer, 0))
		{
			return 0;
		}

		working_directory.start = buffer_data(working_dir_in_a_buffer, 0);
		working_directory.finish = working_directory.start + buffer_size(working_dir_in_a_buffer);
	}
	else
	{
		working_directory.start = working_directory.finish = NULL;
	}

	struct buffer* time_out_in_a_buffer = buffer_buffer_data(task_arguments, TIME_OUT_POSITION);

	uint64_t time_out = 0;

	if (buffer_size(time_out_in_a_buffer))
	{
		const uint8_t* start = buffer_data(time_out_in_a_buffer, 0);
		const uint8_t* finish = start + buffer_size(time_out_in_a_buffer);
		time_out = uint64_parse(start, finish);

		if (1000 < time_out)
		{
			time_out = (uint64_t)date_time_millisecond_to_second(time_out);

			if (time_out < 5)
			{
				time_out = 5;
			}
		}
	}

	const struct buffer* environment_in_a_buffer = buffer_buffer_data(task_arguments, ENVIRONMENT_POSITION);
	struct range environment_variables;

	if (buffer_size(environment_in_a_buffer))
	{
		environment_variables.start = buffer_data(environment_in_a_buffer, 0);
		environment_variables.finish = environment_variables.start + buffer_size(environment_in_a_buffer);
	}
	else
	{
		environment_variables.start = environment_variables.finish = NULL;
	}

	return exec(the_project, the_target,
				append, path_to_the_program, &base_directory, &command_line, &output_file,
				pid_property, result_property, &working_directory, &environment_variables,
				spawn, (uint32_t)time_out, verbose);
}
