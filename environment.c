/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#if !defined(_WIN32)
/*#define _POSIX_SOURCE 1
#define _POSIX_C_SOURCE 200112L*/
#include "stdc_secure_api.h"
#endif

#include "environment.h"
#include "buffer.h"
#include "common.h"
#include "operating_system.h"
#include "range.h"
#include "text_encoding.h"
#include "version.h"

#include <string.h>

#if defined(_WIN32)
#include <stdio.h>
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

	const HMODULE shell32Module = LoadLibraryW(L"shell32.dll");

	if (NULL == shell32Module)
	{
		return 0;
	}

	wchar_t* startW = NULL;
	wchar_t* finishW = NULL;
#if defined(_WIN32_WINNT_VISTA) && defined(_WIN32_WINNT) && (_WIN32_WINNT_VISTA <= _WIN32_WINNT)
#if defined(_MSC_VER) && (_MSC_VER >= 1920)
	const GUID folderIDs[] =
	{
		FOLDERID_Desktop, FOLDERID_Programs, FOLDERID_Documents, FOLDERID_Documents,
		FOLDERID_Favorites, FOLDERID_Startup, FOLDERID_Recent, FOLDERID_SendTo,
		FOLDERID_StartMenu, FOLDERID_Music, FOLDERID_Videos, FOLDERID_Desktop,
		FOLDERID_ComputerFolder, FOLDERID_NetHood, FOLDERID_Fonts, FOLDERID_Templates,
		FOLDERID_CommonStartMenu, FOLDERID_CommonPrograms, FOLDERID_CommonStartup,
		FOLDERID_PublicDesktop, FOLDERID_RoamingAppData, FOLDERID_PrintHood,
		FOLDERID_LocalAppData, FOLDERID_InternetCache, FOLDERID_Cookies, FOLDERID_History,
		FOLDERID_ProgramData, FOLDERID_Windows, FOLDERID_System, FOLDERID_ProgramFiles,
		FOLDERID_Pictures, FOLDERID_Profile, FOLDERID_SystemX86, FOLDERID_ProgramFilesX86,
		FOLDERID_ProgramFilesCommon, FOLDERID_ProgramFilesCommonX86, FOLDERID_CommonTemplates,
		FOLDERID_PublicDocuments, FOLDERID_CommonAdminTools, FOLDERID_AdminTools, FOLDERID_PublicMusic,
		FOLDERID_PublicPictures, FOLDERID_PublicVideos, FOLDERID_ResourceDir, FOLDERID_LocalizedResourcesDir,
		FOLDERID_CommonOEMLinks, FOLDERID_CDBurning
	};
#else
	GUID folderIDs[47];
	folderIDs[0] = FOLDERID_Desktop;
	folderIDs[1] = FOLDERID_Programs;
	folderIDs[2] = FOLDERID_Documents;
	folderIDs[3] = FOLDERID_Documents;
	folderIDs[4] = FOLDERID_Favorites;
	folderIDs[5] = FOLDERID_Startup;
	folderIDs[6] = FOLDERID_Recent;
	folderIDs[7] = FOLDERID_SendTo;
	folderIDs[8] = FOLDERID_StartMenu;
	folderIDs[9] = FOLDERID_Music;
	folderIDs[10] = FOLDERID_Videos;
	folderIDs[11] = FOLDERID_Desktop;
	folderIDs[12] = FOLDERID_ComputerFolder;
	folderIDs[13] = FOLDERID_NetHood;
	folderIDs[14] = FOLDERID_Fonts;
	folderIDs[15] = FOLDERID_Templates;
	folderIDs[16] = FOLDERID_CommonStartMenu;
	folderIDs[17] = FOLDERID_CommonPrograms;
	folderIDs[18] = FOLDERID_CommonStartup;
	folderIDs[19] = FOLDERID_PublicDesktop;
	folderIDs[20] = FOLDERID_RoamingAppData;
	folderIDs[21] = FOLDERID_PrintHood;
	folderIDs[22] = FOLDERID_LocalAppData;
	folderIDs[23] = FOLDERID_InternetCache;
	folderIDs[24] = FOLDERID_Cookies;
	folderIDs[25] = FOLDERID_History;
	folderIDs[26] = FOLDERID_ProgramData;
	folderIDs[27] = FOLDERID_Windows;
	folderIDs[28] = FOLDERID_System;
	folderIDs[29] = FOLDERID_ProgramFiles;
	folderIDs[30] = FOLDERID_Pictures;
	folderIDs[31] = FOLDERID_Profile;
	folderIDs[32] = FOLDERID_SystemX86;
	folderIDs[33] = FOLDERID_ProgramFilesX86;
	folderIDs[34] = FOLDERID_ProgramFilesCommon;
	folderIDs[35] = FOLDERID_ProgramFilesCommonX86;
	folderIDs[36] = FOLDERID_CommonTemplates;
	folderIDs[37] = FOLDERID_PublicDocuments;
	folderIDs[38] = FOLDERID_CommonAdminTools;
	folderIDs[39] = FOLDERID_AdminTools;
	folderIDs[40] = FOLDERID_PublicMusic;
	folderIDs[41] = FOLDERID_PublicPictures;
	folderIDs[42] = FOLDERID_PublicVideos;
	folderIDs[43] = FOLDERID_ResourceDir;
	folderIDs[44] = FOLDERID_LocalizedResourcesDir;
	folderIDs[45] = FOLDERID_CommonOEMLinks;
	folderIDs[46] = FOLDERID_CDBurning;
#endif
	typedef HRESULT(WINAPI * LPFN_SHGETKNOWNFOLDERPATH)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*);
	LPFN_SHGETKNOWNFOLDERPATH fnSHGetKnownFolderPath = NULL;
	fnSHGetKnownFolderPath = (LPFN_SHGETKNOWNFOLDERPATH)GetProcAddress(shell32Module, "SHGetKnownFolderPath");

	if (NULL == fnSHGetKnownFolderPath)
	{
		return 0;
	}

	if (S_OK == fnSHGetKnownFolderPath(&folderIDs[folder], KF_FLAG_DEFAULT, NULL, &startW))
	{
		finishW = startW;

		while (0 != *finishW)
		{
			++finishW;
		}

		const uint8_t returned = text_encoding_UTF16LE_to_UTF8(startW, finishW, path);
		CoTaskMemFree(startW);
		return returned;
	}

#endif
#define CSIDL_UNKNOWN -1
	static const int folderCSIDLs[] =
	{
		CSIDL_DESKTOP, CSIDL_PROGRAMS, CSIDL_PERSONAL, CSIDL_PERSONAL,
		CSIDL_FAVORITES, CSIDL_STARTUP, CSIDL_RECENT, CSIDL_SENDTO,
		CSIDL_STARTMENU, CSIDL_MYMUSIC, CSIDL_MYVIDEO, CSIDL_DESKTOP,
		CSIDL_UNKNOWN/*FOLDERID_ComputerFolder*/, CSIDL_NETHOOD, CSIDL_FONTS, CSIDL_TEMPLATES,
		CSIDL_COMMON_STARTMENU, CSIDL_COMMON_PROGRAMS, CSIDL_COMMON_STARTUP,
		CSIDL_COMMON_DESKTOPDIRECTORY, CSIDL_APPDATA, CSIDL_PRINTHOOD,
		CSIDL_LOCAL_APPDATA, CSIDL_INTERNET_CACHE, CSIDL_COOKIES, CSIDL_HISTORY,
		CSIDL_COMMON_APPDATA, CSIDL_WINDOWS, CSIDL_SYSTEM, CSIDL_PROGRAM_FILES,
		CSIDL_MYPICTURES, CSIDL_PROFILE, CSIDL_SYSTEMX86, CSIDL_PROGRAM_FILESX86,
		CSIDL_PROGRAM_FILES_COMMON, CSIDL_PROGRAM_FILES_COMMONX86, CSIDL_COMMON_TEMPLATES,
		CSIDL_COMMON_DOCUMENTS, CSIDL_COMMON_ADMINTOOLS, CSIDL_ADMINTOOLS, CSIDL_COMMON_MUSIC,
		CSIDL_COMMON_PICTURES, CSIDL_COMMON_VIDEO, CSIDL_RESOURCES, CSIDL_UNKNOWN/*FOLDERID_LocalizedResourcesDir*/,
		CSIDL_UNKNOWN/*FOLDERID_CommonOEMLinks*/, CSIDL_CDBURN_AREA
	};

	if (CSIDL_UNKNOWN == folderCSIDLs[folder])
	{
		return 1;
	}

	typedef HRESULT(WINAPI * LPFN_SHGETFOLDERPATHW)(HWND, int, HANDLE, DWORD, LPWSTR);
	LPFN_SHGETFOLDERPATHW fnSHGetFolderPathW = NULL;
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

	if (!buffer_append(path, NULL, 6 * (FILENAME_MAX + 1) + sizeof(uint32_t)))
	{
		return 0;
	}

	startW = (wchar_t*)buffer_data(path,
								   buffer_size(path) - sizeof(uint32_t) - sizeof(wchar_t) * FILENAME_MAX - sizeof(wchar_t));

	if (S_OK != fnSHGetFolderPathW(NULL, folderCSIDLs[folder], NULL, SHGFP_TYPE_DEFAULT, startW))
	{
		return 0;
	}

	finishW = startW;

	while (0 != *finishW)
	{
		++finishW;
	}

	if (!buffer_resize(path, size))
	{
		return 0;
	}

	return text_encoding_UTF16LE_to_UTF8(startW, finishW, path);
}

#else

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

	return buffer_resize(name, size) &&
		   text_encoding_UTF16LE_to_UTF8(nameW, nameW + max_size, name);
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

	return (size < buffer_size(name)) &&
		   buffer_resize(name, size + common_count_bytes_until((const uint8_t*)host_name, '\0'));
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
	static const uint8_t majors[] = { 10, 6, 6, 6, 6 };
	static const uint8_t minors[] = { 0, 3, 2, 1, 0 };
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

static uint8_t operating_system[UINT8_MAX];
static uint8_t is_data_of_operating_system_filled = 0;

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
static uint8_t is_data_of_operating_system_filled = 0;

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

		const uint8_t* version_start = (const uint8_t*)uname_data.version;
		const uint8_t* version_finish = version_start +
										common_count_bytes_until((const uint8_t*)uname_data.version, '\0');

		if (!version_parse(version_start, version_finish, version))
		{
			return NULL;
		}

		const uint8_t* version_string[] =
		{
			(const uint8_t*)uname_data.sysname, (const uint8_t*)uname_data.release,
			version_start, (const uint8_t*)uname_data.machine
		};
#if defined(__APPLE__) && defined(__MACH__)
		const uint8_t platformID = macOS;
#else
		const uint8_t platformID = Unix;
#endif
		is_data_of_operating_system_filled = operating_system_init(
				platformID, version, version_string, (ptrdiff_t)sizeof(operating_system), operating_system);

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

	return buffer_resize(name, size) &&
		   text_encoding_UTF16LE_to_UTF8(nameW, nameW + max_size, name) &&
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

uint8_t environment_get_variable(
	const uint8_t* variable_name_start, const uint8_t* variable_name_finish, struct buffer* variable)
{
	if (range_in_parts_is_null_or_empty(variable_name_start, variable_name_finish))
	{
		return 0;
	}

	uint8_t local_variable_was_used = 0;
	struct buffer l_variable;
	SET_NULL_TO_BUFFER(l_variable);

	if (!variable)
	{
		local_variable_was_used = 1;
		variable = &l_variable;
	}

	const ptrdiff_t size = buffer_size(variable);

	if (!text_encoding_UTF8_to_UTF16LE(
			variable_name_start, variable_name_finish, variable) ||
		!buffer_push_back_uint16(variable, 0))
	{
		buffer_release(&l_variable);
		return 0;
	}

	const ptrdiff_t size_ = buffer_size(variable);

	if (!buffer_push_back_uint16(variable, 0))
	{
		buffer_release(&l_variable);
		return 0;
	}

	const wchar_t* variable_name = (const wchar_t*)buffer_data(variable, size);
	wchar_t* variable_value = (wchar_t*)buffer_data(variable, size_);
	/**/
	DWORD variable_value_size = GetEnvironmentVariableW(
									variable_name, variable_value, 1);
	/**/
	buffer_release(&l_variable);

	if (variable_value_size < 2)
	{
		return 0;
	}

	if (local_variable_was_used)
	{
		return 1;
	}

	if (!buffer_append(variable, NULL, sizeof(uint32_t) + (ptrdiff_t)6 * (variable_value_size + (ptrdiff_t)1)))
	{
		return 0;
	}

	variable_name = (const wchar_t*)buffer_data(variable, size);
	variable_value = (wchar_t*)buffer_data(variable,
										   buffer_size(variable) - sizeof(uint32_t) - sizeof(wchar_t) * variable_value_size - sizeof(wchar_t));
	/**/
	variable_value_size = GetEnvironmentVariableW(
							  variable_name, variable_value, variable_value_size);

	if (variable_value_size < 2)
	{
		return 0;
	}

	return buffer_resize(variable, size) &&
		   text_encoding_UTF16LE_to_UTF8(variable_value, variable_value + variable_value_size, variable);
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
#if defined(_WIN64) || defined(__amd64) || defined(__x86_64) || defined(__aarch64__)
	return 1;
#else
	return 0;
#endif
}

uint8_t environment_is64bit_operating_system()
{
#if defined(_WIN64) || defined(__amd64) || defined(__x86_64) || defined(__aarch64__)
	return 1;
#else
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
#if defined(__ANDROID__) && (__ANDROID_API__ < 23)
	return 1;
#else
	return (uint16_t)get_nprocs();
#endif
#endif
}
