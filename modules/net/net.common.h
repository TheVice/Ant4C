/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 - 2022 TheVice
 *
 */

#ifndef __MODULES_NET_NET_COMMON_H__
#define __MODULES_NET_NET_COMMON_H__

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#include <wchar.h>
#else
#include <stdint.h>
#endif

#if defined(_WIN32)
#define type_of_element wchar_t
#define calling_convention __cdecl
#define delegate_calling_convention __stdcall
#define net_PATH_DELIMITER ';'
#define net_PATH_DELIMITER_wchar_t L';'
#else
#define type_of_element uint8_t
#define calling_convention
#define delegate_calling_convention
#define net_PATH_DELIMITER ':'
#endif

typedef void(calling_convention* error_writer_type)(
	const type_of_element* message);

typedef uint8_t(*is_function_exists_type)(
	const void* ptr_to_object,
	const uint8_t* function_name,
	uint8_t function_name_length);

typedef uint8_t(*loader_type)(
	const type_of_element* path_to_library, void* ptr_to_object, ptrdiff_t size);

struct string_arguments_type
{
	size_t length;
	const type_of_element** arguments;
};

enum net_status_code
{
	net_Success,
	net_Success_HostAlreadyInitialized,
	net_Success_DifferentRuntimeProperties,
	/**/
	win_error_E_INVALIDARG = 0x80070057,
	/**/
	net_InvalidArgFailure = 0x80008081,
	net_CoreHostLibLoadFailure,
	net_CoreHostLibMissingFailure,
	net_CoreHostEntryPointFailure,
	net_CoreHostCurHostFindFailure,
	net_CoreClrResolveFailure = 0x80008087,
	net_CoreClrBindFailure,
	net_CoreClrInitFailure,
	net_CoreClrExeFailure,
	net_ResolverInitFailure,
	net_ResolverResolveFailure,
	net_LibHostCurExeFindFailure,
	net_LibHostInitFailure,
	net_LibHostSdkFindFailure = 0x80008091,
	net_LibHostInvalidArgs,
	net_InvalidConfigFile,
	net_AppArgNotRunnable,
	net_AppHostExeNotBoundFailure,
	net_FrameworkMissingFailure,
	net_HostApiFailed,
	net_HostApiBufferTooSmall,
	net_LibHostUnknownCommand,
	net_LibHostAppRootFindFailure,
	net_SdkResolverResolveFailure,
	net_FrameworkCompatFailure,
	net_FrameworkCompatRetry,
	net_AppHostExeNotBundle,
	net_BundleExtractionFailure,
	net_BundleExtractionIOError,
	net_LibHostDuplicateProperty,
	net_HostApiUnsupportedVersion,
	net_HostInvalidState,
	net_HostPropertyNotFound,
	net_CoreHostIncompatibleConfig,
	net_HostApiUnsupportedScenario
};

#define IS_HOST_FAILED(RESULT) ((RESULT) < (int32_t)net_Success || (int32_t)net_Success_DifferentRuntimeProperties < (RESULT))

#define SET_DATA_FOR_STRING_MEMBER_OF_STRUCTURE(STRUCTURE, INITIALIZE_REQUEST, VALUES, LENGTHS, COUNT, MEMBER, THE_BUFFER)	\
	\
	if (!INITIALIZE_REQUEST ||																								\
		!VALUES ||																											\
		!LENGTHS ||																											\
		!buffer_resize(THE_BUFFER, 0))																						\
	{																														\
		return 0;																											\
	}																														\
	\
	struct STRUCTURE* INITIALIZE_REQUEST_ = (struct STRUCTURE*)INITIALIZE_REQUEST;											\
	INITIALIZE_REQUEST_->MEMBER.arguments = NULL;																			\
	INITIALIZE_REQUEST_->MEMBER.length = COUNT;																				\
	return values_to_arguments(																								\
			VALUES, LENGTHS,																								\
			COUNT, THE_BUFFER,																								\
			&INITIALIZE_REQUEST_->MEMBER.arguments);

const void* string_to_pointer(
	const uint8_t* input,
	uint8_t length,
	void* tmp);

uint8_t convert_function_name(
	const uint8_t* name_space,
	const uint8_t* function_name_start,
	const uint8_t* function_name_finish,
	void* output);

uint8_t get_exists_functions(
	const void* ptr_to_object,
	const uint8_t* name_space,
	const uint8_t* functions,
	const uint8_t* delimiter,
	uint16_t delimiter_length,
	const is_function_exists_type is_function_exists,
	void* output);

uint8_t load_library(
	const uint8_t* path_to_library,
	uint16_t path_to_library_length,
	void* tmp,
	void* ptr_to_object,
	ptrdiff_t size,
	const loader_type loader);

uint8_t is_function_exists(
	const void* ptr_to_object,
	const uint8_t* name_space,
	const uint8_t* function_name,
	uint16_t function_name_length,
	const is_function_exists_type is_exists,
	void* output);

#endif
