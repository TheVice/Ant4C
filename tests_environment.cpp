/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020 https://github.com/TheVice/
 *
 */

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "environment.h"
#include "interpreter.h"
#include "operating_system.h"
#include "range.h"
#include "string_unit.h"
};

#include <string>
#include <cstddef>
#include <cstdint>
#include <cstring>

class TestEnvironment : public TestsBaseXml
{
};

TEST(TestEnvironment_, environment_get_folder_path)
{
	static const uint8_t verbose = 0;
#if defined(_WIN32)
	const SpecialFolder input[] =
	{
		Desktop, Programs, Personal, MyDocuments, Favorites, Startup, Recent, SendTo, StartMenu, MyMusic, MyVideos,
		DesktopDirectory, MyComputer, NetworkShortcuts, Fonts, Templates, CommonStartMenu, CommonPrograms, CommonStartup,
		CommonDesktopDirectory, ApplicationData, PrinterShortcuts, LocalApplicationData, InternetCache, Cookies, History,
		CommonApplicationData, Windows, System, ProgramFiles, MyPictures, UserProfile, SystemX86, ProgramFilesX86,
		CommonProgramFiles, CommonProgramFilesX86, CommonTemplates, CommonDocuments, CommonAdminTools, AdminTools, CommonMusic,
		CommonPictures, CommonVideos, Resources, LocalizedResources, CommonOemLinks, CDBurning
	};
	//
	const std::string expected_result[] =
	{
		"${environment::get-variable('USERPROFILE')}\\Desktop",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\Start Menu\\Programs",
		"${environment::get-variable('USERPROFILE')}\\Documents",
		"${environment::get-variable('USERPROFILE')}\\Documents",
		"${environment::get-variable('USERPROFILE')}\\Favorites",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\Start Menu\\Programs\\Startup",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\Recent",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\SendTo",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\Start Menu",
		"${environment::get-variable('USERPROFILE')}\\Music",
		"${environment::get-variable('USERPROFILE')}\\Videos",
		"${environment::get-variable('USERPROFILE')}\\Desktop",
		"",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\Network Shortcuts",
		"${environment::get-variable('SystemRoot')}\\Fonts",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\Templates",
		"${environment::get-variable('ProgramData')}\\Microsoft\\Windows\\Start Menu",
		"${environment::get-variable('ProgramData')}\\Microsoft\\Windows\\Start Menu\\Programs",
		"${environment::get-variable('ProgramData')}\\Microsoft\\Windows\\Start Menu\\Programs\\Startup",
		"${environment::get-variable('PUBLIC')}\\Desktop",
		"${environment::get-variable('APPDATA')}",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\Printer Shortcuts",
		"${environment::get-variable('LOCALAPPDATA')}",
		"${environment::get-variable('LOCALAPPDATA')}\\Microsoft\\Windows\\INetCache",
		"${environment::get-variable('LOCALAPPDATA')}\\Microsoft\\Windows\\INetCookies",
		"${environment::get-variable('LOCALAPPDATA')}\\Microsoft\\Windows\\History",
		"${environment::get-variable('ProgramData')}",
		"${environment::get-variable('SystemRoot')}",
		"${environment::get-variable('SystemRoot')}\\system32",
		"${environment::get-variable('ProgramFiles')}",
		"${environment::get-variable('USERPROFILE')}\\Pictures",
		"${environment::get-variable('USERPROFILE')}",
		"${environment::get-variable('SystemRoot')}\\SysWOW64",
		"${environment::get-variable('ProgramFiles(X86)')}",
		"${environment::get-variable('ProgramFiles')}\\Common Files",
		"${environment::get-variable('ProgramFiles(x86)')}\\Common Files",
		"${environment::get-variable('ProgramData')}\\Microsoft\\Windows\\Templates",
		"${environment::get-variable('PUBLIC')}\\Documents",
		"${environment::get-variable('ProgramData')}\\Microsoft\\Windows\\Start Menu\\Programs\\Administrative Tools",
		"${environment::get-variable('APPDATA')}\\Microsoft\\Windows\\Start Menu\\Programs\\Administrative Tools",
		"${environment::get-variable('PUBLIC')}\\Music",
		"${environment::get-variable('PUBLIC')}\\Pictures",
		"${environment::get-variable('PUBLIC')}\\Videos",
		"${environment::get-variable('SystemRoot')}\\resources",
		"",
		"",
		"${environment::get-variable('LOCALAPPDATA')}\\Microsoft\\Windows\\Burn\\Burn"
	};
#else
	const SpecialFolder input[] =
	{
		Desktop, DesktopDirectory, Personal, MyDocuments, MyMusic, MyVideos,
		Templates, ApplicationData, LocalApplicationData, CommonApplicationData,
		MyPictures, UserProfile
	};
	//
	const std::string expected_result[] =
	{
		"${environment::get-variable('HOME')}/Desktop",
		"${environment::get-variable('HOME')}/Desktop",
		"${environment::get-variable('HOME')}/Documents",
		"${environment::get-variable('HOME')}/Documents",
		"${environment::get-variable('HOME')}/Music",
		"${environment::get-variable('HOME')}/Videos",
		"${environment::get-variable('HOME')}/Templates",
		"${environment::get-variable('HOME')}/.config",
		"${environment::get-variable('HOME')}/.local/share",
		"/usr/share",
		"${environment::get-variable('HOME')}/Pictures",
		"${environment::get-variable('HOME')}"
	};
#endif
	buffer path;
	SET_NULL_TO_BUFFER(path);

	for (uint8_t i = 0, count = COUNT_OF(input); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
		ASSERT_TRUE(environment_get_folder_path(input[i], &path)) << (uint32_t)i << std::endl
				<< buffer_free(&path);

		if (!expected_result[i].empty())
		{
			const ptrdiff_t size = buffer_size(&path);
			const range code = string_to_range(expected_result[i]);
			ASSERT_TRUE(interpreter_evaluate_code(NULL, NULL, &code, &path, verbose))
					<< "'" << expected_result[i] << "'" << std::endl << buffer_free(&path);
			std::string path_in_string(buffer_to_string(&path));
			ASSERT_FALSE(path_in_string.empty()) << (uint32_t)i << std::endl << buffer_free(&path);
			ASSERT_EQ(2 * size, buffer_size(&path))
					<< (uint32_t)i << std::endl
					<< path_in_string << std::endl << buffer_free(&path);
			ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
			ASSERT_TRUE(string_to_lower(
							(const uint8_t*)path_in_string.c_str(),
							(const uint8_t*)path_in_string.c_str() + path_in_string.size(),
							&path)) << buffer_free(&path);
			path_in_string = buffer_to_string(&path);
			ASSERT_EQ(2 * size, (ptrdiff_t)path_in_string.size())
					<< (uint32_t)i << std::endl
					<< path_in_string << std::endl << buffer_free(&path);
			ASSERT_EQ(path_in_string.substr(size),
					  path_in_string.substr(0, size)) << (uint32_t)i << std::endl << buffer_free(&path);
		}
		else
		{
			const std::string path_str(buffer_to_string(&path));
			ASSERT_EQ(0, buffer_size(&path)) << (uint32_t)i << " '" << path_str << "'" << std::endl << buffer_free(&path);
		}
	}

	buffer_release(&path);
}

TEST(TestEnvironment_, environment_get_machine_name)
{
#if defined(_WIN32)
	const uint8_t* variable_name = (const uint8_t*)"COMPUTERNAME";
	const uint8_t variable_name_length = 12;
#endif
	buffer result;
	SET_NULL_TO_BUFFER(result);
	//
	ASSERT_TRUE(environment_get_machine_name(&result)) << buffer_free(&result);
#if defined(_WIN32)
	buffer expected_result;
	SET_NULL_TO_BUFFER(expected_result);
	//
	const std::string returned(buffer_to_string(&result));
	ASSERT_TRUE(
		environment_get_variable(variable_name, variable_name + variable_name_length, &expected_result))
			<< "Expected value for variable is '" << returned << "'" << std::endl
			<< buffer_free(&expected_result) << buffer_free(&result);
	const std::string expected_return(buffer_to_string(&expected_result));
	ASSERT_EQ(expected_return, returned) << buffer_free(&expected_result) << buffer_free(&result);
	//
	buffer_release(&expected_result);
#endif
	buffer_release(&result);
}

TEST(TestEnvironment_, environment_get_operating_system)
{
	const OperatingSystem* os = environment_get_operating_system();
	ASSERT_NE(nullptr, os);
#if defined(_WIN32)
	ASSERT_EQ(Win32, os->Platform);
#else
	ASSERT_EQ(Unix, os->Platform);
#endif
	ASSERT_LT(0, common_count_bytes_until(os->VersionString, 0));
}

TEST(TestEnvironment_, environment_get_user_name)
{
#if defined(_WIN32)
	const uint8_t* variable_name = (const uint8_t*)"USERNAME";
	const uint8_t variable_name_length = 8;
#else
	const uint8_t* variable_name = (const uint8_t*)"LOGNAME";
	const uint8_t variable_name_length = 7;
#endif
	buffer result;
	SET_NULL_TO_BUFFER(result);
	//
	ASSERT_TRUE(environment_get_user_name(&result)) << buffer_free(&result);
	//
	buffer expected_result;
	SET_NULL_TO_BUFFER(expected_result);
	//
	const std::string returned(buffer_to_string(&result));
	ASSERT_TRUE(
		environment_get_variable(variable_name, variable_name + variable_name_length, &expected_result))
			<< "Expected value for variable is '" << returned << "'" << std::endl
			<< buffer_free(&expected_result) << buffer_free(&result);
	const std::string expected_return(buffer_to_string(&expected_result));
	ASSERT_EQ(expected_return, returned) << buffer_free(&expected_result) << buffer_free(&result);
	//
	buffer_release(&expected_result);
	buffer_release(&result);
}

TEST(TestEnvironment_, environment_newline)
{
	buffer newline;
	SET_NULL_TO_BUFFER(newline);
	ASSERT_TRUE(environment_newline(&newline)) << buffer_free(&newline);
	ASSERT_TRUE(buffer_push_back(&newline, '\0')) << buffer_free(&newline);
#if defined(_WIN32)
	ASSERT_STREQ("\r\n", buffer_char_data(&newline, 0)) << buffer_free(&newline);
#else
	ASSERT_STREQ("\n", buffer_char_data(&newline, 0)) << buffer_free(&newline);
#endif
	buffer_release(&newline);
}

TEST(TestEnvironment_, environment_is64bit)
{
	const uint8_t is64bit_process = environment_is64bit_process();
	const uint8_t is64bit_operating_system = environment_is64bit_operating_system();

	if (is64bit_process)
	{
		ASSERT_EQ(is64bit_process, is64bit_operating_system);
	}

	ASSERT_LT(is64bit_process, 2);
	ASSERT_LT(is64bit_operating_system, 2);
}

TEST_F(TestEnvironment, environment_variable_exists)
{
	for (const auto& node : nodes)
	{
		const uint8_t* variable_name = (const uint8_t*)node.node().select_node("variable_name").node().child_value();
		const uint8_t variable_name_length = (uint8_t)INT_PARSE(
				node.node().select_node("variable_name_length").node().child_value());
		const uint8_t expected_return = (uint8_t)INT_PARSE(
											node.node().select_node("return").node().child_value());
		const uint8_t returned = environment_variable_exists(variable_name, variable_name + variable_name_length);
		ASSERT_EQ(expected_return, returned) << "'" << variable_name << "'" << std::endl;
		//
		--node_count;
	}
}

TEST(TestOperatingSystem, operating_system_at_all)
{
	static const uint8_t zero = '\0';
	//
	buffer input;
	SET_NULL_TO_BUFFER(input);
	//
	ASSERT_TRUE(common_append_string_to_buffer((const uint8_t*)"Microsoft Windows NT 6.2.9200",
				&input)) << buffer_free(&input);
	ASSERT_TRUE(buffer_push_back(&input, zero)) << buffer_free(&input);
	//
	ASSERT_TRUE(common_append_string_to_buffer((const uint8_t*)"OpenBSD 6.5 GENERIC.MP#3 amd64",
				&input)) << buffer_free(&input);
	ASSERT_TRUE(buffer_push_back(&input, zero)) << buffer_free(&input);
	//
	const PlatformID expected_platforms[] = { Win32, Unix };
	const Version expected_versions[] = { {6, 2, 9200, 0}, {6, 5, 0, 0} };
	//
	range input_in_range = buffer_to_range(&input);
	const uint8_t* pos = input_in_range.start;
	uint8_t i = 0;

	while (input_in_range.finish != (pos = find_any_symbol_like_or_not_like_that(pos,
										   input_in_range.finish,
										   &zero, 1, 1, 1)))
	{
		ASSERT_LT(i, COUNT_OF(expected_platforms)) << buffer_free(&input);
		ASSERT_LT(i, COUNT_OF(expected_versions)) << buffer_free(&input);
		//
		OperatingSystem os;
		ASSERT_TRUE(operating_system_parse(input_in_range.start, pos, &os)) << buffer_free(&input);
		ASSERT_EQ(expected_platforms[i], operating_system_get_platform(&os)) << buffer_free(&input);
		//
		const Version returned_version = operating_system_get_version(&os);
		ASSERT_EQ(expected_versions[i].major, returned_version.major) << buffer_free(&input);
		ASSERT_EQ(expected_versions[i].minor, returned_version.minor) << buffer_free(&input);
		ASSERT_EQ(expected_versions[i].build, returned_version.build) << buffer_free(&input);
		ASSERT_EQ(expected_versions[i].revision, returned_version.revision) << buffer_free(&input);
		//
		ASSERT_STREQ((const char*)input_in_range.start,
					 (const char*)operating_system_to_string(&os)) << buffer_free(&input);
		//
		++i;
		++pos;
		input_in_range.start = find_any_symbol_like_or_not_like_that(pos, input_in_range.finish, &zero, 1, 0, 1);
	}

	buffer_release(&input);
}

TEST(TestPlatform, platform_at_all)
{
	ASSERT_NE(nullptr, platform_get_name());
	ASSERT_NE(platform_is_unix(), platform_is_windows());
}