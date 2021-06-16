/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "error_writer.h"
#include "host_fxr.h"
#include "host_policy.h"

#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "text_encoding.h"

#if defined(_WIN32)
extern uint8_t file_open_wchar_t(const wchar_t* path, const wchar_t* mode, void** output);
#endif

static void* host_fx_resolver_error_file_writer = NULL;
static void* host_policy_error_file_writer = NULL;

error_writer_type set_error_writer(
	const void* library_object, error_writer_type writer,
	setter_type setter_function, const type_of_element* path,
	uint8_t writer_number)
{
	error_writer_type error_writer_pointer = NULL;

	if (!path)
	{
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(suppress: 4054)
		error_writer_pointer = setter_function(library_object, NULL);
#else
		error_writer_pointer = setter_function(library_object, NULL);
#endif
	}

	void* file_stream = writer_number ? host_policy_error_file_writer : host_fx_resolver_error_file_writer;

	if (file_stream && !file_close(file_stream))
	{
		return NULL;
	}

	file_stream = NULL;

	if (writer_number)
	{
		host_policy_error_file_writer = file_stream;
	}
	else
	{
		host_fx_resolver_error_file_writer = file_stream;
	}

	if (path)
	{
#if defined(_WIN32)

		if (!file_open_wchar_t(path, L"ab", &file_stream))
		{
			return NULL;
		}

#else

		if (!file_open(path, (const uint8_t*)"ab", &file_stream))
		{
			return NULL;
		}

#endif
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(suppress: 4054)
		error_writer_pointer = setter_function(library_object, writer);
#else
		error_writer_pointer = setter_function(library_object, writer);
#endif

		if (writer_number)
		{
			host_policy_error_file_writer = file_stream;
		}
		else
		{
			host_fx_resolver_error_file_writer = file_stream;
		}
	}

	return error_writer_pointer;
}

#if defined(_WIN32)
static struct buffer host_fx_resolver_error_writer_win32_content;
static uint8_t is_host_fx_resolver_error_writer_win32_content_initialized = 0;

static struct buffer host_policy_error_writer_win32_content;
static uint8_t is_host_policy_error_writer_win32_content_initialized = 0;

void host_fx_resolver_error_writer(const type_of_element* message)
{
	ERROR_WRITER_WIN32(
		message,
		host_fx_resolver_error_writer_win32_content,
		is_host_fx_resolver_error_writer_win32_content_initialized,
		host_fx_resolver_error_file_writer);
}

void host_policy_error_writer(const type_of_element* message)
{
	ERROR_WRITER_WIN32(
		message,
		host_policy_error_writer_win32_content,
		is_host_policy_error_writer_win32_content_initialized,
		host_policy_error_file_writer);
}
#else
void host_fx_resolver_error_writer(const type_of_element* message)
{
	ERROR_WRITER_POSIX(
		message,
		host_fx_resolver_error_file_writer);
}

void host_policy_error_writer(const type_of_element* message)
{
	ERROR_WRITER_POSIX(
		message,
		host_policy_error_file_writer);
}
#endif

void error_writer_release_buffers(void* host_fxr_object, void* host_policy_object)
{
	if (host_fx_resolver_error_file_writer)
	{
		host_fxr_set_error_writer(host_fxr_object, NULL);
		file_close(host_fx_resolver_error_file_writer);
		host_fx_resolver_error_file_writer = NULL;
	}

	if (host_policy_error_file_writer)
	{
		core_host_set_error_writer(host_policy_object, NULL);
		file_close(host_policy_error_file_writer);
		host_policy_error_file_writer = NULL;
	}

#if defined(_WIN32)

	if (is_host_fx_resolver_error_writer_win32_content_initialized)
	{
		buffer_release(&host_fx_resolver_error_writer_win32_content);
	}

	if (is_host_policy_error_writer_win32_content_initialized)
	{
		buffer_release(&host_policy_error_writer_win32_content);
	}

#endif
}
