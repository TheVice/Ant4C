/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 https://github.com/TheVice/
 *
 */

#include "argument_parser.h"
#include "buffer.h"
#include "common.h"
#include "conversion.h"
#include "math_unit.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#if 0
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#endif

struct command_argument
{
	struct buffer build_files_;
	struct buffer build_files;
	uint8_t pause;
	uint8_t verbose;
	uint8_t debug;
	uint8_t quiet;
	uint8_t indent;
	struct buffer properties;
	struct buffer log_file_;
	struct buffer log_file;
	uint8_t project_help;
	uint8_t no_logo;
	uint8_t help;
	/*TODO:
	struct buffer targets_;
	struct buffer targets;*/
};

static struct command_argument argument;
static uint8_t is_argument_init = 0;
static uint8_t is_from_file = 0;

#define INIT_ARGUMENT										\
	if (is_argument_init)									\
	{														\
		property_clear(&argument.properties);				\
		\
		if (!buffer_resize(&argument.build_files_, 0)	||	\
			!buffer_resize(&argument.build_files, 0)	||	\
			!buffer_resize(&argument.log_file_, 0)		||	\
			!buffer_resize(&argument.log_file, 0))			\
		{													\
			return 0;										\
		}													\
	}														\
	else													\
	{														\
		SET_NULL_TO_BUFFER(argument.build_files_);			\
		SET_NULL_TO_BUFFER(argument.build_files);			\
		SET_NULL_TO_BUFFER(argument.properties);			\
		SET_NULL_TO_BUFFER(argument.log_file_);				\
		SET_NULL_TO_BUFFER(argument.log_file);				\
		is_argument_init = 1;								\
	}														\
	\
	argument.pause = 0;										\
	argument.verbose = UINT8_MAX;							\
	argument.debug = 0;										\
	argument.quiet = 0;										\
	argument.indent = 0;									\
	argument.project_help = 0;								\
	argument.no_logo = 0;									\
	argument.help = 0;

#define GET_FILE_PATH_CHAR(CONDITION, ARGUMENT, START_SHIFT, LENGTH, OUTPUT)									\
	else if (CONDITION)																							\
	{																											\
		struct range path;																						\
		path.start = (ARGUMENT) + (START_SHIFT);																\
		path.finish = (ARGUMENT) + (LENGTH);																	\
		\
		if (range_is_null_or_empty(&path))																		\
		{																										\
			return 0;																							\
		}																										\
		\
		path.start = find_any_symbol_like_or_not_like_that(path.start, path.finish, "\"", 1, 0, 1);				\
		path.finish = 1 + find_any_symbol_like_or_not_like_that(path.finish - 1, path.start, "\"", 1, 0, -1);	\
		\
		if (range_is_null_or_empty(&path) ||																	\
			!buffer_append_data_from_range((OUTPUT), &path) ||													\
			!buffer_push_back((OUTPUT), '\0'))																	\
		{																										\
			return 0;																							\
		}																										\
	}

#define GET_FILE_PATH_WCHAR(CONDITION, ARGUMENT, START_SHIFT, LENGTH, OUTPUT)									\
	else if (CONDITION)																							\
	{																											\
		const wchar_t* start = (ARGUMENT) + (START_SHIFT);														\
		const wchar_t* finish = (ARGUMENT) + (LENGTH);															\
		\
		if (NULL == start || finish == NULL || finish <= start)													\
		{																										\
			return 0;																							\
		}																										\
		\
		start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\"", 1, 0, 1);					\
		finish = 1 + find_any_symbol_like_or_not_like_that_wchar_t(finish - 1, start, L"\"", 1, 0, -1);			\
		\
		if (finish <= start)																					\
		{																										\
			return 0;																							\
		}																										\
		\
		const ptrdiff_t size = buffer_size(OUTPUT);																\
		\
		if (!buffer_append_char((OUTPUT), NULL, (LENGTH)))														\
		{																										\
			return 0;																							\
		}																										\
		\
		char* m = (char*)buffer_data((OUTPUT), size);															\
		uint16_t count = (uint16_t)(finish - start);															\
		WIDE2MULTI(start, m, count)																				\
		\
		if (!count ||																							\
			!buffer_resize((OUTPUT), size + (finish - start)) ||												\
			!buffer_push_back((OUTPUT), '\0'))																	\
		{																										\
			return 0;																							\
		}																										\
	}

#define GET_BOOL_VALUE(CONDITION, TYPE, ARGUMENT, LENGTH, PLUS, MINUS, OUTPUT)		\
	else if (CONDITION)																\
	{																				\
		const TYPE ch = *((ARGUMENT) + (LENGTH) - 1);								\
		\
		if ((PLUS) == ch)															\
		{																			\
			(OUTPUT) = 1;															\
		}																			\
		else if ((MINUS) == ch)														\
		{																			\
			(OUTPUT) = 0;															\
		}																			\
		else																		\
		{																			\
			(OUTPUT) = 1;															\
		}																			\
	}

#define GET_INDENT_CHAR(A, LENGTH, OUTPUT) \
	else if (7 < (LENGTH) && 0 == memcmp((A), "-indent:", 8))						\
	{																				\
		if ((LENGTH) < 9)															\
		{																			\
			return 0;																\
		}																			\
		\
		(OUTPUT) = (uint8_t)math_abs(long_parse((A) + 8));							\
	}

#define GET_INDENT_WCHAR(A, LENGTH, OUTPUT) \
	else if (7 < (LENGTH) && 0 == wmemcmp((A), L"-indent:", 8))						\
	{																				\
		if ((LENGTH) < 9)															\
		{																			\
			return 0;																\
		}																			\
		\
		(OUTPUT) = (uint8_t)math_abs(long_parse_wchar_t((A) + 8));					\
	}

#define GET_PROPERTY_CHAR(CONDITION, ARGUMENT, LENGTH, OUTPUT)														\
	else if (CONDITION)																								\
	{																												\
		struct range key;																							\
		struct range value;																							\
		\
		if (!argument_get_key_and_value(																			\
				(ARGUMENT) + 3,																						\
				(ARGUMENT) + (LENGTH),																				\
				&key,																								\
				&value))																							\
		{																											\
			return 0;																								\
		}																											\
		\
		if (!string_trim(&key))																						\
		{																											\
			return 0;																								\
		}																											\
		\
		if (!property_set_by_name(NULL, NULL, (OUTPUT), key.start, (uint8_t)range_size(&key),						\
								  value.start, range_size(&value),													\
								  property_value_is_char_array,														\
								  1, 1, 1, argument_parser_get_verbose()))											\
		{																											\
			return 0;																								\
		}																											\
	}

#define GET_PROPERTY_WCHAR(CONDITION, ARGUMENT, LENGTH, TMP_BUFFER, OUTPUT)											\
	else if (CONDITION)																								\
	{																												\
		const wchar_t* start = (ARGUMENT) + 3;																		\
		const wchar_t* finish = (ARGUMENT) + (LENGTH);																\
		const wchar_t* equal = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"=", 1, 1, 1);			\
		\
		if (start == equal || finish == equal)																		\
		{																											\
			return 0;																								\
		}																											\
		\
		const ptrdiff_t name_length = equal - start;																\
		const ptrdiff_t value_length = finish - (equal + 1);														\
		/*TODO: used external buffer.*/\
		const ptrdiff_t size = buffer_size(TMP_BUFFER);																\
		\
		if (!buffer_append((TMP_BUFFER), NULL, 1 + (LENGTH)))														\
		{																											\
			return 0;																								\
		}																											\
		\
		char* m = (char*)buffer_data((TMP_BUFFER), size);															\
		uint16_t count = (uint16_t)(LENGTH);																		\
		WIDE2MULTI(start, m, count);																				\
		\
		/*if (!count) TODO: for some reason usedDefaultChar is TRUE.*/												\
		\
		struct range name;																							\
		name.start = m;																								\
		name.finish = m + name_length;																				\
		name.start = find_any_symbol_like_or_not_like_that(name.start, name.finish, "\"", 1, 0, 1);					\
		name.finish = 1 + find_any_symbol_like_or_not_like_that(name.finish - 1, name.start, "\"", 1, 0, -1);		\
		\
		if (!string_trim(&name))																					\
		{																											\
			return 0;																								\
		}																											\
		\
		struct range value;																							\
		value.start = m + name_length + 1;																			\
		value.finish = value.start + value_length;																	\
		value.start = find_any_symbol_like_or_not_like_that(value.start, value.finish, "\"", 1, 0, 1);				\
		value.finish = 1 + find_any_symbol_like_or_not_like_that(value.finish - 1, value.start, "\"", 1, 0, -1);	\
		\
		if (!string_trim(&value))																					\
		{																											\
			return 0;																								\
		}																											\
		\
		if (!property_set_by_name(NULL, NULL, (OUTPUT), name.start, (uint8_t)range_size(&name),						\
								  value.start, range_size(&value),													\
								  property_value_is_char_array,														\
								  1, 1, 1,																			\
								  argument_parser_get_verbose() ||													\
								  !buffer_resize((TMP_BUFFER), size)))												\
		{																											\
			return 0;																								\
		}																											\
	}

#define CONTINUE(CONDITION)			\
	else if (CONDITION)				\
	{								\
		continue;					\
	}

#define GET_COMMAND_LINE_FILE(CONDITION)						\
	else if (CONDITION)											\
	{															\
		is_from_file = 1;										\
		/*TODO: process parameters from command file.*/			\
		is_from_file = 0;										\
	}

#define BUILD_FILE_CHAR(A, LENGTH) \
	((10 < (LENGTH) && 0 == memcmp((A), "-buildfile:", 11)) ||		\
	 (2 < (LENGTH) && 0 == memcmp((A), "/f:", 3)))

#define BUILD_FILE_WCHAR(A, LENGTH) \
	((10 < (LENGTH) && 0 == wmemcmp((A), L"-buildfile:", 11)) ||	\
	 (2 < (LENGTH) && 0 == wmemcmp((A), L"/f:", 3)))

#define LOG_FILE_CHAR(A, LENGTH, OUTPUT) \
	(!buffer_size(OUTPUT) && \
	 ((8 < (LENGTH) && 0 == memcmp((A), "-logfile:", 9)) ||			\
	  (2 < length && 0 == memcmp((A), "-l:", 3))))

#define LOG_FILE_WCHAR(A, LENGTH, OUTPUT) \
	(!buffer_size(OUTPUT) && \
	 ((8 < (LENGTH) && 0 == wmemcmp((A), L"-logfile:", 9)) ||		\
	  (2 < length && 0 == wmemcmp((A), L"-l:", 3))))

#define BUILD_FILE_SHIFT_CHAR(A) ('/' == (A)[0] ? 3 : 11)
#define BUILD_FILE_SHIFT_WCHAR(A) (L'/' == (A)[0] ? 3 : 11)

#define LOG_FILE_SHIFT_CHAR(A) (':' == (A)[2] ? 3 : 9)
#define LOG_FILE_SHIFT_WCHAR(A) (L':' == (A)[2] ? 3 : 9)

#define PAUSE_CHAR(A, LENGTH) \
	((6 == (LENGTH) && 0 == memcmp((A), "-pause", 6))		||		\
	 (7 == (LENGTH) && (0 == memcmp((A), "-pause+", 7)		||		\
						0 == memcmp((A), "-pause-", 7))))

#define PAUSE_WCHAR(A, LENGTH) \
	((6 == (LENGTH) && 0 == wmemcmp((A), L"-pause", 6))		||		\
	 (7 == (LENGTH) && (0 == wmemcmp((A), L"-pause+", 7)	||		\
						0 == wmemcmp((A), L"-pause-", 7))))

#define VERBOSE_CHAR(A, LENGTH) \
	((8 == (LENGTH) && 0 == memcmp((A), "-verbose", 8))		||		\
	 (2 == (LENGTH) && 0 == memcmp((A), "-v", 2))			||		\
	 (9 == (LENGTH) && (0 == memcmp((A), "-verbose+", 9)	||		\
						0 == memcmp((A), "-verbose-", 9)))	||		\
	 (3 == (LENGTH) && (0 == memcmp((A), "-v+", 3)			||		\
						0 == memcmp((A), "-v-", 3))))

#define VERBOSE_WCHAR(A, LENGTH) \
	((8 == (LENGTH) && 0 == wmemcmp((A), L"-verbose", 8))		||		\
	 (2 == (LENGTH) && 0 == wmemcmp((A), L"-v", 2))				||		\
	 (9 == (LENGTH) && (0 == wmemcmp((A), L"-verbose+", 9)		||		\
						0 == wmemcmp((A), L"-verbose-", 9)))	||		\
	 (3 == (LENGTH) && (0 == wmemcmp((A), L"-v+", 3)			||		\
						0 == wmemcmp((A), L"-v-", 3))))
#define DEBUG_CHAR(A, LENGTH) \
	(6 == (LENGTH) && 0 == memcmp((A), "-debug", 6))

#define DEBUG_WCHAR(A, LENGTH) \
	(6 == (LENGTH) && 0 == wmemcmp((A), L"-debug", 6))

#define QUIET_CHAR(A, LENGTH) \
	((6 == (LENGTH) && 0 == memcmp((A), "-quiet", 6)) ||		\
	 (2 == (LENGTH) && 0 == memcmp((A), "-q", 2)))

#define QUIET_WCHAR(A, LENGTH) \
	((6 == (LENGTH) && 0 == wmemcmp((A), L"-quiet", 6)) ||		\
	 (2 == (LENGTH) && 0 == wmemcmp((A), L"-q", 2)))

#define DEFINE_CHAR(A, LENGTH) \
	(2 < (LENGTH) && 0 == memcmp((A), "-D:", 3))

#define DEFINE_WCHAR(A, LENGTH) \
	(2 < (LENGTH) && 0 == wmemcmp((A), L"-D:", 3))

#define PROJECT_HELP_CHAR(A, LENGTH) \
	(12 == (LENGTH) && 0 == memcmp((A), "-projecthelp", 12))

#define PROJECT_HELP_WCHAR(A, LENGTH) \
	(12 == (LENGTH) && 0 == wmemcmp((A), L"-projecthelp", 12))

#define NO_LOGO_CHAR(A, LENGTH) \
	(7 == (LENGTH) && 0 == memcmp((A), "-nologo", 7))

#define NO_LOGO_WCHAR(A, LENGTH) \
	(7 == (LENGTH) && 0 == wmemcmp((A), L"-nologo", 7))

#define HELP_CHAR(A, LENGTH) \
	((5 == (LENGTH) && 0 == memcmp((A), "-help", 5)) ||		\
	 (2 == (LENGTH) && 0 == memcmp((A), "-h", 2)))

#define HELP_WCHAR(A, LENGTH) \
	((5 == (LENGTH) && 0 == wmemcmp((A), L"-help", 5)) ||	\
	 (2 == (LENGTH) && 0 == wmemcmp((A), L"-h", 2)))

#define COMMAND_LINE_FILE_CHAR(A) \
	(!is_from_file && '@' == (A)[0])

#define COMMAND_LINE_FILE_WCHAR(A) \
	(!is_from_file && L'@' == (A)[0])

#define GET_VERBOSE(CONDITION, ARGUMENT, MAX_I, STRLEN, TYPE, PLUS, MINUS, OUTPUT)						\
	if (NULL == (ARGUMENT))																				\
	{																									\
		return;																							\
	}																									\
	\
	for (int i = 0; i < (MAX_I); ++i)																	\
	{																									\
		const ptrdiff_t length = (ptrdiff_t)(STRLEN)((ARGUMENT)[i]);									\
		\
		if (length < 2)																					\
		{																								\
			continue;																					\
		}																								\
		\
		GET_BOOL_VALUE((CONDITION), TYPE, (ARGUMENT)[i], length, (PLUS), (MINUS), (OUTPUT))				\
	}

void argument_parser_get_verbose_char(int argc, char** argv)
{
	GET_VERBOSE(VERBOSE_CHAR(argv[i], length), argv, argc, strlen, char, '+', '-', argument.verbose)
}
#if defined(_WIN32)
void argument_parser_get_verbose_wchar_t(int argc, wchar_t** argv)
{
	GET_VERBOSE(VERBOSE_WCHAR(argv[i], length), argv, argc, wcslen, wchar_t, L'+', L'-', argument.verbose)
}
#endif
uint8_t argument_parser_char(int i, int argc, char** argv)
{
	if (!is_from_file)
	{
		INIT_ARGUMENT;
	}

	if (argc <= i || NULL == argv)
	{
		return 0;
	}

	argument_parser_get_verbose_char(argc, argv);

	for (; i < argc; ++i)
	{
		const ptrdiff_t length = (ptrdiff_t)strlen(argv[i]);

		if (length < 2 ||
			('-' != argv[i][0] &&
			 '@' != argv[i][0] &&
			 '/' != argv[i][0]))
		{
			continue;
		}

		GET_FILE_PATH_CHAR(BUILD_FILE_CHAR(argv[i], length), argv[i],
						   BUILD_FILE_SHIFT_CHAR(argv[i]), length, &argument.build_files)
		GET_BOOL_VALUE(PAUSE_CHAR(argv[i], length), char, argv[i], length, '+', '-', argument.pause)
		CONTINUE(VERBOSE_CHAR(argv[i], length))
		GET_BOOL_VALUE(DEBUG_CHAR(argv[i], length), char, argv[i], length, '+', '-', argument.debug)
		GET_BOOL_VALUE(QUIET_CHAR(argv[i], length), char, argv[i], length, '+', '-', argument.quiet)
		GET_INDENT_CHAR(argv[i], length, argument.indent)
		GET_PROPERTY_CHAR(DEFINE_CHAR(argv[i], length), argv[i], length, &argument.properties)
		GET_FILE_PATH_CHAR(LOG_FILE_CHAR(argv[i], length, &argument.log_file), argv[i],
						   LOG_FILE_SHIFT_CHAR(argv[i]), length, &argument.log_file)
		GET_BOOL_VALUE(PROJECT_HELP_CHAR(argv[i], length), char, argv[i], length, '+', '-',
					   argument.project_help)
		GET_BOOL_VALUE(NO_LOGO_CHAR(argv[i], length), char, argv[i], length, '+', '-', argument.no_logo)
		GET_BOOL_VALUE(HELP_CHAR(argv[i], length), char, argv[i], length, '+', '-', argument.help)
		/*TODO:if (INT8_MAX < length)
		{
			continue;
		}

		if (!buffer_append_char(argument.targets, argv[i], length))
		{
			return 0;
		}*/
	}

	if (UINT8_MAX == argument.verbose)
	{
		argument.verbose = 0;
	}

	return 1;
}
#if defined(_WIN32)
uint8_t argument_parser_wchar_t(int i, int argc, wchar_t** argv)
{
	if (!is_from_file)
	{
		INIT_ARGUMENT;
	}

	if (argc <= i || NULL == argv)
	{
		return 0;
	}

	argument_parser_get_verbose_wchar_t(argc, argv);

	for (; i < argc; ++i)
	{
		const ptrdiff_t length = (ptrdiff_t)wcslen(argv[i]);

		if (length < 2 ||
			(L'-' != argv[i][0] &&
			 L'@' != argv[i][0] &&
			 L'/' != argv[i][0]))
		{
			continue;
		}

		GET_FILE_PATH_WCHAR(BUILD_FILE_WCHAR(argv[i], length), argv[i],
							BUILD_FILE_SHIFT_WCHAR(argv[i]), length, &argument.build_files)
		GET_BOOL_VALUE(PAUSE_WCHAR(argv[i], length), wchar_t, argv[i], length, L'+', L'-', argument.pause)
		CONTINUE(VERBOSE_WCHAR(argv[i], length))
		GET_BOOL_VALUE(DEBUG_WCHAR(argv[i], length), wchar_t, argv[i], length, L'+', L'-', argument.debug)
		GET_BOOL_VALUE(QUIET_WCHAR(argv[i], length), wchar_t, argv[i], length, L'+', L'-', argument.quiet)
		GET_INDENT_WCHAR(argv[i], length, argument.indent)
		GET_PROPERTY_WCHAR(DEFINE_WCHAR(argv[i], length), argv[i], length, &argument.log_file, &argument.properties)
		GET_FILE_PATH_WCHAR(LOG_FILE_WCHAR(argv[i], length, &argument.log_file), argv[i],
							LOG_FILE_SHIFT_WCHAR(argv[i]), length, &argument.log_file)
		GET_BOOL_VALUE(PROJECT_HELP_WCHAR(argv[i], length), wchar_t, argv[i], length, L'+', L'-',
					   argument.project_help)
		GET_BOOL_VALUE(NO_LOGO_WCHAR(argv[i], length), wchar_t, argv[i], length, L'+', L'-', argument.no_logo)
		GET_BOOL_VALUE(HELP_WCHAR(argv[i], length), wchar_t, argv[i], length, L'+', L'-', argument.help)
	}

	if (UINT8_MAX == argument.verbose)
	{
		argument.verbose = 0;
	}

	return 1;
}
#endif
void argument_parser_release()
{
	if (is_argument_init)
	{
		buffer_release(&argument.build_files_);
		buffer_release(&argument.build_files);
		property_clear(&argument.properties);
		buffer_release(&argument.log_file_);
		buffer_release(&argument.log_file);
	}
}

uint8_t argument_get_key_and_value(
	const char* input_start, const char* input_finish,
	struct range* key, struct range* value)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == key || NULL == value)
	{
		return 0;
	}

	value->start = find_any_symbol_like_or_not_like_that(input_start, input_finish, "=", 1, 1, 1);
	key->finish = value->start;
	//
	value->start = find_any_symbol_like_or_not_like_that(value->start + 1, input_finish, "\"", 1, 0, 1);
	value->finish = 1 + find_any_symbol_like_or_not_like_that(input_finish - 1, value->start, "\"", 1, 0, -1);
	//
	key->start = find_any_symbol_like_or_not_like_that(input_start, key->finish, "\"", 1, 0, 1);
	key->finish = 1 + find_any_symbol_like_or_not_like_that(key->finish - 1, key->start, "\"", 1, 0, -1);
	//
	return !range_is_null_or_empty(key) && value->start <= value->finish;
}

uint8_t argument_append_arguments(
	const char* input_start, const char* input_finish, struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == output)
	{
		return 0;
	}

	for (; input_start < input_finish; ++input_start)
	{
		const char* finish = NULL;

		if (finish == input_start)
		{
			break;
		}

		if ('\"' == *input_start)
		{
			input_start = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, "\"", 1, 0, 1);
			finish = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, "\"", 1, 1, 1);
		}
		else if (' ' == *input_start)
		{
			continue;
		}
		else
		{
			finish = find_any_symbol_like_or_not_like_that(input_start + 1, input_finish, " ", 1, 1, 1);
		}

		if (input_start < finish)
		{
			if (!buffer_append_char(output, input_start, finish - input_start) ||
				!buffer_push_back(output, '\0'))
			{
				return 0;
			}
		}

		input_start = finish;
	}

	return 1;
}

uint8_t argument_create_arguments(struct buffer* output, int* argc, char*** argv)
{
	if (NULL == output ||
		NULL == argc ||
		NULL == argv)
	{
		return 0;
	}

	const int index = *argc;
	const char* start = (const char*)buffer_data(output, index);
	*argc = 0;

	if (NULL == start)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(char*)))
		{
			return 0;
		}

		(*argv) = (char**)buffer_data(output, buffer_size(output) - sizeof(char*));
		return 1;
	}

	const char* finish = (const char*)(buffer_data(output, 0) + buffer_size(output));

	while ((start = find_any_symbol_like_or_not_like_that(start, finish, "\0", 1, 1, 1)) < finish)
	{
		++start;
		(*argc)++;
	}

	const ptrdiff_t size = buffer_size(output);

	if ((*argc) < 1)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(char*)))
		{
			return 0;
		}

		(*argv) = (char**)buffer_data(output, size);
		return 1;
	}

	if (!buffer_append(output, NULL, ((ptrdiff_t)1 + (*argc)) * sizeof(char*)))
	{
		return 0;
	}

	start = (const char*)buffer_data(output, index);
	finish = (const char*)buffer_data(output, size);

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	while ((start = find_any_symbol_like_or_not_like_that(start, finish, "\0", 1, 0, 1)) < finish)
	{
		if (!buffer_append(output, (const uint8_t*)&start, sizeof(char*)))
		{
			return 0;
		}

		start = find_any_symbol_like_or_not_like_that(start, finish, "\0", 1, 1, 1);
	}

	start = NULL;

	if (!buffer_append(output, (const uint8_t*)&start, sizeof(char*)))
	{
		return 0;
	}

	(*argv) = (char**)buffer_data(output, size);
	return 1;
}

uint8_t argument_from_char(const char* input_start, const char* input_finish,
						   struct buffer* output, int* argc, char*** argv)
{
	return argument_append_arguments(input_start, input_finish, output) &&
		   argument_create_arguments(output, argc, argv);
}

#if defined(_WIN32)

uint8_t argument_append_arguments_wchar_t(
	const wchar_t* input_start, const wchar_t* input_finish, struct buffer* output)
{
	if (NULL == input_start ||
		NULL == input_finish ||
		input_finish <= input_start ||
		NULL == output)
	{
		return 0;
	}

	for (; input_start < input_finish; ++input_start)
	{
		const wchar_t* finish = NULL;

		if (finish == input_start)
		{
			break;
		}

		if (L'\"' == *input_start)
		{
			input_start = find_any_symbol_like_or_not_like_that_wchar_t(input_start + 1, input_finish, L"\"", 1, 0, 1);
			finish = find_any_symbol_like_or_not_like_that_wchar_t(input_start + 1, input_finish, L"\"", 1, 1, 1);
		}
		else if (L' ' == *input_start)
		{
			continue;
		}
		else
		{
			finish = find_any_symbol_like_or_not_like_that_wchar_t(input_start + 1, input_finish, L" ", 1, 1, 1);
		}

		if (input_start < finish)
		{
			if (!buffer_append_wchar_t(output, input_start, finish - input_start) ||
				!buffer_push_back_uint16(output, L'\0'))
			{
				return 0;
			}
		}

		input_start = finish;
	}

	return 1;
}

uint8_t argument_create_arguments_wchar_t(struct buffer* output, int* argc, wchar_t*** argv)
{
	if (NULL == output ||
		NULL == argc ||
		NULL == argv)
	{
		return 0;
	}

	const int index = *argc;
	const wchar_t* start = (const wchar_t*)buffer_data(output, index);
	*argc = 0;

	if (NULL == start)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(wchar_t*)))
		{
			return 0;
		}

		(*argv) = (wchar_t**)buffer_data(output, buffer_size(output) - sizeof(wchar_t*));
		return 1;
	}

	const wchar_t* finish = (const wchar_t*)(buffer_data(output, 0) + buffer_size(output));

	while ((start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 1, 1)) < finish)
	{
		++start;
		(*argc)++;
	}

	const ptrdiff_t size = buffer_size(output);

	if ((*argc) < 1)
	{
		start = NULL;

		if (!buffer_append(output, (const uint8_t*)&start, sizeof(wchar_t*)))
		{
			return 0;
		}

		(*argv) = (wchar_t**)buffer_data(output, size);
		return 1;
	}

	if (!buffer_append(output, NULL, ((ptrdiff_t)2 + (*argc)) * sizeof(wchar_t*)))
	{
		return 0;
	}

	start = (const wchar_t*)buffer_data(output, index);
	finish = (const wchar_t*)buffer_data(output, size);

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	while ((start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 0, 1)) < finish)
	{
		if (!buffer_append(output, (const uint8_t*)&start, sizeof(wchar_t*)))
		{
			return 0;
		}

		start = find_any_symbol_like_or_not_like_that_wchar_t(start, finish, L"\0", 1, 1, 1);
	}

	start = NULL;

	if (!buffer_append(output, (const uint8_t*)&start, sizeof(wchar_t*)))
	{
		return 0;
	}

	(*argv) = (wchar_t**)buffer_data(output, size);
	return 1;
}

uint8_t argument_from_wchar_t(const wchar_t* input_start, const wchar_t* input_finish,
							  struct buffer* output, int* argc, wchar_t*** argv)
{
	return argument_append_arguments_wchar_t(input_start, input_finish, output) &&
		   argument_create_arguments_wchar_t(output, argc, argv);
}

#endif

const struct range* argument_parser_get_build_file(int index)
{
	if (!is_argument_init || !buffer_size(&argument.build_files))
	{
		return NULL;
	}

	if (!buffer_size(&argument.build_files_))
	{
		const char* start = buffer_char_data(&argument.build_files, 0);
		const char* finish = buffer_char_data(&argument.build_files, 0) + buffer_size(&argument.build_files);

		while (start < finish)
		{
			struct range build_file;
			build_file.start = find_any_symbol_like_or_not_like_that(start, finish, "\0", 1, 0, 1);
			build_file.finish = find_any_symbol_like_or_not_like_that(build_file.start, finish, "\0", 1, 1, 1);

			if (range_is_null_or_empty(&build_file))
			{
				break;
			}

			if (!buffer_append_range(&argument.build_files_, &build_file, 1))
			{
				buffer_release(&argument.build_files_);
				return NULL;
			}

			start = build_file.finish;
		}
	}

	return buffer_range_data(&argument.build_files_, index);
}

uint8_t argument_parser_get_pause()
{
	return is_argument_init ? argument.pause : 0;
}

uint8_t argument_parser_get_verbose()
{
	return (is_argument_init && argument.verbose < 2) ? argument.verbose : 0;
}

uint8_t argument_parser_get_debug()
{
	return is_argument_init ? argument.debug : 0;
}

uint8_t argument_parser_get_quiet()
{
	return is_argument_init ? argument.quiet : 0;
}

uint8_t argument_parser_get_indent()
{
	return is_argument_init ? argument.indent : 0;
}

const struct buffer* argument_parser_get_properties()
{
	return is_argument_init ? &argument.properties : NULL;
}

const struct range* argument_parser_get_log_file()
{
	if (!is_argument_init || !buffer_size(&argument.log_file))
	{
		return NULL;
	}

	if (!buffer_size(&argument.log_file_))
	{
		struct range log_file;
		log_file.start = buffer_char_data(&argument.log_file, 0);
		log_file.finish = buffer_char_data(&argument.log_file, buffer_size(&argument.log_file) - 1);

		if (!buffer_append_range(&argument.log_file_, &log_file, 1))
		{
			buffer_release(&argument.log_file_);
			return NULL;
		}
	}

	return buffer_range_data(&argument.log_file_, 0);
}

uint8_t argument_parser_get_project_help()
{
	return is_argument_init ? argument.project_help : 0;
}

uint8_t argument_parser_get_no_logo()
{
	return is_argument_init ? argument.no_logo : 0;
}

uint8_t argument_parser_get_help()
{
	return is_argument_init ? argument.help : 0;
}
#else
#endif
