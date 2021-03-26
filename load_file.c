/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 https://github.com/TheVice/
 *
 */

#include "load_file.h"
#include "buffer.h"
#include "common.h"
#include "file_system.h"
#include "project.h"
#include "property.h"
#include "range.h"
#include "string_unit.h"
#include "text_encoding.h"

#if !defined(__STDC_SEC_API__)
#define __STDC_SEC_API__ ((__STDC_LIB_EXT1__) || (__STDC_SECURE_LIB__) || (__STDC_WANT_LIB_EXT1__) || (__STDC_WANT_SECURE_LIB__))
#endif

#define READ_WITH_ENCODING_FROM_BOM_AND_CONVERT_TO_UTF(FILE_, START, FINISH, SIZE, USE_OFFSET, TYPE, OUTPUT, CONVERTER)	\
	if (!buffer_resize((OUTPUT), sizeof(uint64_t)) ||																	\
		!file_read_with_several_steps((FILE_), (OUTPUT)))																\
	{																													\
		file_close(FILE_);																								\
		return 0;																										\
	}																													\
	\
	if (!file_close(FILE_))																								\
	{																													\
		return 0;																										\
	}																													\
	\
	(SIZE) = buffer_size(OUTPUT);																						\
	(START) = buffer_data((OUTPUT), sizeof(uint64_t));																	\
	(FINISH) = NULL == (START) ? NULL : buffer_data((OUTPUT), 0) + (SIZE);												\
	\
	if (!range_in_parts_is_null_or_empty((START), (FINISH)))															\
	{																													\
		if (!buffer_append((OUTPUT), NULL, 2 * (SIZE)))																	\
		{																												\
			return 0;																									\
		}																												\
		\
		(FINISH) = buffer_data((OUTPUT), (SIZE));																		\
		\
		if (USE_OFFSET)																									\
		{																												\
			(START) = buffer_data((OUTPUT), 0);																			\
			\
			for ((SIZE) = 0; (SIZE) < 2u; ++(SIZE))																		\
			{																											\
				(START)[sizeof(uint64_t) - 1 - (SIZE)] = (START)[3 - (SIZE)];											\
			}																											\
			\
			(START) = buffer_data((OUTPUT), sizeof(uint64_t) - 2u);														\
		}																												\
		else																											\
		{																												\
			(START) = buffer_data((OUTPUT), sizeof(uint64_t));															\
		}																												\
	}																													\
	\
	if (!buffer_resize((OUTPUT), 0) ||																					\
		((START) < (FINISH) && !CONVERTER((const TYPE*)(START), (const TYPE*)(FINISH), (OUTPUT))))						\
	{																													\
		return 0;																										\
	}

#define READ_WITH_ENCODING_FROM_ARGUMENT_AND_CONVERT_TO_UTF(START, FINISH, OUTPUT, SIZE, FILE_, CONVERTER, TYPE)	\
	(START) = buffer_data((OUTPUT), 0);																				\
	(FINISH) = buffer_data((OUTPUT), 0) + 2 * sizeof(uint64_t) - 1;													\
	\
	for ((SIZE) = buffer_size(OUTPUT); 0 < (SIZE); --(SIZE))														\
	{\
		(*(FINISH)) = (START)[(SIZE) - 1];																			\
		--(FINISH);																									\
	}																												\
	\
	(SIZE) = 2 * sizeof(uint64_t) - buffer_size(OUTPUT);															\
	\
	if (!buffer_resize((OUTPUT), 2 * sizeof(uint64_t)) ||															\
		!file_read_with_several_steps((FILE_), (OUTPUT)))															\
	{																												\
		file_close(FILE_);																							\
		return 0;																									\
	}																												\
	\
	if (!file_close(FILE_))																							\
	{																												\
		return 0;																									\
	}																												\
	\
	{																												\
		const ptrdiff_t sz = buffer_size(OUTPUT);																	\
		\
		if (!buffer_append((OUTPUT), NULL, 2 * sz))																	\
		{																											\
			return 0;																								\
		}																											\
		\
		(START) = buffer_data((OUTPUT), (SIZE));																	\
		(FINISH) = buffer_data((OUTPUT), 0) + sz;																	\
		\
		if (!buffer_resize((OUTPUT), 0) ||																			\
			!CONVERTER((const TYPE*)(START), (const TYPE*)(FINISH), (OUTPUT)))										\
		{																											\
			return 0;																								\
		}																											\
	}

#define READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(START, FINISH, OUTPUT, SIZE, FILE_, CONVERTER, CODE_PAGE, TYPE)	\
	(START) = buffer_data((OUTPUT), 0);																							\
	(FINISH) = buffer_data((OUTPUT), 0) + 2 * sizeof(uint64_t) - 1;																\
	\
	for ((SIZE) = buffer_size(OUTPUT); 0 < (SIZE); --(SIZE))																	\
	{\
		(*(FINISH)) = (START)[(SIZE) - 1];																						\
		--(FINISH);																												\
	}																															\
	\
	(SIZE) = 2 * sizeof(uint64_t) - buffer_size(OUTPUT);																		\
	\
	if (!buffer_resize((OUTPUT), 2 * sizeof(uint64_t)) ||																		\
		!file_read_with_several_steps((FILE_), (OUTPUT)))																		\
	{																															\
		file_close(FILE_);																										\
		return 0;																												\
	}																															\
	\
	if (!file_close(FILE_))																										\
	{																															\
		return 0;																												\
	}																															\
	\
	(START) = buffer_data((OUTPUT), (SIZE));																					\
	(FINISH) = buffer_data((OUTPUT), 0) + buffer_size(OUTPUT);																	\
	\
	if (!buffer_resize((OUTPUT), 0) ||																							\
		!CONVERTER((const TYPE*)(START), (const TYPE*)(FINISH), (CODE_PAGE), (OUTPUT)))											\
	{																															\
		return 0;																												\
	}

#define READ_AS_ASCII_FROM_ARGUMENT_AND_CONVERT_TO_UTF(START, FINISH, OUTPUT, SIZE, FILE_, ENCODING)							\
	(START) = buffer_data((OUTPUT), 0);																							\
	(FINISH) = buffer_data((OUTPUT), 0) + 2 * sizeof(uint64_t) - 1;																\
	\
	for ((SIZE) = buffer_size(OUTPUT); 0 < (SIZE); --(SIZE))																	\
	{\
		(*(FINISH)) = (START)[(SIZE) - 1];																						\
		--(FINISH);																												\
	}																															\
	\
	(SIZE) = 2 * sizeof(uint64_t) - buffer_size(OUTPUT);																		\
	\
	if (!buffer_resize((OUTPUT), 2 * sizeof(uint64_t)) ||																		\
		!file_read_with_several_steps((FILE_), (OUTPUT)))																		\
	{																															\
		file_close(FILE_);																										\
		return 0;																												\
	}																															\
	\
	if (!file_close(FILE_))																										\
	{																															\
		return 0;																												\
	}																															\
	\
	(START) = buffer_data((OUTPUT), (SIZE));																					\
	(FINISH) = buffer_data((OUTPUT), 0) + buffer_size(OUTPUT);																	\
	\
	if (!buffer_resize((OUTPUT), 0) ||																							\
		!text_encoding_UTF_from_ASCII((START), (FINISH), (ENCODING), (OUTPUT)))													\
	{																															\
		return 0;																												\
	}

uint8_t load_file_to_buffer(const uint8_t* path, uint16_t encoding, struct buffer* output, uint8_t verbose)
{
	(void)verbose;

	if (NULL == path ||
		NULL == output)
	{
		return 0;
	}

	if (Default == encoding)
	{
		return buffer_resize(output, 0) && file_read_all(path, output);
	}

	void* file = NULL;

	if (!file_open(path, (const uint8_t*)"rb", &file))
	{
		return 0;
	}

	if (!buffer_resize(output, 8192))
	{
		return 0;
	}

	uint8_t* data = buffer_data(output, 0);
	size_t readed = 0;

	if (1 > (readed = file_read(data, sizeof(uint8_t), 4, file)))
	{
		return file_close(file) && buffer_resize(output, 0);
	}

	if (!buffer_resize(output, readed))
	{
		file_close(file);
		return 0;
	}

	readed = text_encoding_get_one_of_data_by_BOM(data, (ptrdiff_t)readed);
	const uint8_t use_offset = (UTF16BE == readed || UTF16LE == readed);

	switch (readed)
	{
		case UTF8:
			if (3 < buffer_size(output))
			{
				data[0] = data[3];

				if (!buffer_resize(output, 1) ||
					!file_read_with_several_steps(file, output))
				{
					file_close(file);
					return 0;
				}
			}
			else
			{
				if (!buffer_resize(output, 0))
				{
					file_close(file);
					return 0;
				}
			}

			return file_close(file);

		case UTF16BE:
			READ_WITH_ENCODING_FROM_BOM_AND_CONVERT_TO_UTF(
				file, data, path, readed, use_offset, uint16_t, output,
				text_encoding_UTF16BE_to_UTF8);
			return 1;

		case UTF16LE:
			READ_WITH_ENCODING_FROM_BOM_AND_CONVERT_TO_UTF(
				file, data, path, readed, use_offset, uint16_t, output,
				text_encoding_UTF16LE_to_UTF8);
			return 1;

		case UTF32BE:
			READ_WITH_ENCODING_FROM_BOM_AND_CONVERT_TO_UTF(
				file, data, path, readed, use_offset, uint32_t, output,
				text_encoding_UTF32BE_to_UTF8);
			return 1;

		case UTF32LE:
			READ_WITH_ENCODING_FROM_BOM_AND_CONVERT_TO_UTF(
				file, data, path, readed, use_offset, uint32_t, output,
				text_encoding_encode_UTF8);
			return 1;

		default:
			break;
	}

	switch (encoding)
	{
		case UTF8:
			if (!file_read_with_several_steps(file, output))
			{
				file_close(file);
				return 0;
			}

			return file_close(file);

		case BigEndianUnicode:
		case UTF16BE:
			READ_WITH_ENCODING_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF16BE_to_UTF8, uint16_t);
			return 1;

		case Unicode:
		case UTF16LE:
			READ_WITH_ENCODING_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF16LE_to_UTF8, uint16_t);
			return 1;

		case UTF32BE:
			READ_WITH_ENCODING_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF32BE_to_UTF8, uint32_t);
			return 1;

		case UTF32:
		case UTF32LE:
			READ_WITH_ENCODING_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_encode_UTF8, uint32_t);
			return 1;

		case Windows_874:
		case ISO_8859_11:
			READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF8_from_code_page, Windows_874, uint8_t);
			return 1;

		case Windows_1250:
		case ISO_8859_2:/*TODO:161...190*/
			READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF8_from_code_page, Windows_1250, uint8_t);
			return 1;

		case Windows_1251:
		case Windows_1255:
		case Windows_1256:
		case Windows_1258:
			READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF8_from_code_page, encoding, uint8_t);
			return 1;

		case Windows_1252:
		case ISO_8859_1:
			READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF8_from_code_page, Windows_1252, uint8_t);
			return 1;

		case Windows_1253:
		case ISO_8859_7:
			READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF8_from_code_page, Windows_1253, uint8_t);
			return 1;

		case Windows_1254:
		case ISO_8859_9:
			READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF8_from_code_page, Windows_1254, uint8_t);
			return 1;

		case Windows_1257:
		case ISO_8859_13:/*TODO:...*/
			READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file,
					text_encoding_UTF8_from_code_page, Windows_1257, uint8_t);
			return 1;

		case ASCII:
			READ_AS_ASCII_FROM_ARGUMENT_AND_CONVERT_TO_UTF(path, data, output, readed, file, UTF8);
			return 1;

		default:
			break;
	}

	file_close(file);
	return 0;
#if 0
	/*
	ISO_8859_3:
	ISO_8859_4:
	ISO_8859_5:
	ISO_8859_6:
	ISO_8859_8:
	*/

	if (!file_read_with_several_steps(file, output))
	{
		file_close(file);
		return 0;
	}

	return file_close(file);
#endif
}

uint16_t load_file_get_encoding(struct buffer* encoding_name)
{
	static const uint8_t* code_pages[] =
	{
		(const uint8_t*)"WINDOWS_874",
		(const uint8_t*)"WINDOWS_1250",
		(const uint8_t*)"WINDOWS_1251",
		(const uint8_t*)"WINDOWS_1252",
		(const uint8_t*)"WINDOWS_1253",
		(const uint8_t*)"WINDOWS_1254",
		(const uint8_t*)"WINDOWS_1255",
		(const uint8_t*)"WINDOWS_1256",
		(const uint8_t*)"WINDOWS_1257",
		(const uint8_t*)"WINDOWS_1258"
	};
	/**/
	static const uint8_t* file_encodings[] =
	{
		(const uint8_t*)"ISO_8859_1",
		(const uint8_t*)"ISO_8859_2",
		/*(const uint8_t*)"ISO_8859_3",
		(const uint8_t*)"ISO_8859_4",
		(const uint8_t*)"ISO_8859_5",
		(const uint8_t*)"ISO_8859_6",*/
		(const uint8_t*)"ISO_8859_7",
		/*(const uint8_t*)"ISO_8859_8",*/
		(const uint8_t*)"ISO_8859_9",
		(const uint8_t*)"ISO_8859_11",
		(const uint8_t*)"ISO_8859_13"
	};

	if (NULL == encoding_name)
	{
		return FILE_ENCODING_UNKNOWN;
	}

	uint16_t result = (uint16_t)buffer_size(encoding_name);

	if (!buffer_append(encoding_name, NULL, (ptrdiff_t)2 * result + sizeof(uint32_t)) ||
		!buffer_resize(encoding_name, result))
	{
		return FILE_ENCODING_UNKNOWN;
	}

	const uint8_t* start = buffer_data(encoding_name, 0);
	const uint8_t* finish = start + result;

	if (!string_to_upper(start, finish, encoding_name))
	{
		return FILE_ENCODING_UNKNOWN;
	}

	finish = buffer_data(encoding_name, 0) + buffer_size(encoding_name);
	start = buffer_data(encoding_name, result);
	result = text_encoding_get_one(start, finish);

	if (TEXT_ENCODING_UNKNOWN != result)
	{
		return result;
	}

	static const uint8_t* encodings[] = { (const uint8_t*)"BIGENDIANUNICODE", (const uint8_t*)"UNICODE", (const uint8_t*)"DEFAULT" };
	result = common_string_to_enum(start, finish, encodings, 3);

	if (result < 3)
	{
		return 0 == result ? BigEndianUnicode : (1 == result ? Unicode : Default);
	}

	result = common_string_to_enum(start, finish, code_pages, COUNT_OF(code_pages));

	if (COUNT_OF(code_pages) != result)
	{
		if (0 == result)
		{
			result += Windows_874;
		}
		else
		{
			result += Windows_1250 - 1;
		}

		return result;
	}

	result = common_string_to_enum(start, finish, file_encodings, COUNT_OF(file_encodings));

	if (COUNT_OF(file_encodings) != result)
	{
		result += ISO_8859_1;
		return result;
	}

	return FILE_ENCODING_UNKNOWN;
}

#define FILE_POSITION		0
#define PROPERTY_POSITION	1
#define ENCODING_POSITION	2

static const uint8_t* attributes[] =
{
	(const uint8_t*)"file",
	(const uint8_t*)"property",
	(const uint8_t*)"encoding"
};

static const uint8_t attributes_lengths[] = { 4, 8, 8 };

uint8_t load_file_get_attributes_and_arguments_for_task(
	const uint8_t*** task_attributes, const uint8_t** task_attributes_lengths,
	uint8_t* task_attributes_count, struct buffer* task_arguments)
{
	return common_get_attributes_and_arguments_for_task(
			   attributes, attributes_lengths,
			   COUNT_OF(attributes),
			   task_attributes, task_attributes_lengths,
			   task_attributes_count, task_arguments);
}

uint8_t load_file_evaluate_task(void* project, struct buffer* task_arguments, uint8_t verbose)
{
	if (NULL == project ||
		NULL == task_arguments)
	{
		return 0;
	}

	struct buffer* file_path_in_a_buffer = buffer_buffer_data(task_arguments, FILE_POSITION);

	const struct buffer* property_in_a_buffer = buffer_buffer_data(task_arguments, PROPERTY_POSITION);

	struct buffer* encoding_name_in_a_buffer = buffer_buffer_data(task_arguments, ENCODING_POSITION);

	uint8_t property_name_length = 0;

	if (!buffer_size(file_path_in_a_buffer) ||
		0 == (property_name_length = (uint8_t)buffer_size(property_in_a_buffer)))
	{
		return 0;
	}

	const uint8_t* file = NULL;

	if (!buffer_push_back(file_path_in_a_buffer, 0))
	{
		return 0;
	}

	file = buffer_data(file_path_in_a_buffer, 0);
	const uint8_t* property_name = buffer_data(property_in_a_buffer, 0);
	uint16_t encoding = Default;

	if (buffer_size(encoding_name_in_a_buffer))
	{
		encoding = load_file_get_encoding(encoding_name_in_a_buffer);

		if (FILE_ENCODING_UNKNOWN == encoding)
		{
			return 0;
		}
	}

	uint8_t dynamic = 0;
	uint8_t read_only = 0;
	/**/
	void* the_property = NULL;

	if (!project_property_set_value(project, property_name,
									property_name_length,
									(const uint8_t*)&the_property, 0, dynamic, 1, read_only, verbose) ||
		!project_property_exists(project, property_name,
								 property_name_length, &the_property, verbose))
	{
		return 0;
	}

	return property_set_from_file(the_property, file, encoding, dynamic, read_only, verbose);
}
