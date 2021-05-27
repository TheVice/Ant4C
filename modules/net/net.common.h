/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 TheVice
 *
 */

#ifndef __NET_COMMON_H__
#define __NET_COMMON_H__

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

#endif
