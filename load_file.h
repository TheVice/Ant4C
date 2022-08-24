/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2020, 2022 TheVice
 *
 */

#ifndef _LOAD_FILE_H_
#define _LOAD_FILE_H_

#include <stdint.h>

enum FileEncoding
{
	ISO_8859_1 = 8861, ISO_8859_2, /*ISO_8859_3, ISO_8859_4,
	ISO_8859_5, ISO_8859_6,*/ ISO_8859_7, /*ISO_8859_8,*/ ISO_8859_9,
	ISO_8859_11, ISO_8859_13
};

#define FILE_ENCODING_UNKNOWN (ISO_8859_13 + 1)

uint8_t load_file(const uint8_t* path, uint16_t encoding, void* output, uint8_t verbose);

#endif
