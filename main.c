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
#include "load_file.h"
#include "math_unit.h"
#include "operating_system.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
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

#define LOGO (const uint8_t*)"Program version "PROGRAM_VERSION"\n"	\
	"The MIT License (MIT)\n"						\
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
	"\t-help - print this message. Short form -h."
#define OPTIONS_LENGTH common_count_bytes_until(OPTIONS, 0)

uint8_t print_status(int status)
{
	return 8 == echo(0, Default, NULL,
					 status ? Info : Error,
					 status ? (const uint8_t*)"SUCCESS." : (const uint8_t*)"FAILURE.",
					 8, 1, 0);
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
#if 0
	uint64_t time_now = datetime_now();
#if defined(_MSC_VER)

	if (!argument_parser_wchar_t(1, argc, argv))
#else
	if (!argument_parser_char(1, argc, argv))
#endif
	{
		argument_parser_release();

		if (!echo(0, Default, NULL, NoLevel, LOGO, LOGO_LENGTH, 1, 0))
		{
			argc = 0;
		}

		if (!echo(0, Default, NULL, Error, (const uint8_t*)"Failed to parse command arguments.", 34, 1, 0))
		{
			argc = 0;
		}

		return EXIT_FAILURE;
	}

	if (!argument_parser_get_no_logo())
	{
		if (!echo(0, Default, NULL, NoLevel, LOGO, LOGO_LENGTH, 1, argument_parser_get_verbose()))
		{
			argument_parser_release();
			/*TODO: echo.*/
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

		if (!directory_enumerate_file_system_entries(&current_directory, 1, 0, build_files))
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

	for (argc = 0; ; ++argc)
	{
		const struct range* build_file = argument_parser_get_build_file(argc);

		if (NULL == build_file)
		{
			break;
		}

		void* project = NULL;

		if (!project_new(&project))
		{
			argc = 0;
			break;
		}

		if (!property_add_at_project(project, argument_parser_get_properties(), argument_parser_get_verbose()))
		{
			project_unload(project);
			argc = 0;
			break;
		}

		const uint8_t is_loaded = project_load_from_build_file(
									  build_file, &current_directory_in_range,
									  argument_parser_get_encoding(),
									  project, argument_parser_get_project_help(),
									  argument_parser_get_verbose());

		if (!is_loaded)
		{
			project_unload(project);
			argc = 0;
			break;
		}

		project_unload(project);
	}

	buffer_release(&current_directory);
	time_now = datetime_now() - time_now;

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
			/*TODO: echo.*/
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
