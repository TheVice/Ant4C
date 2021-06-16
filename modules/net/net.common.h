/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __MODULES_NET_COMMON_H__
#define __MODULES_NET_COMMON_H__

#include <stddef.h>

#if defined(_WIN32)
#include <wchar.h>
#else
#include <stdint.h>
#endif

#if defined(_WIN32)
#define type_of_element wchar_t
#define calling_convention __cdecl
#define delegate_calling_convention __stdcall
#define host_fxr_PATH_DELIMITER ';'
#define host_fxr_PATH_DELIMITER_wchar_t L';'
#else
#define type_of_element uint8_t
#define calling_convention
#define delegate_calling_convention
#define host_fxr_PATH_DELIMITER ':'
#endif

typedef void(calling_convention* error_writer_type)(
	const type_of_element* message);

struct string_arguments_type
{
	size_t length;
	const type_of_element** arguments;
};

enum hostfxr_status_code
{
	host_fxr_Success,
	host_fxr_Success_HostAlreadyInitialized,
	host_fxr_Success_DifferentRuntimeProperties,
	/**/
	win_error_E_INVALIDARG = 0x80070057,
	/**/
	host_fxr_InvalidArgFailure = 0x80008081,
	host_fxr_CoreHostLibLoadFailure,
	host_fxr_CoreHostLibMissingFailure,
	host_fxr_CoreHostEntryPointFailure,
	host_fxr_CoreHostCurHostFindFailure,
	host_fxr_CoreClrResolveFailure = 0x80008087,
	host_fxr_CoreClrBindFailure,
	host_fxr_CoreClrInitFailure,
	host_fxr_CoreClrExeFailure,
	host_fxr_ResolverInitFailure,
	host_fxr_ResolverResolveFailure,
	host_fxr_LibHostCurExeFindFailure,
	host_fxr_LibHostInitFailure,
	host_fxr_LibHostSdkFindFailure = 0x80008091,
	host_fxr_LibHostInvalidArgs,
	host_fxr_InvalidConfigFile,
	host_fxr_AppArgNotRunnable,
	host_fxr_AppHostExeNotBoundFailure,
	host_fxr_FrameworkMissingFailure,
	host_fxr_HostApiFailed,
	host_fxr_HostApiBufferTooSmall,
	host_fxr_LibHostUnknownCommand,
	host_fxr_LibHostAppRootFindFailure,
	host_fxr_SdkResolverResolveFailure,
	host_fxr_FrameworkCompatFailure,
	host_fxr_FrameworkCompatRetry,
	host_fxr_AppHostExeNotBundle,
	host_fxr_BundleExtractionFailure,
	host_fxr_BundleExtractionIOError,
	host_fxr_LibHostDuplicateProperty,
	host_fxr_HostApiUnsupportedVersion,
	host_fxr_HostInvalidState,
	host_fxr_HostPropertyNotFound,
	host_fxr_CoreHostIncompatibleConfig,
	host_fxr_HostApiUnsupportedScenario
};

#define HOST_FX_RESOLVER_NON_SUCCESS(RESULT) ((RESULT) < host_fxr_Success || host_fxr_Success_DifferentRuntimeProperties < (RESULT))

#define ERROR_WRITER_WIN32(MESSAGE, CONTENT, IS_CONTENT_INITIALIZED, ERROR_FILE_WRITER)				\
	\
	if (IS_CONTENT_INITIALIZED)																		\
	{																								\
		if (!buffer_resize(&(CONTENT), 0))															\
		{																							\
			return;																					\
		}																							\
	}																								\
	else																							\
	{																								\
		SET_NULL_TO_BUFFER(CONTENT);																\
		(IS_CONTENT_INITIALIZED) = 1;																\
	}																								\
	\
	if (ERROR_FILE_WRITER)																			\
	{																								\
		if (!text_encoding_UTF16LE_to_UTF8((MESSAGE), (MESSAGE) + wcslen(MESSAGE), &(CONTENT)) ||	\
			!buffer_push_back(&(CONTENT), '\n'))													\
		{																							\
			return;																					\
		}																							\
		\
		if (!file_write_with_several_steps(&(CONTENT), (ERROR_FILE_WRITER)))						\
		{																							\
			return;																					\
		}																							\
		\
		file_flush(ERROR_FILE_WRITER);																\
	}

#define ERROR_WRITER_POSIX(MESSAGE, ERROR_FILE_WRITER)											\
	if (ERROR_FILE_WRITER)																		\
	{																							\
		static const uint8_t n = '\n';															\
		\
		if (!file_write(MESSAGE, sizeof(type_of_element), common_count_bytes_until(MESSAGE, 0),	\
						ERROR_FILE_WRITER) ||													\
			!file_write(&n, sizeof(type_of_element), 1, ERROR_FILE_WRITER))						\
		{																						\
			return;																				\
		}																						\
		\
		file_flush(ERROR_FILE_WRITER);															\
	}

#endif
