/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 https://github.com/TheVice/
 *
 */

#ifndef __RUN_TIME_HOST_H__
#define __RUN_TIME_HOST_H__

#include <stdint.h>

uint8_t runtime_host_start(void* host);
uint8_t runtime_host_stop(void* host);

uint8_t runtime_host_set_host_control(void* host, void* control);
uint8_t runtime_host_get_clr_control(void* host, void** control);

uint8_t runtime_host_unload_app_domain(void* host, unsigned long app_domain_id, uint8_t wait_untile_done);
/*uint8_t runtime_host_execute_in_app_domain(
	void* host, unsigned long app_domain_id, void* call_back, void* cookie);*/
uint8_t runtime_host_get_current_app_domain_id(void* host, unsigned long* app_domain_id);

/*uint8_t runtime_host_execute_application(
	void* host, const uint8_t* app_full_name,
	unsigned long manifest_paths_count, const uint8_t** manifest_paths,
	unsigned long activation_data_count, const uint8_t** activation_data,
	int32_t* return_value);*/

/*
This function can execute next C# Method.

namespace Application
{
	class Library
	{
		static int Method(string argument)
		{
			Console.WriteLine($"Hello {argument}");
			return 123;
		}
	}
}

Sample using:
* const uint8_t* assembly_path = (const uint8_t*)"CSharpLibrary.dll";
* const uint8_t* type_name = (const uint8_t*)"Application.Library";
* const uint8_t* method_name = (const uint8_t*)"Method";
* const uint8_t* argument = (const uint8_t*)"const uint8_t * argument";
* unsigned long return_value = 0;
*
* if (!runtime_host_execute_in_default_app_domain(clr_host, assembly_path, type_name, method_name, argument, &return_value))
* {
* 	return EXIT_FAILURE;
* }
*/
uint8_t runtime_host_execute_in_default_app_domain(
	void* host, const uint8_t* assembly_path,
	const uint8_t* type_name, const uint8_t* method_name,
	const uint8_t* argument, unsigned long* return_value);

#endif
