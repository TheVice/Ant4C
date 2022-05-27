/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "stdc_secure_api.h"

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

#define LOGO (const uint8_t*)"Program version "PROGRAM_VERSION"\n"																\
	"The MIT License (MIT)\n"																									\
	"Copyright (c) 2019 - 2022 TheVice"
#define LOGO_LENGTH common_count_bytes_until(LOGO, 0)
#define SAMPLE_USING (const uint8_t*)"Sample using - [options] <target> ..."
#define SAMPLE_USING_LENGTH 37
#define OPTIONS (const uint8_t*)"Options:\n"																					\
	"\t-buildfile: - set path to project file. Short form /f:.\n"																\
	"\t-encoding: - set encoding of input file.\n"																				\
	"\t-D: - define property. For example -D:\"property name\"=\"property value\".\n"											\
	"\t-projecthelp - show description of project and target(s).\n"																\
	"\t-nologo - do not display program version, license, copyright information and STATUS of completed script.\n"				\
	"\t-listener: - set path to the module with listener.\n"																	\
	"\t-modulepriority - first try to evaluate tasks and functions from modules than from core of the library.\n"				\
	"\t-debug - display message with Debug level.\n"																			\
	"\t-logfile: - set path to the file for logging. Short form -l:.\n"															\
	"\t-verbose - display message with Verbose level. Set verbose parameter of functions to the true. Short form -v.\n"			\
	"\t-quiet - display messages only with Warning or/and Error levels. Short form -q.\n"										\
	"\t-help - print this message. Short form -h."
#define OPTIONS_LENGTH common_count_bytes_until(OPTIONS, 0)

uint8_t print_status(int status)
{
	return 8 == echo(0, Default, NULL,
					 status ? Info : Error,
					 status ? (const uint8_t*)"SUCCESS." : (const uint8_t*)"FAILURE.",
					 8, 1, 0);
}

uint8_t print_elapsed_time(int64_t delta, struct buffer* argument_value)
{
	if (10 < delta)
	{
		if (!buffer_resize(argument_value, 0))
		{
			return 0;
		}

		if (!buffer_append_char(argument_value, "Total time: ", 12))
		{
			return 0;
		}

		if (!int64_to_string(delta, argument_value))
		{
			return 0;
		}

		if (!buffer_append_char(argument_value, " second(s).", 11))
		{
			return 0;
		}

		return echo(0, Default, NULL, Info, buffer_data(argument_value, 0), buffer_size(argument_value), 1, 0);
	}

	return 1;
}

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
#if 1
	int64_t time_now = datetime_now();

	if (!argument_parser_init())
	{
		argument_parser_release();
		return EXIT_FAILURE;
	}

#if defined(_MSC_VER)

	if (!argument_parser_wchar_t(1, argc, argv))
#else
	if (!argument_parser_char(1, argc, argv))
#endif
	{
		argc = echo(0, Default, NULL, Info, LOGO, LOGO_LENGTH, 1, 0);

		if (argc)
		{
			echo(0, Default, NULL, Error, (const uint8_t*)"Failed to parse command arguments.", 34, 1, 0);
		}

		argument_parser_release();
		return EXIT_FAILURE;
	}

	if (!argument_parser_get_no_logo() &&
		!echo(0, Default, NULL, Info, LOGO, LOGO_LENGTH, 1, 0))
	{
		argument_parser_release();
		return EXIT_FAILURE;
	}

	if (argument_parser_get_program_help() ||
		NULL == argument_parser_get_build_file(0))
	{
		if (!echo(0, Default, NULL, Info, SAMPLE_USING, SAMPLE_USING_LENGTH, 1, 0) ||
			!echo(0, Default, NULL, Info, OPTIONS, OPTIONS_LENGTH, 1, 0))
		{
			argc = 0;
		}

		argument_parser_release();
		return 0 < argc ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	const uint8_t verbose = argument_parser_get_verbose();

	if (argument_parser_get_quiet())
	{
		echo_set_level(Debug, argument_parser_get_debug());
		echo_set_level(Error, 1);
		echo_set_level(Info, 0);
		echo_set_level(Verbose, verbose);
		echo_set_level(Warning, 1);
	}
	else
	{
		echo_set_level(Debug, argument_parser_get_debug());
		echo_set_level(Verbose, verbose);
	}

	common_set_module_priority(argument_parser_get_module_priority());
	struct buffer current_directory;
	SET_NULL_TO_BUFFER(current_directory);

	if (!path_get_directory_for_current_process(&current_directory))
	{
		buffer_release(&current_directory);
		echo(0, Default, NULL, Error, (const uint8_t*)"Failed to get current directory.", 32, 1, 0);
		argument_parser_release();
		return EXIT_FAILURE;
	}

	struct buffer the_project;

	SET_NULL_TO_BUFFER(the_project);

	struct range current_directory_in_a_range;

	BUFFER_TO_RANGE(current_directory_in_a_range, &current_directory);

	void* file_stream = NULL;

	struct range path_in_a_range;

	const uint8_t* log_file = argument_parser_get_log_file();

	if (NULL != log_file)
	{
		path_in_a_range.start = log_file;
		path_in_a_range.finish = log_file + common_count_bytes_until(log_file, 0);
	}
	else
	{
		path_in_a_range.start = path_in_a_range.finish = NULL;
	}

	if (path_in_a_range.start < path_in_a_range.finish &&
		!program_set_log_file(&path_in_a_range, &current_directory_in_a_range, &the_project, &file_stream))
	{
		buffer_release(&the_project);
		buffer_release(&current_directory);
		/**/
		argc = echo(0, Default, NULL, Error, (const uint8_t*)"Failed to open log file '", 25, 0, 0);

		if (argc)
		{
			argc = echo(0, Default, NULL, Error, path_in_a_range.start, range_size(&path_in_a_range), 0, 0);

			if (argc)
			{
				echo(0, Default, NULL, Error, (const uint8_t*)"'.", 2, 1, 0);
			}
		}

		file_flush(file_stream);
		file_close(file_stream);
		/**/
		argument_parser_release();
		return EXIT_FAILURE;
	}

	if (!argument_parser_get_build_file(0) &&
		!project_get_build_files_from_directory(&current_directory, verbose))
	{
		buffer_release(&the_project);
		buffer_release(&current_directory);
		/**/
		argc = echo(0, Default, NULL, Warning, (const uint8_t*)"No file(s) specified at the input.", 34, 1, 0);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		argument_parser_release();
		return argc ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	if (!buffer_resize(&the_project, 0))
	{
		buffer_release(&the_project);
		buffer_release(&current_directory);
		/**/
		echo(0, Default, NULL, Error, (const uint8_t*)"Failed to resize buffer before check listener argument.", 55,
			 1, 0);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		argument_parser_release();
		return EXIT_FAILURE;
	}

	void* listener_object = NULL;
	const uint8_t* listener = argument_parser_get_listener();

	if (NULL != listener)
	{
		path_in_a_range.start = listener;
		path_in_a_range.finish = listener + common_count_bytes_until(listener, 0);
	}
	else
	{
		path_in_a_range.start = path_in_a_range.finish = NULL;
	}

	if (path_in_a_range.start < path_in_a_range.finish &&
		!program_set_listener(&path_in_a_range, &current_directory_in_a_range, &the_project, &listener_object))
	{
		if (!echo(0, Default, NULL, Warning, (const uint8_t*)"Listener '", 10, 0, 0) ||
			!echo(0, Default, NULL, Warning, path_in_a_range.start, range_size(&path_in_a_range), 0, 0) ||
			!echo(0, Default, NULL, Warning, (const uint8_t*)"' not loaded.", 13, 1, 0))
		{
			buffer_release(&the_project);
			shared_object_unload(listener_object);
			buffer_release(&current_directory);
			/**/
			file_flush(file_stream);
			file_close(file_stream);
			/**/
			argument_parser_release();
			return EXIT_FAILURE;
		}
	}

	if (!buffer_resize(&the_project, 0))
	{
		buffer_release(&the_project);
		shared_object_unload(listener_object);
		buffer_release(&current_directory);
		/**/
		echo(0, Default, NULL, Error, (const uint8_t*)"Failed to resize buffer before create project.", 46, 1, 0);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		argument_parser_release();
		return EXIT_FAILURE;
	}

	if (!project_new(&the_project))
	{
		project_unload(&the_project);
		shared_object_unload(listener_object);
		buffer_release(&current_directory);
		/**/
		echo(0, Default, NULL, Error, (const uint8_t*)"Failed to create the project.", 29, 1, 0);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		argument_parser_release();
		return EXIT_FAILURE;
	}

	const struct buffer* properties = argument_parser_get_properties();
	const uint8_t project_help = argument_parser_get_project_help();
	const uint16_t encoding = argument_parser_get_encoding();
	const uint8_t* build_file;
	argc = 0;

	while (NULL != (build_file = argument_parser_get_build_file(argc++)))
	{
		if (!project_help && !property_add_at_project(&the_project, properties, verbose))
		{
			argc = 0;
			echo(0, Default, NULL, Error, (const uint8_t*)"Failed to set properties at the project.", 40, 1, 0);
			break;
		}

		path_in_a_range.start = build_file;
		path_in_a_range.finish = build_file + common_count_bytes_until(build_file, 0);

		if (!project_load_and_evaluate_target(
				&the_project, &path_in_a_range, &current_directory_in_a_range,
				project_help, encoding, verbose))
		{
			argc = 0;
			echo(0, Default, NULL, Error, (const uint8_t*)"Evaluation of the project was fail.", 35, 1, 0);
			break;
		}

		project_clear(&the_project);
	}

	project_unload(&the_project);
	shared_object_unload(listener_object);
	/**/
	time_now = datetime_now() - time_now;

	if (!print_elapsed_time(time_now, &current_directory))
	{
		buffer_release(&current_directory);
		echo(0, Default, NULL, Error, (const uint8_t*)"Failed to print elapsed time.", 29, 1, 0);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		argument_parser_release();
		return EXIT_FAILURE;
	}

	buffer_release(&current_directory);
	argc = 0 < argc;

	if (!argument_parser_get_no_logo())
	{
		argc = print_status(argc) ? argc : 0;
	}

	file_flush(file_stream);
	file_close(file_stream);
	/**/
	argument_parser_release();
	return argc ? EXIT_SUCCESS : EXIT_FAILURE;
#else
	(void)argc;
	(void)argv;
	/**/
	return EXIT_SUCCESS;
#endif
}
