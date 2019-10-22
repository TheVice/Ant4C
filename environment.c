/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "environment.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "operating_system.h"
#include "range.h"

#include <string.h>

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

static struct OperatingSystem operating_system;
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
		uint16_t count = (uint16_t)wcslen(pathW);
		const ptrdiff_t path_size = buffer_size(path);

		if (!buffer_append_char(path, NULL, count))
		{
			CoTaskMemFree(pathW);
			pathW = NULL;
			return 0;
		}

		char* m = (char*)buffer_data(path, path_size);
		WIDE2MULTI(pathW, m, count);
		CoTaskMemFree(pathW);
		pathW = NULL;
		return 0 < count;
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
	ptrdiff_t path_size = buffer_size(path);

	if (!buffer_append_char(path, NULL, MAX_PATH + 1) || !buffer_append_char(path, NULL, MAX_PATH + 1))
	{
		return 0;
	}

	char* m = (char*)buffer_data(path, path_size);
	wchar_t* w = (wchar_t*)buffer_data(path, path_size + MAX_PATH + 1);

	if (S_OK != fnSHGetFolderPathW(NULL, folderID, NULL, SHGFP_TYPE_DEFAULT, w))
	{
		return 0;
	}

	if (!buffer_resize(path, path_size + (ptrdiff_t)wcslen(w)))
	{
		return 0;
	}

	uint16_t count  = (uint16_t)(buffer_size(path) - path_size);
	WIDE2MULTI(w, m, count);
	return 0 < count;
#endif
}

#else

#define _POSIXSOURCE 1

#include <pwd.h>
#include <unistd.h>
#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>

uint8_t environment_get_folder_path(enum SpecialFolder folder, struct buffer* path)
{
	switch (folder)
	{
		case Desktop:
		case DesktopDirectory:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer("/Desktop", path);
			}

			break;

		case Personal:
		case MyDocuments:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer("/Documents", path);
			}

			break;

		case MyMusic:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer("/Music", path);
			}

			break;

		case MyVideos:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer("/Videos", path);
			}

			break;

		case Templates:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer("/Templates", path);
			}

			break;

		case ApplicationData:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer("/.config", path);
			}

			break;

		case LocalApplicationData:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer("/.local/share", path);
			}

			break;

		case CommonApplicationData:
			return common_append_string_to_buffer("/usr/share", path);

		case MyPictures:
			if (environment_get_folder_path(UserProfile, path))
			{
				return common_append_string_to_buffer("/Pictures", path);
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

			return common_append_string_to_buffer(user_info->pw_dir, path);
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

	const ptrdiff_t name_size = buffer_size(name);
	DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

	if (!buffer_append_char(name, NULL, size) || !buffer_append_wchar_t(name, NULL, size))
	{
		return 0;
	}

	wchar_t* nameW = (wchar_t*)buffer_data(name, name_size + size);

	if (!GetComputerNameW(nameW, &size))
	{
		return 0;
	}

	const DWORD computer_name_size = size;
	char* m = (char*)buffer_data(name, name_size);
	WIDE2MULTI(nameW, m, size);
	return size && buffer_resize(name, name_size + computer_name_size);
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

	if (0 != gethostname((char*)buffer_data(name, size), UINT8_MAX))
	{
		return 0;
	}

	return (size < buffer_size(name)) && buffer_resize(name, size + strlen((char*)buffer_data(name, size)));
}

#endif
#if defined(_WIN32)

struct Version GetWindowsVersion()
{
	struct Version ver;
	ver.major = ver.minor = ver.build = ver.revision = 0;
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#if _WIN32_WINNT > 0x0603

	if (IsWindows10OrGreater())
#else
	if (IsWindowsVersionOrGreater(10, 0, 0))
#endif
	{
		ver.major = 10;
	}
	else if (IsWindows8Point1OrGreater())
	{
		ver.major = 6;
		ver.minor = 3;
	}
	else if (IsWindows8OrGreater())
	{
		ver.major = 6;
		ver.minor = 2;
	}
	else if (IsWindows7OrGreater())
	{
		ver.major = 6;
		ver.minor = 1;
	}
	else if (IsWindowsVistaOrGreater())
	{
		ver.major = 6;
	}

	/*
		IsWindowsXPOrGreater();
		IsWindowsServer();
	*/
#else
	/*TODO: call VerSetConditionMask, VerifyVersionInfoW via pointers.*/
	static const struct Version versions[] = { {10, 0, 0, 0}, {6, 3, 0, 0}, {6, 2, 0, 0}, {6, 1, 0, 0}, {6, 0, 0, 0} };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(
										   VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
										   VER_MINORVERSION, VER_GREATER_EQUAL);

	for (uint8_t i = 0, count = sizeof(versions) / sizeof(*versions); i < count; ++i)
	{
		OSVERSIONINFOEXW osvi = { sizeof(OSVERSIONINFOEXW), 0, 0, 0, 0, { 0 }, 0, 0, 0, 0, 0 };
		osvi.dwMajorVersion = versions[i].major;
		osvi.dwMinorVersion = versions[i].minor;

		if (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask))
		{
			return versions[i];
		}
	}

#endif
	return ver;
}

const struct OperatingSystem* environment_get_operating_system()
{
	if (!is_data_of_operating_system_filled)
	{
		operating_system.Platform = Win32;
		operating_system.Version = GetWindowsVersion();
#if __STDC_SEC_API__
		is_data_of_operating_system_filled = (0 == strcpy_s(operating_system.VersionString, INT8_MAX, Win32NT_str));
#else
		strcpy(operating_system.VersionString, Win32NT_str);
		is_data_of_operating_system_filled = 1;
#endif

		if (is_data_of_operating_system_filled)
		{
			char* ptr = operating_system.VersionString + strlen(operating_system.VersionString);
			*ptr = ' ';
			++ptr;
			is_data_of_operating_system_filled = 0 < version_to_char_array(&operating_system.Version, ptr);
		}
	}

	return &operating_system;
}

#else

const struct OperatingSystem* environment_get_operating_system()
{
	if (!is_data_of_operating_system_filled)
	{
		struct utsname uname_data;
		operating_system.Platform = Unix;

		if (-1 == uname(&uname_data))
		{
			return NULL;
		}

		if (!version_parse(uname_data.version, uname_data.version + strlen(uname_data.version),
						   &operating_system.Version))
		{
			/*TODO: call echo with verbose or debug level.*/
		}

		const uint16_t max_count = sizeof(operating_system.VersionString) / sizeof(operating_system.VersionString[0]);
		const uint16_t count = 4 + strlen(uname_data.sysname) + strlen(uname_data.release) + strlen(
								   uname_data.version) + strlen(uname_data.machine);

		if (max_count - 1 < count)
		{
			return NULL;
		}

		sprintf(operating_system.VersionString, "%s %s %s %s",
				uname_data.sysname, uname_data.release,
				uname_data.version, uname_data.machine);
		is_data_of_operating_system_filled = 1;
	}

	return &operating_system;
}

#endif
#if defined(_WIN32)

uint8_t environment_get_user_name(struct buffer* name)
{
	if (NULL == name)
	{
		return 0;
	}

	const ptrdiff_t name_size = buffer_size(name);
	DWORD size = UNLEN + 1;

	if (!buffer_append_char(name, NULL, size) || !buffer_append_wchar_t(name, NULL, size))
	{
		return 0;
	}

	wchar_t* nameW = (wchar_t*)buffer_data(name, name_size + size);

	if (!GetUserNameW(nameW, &size))
	{
		return 0;
	}

	const DWORD user_name_size = size - 1;
	char* m = (char*)buffer_data(name, name_size);
	WIDE2MULTI(nameW, m, size);
	return size && buffer_resize(name, name_size + user_name_size);
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

	return common_append_string_to_buffer(user_info->pw_name, name);
}

#endif
#if defined(_WIN32)

uint8_t environment_get_variable(const char* variable_name, uint8_t variable_name_length,
								 struct buffer* variable)
{
	if (NULL == variable_name || 0 == variable_name_length)
	{
		return 0;
	}

	struct buffer variable_name_w;

	SET_NULL_TO_BUFFER(variable_name_w);

	if (!buffer_append_wchar_t(&variable_name_w, NULL, (ptrdiff_t)variable_name_length + 2))
	{
		buffer_release(&variable_name_w);
		return 0;
	}

	wchar_t* ptr = buffer_wchar_t_data(&variable_name_w, 0);

	for (uint8_t i = 0; i < variable_name_length; ++i)/*TODO: Use WIDE2MULTI*/
	{
		ptr[i] = variable_name[i];
	}

	ptr[variable_name_length] = L'\0';
	const uint16_t pos = variable_name_length + 1;
	DWORD size = GetEnvironmentVariableW(buffer_wchar_t_data(&variable_name_w, 0),
										 buffer_wchar_t_data(&variable_name_w, pos),
										 1);

	if (0 == size)
	{
		buffer_release(&variable_name_w);
		return 0;
	}

	if (NULL == variable)
	{
		buffer_release(&variable_name_w);
		return 1;
	}

	if (1 < size)
	{
		if (!buffer_append_wchar_t(&variable_name_w, NULL, size))
		{
			buffer_release(&variable_name_w);
			return 0;
		}

		size = GetEnvironmentVariableW(buffer_wchar_t_data(&variable_name_w, 0),
									   buffer_wchar_t_data(&variable_name_w, pos),
									   size);

		if (0 == size)
		{
			buffer_release(&variable_name_w);
			return 0;
		}
	}

	++size;
	const ptrdiff_t variable_size = buffer_size(variable);

	if (!buffer_append_char(variable, NULL, size))
	{
		buffer_release(&variable_name_w);
		return 0;
	}

	--size;
	const wchar_t* w = buffer_wchar_t_data(&variable_name_w, pos);
	char* m = (char*)buffer_data(variable, variable_size);
	WIDE2MULTI(w, m, size);
	buffer_release(&variable_name_w);
	return size && buffer_resize(variable, buffer_size(variable) - 1);
}

#else

uint8_t environment_get_variable(const char* variable_name, uint8_t variable_name_length,
								 struct buffer* variable)
{
	if (NULL == variable_name ||
		0 == variable_name_length)
	{
		return 0;
	}

	struct buffer variable_name_;

	SET_NULL_TO_BUFFER(variable_name_);

	if (!buffer_append_char(&variable_name_, variable_name, variable_name_length) ||
		!buffer_push_back(&variable_name_, '\0'))
	{
		buffer_release(&variable_name_);
		return 0;
	}

	const char* value = getenv(buffer_char_data(&variable_name_, 0));
	buffer_release(&variable_name_);

	if (NULL == value)
	{
		return 0;
	}

	if (NULL == variable)
	{
		return 1;
	}

	return common_append_string_to_buffer(value, variable);
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

uint8_t environment_variable_exists(const char* variable_name, uint8_t variable_name_length)
{
	return environment_get_variable(variable_name, variable_name_length, NULL);
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
	const struct OperatingSystem* os = environment_get_operating_system();
	return NULL != strstr(os->VersionString, "x86_64") || NULL != strstr(os->VersionString, "amd64");
#endif
}

static const char* environment_str[] =
{
	"get-folder-path", "get-machine-name", "get-operating-system",
	"get-user-name", "get-variable", "newline", "variable-exists",
	"is64bit-process", "is64bit-operating-system"
};

enum environment_function
{
	get_folder_path, get_machine_name, get_operating_system,
	get_user_name, get_variable, newline, variable_exists,
	is64bit_process, is64bit_operating_system,
	UNKNOWN_ENVIRONMENT
};

uint8_t environment_get_function(const char* name_start, const char* name_finish)
{
	return common_string_to_enum(name_start, name_finish, environment_str, UNKNOWN_ENVIRONMENT);
}

static const char* special_folder_str[] =
{
	"Desktop", "Programs", "Personal", "MyDocuments", "Favorites",
	"Startup", "Recent", "SendTo", "StartMenu",	"MyMusic", "MyVideos",
	"DesktopDirectory",	"MyComputer", "NetworkShortcuts", "Fonts",
	"Templates", "CommonStartMenu", "CommonPrograms", "CommonStartup",
	"CommonDesktopDirectory", "ApplicationData", "PrinterShortcuts",
	"LocalApplicationData", "InternetCache", "Cookies", "History",
	"CommonApplicationData", "Windows", "System", "ProgramFiles",
	"MyPictures", "UserProfile", "SystemX86", "ProgramFilesX86",
	"CommonProgramFiles", "CommonProgramFilesX86", "CommonTemplates",
	"CommonDocuments", "CommonAdminTools", "AdminTools", "CommonMusic",
	"CommonPictures", "CommonVideos", "Resources", "LocalizedResources",
	"CommonOemLinks", "CDBurning"
};

uint8_t environment_exec_function(uint8_t function, const struct buffer* arguments, uint8_t arguments_count,
								  struct buffer* output)
{
	if (UNKNOWN_ENVIRONMENT <= function || NULL == arguments || 1 < arguments_count || NULL == output)
	{
		return 0;
	}

	struct range argument;

	argument.start = argument.finish = NULL;

	if (1 == arguments_count && !common_get_one_argument(arguments, &argument, 0))
	{
		return 0;
	}

	switch (function)
	{
		case get_folder_path:
		{
			if (1 != arguments_count)
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
		{
			const struct OperatingSystem* os = environment_get_operating_system();
			return !arguments_count && common_append_string_to_buffer(os->VersionString, output);
		}

		case get_user_name:
			return !arguments_count && environment_get_user_name(output);

		case get_variable:
			return (1 == arguments_count) &&
				   environment_get_variable(argument.start, (uint8_t)range_size(&argument), output);

		case newline:
			return !arguments_count && environment_newline(output);

		case variable_exists:
			return (1 == arguments_count) &&
				   bool_to_string(environment_variable_exists(argument.start, (uint8_t)range_size(&argument)),
								  output);

		case is64bit_process:
			return !arguments_count && bool_to_string(environment_is64bit_process(), output);

		case is64bit_operating_system:
			return !arguments_count && bool_to_string(environment_is64bit_operating_system(), output);

		case UNKNOWN_ENVIRONMENT:
		default:
			break;
	}

	return 0;
}
