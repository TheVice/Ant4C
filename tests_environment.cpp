/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#if !defined(_WIN32)
#if defined(__linux)
#define _POSIX_SOURCE 1
#else
#define _BSD_SOURCE 1
#endif
#endif

#include "tests_base_xml.h"

extern "C" {
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "environment.h"
#include "interpreter.h"
#include "operating_system.h"
#include "path.h"
#include "range.h"
#include "string_unit.h"
#include "version.h"
};

#include <string>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ostream>
#include <iostream>

#if !defined(_WIN32)
#include <sys/utsname.h>
#endif

class TestEnvironment : public TestsBaseXml
{
};

class TestOperatingSystem : public TestsBaseXml
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
		ASSERT_TRUE(environment_get_folder_path(input[i], &path))
				<< static_cast<uint32_t>(i) << std::endl
				<< buffer_free(&path);

		if (!expected_result[i].empty())
		{
			const auto size = buffer_size(&path);
			const auto code(string_to_range(expected_result[i]));
			ASSERT_TRUE(interpreter_evaluate_code(nullptr, nullptr, &code, &path, verbose))
					<< "'" << expected_result[i] << "'" << std::endl
					<< buffer_free(&path);
			//
			auto path_str(buffer_to_string(&path));
			ASSERT_FALSE(path_str.empty())
					<< static_cast<uint32_t>(i) << std::endl << buffer_free(&path);
			ASSERT_EQ(2 * size, buffer_size(&path))
					<< static_cast<uint32_t>(i) << std::endl
					<< path_str << std::endl << buffer_free(&path);
			//
			ASSERT_TRUE(buffer_resize(&path, 0)) << buffer_free(&path);
			const auto path_in_a_range(string_to_range(path_str));
			ASSERT_TRUE(string_to_lower(path_in_a_range.start, path_in_a_range.finish, &path))
					<< path_str << std::endl << buffer_free(&path);
			path_str = buffer_to_string(&path);
			//
			ASSERT_EQ(2 * size, static_cast<ptrdiff_t>(path_str.size()))
					<< static_cast<uint32_t>(i) << std::endl
					<< path_str << std::endl << buffer_free(&path);
			ASSERT_EQ(path_str.substr(size), path_str.substr(0, size))
					<< static_cast<uint32_t>(i) << std::endl << buffer_free(&path);
		}
		else
		{
			const auto path_str(buffer_to_string(&path));
			ASSERT_EQ(0, buffer_size(&path))
					<< static_cast<uint32_t>(i)
					<< " '" << path_str << "'" << std::endl
					<< buffer_free(&path);
		}
	}

	buffer_release(&path);
}

TEST(TestEnvironment_, environment_get_machine_name)
{
#if defined(_WIN32)
	const std::string variable_name("COMPUTERNAME");
#endif
	buffer result;
	SET_NULL_TO_BUFFER(result);
	//
	ASSERT_TRUE(environment_get_machine_name(&result)) << buffer_free(&result);
#if defined(_WIN32)
	buffer expected_result;
	SET_NULL_TO_BUFFER(expected_result);
	//
	const auto returned(buffer_to_string(&result));
	const auto variable_name_in_a_range(string_to_range(variable_name));
	ASSERT_TRUE(
		environment_get_variable(variable_name_in_a_range.start, variable_name_in_a_range.finish, &expected_result))
			<< "Expected value for variable is '" << returned << "'" << std::endl
			<< buffer_free(&expected_result) << buffer_free(&result);
	const auto expected_return(buffer_to_string(&expected_result));
	ASSERT_EQ(expected_return, returned) << buffer_free(&expected_result) << buffer_free(&result);
	//
	buffer_release(&expected_result);
#endif
	buffer_release(&result);
}

TEST(TestEnvironment_, environment_get_operating_system)
{
	const auto os = environment_get_operating_system();
	ASSERT_NE(nullptr, os);
#if defined(_WIN32)
	ASSERT_EQ(Win32, operating_system_get_platform(os));
#elif (defined(__APPLE__) && defined(__MACH__))
	ASSERT_EQ(macOS, operating_system_get_platform(os));
#else
	ASSERT_EQ(Unix, operating_system_get_platform(os));
#endif
	ASSERT_LT(0, common_count_bytes_until(operating_system_to_string(os), 0));
}

TEST(TestEnvironment_, environment_get_user_name)
{
#if defined(_WIN32)
	const std::string names[] = { "USERNAME" };
#else
	const std::string names[] = { "LOGNAME", "HOME" };
#endif
	buffer output;
	SET_NULL_TO_BUFFER(output);
	//
	ASSERT_TRUE(environment_get_user_name(&output)) << buffer_free(&output);
	ASSERT_LT(0L, buffer_size(&output)) << buffer_free(&output);
	const auto user_name(buffer_to_string(&output));

	for (uint8_t i = 0, count = COUNT_OF(names); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&output, 0))
				<< "'" << user_name << "'" << std::endl
				<< buffer_free(&output);
		const auto name_in_a_range(string_to_range(names[i]));

		if (environment_get_variable(name_in_a_range.start, name_in_a_range.finish, &output))
		{
			break;
		}
	}

	ASSERT_LT(0L, buffer_size(&output))
			<< "'" << user_name << "'" << std::endl
			<< buffer_free(&output);
	//
	auto variable_value(buffer_to_range(&output));
	auto expected_user_name = range_to_string(variable_value);

	if (string_contains(variable_value.start, variable_value.finish, &PATH_DELIMITER, &PATH_DELIMITER))
	{
		ASSERT_TRUE(
			path_get_file_name(variable_value.start, variable_value.finish, &variable_value))
				<< "'" << user_name << "'" << std::endl
				<< "'" << expected_user_name << "'" << std::endl
				<< buffer_free(&output);
		expected_user_name = range_to_string(variable_value);
	}

	buffer_release(&output);

	if (expected_user_name != user_name)
	{
		std::cout << "Could not get username via environment variable." << std::endl;
		std::cout << "Comparing will not be provided." << std::endl;
		std::cout << "User name: '" << user_name << "'." << std::endl;
		std::cout << "Expected user name: '" << expected_user_name << "'." << std::endl;
	}
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
	const auto is64bit_process = environment_is64bit_process();
	const auto is64bit_operating_system = environment_is64bit_operating_system();

	if (is64bit_process)
	{
		ASSERT_EQ(is64bit_process, is64bit_operating_system);
	}

	ASSERT_LT(is64bit_process, 2);
	ASSERT_LT(is64bit_operating_system, 2);
}

TEST(TestEnvironment_, environment_processor_count)
{
	ASSERT_LT(0, environment_processor_count());
}

TEST_F(TestEnvironment, environment_variable_exists)
{
	buffer variable;
	SET_NULL_TO_BUFFER(variable);

	for (const auto& node : nodes)
	{
		ASSERT_TRUE(buffer_resize(&variable, 0))
				<< buffer_free(&variable);
		const auto variable_name = reinterpret_cast<const uint8_t*>(
									   node.node().select_node("variable_name").node().child_value());
		//
		const std::string variable_name_length_str(
			node.node().select_node("variable_name_length").node().child_value());
		auto input_in_a_range = string_to_range(variable_name_length_str);
		const auto variable_name_length =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const std::string return_str(node.node().select_node("return").node().child_value());
		input_in_a_range = string_to_range(return_str);
		const auto expected_return =
			static_cast<uint8_t>(int_parse(input_in_a_range.start, input_in_a_range.finish));
		//
		const auto returned = environment_variable_exists(variable_name, variable_name + variable_name_length);
		ASSERT_EQ(expected_return, returned)
				<< "'" << variable_name << "'" << std::endl << buffer_free(&variable);
		//
		ASSERT_EQ(expected_return, environment_get_variable(
					  variable_name, variable_name + variable_name_length, &variable))
				<< "'" << variable_name << "'" << std::endl << buffer_free(&variable);
		//
		--node_count;
	}

	buffer_release(&variable);
}

TEST_F(TestOperatingSystem, operating_system_init)
{
	uint8_t os[OPERATING_SYSTEM_SIZE];

	for (const auto& node : nodes)
	{
		const std::string platformID_str(node.node().select_node("platformID").node().child_value());
		const auto platformID_in_a_range(string_to_range(platformID_str));
		const auto platformID =
			static_cast<uint8_t>(int_parse(platformID_in_a_range.start, platformID_in_a_range.finish));
#if defined(_WIN32)
		const std::string is_server_value(node.node().select_node("is_server").node().child_value());
		uint8_t is_server = 0;

		if (!is_server_value.empty())
		{
			const auto is_server_value_in_a_range(string_to_range(is_server_value));
			ASSERT_TRUE(bool_parse(is_server_value_in_a_range.start, is_server_value_in_a_range.finish, &is_server));
		}

#else
		const auto version_string_node = node.node().select_node("version_string").node();
		//
		const std::string value_1(version_string_node.attribute("value_1").as_string());
		const std::string value_2(version_string_node.attribute("value_2").as_string());
		const std::string value_3(version_string_node.attribute("value_3").as_string());
		const std::string value_4(version_string_node.attribute("value_4").as_string());
		//
		const uint8_t* version_string[] =
		{
			reinterpret_cast<const uint8_t*>(value_1.c_str()),
			reinterpret_cast<const uint8_t*>(value_2.c_str()),
			reinterpret_cast<const uint8_t*>(value_3.c_str()),
			reinterpret_cast<const uint8_t*>(value_4.c_str())
		};
#endif
		const uint8_t* ptr_version = nullptr;
		uint8_t version[VERSION_SIZE];
		const std::string version_str(node.node().select_node("version").node().child_value());

		if (!version_str.empty())
		{
			const auto version_in_a_range(string_to_range(version_str));
			ASSERT_TRUE(version_parse(version_in_a_range.start, version_in_a_range.finish, version));
			ptr_version = version;
		}

#if !defined(_WIN32)
		ASSERT_TRUE(operating_system_init(platformID, ptr_version, version_string, OPERATING_SYSTEM_SIZE, os));
#else
		ASSERT_TRUE(operating_system_init(platformID, is_server, ptr_version, OPERATING_SYSTEM_SIZE, os));
#endif
		ASSERT_EQ(platformID, operating_system_get_platform(os));
#if defined(_WIN32)
		ASSERT_EQ(is_server, operating_system_is_windows_server(os));
#endif
		const auto returned_version = operating_system_get_version(os);

		if (version_str.empty())
		{
			ASSERT_EQ(0ul, version_get_major(returned_version));
			ASSERT_EQ(0ul, version_get_minor(returned_version));
			ASSERT_EQ(0ul, version_get_build(returned_version));
			ASSERT_EQ(0ul, version_get_revision(returned_version));
#if !defined(_WIN32)
			ptr_version = operating_system_to_string(os);
			std::string returned_os_information(reinterpret_cast<const char*>(ptr_version));
			std::string expected;

			for (uint8_t i = 0; i < 4; ++i)
			{
				expected += reinterpret_cast<const char*>(version_string[i]);
			}

			ASSERT_EQ(expected, returned_os_information);
#endif
		}
		else
		{
			ASSERT_EQ(version_get_major(ptr_version), version_get_major(returned_version));
			ASSERT_EQ(version_get_minor(ptr_version), version_get_minor(returned_version));
			ASSERT_EQ(version_get_build(ptr_version), version_get_build(returned_version));
			ASSERT_EQ(version_get_revision(ptr_version), version_get_revision(returned_version));
		}

		--node_count;
	}
}

TEST_F(TestOperatingSystem, operating_system_parse)
{
	uint8_t os[OPERATING_SYSTEM_SIZE];

	for (const auto& node : nodes)
	{
		std::string input(node.node().select_node("input").node().child_value());
		//
		const auto output_node(node.node().select_node("output").node());
		const auto expected_platform = output_node.attribute("platform").as_int();
		const auto expected_is_server = output_node.attribute("is_server").as_bool();
		const auto expected_major = output_node.attribute("major").as_uint();
		const auto expected_minor = output_node.attribute("minor").as_uint();
		const auto expected_build = output_node.attribute("build").as_uint();
		const auto expected_revision = output_node.attribute("revision").as_uint();
		//
		const auto input_in_a_range(string_to_range(input));
		ASSERT_TRUE(operating_system_parse(
						input_in_a_range.start, input_in_a_range.finish, OPERATING_SYSTEM_SIZE, os)) << input;
		//
		ASSERT_EQ(expected_platform, operating_system_get_platform(os)) << input;
		ASSERT_EQ(expected_is_server, operating_system_is_windows_server(os)) << input;
		//
		const auto returned_version = operating_system_get_version(os);
		ASSERT_NE(nullptr, returned_version) << input;
		ASSERT_EQ(expected_major, version_get_major(returned_version)) << input;
		ASSERT_EQ(expected_minor, version_get_minor(returned_version)) << input;
		ASSERT_EQ(expected_build, version_get_build(returned_version)) << input;
		ASSERT_EQ(expected_revision, version_get_revision(returned_version)) << input;
		//
		ASSERT_STREQ(input.c_str(), reinterpret_cast<const char*>(operating_system_to_string(os))) << input;
		//
		--node_count;
	}
}

TEST(TestPlatform_, platform_get_name)
{
	const std::string platform_name(
		reinterpret_cast<const char*>(platform_get_name()));
	ASSERT_FALSE(platform_name.empty());
#if defined(_WIN32)
	static const std::string win32_str("win32");
	ASSERT_EQ(win32_str, platform_name);
#elif (defined(__APPLE__) && defined(__MACH__))
	static const std::string macos_str("macos");
	ASSERT_EQ(macos_str, platform_name);
#else
	static const std::string unix_str("unix");
	ASSERT_EQ(unix_str, platform_name);
#endif
}

TEST(TestPlatform_, platform_at_all)
{
	if (platform_is_macos())
	{
		ASSERT_EQ(platform_is_macos(), platform_is_unix());
		ASSERT_NE(platform_is_macos(), platform_is_windows());
	}

	ASSERT_NE(platform_is_unix(), platform_is_windows());

	if (platform_is_unix())
	{
		ASSERT_NE(platform_is_unix(), platform_is_windows_server());
	}
}
