/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

extern "C" {
#include "host_fxr.h"
};

#include <string>

#include <cstdlib>
#include <cstdint>

#include <fstream>
#include <ostream>
#include <iostream>

#define HOST_FX_RESOLVER_NON_SUCCESS_(RESULT) ((RESULT) < static_cast<int32_t>(host_fxr_Success) || static_cast<int32_t>(host_fxr_Success_DifferentRuntimeProperties) < (RESULT))

std::string create_json_config(const char* tfm, const char* framework_version)
{
	std::string content;
	content += "{\n";
	content += "  \"runtimeOptions\": {\n";
	content += "    \"tfm\": \"";
	content += tfm;
	content += "\",\n";
	content += "    \"rollForward\": \"LatestMinor\",\n";
	content += "    \"framework\": {\n";
	content += "      \"name\": \"Microsoft.NETCore.App\",\n";
	content += "      \"version\": \"";
	content += framework_version;
	content += "\"\n";
	content += "    }\n";
	content += "  }\n";
	content += "}";
	return content;
}

#if defined(_MSC_VER)
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
	if (argc < 2)
	{
		std::cout << "No input data." << std::endl;
		return EXIT_FAILURE;
	}

	{
		const auto json_config = create_json_config("netcoreapp5.0", "5.0.0");//"netcoreapp3.1", "3.1.0"
		std::ofstream json_config_file("1.json");
		json_config_file << json_config;
		json_config_file.close();
	}

	uint8_t ptr_to_host_fxr_object[UINT8_MAX];
	const type_of_element* path_to_host_fxr = reinterpret_cast<const type_of_element*>(argv[1]);
#if defined(_MSC_VER)
	const type_of_element* runtime_config_path = L"1.json";
#else
	const type_of_element* runtime_config_path = reinterpret_cast<const type_of_element*>("1.json");
#endif

	if (!host_fx_resolver_load(path_to_host_fxr, ptr_to_host_fxr_object, sizeof(ptr_to_host_fxr_object)))
	{
		std::cerr << "Failed to load object from '" << path_to_host_fxr << "' " << __LINE__ << std::endl;
		return EXIT_FAILURE;
	}

	const auto function_name = reinterpret_cast<const uint8_t*>("hostfxr_initialize_for_runtime_config");
	const uint8_t function_name_length = 37;

	if (!host_fx_resolver_is_function_exists(ptr_to_host_fxr_object, function_name, function_name_length))
	{
		host_fx_resolver_unload(ptr_to_host_fxr_object);
		std::cerr << "Host '" << path_to_host_fxr << "' do not have '" << function_name << "' function at line " << __LINE__ << std::endl;
		return EXIT_FAILURE;
	}

	type_of_element* path_to_assembly = nullptr;
	type_of_element* path_to_dot_net_root = nullptr;
	void* context;
	//
	int32_t result = host_fxr_initialize_for_runtime_config_parameters_in_parts(ptr_to_host_fxr_object,
					 runtime_config_path, path_to_assembly, path_to_dot_net_root, &context);

	if (HOST_FX_RESOLVER_NON_SUCCESS_(result))
	{
		host_fx_resolver_unload(ptr_to_host_fxr_object);
		std::cerr << "Failed to call '" << function_name << "' function " << result << " at line " << __LINE__ << std::endl;
		return EXIT_FAILURE;
	}

	size_t count = 0;
	type_of_element** keys = NULL;
	type_of_element** values = NULL;
	//
	result = host_fxr_get_runtime_properties(ptr_to_host_fxr_object, context, &count, keys, values);

	if (static_cast<int32_t>(host_fxr_HostApiBufferTooSmall) != result)
	{
		result = host_fxr_close(ptr_to_host_fxr_object, context);

		if (HOST_FX_RESOLVER_NON_SUCCESS_(result))
		{
			host_fx_resolver_unload(ptr_to_host_fxr_object);
			std::cerr << "Failed to call 'close' function " << result << " at line " << __LINE__ << std::endl;
		}

		host_fx_resolver_unload(ptr_to_host_fxr_object);
		std::cerr << "Failed to call 'host_fxr_get_runtime_properties' function " << result << " at line " << __LINE__ << std::endl;
		return EXIT_FAILURE;
	}

	std::string data(2 * count * sizeof(type_of_element*), '\0');
	keys = reinterpret_cast<type_of_element**>(&data[0]);
	values = reinterpret_cast<type_of_element**>(&data[0] + count * sizeof(type_of_element*));
	//
	result = host_fxr_get_runtime_properties(ptr_to_host_fxr_object, context, &count, keys, values);

	if (HOST_FX_RESOLVER_NON_SUCCESS_(result))
	{
		result = host_fxr_close(ptr_to_host_fxr_object, context);

		if (HOST_FX_RESOLVER_NON_SUCCESS_(result))
		{
			host_fx_resolver_unload(ptr_to_host_fxr_object);
			std::cerr << "Failed to call 'close' function " << result << " at line " << __LINE__ << std::endl;
		}

		host_fx_resolver_unload(ptr_to_host_fxr_object);
		std::cerr << "Failed to call 'host_fxr_get_runtime_properties' function " << result << " at line " << __LINE__ << std::endl;
		return EXIT_FAILURE;
	}

#if defined(_MSC_VER)
	std::wstring key(INT8_MAX, L'\0');
	std::wstring value(INT16_MAX, L'\0');
#else
	std::string key(INT8_MAX, '\0');
	std::string value(INT16_MAX, '\0');
#endif

	for (size_t i = 0; i < count; ++i)
	{
		key.clear();
		value.clear();
#if defined(_MSC_VER)
		key += keys[i];
		value += values[i];
		//
		std::wcout << i << std::endl <<
				   L"PROPERTY NAME: '" << key << L"'" << std::endl <<
				   L"PROPERTY VALUE: '" << value << L"'" << std::endl;
#else
		key += reinterpret_cast<const char*>(keys[i]);
		value += reinterpret_cast<const char*>(values[i]);
		//
		std::cout << i << std::endl <<
				  "PROPERTY NAME: '" << key << "'" << std::endl <<
				  "PROPERTY VALUE: '" << value << "'" << std::endl;
#endif
	}

	result = host_fxr_close(ptr_to_host_fxr_object, context);

	if (HOST_FX_RESOLVER_NON_SUCCESS_(result))
	{
		host_fx_resolver_unload(ptr_to_host_fxr_object);
		std::cerr << "Failed to call 'close' function " << result << " at line " << __LINE__ << std::endl;
		return EXIT_FAILURE;
	}

	host_fx_resolver_unload(ptr_to_host_fxr_object);
	return EXIT_SUCCESS;
}
