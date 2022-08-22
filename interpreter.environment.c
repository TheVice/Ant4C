/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#include "environment.h"

#include "common.h"
#include "conversion.h"
#include "operating_system.h"
#include "range.h"

enum environment_function
{
	get_folder_path, get_machine_name, get_operating_system,
	get_user_name, get_variable, newline, variable_exists,
	is64bit_process, is64bit_operating_system,
	processor_count,
	UNKNOWN_ENVIRONMENT
};

uint8_t environment_get_function(const uint8_t* name_start, const uint8_t* name_finish)
{
	static const uint8_t* environment_str[] =
	{
		(const uint8_t*)"get-folder-path",
		(const uint8_t*)"get-machine-name",
		(const uint8_t*)"get-operating-system",
		(const uint8_t*)"get-user-name",
		(const uint8_t*)"get-variable",
		(const uint8_t*)"newline",
		(const uint8_t*)"variable-exists",
		(const uint8_t*)"is64bit-process",
		(const uint8_t*)"is64bit-operating-system",
		(const uint8_t*)"processor-count"
	};
	/**/
	return common_string_to_enum(name_start, name_finish, environment_str, UNKNOWN_ENVIRONMENT);
}

uint8_t environment_exec_function(uint8_t function, const void* arguments, uint8_t arguments_count,
								  void* output)
{
	if (UNKNOWN_ENVIRONMENT <= function || NULL == arguments || 1 < arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument;

	if (!arguments_count)
	{
		argument.start = argument.finish = NULL;
	}
	else if (!common_get_arguments(arguments, arguments_count, &argument, 0))
	{
		return 0;
	}

	switch (function)
	{
		case get_folder_path:
		{
			static const uint8_t* special_folder_str[] =
			{
				(const uint8_t*)"Desktop",
				(const uint8_t*)"Programs",
				(const uint8_t*)"Personal",
				(const uint8_t*)"MyDocuments",
				(const uint8_t*)"Favorites",
				(const uint8_t*)"Startup",
				(const uint8_t*)"Recent",
				(const uint8_t*)"SendTo",
				(const uint8_t*)"StartMenu",
				(const uint8_t*)"MyMusic",
				(const uint8_t*)"MyVideos",
				(const uint8_t*)"DesktopDirectory",
				(const uint8_t*)"MyComputer",
				(const uint8_t*)"NetworkShortcuts",
				(const uint8_t*)"Fonts",
				(const uint8_t*)"Templates",
				(const uint8_t*)"CommonStartMenu",
				(const uint8_t*)"CommonPrograms",
				(const uint8_t*)"CommonStartup",
				(const uint8_t*)"CommonDesktopDirectory",
				(const uint8_t*)"ApplicationData",
				(const uint8_t*)"PrinterShortcuts",
				(const uint8_t*)"LocalApplicationData",
				(const uint8_t*)"InternetCache",
				(const uint8_t*)"Cookies",
				(const uint8_t*)"History",
				(const uint8_t*)"CommonApplicationData",
				(const uint8_t*)"Windows",
				(const uint8_t*)"System",
				(const uint8_t*)"ProgramFiles",
				(const uint8_t*)"MyPictures",
				(const uint8_t*)"UserProfile",
				(const uint8_t*)"SystemX86",
				(const uint8_t*)"ProgramFilesX86",
				(const uint8_t*)"CommonProgramFiles",
				(const uint8_t*)"CommonProgramFilesX86",
				(const uint8_t*)"CommonTemplates",
				(const uint8_t*)"CommonDocuments",
				(const uint8_t*)"CommonAdminTools",
				(const uint8_t*)"AdminTools",
				(const uint8_t*)"CommonMusic",
				(const uint8_t*)"CommonPictures",
				(const uint8_t*)"CommonVideos",
				(const uint8_t*)"Resources",
				(const uint8_t*)"LocalizedResources",
				(const uint8_t*)"CommonOemLinks",
				(const uint8_t*)"CDBurning"
			};

			if (!arguments_count)
			{
				break;
			}

			const enum SpecialFolder folder = (enum SpecialFolder)common_string_to_enum(
												  argument.start, argument.finish, special_folder_str, ENVIRONMENT_UNKNOWN_SPECIAL_FOLDER);
			return environment_get_folder_path(folder, output);
		}

		case get_machine_name:
			return !arguments_count && environment_get_machine_name(output);

		case get_operating_system:
			return !arguments_count &&
				   common_append_string_to_buffer(operating_system_to_string(environment_get_operating_system()), output);

		case get_user_name:
			return !arguments_count && environment_get_user_name(output);

		case get_variable:
			return arguments_count &&
				   environment_get_variable(argument.start, argument.finish, output);

		case newline:
			return !arguments_count && environment_newline(output);

		case variable_exists:
			return arguments_count &&
				   bool_to_string(environment_variable_exists(argument.start, argument.finish),
								  output);

		case is64bit_process:
			return !arguments_count && bool_to_string(environment_is64bit_process(), output);

		case is64bit_operating_system:
			return !arguments_count && bool_to_string(environment_is64bit_operating_system(), output);

		case processor_count:
			return !arguments_count && int_to_string(environment_processor_count(), output);

		case UNKNOWN_ENVIRONMENT:
		default:
			break;
	}

	return 0;
}

