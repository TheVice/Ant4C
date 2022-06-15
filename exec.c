/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#if !defined(_WIN32)
#define _POSIX_SOURCE 1
#define _POSIX_C_SOURCE 200112L
#endif

#include "exec.h"
#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "date_time.h"
#include "echo.h"
#include "file_system.h"
#include "path.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"
#include "xml.h"

#include <stdio.h>

#if defined(_WIN32)
#include <wchar.h>

#include <windows.h>
#else
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

#if defined(_WIN32)
static const uint8_t space = ' ';
static const wchar_t zeroW = L'\0';
#endif
static const uint8_t zero = '\0';

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

			if (!buffer_append(path_to_the_program, path, common_count_bytes_until(path, zero)) ||
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
							   buffer_size(path_to_the_program) - size, &zero, &zero))
#else
	if (!path_combine_in_place(path_to_the_program, 0, &zero, &zero))
#endif
	{
		return 0;
	}

	return buffer_push_back(path_to_the_program, zero);
}

#if defined(_WIN32)

uint8_t exec_win32_append_command_line(
	const struct range* command_line, struct buffer* output)
{
	if (!output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);
	const uint8_t* path_start = buffer_data(output, 0);
	const uint8_t* path_finish = path_start + size;
	/**/
	const uint8_t contains = string_contains(
								 path_start, path_finish,
								 &space, &space + 1);

	if (!buffer_append(output, NULL, size + 3) ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	path_start = buffer_data(output, 0);
	path_finish = path_start + size;

	if (!file_system_get_position_after_pre_root(&path_start, path_finish))
	{
		return 0;
	}

	const uint8_t* pos = string_find_any_symbol_like_or_not_like_that(
							 path_finish, path_start,
							 &zero, &zero + 1, 0, -1);
	path_finish = string_find_any_symbol_like_or_not_like_that(
					  pos, path_finish,
					  &zero, &zero + 1, 1, 1);

	if (contains)
	{
		if (!string_quote(path_start, path_finish, output))
		{
			return 0;
		}
	}
	else
	{
		if (!buffer_append(output, path_start, path_finish - path_start))
		{
			return 0;
		}
	}

	if (!range_is_null_or_empty(command_line))
	{
		if (!buffer_push_back(output, space) ||
			!buffer_append_data_from_range(output, command_line))
		{
			return 0;
		}
	}

	return buffer_push_back(output, zero);
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
		if (!echo(1, Default, file, Info, (const uint8_t*)(security_attributes.lpSecurityDescriptor),
				  numberOfBytesRead, 0, verbose))
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

	uint8_t is_path_rooted;

	if (!path_is_path_rooted(program_in_the_range.start, program_in_the_range.finish, &is_path_rooted))
	{
		return 0;
	}

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

		if (!buffer_push_back(&application, zero))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (!range_is_null_or_empty(environment_variables))
	{
		if (!buffer_append_data_from_range(&application, environment_variables) ||
			!buffer_push_back(&application, zero))
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

	if (!range_is_null_or_empty(base_dir))
	{
		uint8_t is_base_dir_rooted;

		if (!path_is_path_rooted(base_dir->start, base_dir->finish, &is_base_dir_rooted))
		{
			buffer_release(&application);
			return 0;
		}

		if (is_base_dir_rooted && !file_exists_wchar_t(programW))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (is_path_rooted && !file_exists_wchar_t(programW))
	{
		buffer_release(&application);
		return 0;
	}

	const wchar_t* start = programW;
	const wchar_t* finish = (const wchar_t*)(buffer_data(&application, 0) + buffer_size(&application));
	/**/
	ptrdiff_t indices[3];
	memset(indices, 0, sizeof(indices));
	uint8_t count = 0;

	while (finish != (start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, &zeroW, 1, 1,
							  1)) &&
		   count < COUNT_OF(indices))
	{
		if (finish == (start = find_any_symbol_like_or_not_like_that_wchar_t(start + 1, finish, &zeroW, 1, 0,
							   1)))
		{
			break;
		}

		indices[count++] = start - programW;
	}

	wchar_t* ptr = (wchar_t*)buffer_data(&application, size);
	wchar_t* command_lineW = ptr + indices[0];
	const wchar_t* working_dirW = NULL;
	wchar_t* environment_variablesW = NULL;

	for (uint8_t i = 1; i < count; ++i)
	{
		if (NULL == working_dirW && !range_is_null_or_empty(working_dir))
		{
			working_dirW = programW + indices[i];
			continue;
		}

		if (NULL == environment_variablesW && !range_is_null_or_empty(environment_variables))
		{
			environment_variablesW = ptr + indices[i];
			continue;
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

	const uint8_t* program_start = buffer_data(program, 0);
	const uint8_t* program_finish = program_start + buffer_size(program);
	uint8_t is_path_rooted;

	if (!path_is_path_rooted(program_start, program_finish, &is_path_rooted))
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

		if (!buffer_push_back(&application, zero))
		{
			buffer_release(&application);
			return 0;
		}

		struct range work_dir_path;

		work_dir_path.start = buffer_data(&application, working_dir_index);

		work_dir_path.finish = buffer_data(&application, 0) + buffer_size(&application);

		if (!path_is_path_rooted(work_dir_path.start, work_dir_path.finish, &is_path_rooted))
		{
			buffer_release(&application);
			return 0;
		}

		if (is_path_rooted && !directory_exists(work_dir_path.start))
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
