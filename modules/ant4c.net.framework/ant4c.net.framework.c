/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 TheVice
 *
 */

#include "ant4c.net.framework.h"

#include "buffer.h"
#include "conversion.h"
#include "file_system.h"
#include "text_encoding.h"

#include "meta_host.h"
#include "clr_control.h"
#include "runtime_info.h"
#include "runtime_host.h"
#include "host_controller.h"

#include "echo.h"

#include <unknwn.h>

static struct buffer output_data;
static uint8_t is_buffer_initialized = 0;

static void* the_runtime = NULL;
static void* clr_host = NULL;

#define IUNKNOWN_RELEASE(A)								\
	if (A)												\
	{													\
		((struct IUnknown*)(A))->lpVtbl->Release(A);	\
		(A) = NULL;										\
	}

uint8_t runtime_init()
{
	if (the_runtime)
	{
		return 1;
	}

	if (meta_host_get_runtime_v4(&the_runtime) ||
		meta_host_get_runtime_v2(&the_runtime))
	{
		return 1;
	}

	return 0;
}

uint8_t clr_host_init()
{
	if (clr_host)
	{
		return 1;
	}

	if (!runtime_init() ||
		!runtime_info_get_interface_of_clr_runtime_host(the_runtime, &clr_host))
	{
		return 0;
	}

	return 1;
}

uint8_t metahost_get_clr_version_from_file(const uint8_t* path, uint16_t length)
{
	if (!path ||
		!length)
	{
		return 0;
	}

	return meta_host_get_version_from_file(path, path + length, &output_data);
}

uint8_t file_is_assembly(const uint8_t* path, uint16_t length, uint8_t* returned)
{
	if (!path ||
		!length ||
		!returned)
	{
		return 0;
	}

	if (!metahost_get_clr_version_from_file(path, length))
	{
		return 0;
	}

	*returned = 0 < buffer_size(&output_data);
	return buffer_resize(&output_data, 0);
}

#define FRAMEWORK_GET_FRAMEWORK_DIRECTORY() (runtime_init() && runtime_info_get_runtime_directory(the_runtime, &output_data))

#define FRAMEWORK_GET_FRAMEWORKS() meta_host_enumerate_installed_runtimes(&output_data)

#define FRAMEWORK_GET_CLR_VERSION() (runtime_init() && runtime_info_get_version_string(the_runtime, &output_data))

static GUID manager_id;
static uint8_t is_host_init = 0;
static struct HostControllerPtr host_controller_ptr;
static struct HostController host_controller;

uint8_t host_init()
{
	if (is_host_init)
	{
		return 1;
	}

	/*"49467261-6D65-776F-726B-4E616D657370"*/
	manager_id.Data1 = 1229353569;
	manager_id.Data2 = 28005;
	manager_id.Data3 = 30575;
	manager_id.Data4[0] = 114;
	manager_id.Data4[1] = 107;
	manager_id.Data4[2] = 78;
	manager_id.Data4[3] = 97;
	manager_id.Data4[4] = 109;
	manager_id.Data4[5] = 101;
	manager_id.Data4[6] = 115;
	manager_id.Data4[7] = 112;
	/**/
	host_controller.ptr = &host_controller_ptr;
	/**/
	is_host_init = host_control_init(&host_controller, &manager_id);
	/**/
	return is_host_init;
}

struct FrameworkNamespaceInterface;

struct FrameworkNamespaceInterfacePtr
{
	IUNKNOWN_METHODS(struct FrameworkNamespaceInterface)

	HRESULT(STDMETHODCALLTYPE* GetRuntimeFramework)(
		struct FrameworkNamespaceInterface* this_, const wchar_t** result);

	HRESULT(STDMETHODCALLTYPE* Exists)(
		struct FrameworkNamespaceInterface* this_, const wchar_t* framework, uint8_t* result);
};

struct FrameworkNamespaceInterface
{
	struct FrameworkNamespaceInterfacePtr* ptr;
};

struct FrameworkNamespaceInterface* framework = NULL;

struct FrameworkNamespaceInterface* get_framework()
{
	if (framework)
	{
		return framework;
	}

	if (!host_init() || !clr_host_init())
	{
		return NULL;
	}

	if (!runtime_host_set_host_control(clr_host, &host_controller))
	{
		return NULL;
	}

	void* clr_control = NULL;

	if (!runtime_host_get_clr_control(clr_host, &clr_control))
	{
		return NULL;
	}

	if (!clr_control_set_app_domain_manager_type(clr_control,
			(const uint8_t*)"ant4c.net.framework.module.clr",
			(const uint8_t*)"Ant4C.Net.Framework.Module.CustomAppDomainManager"))
	{
		return NULL;
	}

	IUNKNOWN_RELEASE(clr_control);

	if (!runtime_host_start(clr_host))
	{
		return NULL;
	}

	framework = (struct FrameworkNamespaceInterface*)host_controller.ptr->GetDomainManager(&host_controller);

	if (!framework)
	{
		return NULL;
	}

	return framework;
}

uint8_t framework_get_function_return(const wchar_t* version)
{
	if (!get_framework())
	{
		return 0;
	}

	uint8_t exists = 0;
	const wchar_t* runtime_framework = NULL;
	const HRESULT result = version ?
						   framework->ptr->Exists(framework, version, &exists) :
						   framework->ptr->GetRuntimeFramework(framework, &runtime_framework);

	if (FAILED(result))
	{
		return 0;
	}

	if (version)
	{
		return buffer_resize(&output_data, 0) && bool_to_string(exists, &output_data);
	}
	else
	{
		const wchar_t* finish = runtime_framework;

		while (L'\0' != (*finish))
		{
			++finish;
		}

		if (!buffer_push_back(&output_data, 'v') ||
			!text_encoding_UTF16LE_to_UTF8(runtime_framework, finish, &output_data) ||
			!buffer_push_back(&output_data, 0))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t metahost_runtime(const uint8_t* input, uint8_t length, uint8_t* returned)
{
	if (!input ||
		!length ||
		!returned)
	{
		return 0;
	}

	void* the_runtime_ = NULL;
	*returned = meta_host_get_runtime(input, input + length, &the_runtime_);
	IUNKNOWN_RELEASE(the_runtime_);
	return 1;
}

static const uint8_t* name_spaces[] =
{
	(const uint8_t*)"file",
	(const uint8_t*)"framework",
	(const uint8_t*)"metahost"
};

enum ant4c_net_framework_module_
{
	file_is_assembly_, framework_exists_, framework_get_clr_version_,
	framework_get_framework_directory_, framework_get_frameworks_, framework_get_runtime_framework_,
	metahost_runtime_, metahost_get_clr_version_from_file_
};

static const uint8_t* all_functions[][6] =
{
	{ (const uint8_t*)"is-assembly", NULL },
	{
		(const uint8_t*)"exists", (const uint8_t*)"get-clr-version", (const uint8_t*)"get-framework-directory",
		(const uint8_t*)"get-frameworks", (const uint8_t*)"get-runtime-framework", NULL
	},
	{ (const uint8_t*)"runtime", (const uint8_t*)"get-clr-version-from-file", NULL }
};

const uint8_t* enumerate_name_spaces(ptrdiff_t index)
{
	static const ptrdiff_t count = sizeof(name_spaces) / sizeof(*name_spaces);

	if (index < 0 || count <= index)
	{
		return NULL;
	}

	return name_spaces[index];
}

const uint8_t* enumerate_functions(const uint8_t* name_space, ptrdiff_t index)
{
	static const ptrdiff_t count = sizeof(all_functions) / sizeof(*all_functions);

	if (NULL == name_space)
	{
		return NULL;
	}

	ptrdiff_t i = 0;
	const uint8_t* ptr = NULL;

	while (NULL != (ptr = enumerate_name_spaces(i++)))
	{
		if (ptr == name_space)
		{
			break;
		}
	}

	if (NULL == ptr || (count <= (i - 1)))
	{
		return NULL;
	}

	const uint8_t** function_from_name_space = all_functions[i - 1];
	i = 0;

	while (NULL != (ptr = function_from_name_space[i++]))
	{
		if (index == (i - 1))
		{
			return ptr;
		}
	}

	return NULL;
}

uint8_t evaluate_function(const uint8_t* function,
						  const uint8_t** values, const uint16_t* values_lengths, uint8_t values_count,
						  const uint8_t** output, uint16_t* output_length)
{
	if (NULL == function ||
		NULL == values ||
		NULL == values_lengths ||
		NULL == output ||
		NULL == output_length)
	{
		return 0;
	}

	if (is_buffer_initialized)
	{
		if (!buffer_resize(&output_data, 0))
		{
			return 0;
		}
	}
	else
	{
		SET_NULL_TO_BUFFER(output_data);
		is_buffer_initialized = 1;
	}

	const uint8_t* ptr = NULL;
	ptrdiff_t function_number = 0;

	for (ptrdiff_t i = 0, count = sizeof(all_functions) / sizeof(*all_functions); i < count; ++i)
	{
		ptrdiff_t j = 0;
		const uint8_t** functions_from_name_space = all_functions[i];

		while (NULL != (ptr = functions_from_name_space[j++]))
		{
			if (function == ptr)
			{
				i = count;
				break;
			}

			++function_number;
		}
	}

	if (NULL == ptr)
	{
		return 0;
	}

	switch (function_number)
	{
		case file_is_assembly_:
			if (1 != values_count ||
				!file_is_assembly(values[0], values_lengths[0], &values_count) ||
				!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case framework_exists_:
			if (1 != values_count ||
				!text_encoding_UTF8_to_UTF16LE(values[0], values[0] + values_lengths[0], &output_data) ||
				!framework_get_function_return(buffer_wchar_t_data(&output_data, 0)))
			{
				return 0;
			}

			break;

		case framework_get_clr_version_:
			if (0 < values_count ||
				!FRAMEWORK_GET_CLR_VERSION())
			{
				return 0;
			}

			break;

		case framework_get_framework_directory_:
			if (1 < values_count ||
				!FRAMEWORK_GET_FRAMEWORK_DIRECTORY())
			{
				return 0;
			}

			break;

		case framework_get_frameworks_:
			if (1 < values_count ||
				!FRAMEWORK_GET_FRAMEWORKS())
			{
				return 0;
			}

			break;

		case framework_get_runtime_framework_:
			if (0 < values_count ||
				!framework_get_function_return(NULL))
			{
				return 0;
			}

			break;

		case metahost_runtime_:
			if (1 != values_count ||
				!metahost_runtime(values[0], (uint8_t)values_lengths[0], &values_count) ||
				!bool_to_string(values_count, &output_data))
			{
				return 0;
			}

			break;

		case metahost_get_clr_version_from_file_:
			if (1 != values_count ||
				!metahost_get_clr_version_from_file(values[0], values_lengths[0]))
			{
				return 0;
			}

			break;

		default:
			break;
	}

	*output = buffer_data(&output_data, 0);
	*output_length = (uint16_t)buffer_size(&output_data);
	/**/
	return 1;
}

void module_release()
{
	if (framework)
	{
		framework->ptr->Release(framework);
		framework = NULL;
	}

	runtime_host_stop(clr_host);

	if (is_host_init)
	{
		host_controller.ptr->Release(&host_controller);
		is_host_init = 0;
	}

	IUNKNOWN_RELEASE(clr_host);
	IUNKNOWN_RELEASE(the_runtime);
	meta_host_release();

	if (is_buffer_initialized)
	{
		buffer_release(&output_data);
	}
}
