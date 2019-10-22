/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "exec.h"
#include "argument_parser.h"
#include "buffer.h"
#include "conversion.h"
#include "common.h"
#include "echo.h"
#include "file_system.h"
#include "interpreter.h"
#include "math_unit.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "xml.h"

#if defined(_WIN32)

#include <windows.h>

uint8_t exec_win32(const wchar_t* program, wchar_t* cmd,
				   wchar_t* env, const wchar_t* working_dir,
				   HANDLE hWritePipe, void* pid_property,
				   uint8_t spawn, uint32_t time_out, uint8_t verbose)
{
	(void)time_out;/*TODO:*/

	if (NULL == program || L'\0' == *program)
	{
		return 0;
	}

	STARTUPINFOW start_up_info = { 0 };
	start_up_info.cb = sizeof(STARTUPINFO);
	start_up_info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	start_up_info.wShowWindow = SW_HIDE;
	start_up_info.hStdOutput = hWritePipe;
	start_up_info.hStdError = hWritePipe;
	/**/
	PROCESS_INFORMATION process_information;
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
		!property_set_by_pointer(NULL, NULL,
								 pid_property, &process_information.dwProcessId, sizeof(HANDLE),
								 property_value_is_integer, 0, 0, verbose))
	{
		CloseHandle(process_information.hProcess);
		CloseHandle(process_information.hThread);
		return 0;
	}

	CloseHandle(process_information.hProcess);
	CloseHandle(process_information.hThread);
	return 1;
}

uint8_t exec_win32_with_redirect(
	const wchar_t* program, wchar_t* cmd, wchar_t* env, const wchar_t* working_dir,
	const char* file, struct buffer* tmp, uint32_t time_out, uint8_t verbose)
{
	if (NULL == tmp)
	{
		return 0;
	}

	HANDLE hReadPipe = INVALID_HANDLE_VALUE;
	HANDLE hWritePipe = INVALID_HANDLE_VALUE;
	/**/
	SECURITY_ATTRIBUTES security_attributes;
	security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	security_attributes.lpSecurityDescriptor = NULL;
	security_attributes.bInheritHandle = TRUE;

	if (!CreatePipe(&hReadPipe, &hWritePipe, &security_attributes, 0))
	{
		return 0;
	}

	if (!exec_win32(program, cmd, env, working_dir, hWritePipe, NULL, 0, time_out, verbose))
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

	while (1)
	{
		DWORD numberOfBytesRead = (DWORD)(buffer_size(tmp) - 1);
		BOOL ret = ReadFile(hReadPipe, buffer_char_data(tmp, 0),
							numberOfBytesRead, &numberOfBytesRead, 0);
		/*
		static const char* new_line = "\n";
		static const char* new_line_with_info = "\n[Info]: ";*/

		if (ret && 0 < numberOfBytesRead)
		{
			if (/*!string_replace_in_buffer(tmp, new_line, 1, new_line_with_info, 9) ||*/
				!echo(1, Default, file, NoLevel, buffer_char_data(tmp, 0), numberOfBytesRead, 0, verbose))
			{
				CloseHandle(hReadPipe);
				return 0;
			}
		}
		else
		{
			break;
		}
	}

	CloseHandle(hReadPipe);
	return 1;
}

uint8_t exec(
	uint8_t append,
	const struct range* program,
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
	(void)result_property;

	if (range_is_null_or_empty(program))
	{
		return 0;
	}

	const char* file = range_is_null_or_empty(output_file) ? NULL : output_file->start;

	if (!spawn && !append && NULL != file)
	{
		if (!echo(0, Default, file, NoLevel, NULL, 0, 1, verbose))
		{
			return 0;
		}
	}

	struct buffer application;

	SET_NULL_TO_BUFFER(application);

	if (!range_is_null_or_empty(base_dir) &&
		!path_is_path_rooted(program->start, program->finish))
	{
		if (!path_combine(base_dir->start, base_dir->finish,
						  program->start, program->finish, &application))
		{
			buffer_release(&application);
			return 0;
		}

		if (!buffer_push_back(&application, '\0'))
		{
			buffer_release(&application);
			return 0;
		}

		if (!file_exists(buffer_char_data(&application, 0)))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (!buffer_size(&application))
	{
		if (!buffer_append_data_from_range(&application, program))
		{
			buffer_release(&application);
			return 0;
		}

		if (!buffer_push_back(&application, '\0'))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (!range_is_null_or_empty(command_line))
	{
		static const char space = ' ';
		const char* ptr = buffer_char_data(&application, 0);
		ptrdiff_t size_ = buffer_size(&application);
		const uint8_t contains = string_contains(ptr, ptr + size_, &space, &space + 1);

		if (!buffer_append_char(&application, NULL, size_ + 3) ||
			!buffer_resize(&application, size_))
		{
			buffer_release(&application);
			return 0;
		}

		ptr = buffer_char_data(&application, 0);

		if ((contains && !buffer_push_back(&application, '"')) ||
			!buffer_append_char(&application, ptr, size_ - 1) ||
			(contains && !buffer_push_back(&application, '"')))
		{
			buffer_release(&application);
			return 0;
		}

		if (!buffer_push_back(&application, ' ') ||
			!buffer_append_data_from_range(&application, command_line))
		{
			buffer_release(&application);
			return 0;
		}

		if (!buffer_push_back(&application, '\0'))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (!range_is_null_or_empty(working_dir))
	{
		if (!buffer_append_data_from_range(&application, working_dir))
		{
			buffer_release(&application);
			return 0;
		}

		if (!buffer_push_back(&application, '\0'))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (!range_is_null_or_empty(environment_variables))
	{
		if (!buffer_append_data_from_range(&application, environment_variables) ||
			!buffer_push_back(&application, '\0'))
		{
			buffer_release(&application);
			return 0;
		}
	}

	const ptrdiff_t size = buffer_size(&application);

	if (!buffer_append_wchar_t(&application, NULL, (ptrdiff_t)2 + size))
	{
		buffer_release(&application);
		return 0;
	}

	char* m = buffer_char_data(&application, 0);
	wchar_t* programW = (wchar_t*)buffer_data(&application, size);

	if (NULL == programW ||
		!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, m, (int32_t)size, programW, (int32_t)size))
	{
		buffer_release(&application);
		return 0;
	}

	const wchar_t* finish = (wchar_t*)(buffer_data(&application, 0) + buffer_size(&application));
	wchar_t* command_lineW = NULL;

	if (!range_is_null_or_empty(command_line))
	{
		command_lineW = (wchar_t*)find_any_symbol_like_or_not_like_that_wchar_t(programW, finish, L"\0", 1, 1, 1);
		command_lineW = (wchar_t*)find_any_symbol_like_or_not_like_that_wchar_t(command_lineW + 1, finish, L"\0", 1,
						0, 1);

		if (finish == command_lineW)
		{
			buffer_release(&application);
			return 0;
		}
	}

	wchar_t* working_dirW = NULL;

	if (!range_is_null_or_empty(working_dir))
	{
		working_dirW = (wchar_t*)find_any_symbol_like_or_not_like_that_wchar_t((NULL == command_lineW) ? programW :
					   command_lineW, finish, L"\0", 1, 1, 1);
		working_dirW = (wchar_t*)find_any_symbol_like_or_not_like_that_wchar_t(working_dirW + 1, finish, L"\0", 1, 0,
					   1);

		if (finish == working_dirW)
		{
			buffer_release(&application);
			return 0;
		}
	}

	wchar_t* environment_variablesW = NULL;

	if (!range_is_null_or_empty(environment_variables))
	{
		wchar_t* ptr = (NULL == command_lineW) ? programW : command_lineW;
		ptr = (NULL == working_dirW) ? ptr : working_dirW;
		/**/
		environment_variablesW = (wchar_t*)find_any_symbol_like_or_not_like_that_wchar_t(ptr, finish, L"\0", 1, 1, 1);
		environment_variablesW = (wchar_t*)find_any_symbol_like_or_not_like_that_wchar_t(environment_variablesW + 1,
								 finish, L"\0", 1, 0, 1);

		if (finish == environment_variablesW)
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (spawn)
	{
		spawn = exec_win32(programW, command_lineW, environment_variablesW, working_dirW, NULL, pid_property, spawn,
						   time_out, verbose);
	}
	else
	{
		spawn = exec_win32_with_redirect(programW, command_lineW, environment_variablesW, working_dirW,
										 file, &application, time_out, verbose);
	}

	buffer_release(&application);
	return spawn;
}

#else

#define _POSIXSOURCE 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

uint8_t exec_posix_no_redirect(
	const char* program, char** cmd, char** env, const char* working_dir,
	void* pid_property, uint8_t verbose)
{
	if (NULL == program ||
		NULL == cmd ||
		NULL == pid_property)
	{
		return 0;
	}

	const pid_t pid = fork();

	if (NULL != pid_property &&
		!property_set_by_pointer(NULL, NULL,
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
			return 0;
		}

		NULL == env ? execv(program, cmd) : execve(program, cmd, env);
		return 0;
	}

	return 1;
}

uint8_t exec_posix_with_redirect(
	const char* program, char** cmd, char** env, const char* working_dir,
	const char* file, struct buffer* tmp, uint32_t time_out, uint8_t verbose)
{
	(void)time_out;

	if (NULL == program ||
		NULL == cmd ||
		NULL == tmp)
	{
		return 0;
	}

	int file_des[2];

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
			return 0;
		}

		NULL == env ? execv(program, cmd) : execve(program, cmd, env);
		return 0;
	}

	close(file_des[1]);

	if (!buffer_resize(tmp, 4096))
	{
		close(file_des[0]);
		return 0;
	}

	while (1)
	{
		const ssize_t count = read(file_des[0], buffer_char_data(tmp, 0), 4096);

		if (count == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				close(file_des[0]);
				return 0;
			}
		}
		else if (count == 0)
		{
			break;
		}
		else
		{
			if (!echo(1, Default, file, NoLevel, buffer_char_data(tmp, 0), count, 0, verbose))
			{
				close(file_des[0]);
				return 0;
			}
		}
	}

	close(file_des[0]);
	wait(0);
	return 1;
}

uint8_t exec(
	uint8_t append,
	const struct range* program,
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
	if (range_is_null_or_empty(program))
	{
		return 0;
	}

	const char* file = range_is_null_or_empty(output_file) ? NULL : output_file->start;

	if (!spawn && !append && NULL != file)
	{
		if (!echo(0, Default, file, NoLevel, NULL, 0, 1, verbose))
		{
			return 0;
		}
	}

	ptrdiff_t expected_size = range_size(program) + 1;
	expected_size += sizeof(char*);
	expected_size += range_size(base_dir);
	expected_size += range_size(command_line) + 1;
	expected_size += range_size(working_dir) + 1;
	expected_size += range_size(environment_variables) + 1;
	expected_size += (range_size(command_line) / 2) * sizeof(char*);
	expected_size += (range_size(environment_variables) / 2) * sizeof(char*);
	expected_size += 2 * sizeof(char*) + 2;
	//
	static const char space = ' ';
	struct buffer application;
	SET_NULL_TO_BUFFER(application);

	if (!buffer_append(&application, NULL, expected_size) ||
		!buffer_resize(&application, 0))
	{
		buffer_release(&application);
		return 0;
	}

	if (!range_is_null_or_empty(base_dir) &&
		!path_is_path_rooted(program->start, program->finish))
	{
		const uint8_t contains = string_contains(base_dir->start, base_dir->finish, &space, &space + 1) ||
								 string_contains(program->start, program->finish, &space, &space + 1);

		if (contains)
		{
			if (!buffer_append(&application, NULL, range_size(base_dir) + range_size(program) + 3) ||
				!buffer_resize(&application, 0) ||
				!buffer_push_back(&application, '"'))
			{
				buffer_release(&application);
				return 0;
			}
		}

		if (!path_combine(base_dir->start, base_dir->finish,
						  program->start, program->finish, &application))
		{
			buffer_release(&application);
			return 0;
		}

		if (contains)
		{
			if (!buffer_push_back(&application, '"'))
			{
				buffer_release(&application);
				return 0;
			}
		}

		if (!buffer_push_back(&application, '\0'))
		{
			buffer_release(&application);
			return 0;
		}

		if (!file_exists(buffer_char_data(&application, 0)))
		{
			buffer_release(&application);
			return 0;
		}
	}

	if (!buffer_size(&application))
	{
		const uint8_t contains = string_contains(program->start, program->finish, &space, &space + 1);

		if (contains)
		{
			if (!buffer_append(&application, NULL, range_size(program) + 3) ||
				!buffer_resize(&application, 0) ||
				!buffer_push_back(&application, '"'))
			{
				buffer_release(&application);
				return 0;
			}
		}

		if (!buffer_append_data_from_range(&application, program))
		{
			buffer_release(&application);
			return 0;
		}

		if (contains)
		{
			if (!buffer_push_back(&application, '"'))
			{
				buffer_release(&application);
				return 0;
			}
		}

		if (!buffer_push_back(&application, '\0'))
		{
			buffer_release(&application);
			return 0;
		}
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
		const uint8_t contains = string_contains(working_dir->start, working_dir->finish, &space, &space + 1);

		if (contains)
		{
			if (!buffer_append(&application, NULL, range_size(working_dir) + 3) ||
				!buffer_resize(&application, working_dir_index) ||
				!buffer_push_back(&application, '"'))
			{
				buffer_release(&application);
				return 0;
			}
		}

		if (!buffer_append_data_from_range(&application, working_dir))
		{
			buffer_release(&application);
			return 0;
		}

		if (contains)
		{
			if (!buffer_push_back(&application, '"'))
			{
				buffer_release(&application);
				return 0;
			}
		}

		if (!buffer_push_back(&application, '\0'))
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

	cmd = (char**)(buffer_data(&application, 0) + cmd_index);
	env = 0 < env_index ? (char**)(buffer_data(&application, 0) + env_index) : NULL;
	const char* work = 0 < working_dir_index ? (const char*)(buffer_data(&application,
					   0) + working_dir_index) : NULL;

	if (spawn)
	{
		spawn = exec_posix_no_redirect((const char*)buffer_data(&application, 0), cmd, env, work,
									   pid_property, verbose);
	}
	else
	{
		spawn = exec_posix_with_redirect((const char*)buffer_data(&application, 0), cmd, env, work,
										 file, &application, time_out, verbose);
	}

	/*spawn = 1;
	printf("\n\n<!---\n");
	char* c = NULL;
	argc = 0;

	while (NULL != cmd[argc])
	{
		c = cmd[argc++];
		printf("cmd - %s\n", c);
	}

	if (NULL != env)
	{
		char* e = NULL;
		argc = 0;

		while (NULL != env[argc])
		{
			e = env[argc++];
			printf("env - %s\n", e);
		}
	}

	printf("work - %s\n", work);
	printf("--->\n\n");*/
	/*(void)append;
	(void)program;
	(void)base_dir;
	(void)command_line;
	(void)output_file;
	(void)pid_property;*/
	(void)result_property;
	/*(void)working_dir;
	(void)environment_variables;
	(void)spawn;
	(void)time_out;
	(void)verbose;*/
	buffer_release(&application);
	return spawn;
}

#endif

uint8_t exec_get_environments(const char* start, const char* finish, struct buffer* environments)
{
	if (range_in_parts_is_null_or_empty(start, finish) ||
		NULL == environments)
	{
		return 0;
	}

	struct buffer elements;

	SET_NULL_TO_BUFFER(elements);

	if (!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		return 0;
	}

	uint16_t count = xml_get_sub_nodes_elements(start, finish, &elements);

	if (!count)
	{
		buffer_release(&elements);
		return 1;
	}

	count = 0;
	struct range name;
	struct range* env_ptr = NULL;

	while (NULL != (env_ptr = buffer_range_data(&elements, count++)))
	{
		static const char* env_name = "environment";

		if (!xml_get_tag_name(env_ptr->start, env_ptr->finish, &name))
		{
			buffer_release(&elements);
			return 0;
		}

		if (string_equal(name.start, name.finish, env_name, env_name + 11))
		{
			break;
		}
	}

	if (NULL == env_ptr)
	{
		buffer_release(&elements);
		return 1;
	}

	name = *env_ptr;

	if (!buffer_resize(&elements, 0))
	{
		buffer_release(&elements);
		return 0;
	}

	count = xml_get_sub_nodes_elements(name.start, name.finish, &elements);

	if (!count)
	{
		buffer_release(&elements);
		return 1;
	}

	count = 0;

	while (NULL != (env_ptr = buffer_range_data(&elements, count++)))
	{
		static const char* var_name = "variable";

		if (!xml_get_tag_name(env_ptr->start, env_ptr->finish, &name))
		{
			buffer_release(&elements);
			return 0;
		}

		if (string_equal(name.start, name.finish, var_name, var_name + 8))
		{
			if (!xml_get_attribute_value(env_ptr->start, env_ptr->finish, "name", 4, &name) ||
				range_is_null_or_empty(&name))
			{
				buffer_release(&elements);
				return 0;
			}

			static const char space = ' ';
			uint8_t contains = string_contains(name.start, name.finish, &space, &space + 1);

			if ((contains && !buffer_push_back(environments, '"')) ||
				!buffer_append_data_from_range(environments, &name) ||
				(contains && !buffer_push_back(environments, '"')) ||
				!buffer_push_back(environments, '='))
			{
				buffer_release(&elements);
				return 0;
			}

			if (xml_get_attribute_value(env_ptr->start, env_ptr->finish, "value", 5, &name))
			{
				contains = string_contains(name.start, name.finish, &space, &space + 1);

				if ((contains && !buffer_push_back(environments, '"')) ||
					!buffer_append_data_from_range(environments, &name) ||
					(contains && !buffer_push_back(environments, '"')))
				{
					buffer_release(&elements);
					return 0;
				}
			}

			if (!buffer_push_back(environments, '\0'))
			{
				buffer_release(&elements);
				return 0;
			}
		}
	}

	buffer_release(&elements);
	return 1;
}

uint32_t millisecond_to_second(uint64_t millisecond)
{
	return (uint32_t)math_truncate(math_ceiling((double)millisecond / 1000));
}

#define PROGRAM_POSITION			0
#define APPEND_POSITION				1
#define BASE_DIR_POSITION			2
#define COMMAND_LINE_POSITION		3
#define OUTPUT_POSITION				4
#define PID_PROPERTY_POSITION		5
#define RESULT_PROPERTY_POSITION	6
#define SPAWN_POSITION				7
#define WORKING_DIR_POSITION		8
#define FAIL_ON_ERROR_POSITION		9
#define TIME_OUT_POSITION			10
#define VERBOSE_POSITION			11
#define ENVIRONMENT_POSITION		12

uint8_t exec_get_arguments_for_task(
	void* project,
	const void* target,
	const char* attributes_start,
	const char* attributes_finish,
	const char* element_finish,
	struct buffer* arguments)
{
	if (range_in_parts_is_null_or_empty(attributes_start, attributes_finish) ||
		NULL == element_finish || element_finish < attributes_finish ||
		NULL == arguments)
	{
		return 0;
	}

	static const char* attributes[] = { "program", "append", "basedir", "commandline",
										"output", "pidproperty", "resultproperty", "spawn",
										"workingdir", "failonerror", "timeout", "verbose"
									  };
	static const uint8_t attributes_lengths[] = { 7, 6, 7, 11,
												  6, 11, 14, 5,
												  10, 11, 7, 7
												};
	/**/
	static const uint8_t default_append_value = 0;
	static const char* default_basedir_value = NULL;
	static const char* default_command_line_value = NULL;
	static const char* default_output_value = NULL;
	static const char* default_pid_property_value = NULL;
	static const char* default_result_property_value = NULL;
	static const uint8_t default_spawn_value = 0;
	static const char* default_working_dir_value = NULL;
	static const uint8_t default_fail_on_error_value = 1;
	static const uint32_t default_time_out_value = 0;
	static const uint8_t default_verbose_value = 0;
	/**/
	struct buffer argument;
	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_buffer(arguments, &argument, 1)) /*NOTE: reserve space for program.*/
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!bool_to_string(default_append_value, &argument) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_basedir_value, 0) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_command_line_value, 0) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_output_value, 0) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_pid_property_value, 0) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_result_property_value, 0) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!bool_to_string(default_spawn_value, &argument) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!buffer_append_char(&argument, default_working_dir_value, 0) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!bool_to_string(default_fail_on_error_value, &argument) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if ((0 < default_time_out_value && !int_to_string(default_time_out_value, &argument)) ||
		!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!bool_to_string(default_verbose_value, &argument) || !buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	if (!interpreter_get_arguments_from_xml_tag_record(project, target, attributes_start, attributes_finish,
			attributes, attributes_lengths, sizeof(attributes_lengths) / sizeof(*attributes_lengths), arguments))
	{
		return 0;
	}

	SET_NULL_TO_BUFFER(argument);

	if (!range_in_parts_is_null_or_empty(attributes_finish, element_finish) &&
		!exec_get_environments(attributes_finish, element_finish, &argument))
	{
		buffer_release(&argument);
		return 0;
	}

	if (!buffer_append_buffer(arguments, &argument, 1))
	{
		buffer_release(&argument);
		return 0;
	}

	struct buffer* argument_value = buffer_buffer_data(arguments, VERBOSE_POSITION);

	if (NULL == argument_value)
	{
		return 0;
	}

	/*TODO: verbose of current function should be set outside.*/
	uint8_t verbose = 0;

	if (!bool_parse(buffer_char_data(argument_value, 0),
					buffer_char_data(argument_value, 0) + buffer_size(argument_value), &verbose))
	{
		return 0;
	}

	for (uint8_t index = PID_PROPERTY_POSITION; ; index = RESULT_PROPERTY_POSITION)
	{
		argument_value = buffer_buffer_data(arguments, index);

		if (NULL == argument_value)
		{
			return 0;
		}

		if (buffer_size(argument_value))
		{
			void* the_property = NULL;

			if (NULL == project)
			{
				return 0;
			}

			if (!project_property_set_value(project, NULL, buffer_char_data(argument_value, 0),
											(uint8_t)buffer_size(argument_value), NULL, 0, 0, 1, 0, verbose) ||
				!project_property_get_pointer(project, buffer_char_data(argument_value, 0),
											  (uint8_t)buffer_size(argument_value),
											  &the_property))
			{
				return 0;
			}

			if (!buffer_resize(argument_value, 0) ||
				!buffer_append(argument_value, the_property, sizeof(void*)))
			{
				return 0;
			}
		}

		if (RESULT_PROPERTY_POSITION == index)
		{
			break;
		}
	}

	argument_value = buffer_buffer_data(arguments, TIME_OUT_POSITION);

	if (NULL == argument_value)
	{
		return 0;
	}

	if (buffer_size(argument_value))
	{
		if (!buffer_push_back(argument_value, '\0'))
		{
			return 0;
		}

		int64_t data = int64_parse(buffer_char_data(argument_value, 0));

		if (!buffer_resize(argument_value, 0))
		{
			return 0;
		}

		if (1000 < data)
		{
			data = millisecond_to_second(data);

			if (data < 5)
			{
				data = 5;
			}

			if (!int64_to_string((uint32_t)data, argument_value))
			{
				return 0;
			}
		}
	}

	return 1;
}

uint8_t exec_evaluate_task(void* project, const void* target,
						   const char* attributes_start, const char* attributes_finish,
						   const char* element_finish)
{
	struct buffer arguments;
	SET_NULL_TO_BUFFER(arguments);

	if (!buffer_resize(&arguments, 2 * UINT8_MAX) || !buffer_resize(&arguments, 0))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	if (!exec_get_arguments_for_task(project, target,
									 attributes_start, attributes_finish, element_finish, &arguments))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	uint8_t append;
	struct range program;
	struct range base_dir;
	struct range command_line;
	struct range output_file;
	void* pid_property;
	void* result_property;
	struct range working_dir;
	struct range environment_variables;
	uint8_t spawn;
	uint8_t fail_on_error;
	int64_t time_out;
	uint8_t verbose;

	if (!common_unbox_char_data(&arguments, PROGRAM_POSITION, 0, &program, 0))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	if (!common_unbox_bool_data(&arguments, APPEND_POSITION, 0, &append))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	if (!common_unbox_char_data(&arguments, BASE_DIR_POSITION, 0, &base_dir, 0))
	{
		base_dir.start = base_dir.finish = NULL;
	}

	if (!common_unbox_char_data(&arguments, COMMAND_LINE_POSITION, 0, &command_line, 0))
	{
		command_line.start = command_line.finish = NULL;
	}

	if (!common_unbox_char_data(&arguments, OUTPUT_POSITION, 0, &output_file, 0))
	{
		output_file.start = output_file.finish = NULL;
	}

	if (!common_unbox_char_data(&arguments, PID_PROPERTY_POSITION, 0, &working_dir, 0))
	{
		pid_property = NULL;
	}
	else
	{
		pid_property = (void*)working_dir.start;
	}

	if (!common_unbox_char_data(&arguments, RESULT_PROPERTY_POSITION, 0, &working_dir, 0))
	{
		result_property = NULL;
	}
	else
	{
		result_property = (void*)working_dir.start;
	}

	if (!common_unbox_bool_data(&arguments, SPAWN_POSITION, 0, &spawn))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	if (!common_unbox_char_data(&arguments, WORKING_DIR_POSITION, 0, &working_dir, 0))
	{
		working_dir.start = working_dir.finish = NULL;
	}

	if (!common_unbox_bool_data(&arguments, FAIL_ON_ERROR_POSITION, 0, &fail_on_error))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	time_out = common_unbox_int64_data(&arguments, TIME_OUT_POSITION, 0);

	if (!common_unbox_bool_data(&arguments, VERBOSE_POSITION, 0, &verbose))
	{
		buffer_release_with_inner_buffers(&arguments);
		return 0;
	}

	if (!common_unbox_char_data(&arguments, ENVIRONMENT_POSITION, 0, &environment_variables, 0))
	{
		environment_variables.start = environment_variables.finish = NULL;
	}

	spawn = exec(append, &program, &base_dir, &command_line, &output_file,
				 pid_property, result_property, &working_dir, &environment_variables,
				 spawn, (uint32_t)time_out, verbose);
	/**/
	buffer_release_with_inner_buffers(&arguments);
	/*TODO: comment about fail_on_error, if verbose mode,
	and manipulate of return value.
	return fail_on_error ? spawn : 1;*/
	return spawn;
}
