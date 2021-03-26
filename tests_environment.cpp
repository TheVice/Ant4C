/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 https://github.com/TheVice/
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
#include "path.h"
#include "range.h"
#include "string_unit.h"
#include "version.h"
};

#include <string>
#include <cstddef>
#include <cstdint>
#include <cstring>

#if !defined(_WIN32)
#define _POSIXSOURCE 1
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
	const void* os = environment_get_operating_system();
	ASSERT_NE(nullptr, os);
#if defined(_WIN32)
	ASSERT_EQ(Win32, operating_system_get_platform(os));
#else
	ASSERT_EQ(Unix, operating_system_get_platform(os));
#endif
	ASSERT_LT(0, common_count_bytes_until(operating_system_to_string(os), 0));
}

TEST(TestEnvironment_, environment_get_user_name)
{
#if defined(_WIN32)
	const uint8_t* names[] = { (const uint8_t*)"USERNAME" };
	const uint8_t lengths[] = { 8 };
#else
	const uint8_t* names[] = { (const uint8_t*)"LOGNAME", (const uint8_t*)"HOME" };
	const uint8_t lengths[] = { 7, 4 };
#endif
	buffer result;
	SET_NULL_TO_BUFFER(result);
	//
	ASSERT_TRUE(environment_get_user_name(&result)) << buffer_free(&result);
	ASSERT_LT(0L, buffer_size(&result)) << buffer_free(&result);
	const auto returned(buffer_to_string(&result));

	for (uint8_t i = 0, count = COUNT_OF(names); i < count; ++i)
	{
		ASSERT_TRUE(buffer_resize(&result, 0))
				<< "'" << returned << "'" << std::endl << buffer_free(&result);

		if (environment_get_variable(names[i], names[i] + lengths[i], &result))
		{
			break;
		}
	}

	ASSERT_LT(0L, buffer_size(&result))
			<< "'" << returned << "'" << std::endl << buffer_free(&result);
	//
	auto variable_value(buffer_to_range(&result));
	auto expected_result = range_to_string(variable_value);
	//
	ASSERT_TRUE(
		path_get_file_name(variable_value.start, variable_value.finish, &variable_value))
			<< "'" << returned << "'" << std::endl
			<< "'" << expected_result << "'" << std::endl
			<< buffer_free(&result);
	//
	expected_result = range_to_string(variable_value);
	buffer_release(&result);
	//
	ASSERT_EQ(expected_result, returned);
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

#if defined(_WIN32)
#define OS_SIZE UINT8_MAX
#else
#define OS_SIZE sizeof(struct utsname) + INT8_MAX
#endif

TEST_F(TestOperatingSystem, operating_system_init)
{
	for (const auto& node : nodes)
	{
		const auto platformID = (uint8_t)INT_PARSE(
									node.node().select_node("platformID").node().child_value());
#if defined(_WIN32)
		const std::string is_server_value(node.node().select_node("is_server").node().child_value());
		uint8_t is_server = 0;

		if (!is_server_value.empty())
		{
			ASSERT_TRUE
			(bool_parse(reinterpret_cast<const uint8_t*>(is_server_value.data()), is_server_value.size(), &is_server));
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
			reinterpret_cast<const uint8_t*>(value_1.data()),
			reinterpret_cast<const uint8_t*>(value_2.data()),
			reinterpret_cast<const uint8_t*>(value_3.data()),
			reinterpret_cast<const uint8_t*>(value_4.data())
		};
#endif
		const uint8_t* ptr_version = NULL;
		uint8_t version[VERSION_SIZE];
		const std::string version_str(node.node().select_node("version").node().child_value());

		if (!version_str.empty())
		{
			const auto version_str_range(string_to_range(version_str));
			ASSERT_TRUE(version_parse(version_str_range.start, version_str_range.finish, version));
			ptr_version = version;
		}

		uint8_t os[OS_SIZE];
#if !defined(_WIN32)
		ASSERT_TRUE(operating_system_init(platformID, ptr_version, version_string, OS_SIZE, os));
#else
		ASSERT_TRUE(operating_system_init(platformID, is_server, ptr_version, OS_SIZE, os));
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
			std::string returned_os_information((const char*)ptr_version);
			std::string expected;

			for (auto i = 0; i < 4; ++i)
			{
				expected += (const char*)version_string[i];
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

TEST(TestOperatingSystem_, operating_system_at_all)
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
	//
	const uint8_t expected_majors[] = { 6, 6 };
	const uint8_t expected_minors[] = { 2, 5 };
	const uint16_t expected_builds[] = { 9200, 0 };
	const uint8_t expected_revisions[] = { 0, 0 };
	//
	range input_in_range = buffer_to_range(&input);
	const uint8_t* pos = input_in_range.start;
	uint8_t i = 0;

	while (input_in_range.finish != (pos = find_any_symbol_like_or_not_like_that(pos,
										   input_in_range.finish,
										   &zero, 1, 1, 1)))
	{
		ASSERT_LT(i, COUNT_OF(expected_platforms)) << buffer_free(&input);
		ASSERT_LT(i, COUNT_OF(expected_majors)) << buffer_free(&input);
		//
		uint8_t os[OS_SIZE];
		ASSERT_TRUE(operating_system_parse(input_in_range.start, pos, OS_SIZE, os)) << buffer_free(&input);
		ASSERT_EQ(expected_platforms[i], operating_system_get_platform(os)) << buffer_free(&input);
		//
		const auto returned_version = operating_system_get_version(os);
		ASSERT_NE(nullptr, returned_version) << buffer_free(&input);
		ASSERT_EQ(expected_majors[i], version_get_major(returned_version)) << buffer_free(&input);
		ASSERT_EQ(expected_minors[i], version_get_minor(returned_version)) << buffer_free(&input);
		ASSERT_EQ(expected_builds[i], version_get_build(returned_version)) << buffer_free(&input);
		ASSERT_EQ(expected_revisions[i], version_get_revision(returned_version)) << buffer_free(&input);
		//
		ASSERT_STREQ((const char*)input_in_range.start,
					 (const char*)operating_system_to_string(os)) << buffer_free(&input);
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
