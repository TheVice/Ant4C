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
	"\t-encoding: - set encoding of input file.\n"																				\
	"\t-D: - define property. For example -D:\"property name\"=\"property value\".\n"											\
	"\t-projecthelp - show description of project and target(s).\n"																\
	"\t-nologo - do not display program version, license and copyright information.\n"											\
	"\t-listener: - set path to the module with listener.\n"																	\
	"\t-modulepriority - first try to evaluate tasks and functions from modules than from core of the library.\n"				\
	"\t-debug - display message with Debug level.\n"																			\
	"\t-logfile: - set path to the file for logging. Short form -l:.\n"															\
	"\t-verbose - display message with Verbose level. Set verbose parameter of functions to the true.\n"						\
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
	/**/
	struct buffer arguments;
	SET_NULL_TO_BUFFER(arguments);
#if defined(_MSC_VER)
	const uint8_t verbose = argument_parser_get_verbose_wchar_t(1, argc, argv);

	if (!argument_parser_wchar_t(1, argc, argv, &arguments, verbose))
#else
	const uint8_t verbose = argument_parser_get_verbose_char(1, argc, argv);

	if (!argument_parser_char(1, argc, argv, &arguments, verbose))
#endif
	{
		property_release(&arguments);
		argc = echo(0, Default, NULL, Info, LOGO, LOGO_LENGTH, 1, 0);

		if (argc)
		{
			echo(0, Default, NULL, Error, (const uint8_t*)"Failed to parse command arguments.", 34, 1, 0);
		}

		return EXIT_FAILURE;
	}

	struct buffer argument_value;

	SET_NULL_TO_BUFFER(argument_value);

	if (argument_parser_get_quiet(&arguments, &argument_value))
	{
		echo_set_level(Debug, argument_parser_get_debug(&arguments, &argument_value));
		echo_set_level(Error, 1);
		echo_set_level(Info, 0);
		echo_set_level(Verbose, verbose);
		echo_set_level(Warning, 1);
	}
	else
	{
		echo_set_level(Debug, argument_parser_get_debug(&arguments, &argument_value));
		echo_set_level(Verbose, verbose);
	}

	void* file_stream = NULL;
	const struct range* ptr_to_range = argument_parser_get_log_file(&arguments, &argument_value);

	if (ptr_to_range)
	{
		if (!file_open(ptr_to_range->start, (const uint8_t*)"ab", &file_stream))
		{
			buffer_release(&argument_value);
			property_release(&arguments);
			/**/
			argc = echo(0, Default, NULL, Error, (const uint8_t*)"Failed to open log file '", 25, 0, 0);

			if (argc)
			{
				argc = echo(0, Default, NULL, Error, ptr_to_range->start, range_size(ptr_to_range), 0, 0);
			}

			if (argc)
			{
				echo(0, Default, NULL, Error, (const uint8_t*)"'.", 2, 1, 0);
			}

			file_flush(file_stream);
			file_close(file_stream);
			/**/
			return EXIT_FAILURE;
		}

		common_set_output_stream(file_stream);
		common_set_error_output_stream(file_stream);
	}

	common_set_module_priority(argument_parser_get_module_priority(&arguments, &argument_value));

	if (!argument_parser_get_no_logo(&arguments, &argument_value))
	{
		if (!echo(0, Default, NULL, Info, LOGO, LOGO_LENGTH, 1, 0))
		{
			buffer_release(&argument_value);
			property_release(&arguments);
			/**/
			file_flush(file_stream);
			file_close(file_stream);
			/**/
			return EXIT_FAILURE;
		}
	}

	struct buffer current_directory;

	SET_NULL_TO_BUFFER(current_directory);

	if (!argument_parser_get_program_help(&arguments, &argument_value) &&
		!path_get_directory_for_current_process(&current_directory))
	{
		buffer_release(&current_directory);
		buffer_release(&argument_value);
		property_release(&arguments);
		/**/
		echo(0, Default, NULL, Error, (const uint8_t*)"Failed to get current directory.", 32, 1, 0);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		return EXIT_FAILURE;
	}

	if (!argument_parser_get_build_file(&arguments, &argument_value, 0))
	{
		if (!argument_parser_get_program_help(&arguments, &argument_value) &&
			!project_get_build_files_from_directory(&arguments, &argument_value, &current_directory, verbose))
		{
			buffer_release(&current_directory);
			buffer_release(&argument_value);
			property_release(&arguments);
			/**/
			argc = echo(0, Default, NULL, Warning, (const uint8_t*)"No file(s) specified at the input.", 34, 1, 0);
			/**/
			file_flush(file_stream);
			file_close(file_stream);
			/**/
			return argc ? EXIT_SUCCESS : EXIT_FAILURE;
		}
	}

	if (argument_parser_get_program_help(&arguments, &argument_value) ||
		NULL == argument_parser_get_build_file(&arguments, &argument_value, 0))
	{
		if (!echo(0, Default, NULL, Info, SAMPLE_USING, SAMPLE_USING_LENGTH, 1, 0) ||
			!echo(0, Default, NULL, Info, OPTIONS, OPTIONS_LENGTH, 1, 0))
		{
			argc = 0;
		}

		buffer_release(&current_directory);
		buffer_release(&argument_value);
		property_release(&arguments);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		return 0 < argc ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	void* listener_object = NULL;
	ptr_to_range = argument_parser_get_listener(&arguments, &argument_value);

	if (ptr_to_range && !load_listener(ptr_to_range->start, &listener_object))
	{
		if (!echo(0, Default, NULL, Warning, (const uint8_t*)"Listener '", 10, 0, 0) ||
			!echo(0, Default, NULL, Warning, ptr_to_range->start, range_size(ptr_to_range), 0, 0) ||
			!echo(0, Default, NULL, Warning, (const uint8_t*)"' not loaded.", 13, 1, 0))
		{
			shared_object_unload(listener_object);
			buffer_release(&current_directory);
			buffer_release(&argument_value);
			property_release(&arguments);
			/**/
			file_flush(file_stream);
			file_close(file_stream);
			/**/
			return EXIT_FAILURE;
		}
	}

	struct buffer the_project;

	SET_NULL_TO_BUFFER(the_project);

	if (!project_new(&the_project))
	{
		project_unload(&the_project);
		shared_object_unload(listener_object);
		buffer_release(&current_directory);
		buffer_release(&argument_value);
		property_release(&arguments);
		/**/
		echo(0, Default, NULL, Error, (const uint8_t*)"Failed to create the project.", 29, 1, 0);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		return EXIT_FAILURE;
	}

	struct buffer properties;

	SET_NULL_TO_BUFFER(properties);

	if (!argument_parser_get_properties(&arguments, &properties, verbose))
	{
		property_release(&properties);
	}

	struct range current_directory_in_range;

	BUFFER_TO_RANGE(current_directory_in_range, &current_directory);

	const uint8_t project_help = argument_parser_get_project_help(&arguments, &argument_value);

	const uint16_t encoding = argument_parser_get_encoding(&arguments, &argument_value);

	for (argc = 0;
		 NULL != (ptr_to_range = argument_parser_get_build_file(&arguments, &argument_value, argc++));)
	{
		if (!project_help && !property_add_at_project(&the_project, &properties, verbose))
		{
			argc = 0;
			echo(0, Default, NULL, Error, (const uint8_t*)"Failed to set properties at the project.", 40, 1, 0);
			break;
		}

		if (!project_load_and_evaluate_target(
				&the_project, ptr_to_range, &current_directory_in_range,
				&arguments, project_help, encoding, verbose))
		{
			argc = 0;
			echo(0, Default, NULL, Error, (const uint8_t*)"Evaluation of the project was fail.", 35, 1, 0);
			break;
		}

		project_clear(&the_project);
	}

	property_release(&properties);
	project_unload(&the_project);
	shared_object_unload(listener_object);
	buffer_release(&current_directory);
	property_release(&arguments);
	/**/
	time_now = datetime_now() - time_now;

	if (!print_elapsed_time(time_now, &argument_value))
	{
		buffer_release(&argument_value);
		echo(0, Default, NULL, Error, (const uint8_t*)"Failed to print elapsed time.", 29, 1, 0);
		/**/
		file_flush(file_stream);
		file_close(file_stream);
		/**/
		return EXIT_FAILURE;
	}

	buffer_release(&argument_value);
	argc = 0 < argc;
	argc = print_status(argc) ? argc : 0;
	/**/
	file_flush(file_stream);
	file_close(file_stream);
	/**/
	return argc ? EXIT_SUCCESS : EXIT_FAILURE;
#else
	(void)argc;
	(void)argv;
	/**/
	return EXIT_SUCCESS;
#endif
}
