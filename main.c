/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
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
#include "interpreter.h"
#include "math_unit.h"
#include "operating_system.h"
#include "path.h"
#include "project.h"
#include "property.h"
#include "range.h"
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

#ifndef _WIN32
#include <unistd.h>

static pid_t pid = 0;
#endif

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#define LOGO "Program version "PROGRAM_VERSION"\n"	\
	"The MIT License (MIT)\n"						\
	"Copyright(c) 2019 https://github.com/TheVice/"
#define LOGO_LENGTH strlen(LOGO)
#define SAMPLE_USING "Sample using - [options]          ..." /*<target>*/
#define SAMPLE_USING_LENGTH 37
#define OPTIONS "Options:\n"															\
	"\t-buildfile: - set path to project file. Short form /f:.\n"						\
	"\t-D: - define property. For example -D:\"property name\"=\"property value\".\n"	\
	"\t-nologo - do not display program version, license and copyright information.\n"	\
	"\t-help - print this message. Short form -h."
#define OPTIONS_LENGTH 260

uint8_t print_status(int status)
{
#ifndef _WIN32

	if (getpid() != pid)
	{
		return 0;
	}

#endif
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
#ifndef _WIN32
	pid = getpid();
#endif
	/*uint64_t time_now = datetime_now();*/

	/*if (argc < 2)
	{
		TODO: list current directory.
	}*/

	if (argc < 2)
	{
		if (!echo(0, Default, NULL, NoLevel, LOGO, LOGO_LENGTH, 1, 0) ||
			!echo(0, Default, NULL, Info, SAMPLE_USING, SAMPLE_USING_LENGTH, 1, 0) ||
			!echo(0, Default, NULL, Info, OPTIONS, OPTIONS_LENGTH, 1, 0))
		{
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

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

		if (!echo(0, Default, NULL, Error, "Failed to parse command arguments.", 34, 1, 0))
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
			return EXIT_FAILURE;
		}
	}

	if (argument_parser_get_help() || NULL == argument_parser_get_build_file(0))
	{
		if (!echo(0, Default, NULL, Info, SAMPLE_USING, SAMPLE_USING_LENGTH, 1, argument_parser_get_verbose()) ||
			!echo(0, Default, NULL, Info, OPTIONS, OPTIONS_LENGTH, 1, argument_parser_get_verbose()))
		{
			argc = 0;
		}

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

		if (!project_add_properties(project, NULL, argument_parser_get_properties(), argument_parser_get_verbose()))
		{
			project_unload(project);
			argc = 0;
			break;
		}

		const uint8_t is_loaded = project_load_from_build_file(build_file->start, project,
								  argument_parser_get_verbose());

		if (!is_loaded)
		{
			project_unload(project);
			argc = 0;
			break;
		}

		project_unload(project);
	}

	argument_parser_release();
	/*time_now = datetime_now() - time_now;
	printf("Total time: %"PRId64" second(s).\n", time_now);*/
	argc = 0 < argc;
	print_status(argc);
	return argc ? EXIT_SUCCESS : EXIT_FAILURE;
#endif
	(void)argc;
	(void)argv;
	return EXIT_SUCCESS;
}
