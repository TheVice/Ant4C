/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "net.file.h"
#include "arguments.h"
#include "net.common.h"
#include "host_fxr.h"
#include "net.host_fxr.h"
#include "net_delegate.h"

#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "path.h"
#include "file_system.h"

struct assembly_argument
{
	const type_of_element* path;
};
/**/
typedef uint8_t(delegate_calling_convention* custom_entry_point_fn)(struct assembly_argument args);

uint8_t file_is_assembly(
	const void* ptr_to_host_fxr_object,
	const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
	struct buffer* output)
{
	if (!ptr_to_host_fxr_object ||
		!values ||
		!values_lengths ||
		(1 != values_count && 2 != values_count) ||
		!output)
	{
		return 0;
	}

	static const uint8_t* hostfxr_name_space = (const uint8_t*)"hostfxr";
	static const uint8_t* ant4c_net_clr = (const uint8_t*)"ant4c.net.module.clr.dll";
	static const uint8_t* get_runtime_delegate = (const uint8_t*)"get-runtime-delegate";
	static const uint8_t get_runtime_delegate_length = 20;

	if (!is_function_exists(
			ptr_to_host_fxr_object, hostfxr_name_space,
			get_runtime_delegate, get_runtime_delegate_length,
			host_fx_resolver_is_function_exists, output))
	{
		return 0;
	}

	uint8_t is_function_exist = 0;

	if (!bool_parse(buffer_data(output, 0), buffer_size(output), &is_function_exist))
	{
		return 0;
	}

	if (!buffer_resize(output, 0))
	{
		return 0;
	}

	if (is_function_exist)
	{
		const void* the_delegate = NULL;

		if (1 == values_count)
		{
			static const uint8_t* json_file_content =
				(const uint8_t*)""								\
				"{\n"											\
				"  \"runtimeOptions\": {\n"						\
				"    \"tfm\": \"netcoreapp3.1\",\n"				\
				"    \"rollForward\": \"LatestMinor\",\n"		\
				"    \"framework\": {\n"						\
				"      \"name\": \"Microsoft.NETCore.App\",\n"	\
				"      \"version\": \"3.1.0\" \n"				\
				"    }\n"										\
				"  }\n"											\
				"}\n";

			if (!common_append_string_to_buffer(json_file_content, output))
			{
				return 0;
			}

			values_count = (uint8_t)buffer_size(output);

			if (!path_get_temp_file_name(output))
			{
				return 0;
			}

			if (!buffer_resize(output, buffer_size(output) - 2) ||
				!common_append_string_to_buffer((const uint8_t*)".json", output) ||
				!buffer_push_back(output, 0))
			{
				return 0;
			}

#if defined(_MSC_VER) && (_MSC_VER < 1920)
			const uint8_t* sub_values[6];
			sub_values[0] = NULL;
			sub_values[1] = NULL;
			sub_values[2] = buffer_data(output, values_count);
			sub_values[3] = NULL;
			sub_values[4] = NULL;
			sub_values[5] = NULL;
			/**/
			uint16_t sub_values_lengths[6];
			memset(sub_values_lengths, 0, sizeof(sub_values_lengths));
			sub_values_lengths[2] = (uint16_t)(buffer_size(output) - values_count);
#else
			const uint8_t* sub_values[] = { NULL, NULL, buffer_data(output, values_count), NULL, NULL, NULL };
			uint16_t sub_values_lengths[] = { 0, 0, (uint16_t)(buffer_size(output) - values_count), 0, 0, 0 };
#endif

			if (!buffer_resize(output, values_count))
			{
				return 0;
			}

			if (!file_write_all(sub_values[2], output))
			{
				return 0;
			}

			struct buffer sub_output;

			SET_NULL_TO_BUFFER(sub_output);

			if (!hostfxr_initialize_for_runtime_config(
					ptr_to_host_fxr_object,
					sub_values, sub_values_lengths, 3, &sub_output))
			{
				buffer_release(&sub_output);
				return 0;
			}

			sub_values[0] = buffer_data(&sub_output, 0);

			if (!sub_values[0] ||
				0 == *(sub_values[0]))
			{
				buffer_release(&sub_output);
				return 0;
			}

			const void* context = pointer_parse(sub_values[0]);

			if (!buffer_resize(output, 0) ||
				!buffer_append_data_from_buffer(output, &sub_output))
			{
				host_fxr_close(ptr_to_host_fxr_object, context);
				buffer_release(&sub_output);
				return 0;
			}

			sub_values[0] = buffer_data(output, 0);
			sub_values[1] = net_delegate_types_str[net_hdt_load_assembly_and_get_function_pointer];
			sub_values[2] = ant4c_net_clr;
			sub_values[3] = (const uint8_t*)"Ant4C.Net.Module.Delegates, ant4c.net.module.clr";
			sub_values[4] = (const uint8_t*)"FileUnit_IsAssembly";
			sub_values[5] = (const uint8_t*)
							"Ant4C.Net.Module.Delegates+FileUnit_IsAssemblyDelegate, ant4c.net.module.clr";
			/**/
			sub_values_lengths[0] = (uint16_t)buffer_size(output);

			for (uint8_t i = 1; i < 6; ++i)
			{
				sub_values_lengths[i] = (uint16_t)common_count_bytes_until(sub_values[i], 0);
			}

			if (!buffer_resize(&sub_output, 0) ||
				!hostfxr_get_runtime_delegate(ptr_to_host_fxr_object, sub_values, sub_values_lengths, 6, &sub_output))
			{
				host_fxr_close(ptr_to_host_fxr_object, context);
				buffer_release(&sub_output);
				return 0;
			}

			sub_values[0] = buffer_data(&sub_output, 0);

			if (!sub_values[0] ||
				0 == *(sub_values[0]))
			{
				host_fxr_close(ptr_to_host_fxr_object, context);
				buffer_release(&sub_output);
				return 0;
			}

			the_delegate = pointer_parse(sub_values[0]);
			buffer_release(&sub_output);
			const int32_t result = host_fxr_close(ptr_to_host_fxr_object, context);

			if (IS_HOST_FAILED(result))
			{
				return 0;
			}
		}
		else
		{
			the_delegate = string_to_pointer(values[1], (uint8_t)values_lengths[1], output);
		}

		if (!the_delegate)
		{
			return 0;
		}

		if (!buffer_resize(output, 0) ||
			!value_to_system_path(values[0], values_lengths[0], output))
		{
			return 0;
		}

		struct assembly_argument custom_argument;

#if defined(_WIN32)
		custom_argument.path = (const type_of_element*)buffer_data(output, (ptrdiff_t)1 + values_lengths[0]);

#else
		custom_argument.path = (const type_of_element*)buffer_data(output, 0);

#endif
		if (!custom_argument.path)
		{
			return 0;
		}

#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(suppress: 4055)
		const uint8_t is_assembly = ((custom_entry_point_fn)the_delegate)(custom_argument);
#else
		const uint8_t is_assembly = ((custom_entry_point_fn)the_delegate)(custom_argument);
#endif

		if (!buffer_resize(output, 0) ||
			!bool_to_string(is_assembly, output))
		{
			return 0;
		}
	}
	else
	{
		const type_of_element* argv[5];
#if defined(_WIN32)
		argv[0] = L"";
		argv[2] = L"file";
		argv[3] = L"is-assembly";
#else
		argv[0] = (const uint8_t*)"";
		argv[2] = (const uint8_t*)"file";
		argv[3] = (const uint8_t*)"is-assembly";
#endif

		if (1 == values_count)
		{
			if (!value_to_system_path(values[0], values_lengths[0], output))
			{
				return 0;
			}

#if defined(_WIN32)
			argv[1] = L"ant4c.net.module.clr.dll";
			argv[4] = (const type_of_element*)buffer_data(output, (ptrdiff_t)1 + values_lengths[0]);
#else
			argv[1] = ant4c_net_clr;
			argv[4] = buffer_data(output, 0);
#endif
		}
		else
		{
			ptrdiff_t positions[2];

			if (!values_to_system_paths(values, values_lengths, positions, values_count, output))
			{
				return 0;
			}

			argv[1] = (const type_of_element*)buffer_data(output, positions[1]);
			argv[4] = (const type_of_element*)buffer_data(output, positions[0]);
		}

		values_count = COUNT_OF(argv);
		const int32_t result = host_fxr_main(ptr_to_host_fxr_object, values_count, argv);

		if (!buffer_resize(output, 0) ||
			!bool_to_string((uint8_t)result, output))
		{
			return 0;
		}
	}

	return 1;
}

