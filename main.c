/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "date_time.h"
#include "echo.h"
#include "environment.h"
#include "exec.h"
#include "file_system.h"
#include "hash.h"
#include "interpreter.h"
#include "listener.h"
#include "load_file.h"
#include "load_tasks.h"
#include "math_unit.h"
#include "operating_system.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "shared_object.h"
#include "sleep_unit.h"
#include "string_unit.h"
#include "target.h"
#include "text_encoding.h"
#include "version.h"
#include "xml.h"

#include <math.h>
#include <time.h>
#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <stdbool.h>
#include <inttypes.h>

#ifdef _WIN32
#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#define LOGO (const uint8_t*)"Program version "PROGRAM_VERSION"\n"																\
	"The MIT License (MIT)\n"																									\
	"Copyright (c) 2019 - 2020 https://github.com/TheVice/"
#define LOGO_LENGTH common_count_bytes_until(LOGO, 0)
#define SAMPLE_USING (const uint8_t*)"Sample using - [options] <target> ..."
#define SAMPLE_USING_LENGTH 37
#define OPTIONS (const uint8_t*)"Options:\n"																					\
	"\t-buildfile: - set path to project file. Short form /f:.\n"																\
	"\t-encoding: - set encoding of input file. Can be ASCII, UTF8, Unicode, UTF16LE, UTF32 or UTF32LE in any letter case.\n"	\
	"\t-D: - define property. For example -D:\"property name\"=\"property value\".\n"											\
	"\t-projecthelp - show description of project and target(s).\n"																\
	"\t-nologo - do not display program version, license and copyright information.\n"											\
	"\t-listener: - set path to the module with listener.\n"																	\
	"\t-debug - display message with Debug level.\n"																			\
	"\t-verbose - display message with Verbose level. Set verbose parameter of functions to the true.\n"						\
	"\t-quiet - display messages only with Warning or/and Errore levels. Short form -q.\n"										\
	"\t-help - print this message. Short form -h."
#define OPTIONS_LENGTH common_count_bytes_until(OPTIONS, 0)

uint8_t print_status(int status)
{
	return 8 == echo(0, Default, NULL,
					 status ? Info : Error,
					 status ? (const uint8_t*)"SUCCESS." : (const uint8_t*)"FAILURE.",
					 8, 1, 0);
}

uint8_t project_evaluate(void* the_project, const struct range* build_file,
						 const struct range* current_directory_in_range)
{
	if (NULL == build_file)
	{
		/*TODO: echo.*/
		return 0;
	}

	if (!argument_parser_get_project_help())
	{
		if (!property_add_at_project(the_project, argument_parser_get_properties(), NULL,
									 argument_parser_get_verbose()))
		{
			/*TODO: echo.*/
			return 0;
		}
	}

	listener_project_started(build_file->start, the_project);
	/**/
	uint8_t is_loaded = project_load_from_build_file(
							build_file, current_directory_in_range,
							argument_parser_get_encoding(),
							the_project, argument_parser_get_project_help(),
							argument_parser_get_verbose());

	if (!is_loaded)
	{
		/*TODO: echo.*/
		listener_project_finished(build_file->start, the_project);
		return 0;
	}

	if (!argument_parser_get_project_help())
	{
		if (argument_parser_get_target(0))
		{
			int index = 0;
			const struct range* target_name = NULL;

			while (NULL != (target_name = argument_parser_get_target(index++)))
			{
				is_loaded = target_evaluate_by_name(the_project, target_name, argument_parser_get_verbose());

				if (!is_loaded)
				{
					/*TODO: echo.*/
					break;
				}
			}

			if (!is_loaded)
			{
				/*TODO: echo.*/
				listener_project_finished(build_file->start, the_project);
				return 0;
			}
		}
		else
		{
			is_loaded = project_evaluate_default_target(the_project, argument_parser_get_verbose());

			if (!is_loaded)
			{
				/*TODO: echo.*/
				listener_project_finished(build_file->start, the_project);
				return 0;
			}
		}
	}

	listener_project_finished(build_file->start, the_project);
	project_clear(the_project);
	/**/
	return 1;
}

typedef void (*on_project)(const uint8_t* source, const void* the_project);

typedef void (*on_target)(const uint8_t* source, ptrdiff_t offset, const void* the_project,
						  const void* the_target);

typedef void (*on_task_start)(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							  const void* the_target, uint8_t task);
typedef void (*on_task_finish)(const uint8_t* source, ptrdiff_t offset, const void* the_project,
							   const void* the_target, uint8_t task, uint8_t result);

uint8_t load_listener(const struct range* listener, void** object)
{
	if (!listener ||
		!object)
	{
		return 0;
	}

	if (NULL != (*object))
	{
		shared_object_unload(*object);
	}

	(*object) = shared_object_load(listener->start);

	if (NULL == (*object))
	{
		return 0;
	}

	static const uint8_t* procedures_names[] =
	{
		(const uint8_t*)"listener_project_started",
		(const uint8_t*)"listener_project_finished",
		(const uint8_t*)"listener_target_started",
		(const uint8_t*)"listener_target_finished",
		(const uint8_t*)"listener_task_started",
		(const uint8_t*)"listener_task_finished"
	};

	for (uint8_t i = 0, count = COUNT_OF(procedures_names); i < count; ++i)
	{
		void* address = shared_object_get_procedure_address(*object, procedures_names[i]);

		if (!address)
		{
			continue;
		}

		switch (i)
		{
			case 0:
				listener_set_on_project_started((on_project)address);
				break;

			case 1:
				listener_set_on_project_finished((on_project)address);
				break;

			case 2:
				listener_set_on_target_started((on_target)address);
				break;

			case 3:
				listener_set_on_target_finished((on_target)address);
				break;

			case 4:
				listener_set_on_task_started((on_task_start)address);
				break;

			case 5:
				listener_set_on_task_finished((on_task_finish)address);
				break;

			default:
				break;
		}
	}

	return 1;
}

typedef const uint8_t* (*enumerate_tasks)(ptrdiff_t index);
typedef const uint8_t* (*enumerate_namespaces)(ptrdiff_t index);
typedef const uint8_t* (*enumerate_functions)(const uint8_t* name_space, ptrdiff_t index);
typedef uint8_t (*get_attributes_and_arguments_for_task)(
	const uint8_t* task, const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count);
typedef uint8_t (*evaluate_task)(const uint8_t* task,
								 const uint8_t** arguments, const uint16_t* arguments_lengths, uint8_t arguments_count,
								 uint8_t verbose);
typedef uint8_t (*evaluate_function)(const uint8_t* function,
									 const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
									 const uint8_t** output, uint16_t* output_length);

#if defined(_MSC_VER)
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
#ifdef _WIN32
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif
	void* object = shared_object_load((const uint8_t*)""/*"libsample_module.so" "sample_module.dll"*/);
	enumerate_tasks et = (enumerate_tasks)shared_object_get_procedure_address(object,
						 (const uint8_t*)"enumerate_tasks");
	evaluate_task ev_t = (evaluate_task)shared_object_get_procedure_address(object,
						 (const uint8_t*)"evaluate_task");
	ptrdiff_t i = 0;
	const void* ptr = NULL;

	if (NULL != et)
	{
		while (NULL != (ptr = et(i++)))
		{
			printf("%s\n", (const char*)ptr);
			get_attributes_and_arguments_for_task aa = (get_attributes_and_arguments_for_task)
					shared_object_get_procedure_address(object, (const uint8_t*)"get_attributes_and_arguments_for_task");

			if (NULL != aa)
			{
				const uint8_t** ta = NULL;
				const uint8_t* tal = NULL;
				uint8_t tac = 0;

				if (aa(ptr, &ta, &tal, &tac))
				{
					for (uint8_t j = 0; j < tac; ++j)
					{
						printf("\t%s %i %i\n", (const char*)ta[j], tal[j], j);
					}
				}

				static const uint8_t* arguments[] = { (const uint8_t*)"Abc", (const uint8_t*)"qwer", (const uint8_t*)"wt", (const uint8_t*)"h" };
				static uint16_t arguments_lengths[] = { 3, 3, 2, 1 };
				ev_t(ptr, arguments, arguments_lengths, tac, 0);
			}
		}
	}

	enumerate_namespaces en = (enumerate_namespaces)shared_object_get_procedure_address(object,
							  (const uint8_t*)"enumerate_namespaces");
	enumerate_functions ef = (enumerate_functions)shared_object_get_procedure_address(object,
							 (const uint8_t*)"enumerate_functions");
	evaluate_function ev_f = (evaluate_function)shared_object_get_procedure_address(object,
							 (const uint8_t*)"evaluate_function");

	if (NULL != en && NULL != ef && NULL != ev_f)
	{
		i = 0;

		while (NULL != (ptr = en(i++)))
		{
			printf("%s\n", (const char*)ptr);
			ptrdiff_t j = 0;
			const void* func = NULL;

			while (NULL != (func = ef(ptr, j++)))
			{
				printf("%s\n", (const char*)func);
				static const uint8_t* values[] = { (const uint8_t*)"first_value", (const uint8_t*)"second_value", (const uint8_t*)"third_value" };
				static uint16_t values_lengths[] = { 12, 13, 12 };
				const uint8_t* out = NULL;
				uint16_t out_l = 0;

				if (ev_f(func, values, values_lengths, 3, &out, &out_l))
				{
					fwrite(out, sizeof(uint8_t), out_l, stdout);
				}
			}
		}
	}

	shared_object_unload(object);
#if 0
	uint64_t time_now = datetime_now();
#if defined(_MSC_VER)

	if (!argument_parser_wchar_t(1, argc, argv))
#else
	if (!argument_parser_char(1, argc, argv))
#endif
	{
		argument_parser_release();

		if (!echo(0, Default, NULL, Info, LOGO, LOGO_LENGTH, 1, 0))
		{
			argc = 0;
		}

		if (!echo(0, Default, NULL, Error, (const uint8_t*)"Failed to parse command arguments.", 34, 1, 0))
		{
			argc = 0;
		}

		return EXIT_FAILURE;
	}

	if (argument_parser_get_quiet())
	{
		echo_set_level(Debug, argument_parser_get_debug());
		echo_set_level(Error, 1);
		echo_set_level(Info, 0);
		echo_set_level(Verbose, argument_parser_get_verbose());
		echo_set_level(Warning, 1);
	}
	else
	{
		echo_set_level(Debug, argument_parser_get_debug());
		echo_set_level(Verbose, argument_parser_get_verbose());
	}

	if (!argument_parser_get_no_logo())
	{
		if (!echo(0, Default, NULL, Info, LOGO, LOGO_LENGTH, 1, argument_parser_get_verbose()))
		{
			argument_parser_release();
			return EXIT_FAILURE;
		}
	}

	struct buffer current_directory;

	SET_NULL_TO_BUFFER(current_directory);

	if (!path_get_directory_for_current_process(&current_directory))
	{
		buffer_release(&current_directory);
		argument_parser_release();
		/*TODO: echo.*/
		return EXIT_FAILURE;
	}

	struct range current_directory_in_range;

	BUFFER_TO_RANGE(current_directory_in_range, &current_directory);

	struct buffer* build_files = argument_parser_get_build_files();

#if defined(_WIN32)
	if (!buffer_size(build_files))
	{
		static const uint8_t* file_extension = (const uint8_t*)"*.build\0";

		if (!path_combine_in_place(&current_directory, 0, file_extension, file_extension + 8))
		{
			buffer_release(&current_directory);
			argument_parser_release();
			/*TODO: echo.*/
			return EXIT_FAILURE;
		}

		if (!buffer_append(build_files, NULL, INT8_MAX * sizeof(struct range)))
		{
			buffer_release(&current_directory);
			argument_parser_release();
			/*TODO: echo.*/
			return EXIT_FAILURE;
		}

		if (!directory_enumerate_file_system_entries(&current_directory, 1, 0, build_files, 1))
		{
			if (!buffer_resize(build_files, 0))
			{
				buffer_release(&current_directory);
				argument_parser_release();
				/*TODO: echo.*/
				return EXIT_FAILURE;
			}
		}
		else
		{
			if (!argument_parser_fill_ranges_at_storage(build_files, INT8_MAX * sizeof(struct range)))
			{
				if (!echo(0, Default, NULL, Error, (const uint8_t*)"Failed to create ranges for the build files paths.", 50,
						  1, 0))
				{
					argc = 0;
				}

				buffer_release(&current_directory);
				argument_parser_release();
				return EXIT_FAILURE;
			}
		}
	}

#endif

	if (argument_parser_get_help() || NULL == argument_parser_get_build_file(0))
	{
		if (!echo(0, Default, NULL, Info, SAMPLE_USING, SAMPLE_USING_LENGTH, 1, argument_parser_get_verbose()) ||
			!echo(0, Default, NULL, Info, OPTIONS, OPTIONS_LENGTH, 1, argument_parser_get_verbose()))
		{
			argc = 0;
		}

		buffer_release(&current_directory);
		argument_parser_release();
		argc = 0 < argc;
		return argc ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	void* object = NULL;
	const struct range* listener = argument_parser_get_listener();

	if (listener && !load_listener(listener, &object))
	{
		if (!echo(0, Default, NULL, Warning, listener->start, range_size(listener), 1,
				  argument_parser_get_verbose()) ||
			!echo(0, Default, NULL, Warning, (const uint8_t*)"Listener not loaded.", 20, 1,
				  argument_parser_get_verbose()))
		{
			shared_object_unload(object);
			return EXIT_FAILURE;
		}
	}

	void* the_project = NULL;

	if (!project_new(&the_project))
	{
		argc = 0;
	}

	for (argc = 0; ; ++argc)
	{
		const struct range* build_file = argument_parser_get_build_file(argc);

		if (!build_file)
		{
			break;
		}

		if (!project_evaluate(the_project, build_file, &current_directory_in_range))
		{
			argc = 0;
			break;
		}
	}

	project_unload(the_project);
	buffer_release(&current_directory);
	time_now = datetime_now() - time_now;
	shared_object_unload(object);

	if (10 < time_now)
	{
		if (!buffer_resize(build_files, 0))
		{
			argument_parser_release();
			/*TODO: echo.*/
			return EXIT_FAILURE;
		}

		if (!buffer_append_char(build_files, "Total time: ", 12))
		{
			argument_parser_release();
			/*TODO: echo.*/
			return EXIT_FAILURE;
		}

		if (!int64_to_string(time_now, build_files))
		{
			argument_parser_release();
			/*TODO: echo.*/
			return EXIT_FAILURE;
		}

		if (!buffer_append_char(build_files, " second(s).", 11))
		{
			argument_parser_release();
			/*TODO: echo.*/
			return EXIT_FAILURE;
		}

		if (!echo(0, Default, NULL, Info, buffer_data(build_files, 0), buffer_size(build_files), 1, 0))
		{
			argument_parser_release();
			return EXIT_FAILURE;
		}
	}

	argument_parser_release();
	argc = 0 < argc;
	argc = print_status(argc) ? argc : 0;
	return argc ? EXIT_SUCCESS : EXIT_FAILURE;
#else
	(void)argc;
	(void)argv;
	return EXIT_SUCCESS;
#endif
}
