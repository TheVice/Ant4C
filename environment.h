/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <stddef.h>
#include <stdint.h>

struct buffer;
struct OperatingSystem;

enum SpecialFolder
{
	Desktop,
	Programs,
	Personal,
	MyDocuments,
	Favorites,
	Startup,
	Recent,
	SendTo,
	StartMenu,
	MyMusic,
	MyVideos,
	DesktopDirectory,
	MyComputer,
	NetworkShortcuts,
	Fonts,
	Templates,
	CommonStartMenu,
	CommonPrograms,
	CommonStartup,
	CommonDesktopDirectory,
	ApplicationData,
	PrinterShortcuts,
	LocalApplicationData,
	InternetCache,
	Cookies,
	History,
	CommonApplicationData,
	Windows,
	System,
	ProgramFiles,
	MyPictures,
	UserProfile,
	SystemX86,
	ProgramFilesX86,
	CommonProgramFiles,
	CommonProgramFilesX86,
	CommonTemplates,
	CommonDocuments,
	CommonAdminTools,
	AdminTools,
	CommonMusic,
	CommonPictures,
	CommonVideos,
	Resources,
	LocalizedResources,
	CommonOemLinks,
	CDBurning
};

uint8_t environment_get_folder_path(enum SpecialFolder folder, struct buffer* path);
uint8_t environment_get_machine_name(struct buffer* name);
const struct OperatingSystem* environment_get_operating_system();
uint8_t environment_get_user_name(struct buffer* name);
uint8_t environment_get_variable(const uint8_t* variable_name_start, const uint8_t* variable_name_finish,
								 struct buffer* variable);
uint8_t environment_newline(struct buffer* newline);
uint8_t environment_variable_exists(const uint8_t* variable_name_start, const uint8_t* variable_name_finish);
uint8_t environment_is64bit_process();
uint8_t environment_is64bit_operating_system();

uint8_t environment_get_function(const uint8_t* name_start, const uint8_t* name_finish);
uint8_t environment_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
								  struct buffer* output);

#endif
