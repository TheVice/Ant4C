/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 https://github.com/TheVice/
 *
 */

#include "environment.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "operating_system.h"
#include "range.h"
#include "text_encoding.h"
#include "version.h"

#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

static uint8_t is_data_of_operating_system_filled = 0;

#define ENVIRONMENT_UNKNOWN_SPECIAL_FOLDER (CDBurning + 1)

#if defined(_WIN32)
#include <wchar.h>

#include <windows.h>
#include <ShlObj.h>
#include <lmcons.h>

#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#include <VersionHelpers.h>
#endif

uint8_t environment_get_folder_path(enum SpecialFolder folder, struct buffer* path)
{
	if (ENVIRONMENT_UNKNOWN_SPECIAL_FOLDER <= folder || NULL == path)
	{
		return 0;
	}

#if defined(_WIN32_WINNT_VISTA) && defined(_WIN32_WINNT) && (_WIN32_WINNT_VISTA <= _WIN32_WINNT)
	GUID folderID;

	switch (folder)
	{
		case Desktop:
		case DesktopDirectory:
			folderID = FOLDERID_Desktop;
			break;

		case Programs:
			folderID = FOLDERID_Programs;
			break;

		case Personal:
		case MyDocuments:
			folderID = FOLDERID_Documents;
			break;

		case Favorites:
			folderID = FOLDERID_Favorites;
			break;

		case Startup:
			folderID = FOLDERID_Startup;
			break;

		case Recent:
			folderID = FOLDERID_Recent;
			break;

		case SendTo:
			folderID = FOLDERID_SendTo;
			break;

		case StartMenu:
			folderID = FOLDERID_StartMenu;
			break;

		case MyMusic:
			folderID = FOLDERID_Music;
			break;

		case MyVideos:
			folderID = FOLDERID_Videos;
			break;

		case MyComputer:
			folderID = FOLDERID_ComputerFolder;
			break;

		case NetworkShortcuts:
			folderID = FOLDERID_NetHood;
			break;

		case Fonts:
			folderID = FOLDERID_Fonts;
			break;

		case Templates:
			folderID = FOLDERID_Templates;
			break;

		case CommonStartMenu:
			folderID = FOLDERID_CommonStartMenu;
			break;

		case CommonPrograms:
			folderID = FOLDERID_CommonPrograms;
			break;

		case CommonStartup:
			folderID = FOLDERID_CommonStartup;
			break;

		case CommonDesktopDirectory:
			folderID = FOLDERID_PublicDesktop;
			break;

		case ApplicationData:
			folderID = FOLDERID_RoamingAppData;
			break;

		case PrinterShortcuts:
			folderID = FOLDERID_PrintHood;
			break;

		case LocalApplicationData:
			folderID = FOLDERID_LocalAppData;
			break;

		case InternetCache:
			folderID = FOLDERID_InternetCache;
			break;

		case Cookies:
			folderID = FOLDERID_Cookies;
			break;

		case History:
			folderID = FOLDERID_History;
			break;

		case CommonApplicationData:
			folderID = FOLDERID_ProgramData;
			break;

		case Windows:
			folderID = FOLDERID_Windows;
			break;

		case System:
			folderID = FOLDERID_System;
			break;

		case ProgramFiles:
			folderID = FOLDERID_ProgramFiles;
			break;

		case MyPictures:
			folderID = FOLDERID_Pictures;
			break;

		case UserProfile:
			folderID = FOLDERID_Profile;
			break;

		case SystemX86:
			folderID = FOLDERID_SystemX86;
			break;

		case ProgramFilesX86:
			folderID = FOLDERID_ProgramFilesX86;
			break;

		case CommonProgramFiles:
			folderID = FOLDERID_ProgramFilesCommon;
			break;

		case CommonProgramFilesX86:
			folderID = FOLDERID_ProgramFilesCommonX86;
			break;

		case CommonTemplates:
			folderID = FOLDERID_CommonTemplates;
			break;

		case CommonDocuments:
			folderID = FOLDERID_PublicDocuments;
			break;

		case CommonAdminTools:
			folderID = FOLDERID_CommonAdminTools;
			break;

		case AdminTools:
			folderID = FOLDERID_AdminTools;
			break;

		case CommonMusic:
			folderID = FOLDERID_PublicMusic;
			break;

		case CommonPictures:
			folderID = FOLDERID_PublicPictures;
			break;

		case CommonVideos:
			folderID = FOLDERID_PublicVideos;
			break;

		case Resources:
			folderID = FOLDERID_ResourceDir;
			break;

		case LocalizedResources:
			folderID = FOLDERID_LocalizedResourcesDir;
			break;

		case CommonOemLinks:
			folderID = FOLDERID_CommonOEMLinks;
			break;

		case CDBurning:
			folderID = FOLDERID_CDBurning;
			break;

		default:
			return 0;
	}

	typedef HRESULT(WINAPI * LPFN_SHGETKNOWNFOLDERPATH)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*);
	LPFN_SHGETKNOWNFOLDERPATH fnSHGetKnownFolderPath = NULL;
	/**/
	const HMODULE shell32Module = LoadLibraryW(L"shell32.dll");

	if (NULL == shell32Module)
	{
		return 0;
	}

	fnSHGetKnownFolderPath = (LPFN_SHGETKNOWNFOLDERPATH)GetProcAddress(shell32Module, "SHGetKnownFolderPath");

	if (NULL == fnSHGetKnownFolderPath)
	{
		return 0;
	}

	wchar_t* pathW = NULL;

	if (S_OK == fnSHGetKnownFolderPath(&folderID, KF_FLAG_DEFAULT, NULL, &pathW))
	{
		const size_t count = wcslen(pathW);
		const uint8_t returned = text_encoding_UTF16LE_to_UTF8(pathW, pathW + count, path);
		CoTaskMemFree(pathW);
		return returned;
	}

	return 1;
#else
	int folderID = 0;

	switch (folder)
	{
		case Desktop:
		case DesktopDirectory:
			folderID = CSIDL_DESKTOP;
			break;

		case Programs:
			folderID = CSIDL_PROGRAMS;
			break;

		case Personal:
		case MyDocuments:
			folderID = CSIDL_PERSONAL;
			break;

		case Favorites:
			folderID = CSIDL_FAVORITES;
			break;

		case Startup:
			folderID = CSIDL_STARTUP;
			break;

		case Recent:
			folderID = CSIDL_RECENT;
			break;

		case SendTo:
			folderID = CSIDL_SENDTO;
			break;

		case StartMenu:
			folderID = CSIDL_STARTMENU;
			break;

		case MyMusic:
			folderID = CSIDL_MYMUSIC;
			break;

		case MyVideos:
			folderID = CSIDL_MYVIDEO;
			break;

		case MyComputer:
			/*folderID = ;
			break;*/
			return 1;

		case NetworkShortcuts:
			folderID = CSIDL_NETHOOD;
			break;

		case Fonts:
			folderID = CSIDL_FONTS;
			break;

		case Templates:
			folderID = CSIDL_TEMPLATES;
			break;

		case CommonStartMenu:
			folderID = CSIDL_COMMON_STARTMENU;
			break;

		case CommonPrograms:
			folderID = CSIDL_COMMON_PROGRAMS;
			break;

		case CommonStartup:
			folderID = CSIDL_COMMON_STARTUP;
			break;

		case CommonDesktopDirectory:
			folderID = CSIDL_COMMON_DESKTOPDIRECTORY;
			break;

		case ApplicationData:
			folderID = CSIDL_APPDATA;
			break;

		case PrinterShortcuts:
			folderID = CSIDL_PRINTHOOD;
			break;

		case LocalApplicationData:
			folderID = CSIDL_LOCAL_APPDATA;
			break;

		case InternetCache:
			folderID = CSIDL_INTERNET_CACHE;
			break;

		case Cookies:
			folderID = CSIDL_COOKIES;
			break;

		case History:
			folderID = CSIDL_HISTORY;
			break;

		case CommonApplicationData:
			folderID = CSIDL_COMMON_APPDATA;
			break;

		case Windows:
			folderID = CSIDL_WINDOWS;
			break;

		case System:
			folderID = CSIDL_SYSTEM;
			break;

		case ProgramFiles:
			folderID = CSIDL_PROGRAM_FILES;
			break;

		case MyPictures:
			folderID = CSIDL_MYPICTURES;
			break;

		case UserProfile:
			folderID = CSIDL_PROFILE;
			break;

		case SystemX86:
			folderID = CSIDL_SYSTEMX86;
			break;

		case ProgramFilesX86:
			folderID = CSIDL_PROGRAM_FILESX86;
			break;

		case CommonProgramFiles:
			folderID = CSIDL_PROGRAM_FILES_COMMON;
			break;

		case CommonProgramFilesX86:
			folderID = CSIDL_PROGRAM_FILES_COMMONX86;
			break;

		case CommonTemplates:
			folderID = CSIDL_COMMON_TEMPLATES;
			break;

		case CommonDocuments:
			folderID = CSIDL_COMMON_DOCUMENTS;
			break;

		case CommonAdminTools:
			folderID = CSIDL_COMMON_ADMINTOOLS;
			break;

		case AdminTools:
			folderID = CSIDL_ADMINTOOLS;
			break;

		case CommonMusic:
			folderID = CSIDL_COMMON_MUSIC;
			break;

		case CommonPictures:
			folderID = CSIDL_COMMON_PICTURES;
			break;

		case CommonVideos:
			folderID = CSIDL_COMMON_VIDEO;
			break;

		case Resources:
			folderID = CSIDL_RESOURCES;
			break;

		case LocalizedResources:
			/* %SystemRoot%\resources\0409
			folderID = CSIDL_RESOURCES_LOCALIZED;
			break;
			NOTE: Vista approach, see above, return empty path. */
			return 1;

		case CommonOemLinks:
			/* %ProgramData%\OEM Links
			folderID = CSIDL_COMMON_OEM_LINKS;
			break;
			NOTE: Vista approach, see above, return empty path. */
			return 1;

		case CDBurning:
			folderID = CSIDL_CDBURN_AREA;
			break;

		default:
			return 0;
	}

	typedef HRESULT(WINAPI * LPFN_SHGETFOLDERPATHW)(HWND, int, HANDLE, DWORD, LPWSTR);
	LPFN_SHGETFOLDERPATHW fnSHGetFolderPathW = NULL;
	const HMODULE shell32Module = LoadLibraryW(L"shell32.dll");

	if (NULL == shell32Module)
	{
		return 0;
	}

#if defined(__GNUC__) && 8 <= __GNUC__
	fnSHGetFolderPathW = (LPFN_SHGETFOLDERPATHW)(void*)GetProcAddress(shell32Module, "SHGetFolderPathW");
#else
	fnSHGetFolderPathW = (LPFN_SHGETFOLDERPATHW)GetProcAddress(shell32Module, "SHGetFolderPathW");
#endif

	if (NULL == fnSHGetFolderPathW)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(path);

	if (!buffer_append(path, NULL, 6 * (MAX_PATH + 1) + sizeof(uint32_t)))
	{
		return 0;
	}

	wchar_t* pathW = (wchar_t*)buffer_data(path,
										   buffer_size(path) - sizeof(uint32_t) - sizeof(uint16_t) * MAX_PATH - sizeof(uint16_t));

	if (S_OK != fnSHGetFolderPathW(NULL, folderID, NULL, SHGFP_TYPE_DEFAULT, pathW))
	{
		return 0;
	}

	const size_t count = wcslen(pathW);

	if (!buffer_resize(path, size))
	{
		return 0;
	}

	return text_encoding_UTF16LE_to_UTF8(pathW, pathW + count, path);
#endif
}

#else

#define _POSIXSOURCE 1

#include "string_unit.h"

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#endif

#include <pwd.h>
#include <unistd.h>
#if defined(BSD)
#include <sys/sysctl.h>
#else
#include <sys/sysinfo.h>
#endif
#include <sys/utsname.h>

#include <stdlib.h>

uint8_t environment_get_folder_path(enum SpecialFolder folder, struct buffer* path)
{
	switch (folder)
	{
		case Desktop:
		case DesktopDirectory:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer((const uint8_t*)"/Desktop", path);
			}

			break;

		case Personal:
		case MyDocuments:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer((const uint8_t*)"/Documents", path);
			}

			break;

		case MyMusic:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer((const uint8_t*)"/Music", path);
			}

			break;

		case MyVideos:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer((const uint8_t*)"/Videos", path);
			}

			break;

		case Templates:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer((const uint8_t*)"/Templates", path);
			}

			break;

		case ApplicationData:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer((const uint8_t*)"/.config", path);
			}

			break;

		case LocalApplicationData:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer((const uint8_t*)"/.local/share", path);
			}

			break;

		case CommonApplicationData:
			return common_append_string_to_buffer((const uint8_t*)"/usr/share", path);

		case MyPictures:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer((const uint8_t*)"/Pictures", path);
			}

			break;

		case UserProfile:
		{
			const int userID = getuid();
			const struct passwd* user_info = getpwuid(userID);

			if (NULL == user_info)
			{
				break;
			}

			return common_append_string_to_buffer((const uint8_t*)user_info->pw_dir, path);
		}

		default:
			break;
	}

	return 0;
}

#endif
#if defined(_WIN32)

uint8_t environment_get_machine_name(struct buffer* name)
{
	if (NULL == name)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(name);
	DWORD max_size = 6 * (MAX_COMPUTERNAME_LENGTH + 1) + sizeof(uint32_t);

	if (!buffer_append(name, NULL, max_size))
	{
		return 0;
	}

	wchar_t* nameW = (wchar_t*)buffer_data(name,
										   buffer_size(name) - sizeof(uint32_t) - sizeof(uint16_t) * MAX_COMPUTERNAME_LENGTH - sizeof(uint16_t));

	if (!GetComputerNameW(nameW, &max_size))
	{
		return 0;
	}

	if (!buffer_resize(name, size))
	{
		return 0;
	}

	return text_encoding_UTF16LE_to_UTF8(nameW, nameW + max_size, name);
}
#else
uint8_t environment_get_machine_name(struct buffer* name)
{
	if (NULL == name)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(name);

	if (!buffer_append_char(name, NULL, UINT8_MAX + 1))
	{
		return 0;
	}

	char* host_name = (char*)buffer_data(name, size);

	if (0 != gethostname(host_name, UINT8_MAX))
	{
		return 0;
	}

	return (size < buffer_size(name)) && buffer_resize(name, size + strlen(host_name));
}
#endif
#if defined(_WIN32)
uint8_t environment_get_windows_version(void* version)
{
	if (!version)
	{
		return 0;
	}

	uint8_t the_version[2];
	memset(the_version, 0, sizeof(the_version));
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#if _WIN32_WINNT > 0x0603

	if (IsWindows10OrGreater())
#else
	if (IsWindowsVersionOrGreater(10, 0, 0))
#endif
	{
		the_version[0] = 10;
	}
	else if (IsWindows8Point1OrGreater())
	{
		the_version[0] = 6;
		the_version[1] = 3;
	}
	else if (IsWindows8OrGreater())
	{
		the_version[0] = 6;
		the_version[1] = 2;
	}
	else if (IsWindows7OrGreater())
	{
		the_version[0] = 6;
		the_version[1] = 1;
	}
	else if (IsWindowsVistaOrGreater())
	{
		the_version[0] = 6;
	}

#else
	static uint8_t majors[] = { 10, 6, 6, 6, 6 };
	static uint8_t minors[] = { 0, 3, 2, 1, 0 };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(
										   VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
										   VER_MINORVERSION, VER_GREATER_EQUAL);

	for (uint8_t i = 0, count = COUNT_OF(majors); i < count; ++i)
	{
		OSVERSIONINFOEXW osvi;
		memset(&osvi, 0, sizeof(OSVERSIONINFOEXW));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
		osvi.dwMajorVersion = majors[i];
		osvi.dwMinorVersion = minors[i];

		if (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask))
		{
			the_version[0] = majors[i];
			the_version[1] = minors[i];
			/**/
			break;
		}
	}

#endif
	return version_init(version, VERSION_SIZE, the_version[0], the_version[1], 0, 0);
}

static uint8_t operating_system[UINT8_MAX];
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#else
uint8_t IsWindowsServer()
{
	OSVERSIONINFOEXW osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	osvi.wProductType = VER_NT_WORKSTATION;
	DWORDLONG const dwlConditionMask = VerSetConditionMask(0, VER_PRODUCT_TYPE, VER_EQUAL);
	/**/
	return !VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
}
#endif
const void* environment_get_operating_system()
{
	if (!is_data_of_operating_system_filled)
	{
		uint8_t version[VERSION_SIZE];

		if (!environment_get_windows_version(version))
		{
			return 0;
		}

		is_data_of_operating_system_filled = operating_system_init(
				Win32, 0 < IsWindowsServer(), version,
				(ptrdiff_t)sizeof(operating_system), operating_system);

		if (!is_data_of_operating_system_filled)
		{
			return NULL;
		}
	}

	return operating_system;
}

#else

static uint8_t operating_system[sizeof(struct utsname) + INT8_MAX];

const void* environment_get_operating_system()
{
	if (!is_data_of_operating_system_filled)
	{
		uint8_t version[VERSION_SIZE];
		struct utsname uname_data;

		if (-1 == uname(&uname_data))
		{
			return NULL;
		}

		if (!version_parse((const uint8_t*)uname_data.version,
						   (const uint8_t*)uname_data.version + strlen(uname_data.version),
						   version))
		{
			return NULL;
		}

		const uint8_t* version_string[] =
		{
			(const uint8_t*)uname_data.sysname, (const uint8_t*)uname_data.release,
			(const uint8_t*)uname_data.version, (const uint8_t*)uname_data.machine
		};
		/**/
		is_data_of_operating_system_filled = operating_system_init(
				Unix, version, version_string, (ptrdiff_t)sizeof(operating_system), operating_system);

		if (!is_data_of_operating_system_filled)
		{
			return NULL;
		}
	}

	return operating_system;
}

#endif
#if defined(_WIN32)

uint8_t environment_get_user_name(struct buffer* name)
{
	if (NULL == name)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(name);
	DWORD max_size = 6 * (UNLEN + 1) + sizeof(uint32_t);

	if (!buffer_append(name, NULL, max_size))
	{
		return 0;
	}

	wchar_t* nameW = (wchar_t*)buffer_data(name,
										   buffer_size(name) - sizeof(uint32_t) - sizeof(uint16_t) * UNLEN - sizeof(uint16_t));

	if (!GetUserNameW(nameW, &max_size))
	{
		return 0;
	}

	if (!buffer_resize(name, size))
	{
		return 0;
	}

	return text_encoding_UTF16LE_to_UTF8(nameW, nameW + max_size, name) &&
		   buffer_resize(name, buffer_size(name) - 1);
}

#else

uint8_t environment_get_user_name(struct buffer* name)
{
	if (NULL == name)
	{
		return 0;
	}

	const int userID = getuid();
	const struct passwd* user_info = getpwuid(userID);

	if (NULL == user_info)
	{
		return 0;
	}

	return common_append_string_to_buffer((const uint8_t*)user_info->pw_name, name);
}

#endif

#define LOCAL_OR_ARGUMENT(L, A) (NULL != (A) ? (A) : (L))

#if defined(_WIN32)

uint8_t environment_get_variable(const uint8_t* variable_name_start, const uint8_t* variable_name_finish,
								 struct buffer* variable)
{
	if (range_in_parts_is_null_or_empty(variable_name_start, variable_name_finish))
	{
		return 0;
	}

	struct buffer l_variable;

	SET_NULL_TO_BUFFER(l_variable);

	const ptrdiff_t size = buffer_size(variable);

	if (!text_encoding_UTF8_to_UTF16LE(variable_name_start, variable_name_finish, LOCAL_OR_ARGUMENT(&l_variable,
									   variable)) ||
		!buffer_push_back_uint16(LOCAL_OR_ARGUMENT(&l_variable, variable), 0))
	{
		buffer_release(&l_variable);
		return 0;
	}

	const ptrdiff_t value_start = buffer_size(LOCAL_OR_ARGUMENT(&l_variable, variable));

	if (!buffer_push_back_uint16(LOCAL_OR_ARGUMENT(&l_variable, variable), 0))
	{
		buffer_release(&l_variable);
		return 0;
	}

	const DWORD variable_value_size = GetEnvironmentVariableW(
										  (const wchar_t*)buffer_data(LOCAL_OR_ARGUMENT(&l_variable, variable), size),
										  (wchar_t*)buffer_data(LOCAL_OR_ARGUMENT(&l_variable, variable), value_start),
										  1);
	buffer_release(&l_variable);

	if (variable_value_size < 2)
	{
		return 0;
	}

	if (NULL == variable)
	{
		return 1;
	}

	if (!buffer_append(variable, NULL, sizeof(uint32_t) + (ptrdiff_t)6 * (variable_value_size + (ptrdiff_t)1)))
	{
		return 0;
	}

	wchar_t* value = (wchar_t*)buffer_data(variable,
										   buffer_size(variable) - sizeof(uint32_t) - sizeof(uint16_t) * variable_value_size - sizeof(uint16_t));
	const DWORD returned_value_size = GetEnvironmentVariableW(
										  (const wchar_t*)buffer_data(variable, size),
										  value,
										  variable_value_size);

	if (variable_value_size < returned_value_size)
	{
		return 0;
	}

	if (!buffer_resize(variable, size))
	{
		return 0;
	}

	return text_encoding_UTF16LE_to_UTF8(value, value + returned_value_size, variable);
}

#else

uint8_t environment_get_variable(const uint8_t* variable_name_start, const uint8_t* variable_name_finish,
								 struct buffer* variable)
{
	if (range_in_parts_is_null_or_empty(variable_name_start, variable_name_finish))
	{
		return 0;
	}

	struct buffer l_variable;

	SET_NULL_TO_BUFFER(l_variable);

	const ptrdiff_t size = buffer_size(variable);

	if (!buffer_append(LOCAL_OR_ARGUMENT(&l_variable, variable), variable_name_start,
					   variable_name_finish - variable_name_start) ||
		!buffer_push_back(LOCAL_OR_ARGUMENT(&l_variable, variable), 0))
	{
		buffer_release(&l_variable);
		return 0;
	}

	const char* value = getenv((const char*)buffer_data(LOCAL_OR_ARGUMENT(&l_variable, variable), size));
	buffer_release(&l_variable);

	if (NULL == value)
	{
		return 0;
	}

	if (NULL == variable)
	{
		return 1;
	}

	return buffer_resize(variable, size) && common_append_string_to_buffer((const uint8_t*)value, variable);
}

#endif

uint8_t environment_newline(struct buffer* newline)
{
	if (NULL == newline)
	{
		return 0;
	}

#if defined(_WIN32)
	return buffer_append_char(newline, "\r\n", 2);
#else
	return buffer_append_char(newline, "\n", 1);
#endif
}

uint8_t environment_variable_exists(const uint8_t* variable_name_start, const uint8_t* variable_name_finish)
{
	return environment_get_variable(variable_name_start, variable_name_finish, NULL);
}

uint8_t environment_is64bit_process()
{
#if defined(_WIN64) || defined(__amd64) || defined(__x86_64)
	return 1;
#else
	return 0;
#endif
}

uint8_t environment_is64bit_operating_system()
{
	if (environment_is64bit_process())
	{
		return 1;
	}

#if defined(_WIN32)
	typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
	typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS2)(HANDLE, PUSHORT, PUSHORT);
	LPFN_ISWOW64PROCESS2 fnIsWow64Process2 = NULL;
	const HMODULE kernel32Module = GetModuleHandleW(L"kernel32.dll");

	if (NULL == kernel32Module)
	{
		return UINT8_MAX;
	}

#if defined(__GNUC__) && 8 <= __GNUC__
	fnIsWow64Process2 = (LPFN_ISWOW64PROCESS2)(void*)GetProcAddress(kernel32Module, "IsWow64Process2");
#else
	fnIsWow64Process2 = (LPFN_ISWOW64PROCESS2)GetProcAddress(kernel32Module, "IsWow64Process2");
#endif

	if (NULL == fnIsWow64Process2)
	{
		LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
#if defined(__GNUC__) && 8 <= __GNUC__
		fnIsWow64Process = (LPFN_ISWOW64PROCESS)(void*)GetProcAddress(kernel32Module, "IsWow64Process");
#else
		fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(kernel32Module, "IsWow64Process");
#endif

		if (NULL == fnIsWow64Process)
		{
			return 0;
		}

		BOOL isWow64Process = FALSE;

		if (0 == fnIsWow64Process(GetCurrentProcess(), &isWow64Process))
		{
			return UINT8_MAX;
		}

		return 0 < isWow64Process;
	}
	else
	{
		USHORT processMachine = 0;
		USHORT nativeMachine = 0;

		if (0 == fnIsWow64Process2(GetCurrentProcess(), &processMachine, &nativeMachine))
		{
			return UINT8_MAX;
		}

		return IMAGE_FILE_MACHINE_AMD64 == nativeMachine;
	}

#else
	static const uint8_t* x86_64 = (const uint8_t*)"x86_64";
	static const uint8_t* amd64 = (const uint8_t*)"amd64";
	const uint8_t* version_string = operating_system_to_string(environment_get_operating_system());
	return string_contains(version_string, version_string + common_count_bytes_until(version_string, 0),
						   x86_64, x86_64 + 6) ||
		   string_contains(version_string, version_string + common_count_bytes_until(version_string, 0),
						   amd64, amd64 + 5);
#endif
}

uint16_t environment_processor_count()
{
#if defined(_WIN32)
	SYSTEM_INFO system_info;
	memset(&system_info, 0, sizeof(SYSTEM_INFO));
	GetSystemInfo(&system_info);
	return MAX(1, (uint16_t)system_info.dwNumberOfProcessors);
#elif defined(BSD)
	int mib[2];
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	int nproc = 0;
	size_t size = sizeof(nproc);

	if (0 == sysctl(mib, 2, &nproc, &size, NULL, 0))
	{
		return (uint16_t)nproc;
	}

	return 1;
#else
	return (uint16_t)get_nprocs();
#endif
}

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
	return common_string_to_enum(name_start, name_finish, environment_str, UNKNOWN_ENVIRONMENT);
}

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

uint8_t environment_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
								  struct buffer* output)
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
