/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#include "error_writer.h"

#include "file_system.h"

#if defined(_WIN32)
extern uint8_t file_open_wchar_t(const wchar_t* path, const wchar_t* mode, void** output);
#endif

error_writer_type set_error_writer(
	const void* library_object, error_writer_type writer,
	setter_type setter_function, const type_of_element* path,
	void** file_stream)
{
	error_writer_type error_writer_pointer = NULL;

	if (!file_stream)
	{
		return error_writer_pointer;
	}

	if (!path)
	{
#if defined(_MSC_VER) && (_MSC_VER < 1910)
#pragma warning(suppress: 4054)
		error_writer_pointer = setter_function(library_object, NULL);
#else
		error_writer_pointer = setter_function(library_object, NULL);
#endif
	}

	if (*file_stream && !file_close(*file_stream))
	{
		return NULL;
	}

	*file_stream = NULL;

	if (path)
	{
#if defined(_WIN32)

		if (!file_open_wchar_t(path, L"ab", file_stream))
		{
			return NULL;
		}

#else

		if (!file_open(path, (const uint8_t*)"ab", file_stream))
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
	}

	return error_writer_pointer;
}
