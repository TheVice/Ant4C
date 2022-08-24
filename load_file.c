/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2022 TheVice
 *
 */

#include "load_file.h"

#include "buffer.h"
#include "file_system.h"
#include "range.h"
#include "text_encoding.h"

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
	(START) = buffer_uint8_t_data((OUTPUT), sizeof(uint64_t));															\
	(FINISH) = NULL == (START) ? NULL : buffer_uint8_t_data((OUTPUT), 0) + (SIZE);										\
	\
	if (!range_in_parts_is_null_or_empty((START), (FINISH)))															\
	{																													\
		if (!buffer_append((OUTPUT), NULL, 2 * (SIZE)))																	\
		{																												\
			return 0;																									\
		}																												\
		\
		(FINISH) = buffer_uint8_t_data((OUTPUT), (SIZE));																\
		\
		if (USE_OFFSET)																									\
		{																												\
			(START) = buffer_uint8_t_data((OUTPUT), 0);																	\
			\
			for ((SIZE) = 0; (SIZE) < 2u; ++(SIZE))																		\
			{																											\
				(START)[sizeof(uint64_t) - 1 - (SIZE)] = (START)[3 - (SIZE)];											\
			}																											\
			\
			(START) = buffer_uint8_t_data((OUTPUT), sizeof(uint64_t) - 2u);												\
		}																												\
		else																											\
		{																												\
			(START) = buffer_uint8_t_data((OUTPUT), sizeof(uint64_t));													\
		}																												\
	}																													\
	\
	if (!buffer_resize((OUTPUT), 0) ||																					\
		((START) < (FINISH) && !CONVERTER((const TYPE*)(START), (const TYPE*)(FINISH), (OUTPUT))))						\
	{																													\
		return 0;																										\
	}

#define READ_WITH_ENCODING_FROM_ARGUMENT_AND_CONVERT_TO_UTF(START, FINISH, OUTPUT, SIZE, FILE_, CONVERTER, TYPE)	\
	(START) = buffer_uint8_t_data((OUTPUT), 0);																		\
	(FINISH) = buffer_uint8_t_data((OUTPUT), 0) + 2 * sizeof(uint64_t) - 1;												\
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
		(START) = buffer_uint8_t_data((OUTPUT), (SIZE));															\
		(FINISH) = buffer_uint8_t_data((OUTPUT), 0) + sz;															\
		\
		if (!buffer_resize((OUTPUT), 0) ||																			\
			!CONVERTER((const TYPE*)(START), (const TYPE*)(FINISH), (OUTPUT)))										\
		{																											\
			return 0;																								\
		}																											\
	}

#define READ_WITH_CODE_PAGE_FROM_ARGUMENT_AND_CONVERT_TO_UTF(START, FINISH, OUTPUT, SIZE, FILE_, CONVERTER, CODE_PAGE, TYPE)	\
	(START) = buffer_uint8_t_data((OUTPUT), 0);																					\
	(FINISH) = buffer_uint8_t_data((OUTPUT), 0) + 2 * sizeof(uint64_t) - 1;														\
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
	(START) = buffer_uint8_t_data((OUTPUT), (SIZE));																			\
	(FINISH) = buffer_uint8_t_data((OUTPUT), 0) + buffer_size(OUTPUT);															\
	\
	if (!buffer_resize((OUTPUT), 0) ||																							\
		!CONVERTER((const TYPE*)(START), (const TYPE*)(FINISH), (CODE_PAGE), (OUTPUT)))											\
	{																															\
		return 0;																												\
	}

#define READ_AS_ASCII_FROM_ARGUMENT_AND_CONVERT_TO_UTF(START, FINISH, OUTPUT, SIZE, FILE_, ENCODING)							\
	(START) = buffer_uint8_t_data((OUTPUT), 0);																					\
	(FINISH) = buffer_uint8_t_data((OUTPUT), 0) + 2 * sizeof(uint64_t) - 1;														\
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
	(START) = buffer_uint8_t_data((OUTPUT), (SIZE));																			\
	(FINISH) = buffer_uint8_t_data((OUTPUT), 0) + buffer_size(OUTPUT);															\
	\
	if (!buffer_resize((OUTPUT), 0) ||																							\
		!text_encoding_UTF_from_ASCII((START), (FINISH), (ENCODING), (OUTPUT)))													\
	{																															\
		return 0;																												\
	}

uint8_t load_file(const uint8_t* path, uint16_t encoding, void* output, uint8_t verbose)
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

	uint8_t* data = buffer_uint8_t_data(output, 0);
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
