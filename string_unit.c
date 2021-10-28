/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 - 2021 TheVice
 *
 */

#include "string_unit.h"
#include "buffer.h"
#include "common.h"
#include "interpreter.string_unit.h"
#include "range.h"
#include "text_encoding.h"

#include <ctype.h>
#include <string.h>

static const uint8_t* quote_symbols = (const uint8_t*)"\"'";

ptrdiff_t string_cmp_(
	const uint8_t* input_1_start, const uint8_t* input_1_finish,
	const uint8_t* input_2_start, const uint8_t* input_2_finish)
{
	uint32_t input_1_char_set, input_2_char_set;

	while (NULL != (input_1_start = string_enumerate(input_1_start, input_1_finish, &input_1_char_set)) &&
		   NULL != (input_2_start = string_enumerate(input_2_start, input_2_finish, &input_2_char_set)))
	{
		if (input_1_char_set != input_2_char_set)
		{
			return input_1_char_set < input_2_char_set ? -1 : 1;
		}
	}

	return (NULL == input_2_start || input_2_finish == input_2_start) ?
		   0 : (input_1_start < input_2_start ? -1 : 1);
}

ptrdiff_t string_index_of_value(
	const uint8_t* input_start, const uint8_t* input_finish,
	const uint8_t* value_start, const uint8_t* value_finish, int8_t step)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		range_in_parts_is_null_or_empty(value_start, value_finish) ||
		(-1 != step && 1 != step))
	{
		if (input_start == input_finish &&
			value_start == value_finish)
		{
			return 0;
		}

		return -1;
	}

	uint32_t input_char_set, value_char_set;

	if (NULL == (value_start = string_enumerate(value_start, value_finish, &value_char_set)))
	{
		return 0;
	}

	const uint8_t value_empty = range_in_parts_is_null_or_empty(value_start, value_finish);
	ptrdiff_t index = -1, i = 0;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, &input_char_set)))
	{
		if (value_char_set == input_char_set)
		{
			if (!value_empty &&
				!range_in_parts_is_null_or_empty(input_start, input_finish) &&
				string_cmp_(input_start, input_finish, value_start, value_finish))
			{
				++i;
				continue;
			}

			index = i;

			if (0 < step)
			{
				break;
			}
		}

		++i;
	}

	return index;
}

uint8_t string_contains(const uint8_t* input_start, const uint8_t* input_finish,
						const uint8_t* value_start, const uint8_t* value_finish)
{
	return -1 != string_index_of_value(input_start, input_finish, value_start, value_finish, 1);
}

uint8_t string_ends_with(const uint8_t* input_start, const uint8_t* input_finish,
						 const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		if (input_start == input_finish &&
			value_start == value_finish)
		{
			return 1;
		}

		return 0;
	}

	const ptrdiff_t length = value_finish - value_start;

	if (input_finish - input_start < length)
	{
		return 0;
	}

	return 0 == memcmp(input_finish - length, value_start, length);
}

const uint8_t* string_enumerate(const uint8_t* input_start, const uint8_t* input_finish, uint32_t* char_set)
{
	static uint32_t internal_char_set;

	if (NULL == char_set)
	{
		char_set = &internal_char_set;
	}

	const uint8_t offset = text_encoding_decode_UTF8_single(input_start, input_finish, char_set);
	return offset ? input_start + offset : NULL;
}

#define LIKE_OR_NOT()																		\
	step = 0;																				\
	that_pos = that_start;																	\
	\
	while (NULL != (that_pos = string_enumerate(that_pos, that_finish, &that_char_set)))	\
	{																						\
		step = (input_char_set == that_char_set);											\
		\
		if (step)																			\
		{																					\
			break;																			\
		}																					\
	}

const uint8_t* string_find_any_symbol_like_or_not_like_that(
	const uint8_t* start, const uint8_t* finish,
	const uint8_t* that_start, const uint8_t* that_finish,
	uint8_t like, int8_t step)
{
	if (NULL == start || NULL == finish ||
		range_in_parts_is_null_or_empty(that_start, that_finish) ||
		(0 != like && 1 != like) ||
		(-1 != step && 1 != step) ||
		((0 < step) ? (finish < start) : (finish > start)))
	{
		return finish;
	}

	uint32_t input_char_set, that_char_set;
	const uint8_t* input_pos;
	const uint8_t* that_pos;

	if (0 < step)
	{
		input_pos = start;

		while (NULL != (input_pos = string_enumerate(input_pos, finish, &input_char_set)))
		{
			LIKE_OR_NOT();

			if (like == step)
			{
				return start;
			}

			start = input_pos;
		}
	}
	else
	{
		const uint8_t* result = finish;
		input_pos = finish;

		while (NULL != (input_pos = string_enumerate(input_pos, start, &input_char_set)))
		{
			LIKE_OR_NOT();

			if (like == step)
			{
				result = finish;
			}

			finish = input_pos;
		}

		start = result;
	}

	return start;
}

ptrdiff_t string_get_length(const uint8_t* input_start, const uint8_t* input_finish)
{
	if (input_start == input_finish)
	{
		return 0;
	}

	if (NULL == input_start || NULL == input_finish || input_finish < input_start)
	{
		return -1;
	}

	ptrdiff_t length = 0;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, NULL)))
	{
		++length;
	}

	return length;
}

ptrdiff_t string_index_of(const uint8_t* input_start, const uint8_t* input_finish,
						  const uint8_t* value_start, const uint8_t* value_finish)
{
	return string_index_of_value(input_start, input_finish, value_start, value_finish, 1);
}

ptrdiff_t string_last_index_of(const uint8_t* input_start, const uint8_t* input_finish,
							   const uint8_t* value_start, const uint8_t* value_finish)
{
	return string_index_of_value(input_start, input_finish, value_start, value_finish, -1);
}

ptrdiff_t string_index_of_any(const uint8_t* input_start, const uint8_t* input_finish,
							  const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		return -1;
	}

	ptrdiff_t index = -1, i = 0;
	uint32_t input_char_set, value_char_set;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, &input_char_set)))
	{
		const uint8_t* value_pos = value_start;

		while (NULL != (value_pos = string_enumerate(value_pos, value_finish, &value_char_set)))
		{
			if (input_char_set == value_char_set)
			{
				index = i;
				input_start = NULL;
				break;
			}
		}

		++i;
	}

	return index;
}

ptrdiff_t string_last_index_of_any(const uint8_t* input_start, const uint8_t* input_finish,
								   const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		return -1;
	}

	ptrdiff_t index = -1, i = 0;
	uint32_t input_char_set, value_char_set;

	while (NULL != (input_start = string_enumerate(input_start, input_finish, &input_char_set)))
	{
		const uint8_t* value_pos = value_start;

		while (NULL != (value_pos = string_enumerate(value_pos, value_finish, &value_char_set)))
		{
			if (input_char_set == value_char_set)
			{
				index = i;
				break;
			}
		}

		++i;
	}

	return index;
}

enum string_pad_side { string_pad_left_function = 0, string_pad_right_function };

uint8_t string_pad(const uint8_t* input_start, const uint8_t* input_finish,
				   const uint8_t* value_start, const uint8_t* value_finish,
				   ptrdiff_t result_length, struct buffer* output, enum string_pad_side side)
{
	if (input_finish < input_start ||
		range_in_parts_is_null_or_empty(value_start, value_finish) ||
		result_length < 0 ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t length = string_get_length(input_start, input_finish);

	if (result_length < length + 1)
	{
		return buffer_append(output, input_start, input_finish - input_start);
	}

	result_length -= length;
	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL,
					   input_finish - input_start + 4 * result_length + sizeof(uint32_t)) ||
		!buffer_resize(output, size + sizeof(uint32_t)))
	{
		return 0;
	}

	const uint8_t offset = text_encoding_decode_UTF8_single(
							   value_start, value_finish,
							   (uint32_t*)buffer_data(output, size));

	if (!offset ||
		!buffer_resize(output, size))
	{
		return 0;
	}

	if (string_pad_left_function == side)
	{
		while (result_length)
		{
			if (!buffer_append(output, value_start, offset))
			{
				return 0;
			}

			--result_length;
		}
	}

	if (!buffer_append(output, input_start, input_finish - input_start))
	{
		return 0;
	}

	if (string_pad_right_function == side)
	{
		while (result_length)
		{
			if (!buffer_append(output, value_start, offset))
			{
				return 0;
			}

			--result_length;
		}
	}

	return 1;
}

uint8_t string_pad_left(const uint8_t* input_start, const uint8_t* input_finish,
						const uint8_t* value_start, const uint8_t* value_finish,
						ptrdiff_t result_length, struct buffer* output)
{
	return string_pad(input_start, input_finish, value_start, value_finish,
					  result_length, output, string_pad_left_function);
}

uint8_t string_pad_right(const uint8_t* input_start, const uint8_t* input_finish,
						 const uint8_t* value_start, const uint8_t* value_finish,
						 ptrdiff_t result_length, struct buffer* output)
{
	return string_pad(input_start, input_finish, value_start, value_finish,
					  result_length, output, string_pad_right_function);
}

uint8_t string_replace(const uint8_t* input_start, const uint8_t* input_finish,
					   const uint8_t* to_be_replaced_start, const uint8_t* to_be_replaced_finish,
					   const uint8_t* by_replacement_start, const uint8_t* by_replacement_finish,
					   struct buffer* output)
{
	if (NULL == input_start || NULL == input_finish ||
		range_in_parts_is_null_or_empty(to_be_replaced_start, to_be_replaced_finish) ||
		NULL == output || input_finish < input_start)
	{
		return 0;
	}

	const ptrdiff_t input_length = input_finish - input_start;

	if (!input_length)
	{
		return 1;
	}

	if (string_equal(to_be_replaced_start, to_be_replaced_finish, by_replacement_start, by_replacement_finish))
	{
		return buffer_append(output, input_start, input_length);
	}

	const ptrdiff_t to_be_replaced_length = to_be_replaced_finish - to_be_replaced_start;
	const ptrdiff_t by_replacement_length = (NULL == by_replacement_start || NULL == by_replacement_finish ||
											by_replacement_finish < by_replacement_start) ? -1 : (by_replacement_finish - by_replacement_start);
	const uint8_t* start = input_start;

	while (NULL != input_start && to_be_replaced_length <= input_finish - input_start)
	{
		if (memcmp(input_start, to_be_replaced_start, to_be_replaced_length))
		{
			input_start = string_enumerate(input_start, input_finish, NULL);
			continue;
		}

		if (!buffer_append(output, start, input_start - start))
		{
			return 0;
		}

		if (-1 != by_replacement_length &&
			!buffer_append(output, by_replacement_start, by_replacement_length))
		{
			return 0;
		}

		input_start += to_be_replaced_length;
		start = input_start;
	}

	return buffer_append(output, start, input_finish - start);
}

uint8_t string_starts_with(const uint8_t* input_start, const uint8_t* input_finish,
						   const uint8_t* value_start, const uint8_t* value_finish)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		range_in_parts_is_null_or_empty(value_start, value_finish))
	{
		if (input_start == input_finish &&
			value_start == value_finish)
		{
			return 1;
		}

		return 0;
	}

	const ptrdiff_t length = value_finish - value_start;

	if (input_finish - input_start < length)
	{
		return 0;
	}

	return 0 == memcmp(input_start, value_start, length);
}

uint8_t string_substring(const uint8_t* input_start, const uint8_t* input_finish,
						 ptrdiff_t index, ptrdiff_t length, struct buffer* output)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		index < 0 ||
		NULL == output)
	{
		return 0;
	}

	const ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, (input_finish - input_start) + sizeof(uint32_t)))
	{
		return 0;
	}

	uint32_t* ptr = (uint32_t*)buffer_data(output, buffer_size(output) - sizeof(uint32_t));

	if (!buffer_resize(output, size))
	{
		return 0;
	}

	while (index)
	{
		const uint8_t offset = text_encoding_decode_UTF8_single(
								   input_start, input_finish, ptr);

		if (!offset)
		{
			return 0;
		}

		input_start += offset;
		--index;
	}

	if (length < 0)
	{
		return buffer_append(output, input_start, input_finish - input_start);
	}

	while (length)
	{
		const uint8_t offset = text_encoding_decode_UTF8_single(
								   input_start, input_finish, ptr);

		if (!offset ||
			!buffer_append(output, input_start, offset))
		{
			return 0;
		}

		input_start += offset;
		--length;
	}

	return 1;
}

#define CHANGE_CASE_A(I, C, D)	\
	if (string_get_id_of_to_lower_function() == (C))		\
	{							\
		if (!((I) % 2))			\
		{						\
			return (I) + (D);	\
		}						\
	}							\
	else						\
	{							\
		if ((I) % 2)			\
		{						\
			return (I) - (D);	\
		}						\
	}

#define CHANGE_CASE_B(I, C, D)	\
	if (string_get_id_of_to_lower_function() == (C))		\
	{							\
		if ((I) % 2)			\
		{						\
			return (I) + (D);	\
		}						\
	}							\
	else						\
	{							\
		if (!((I) % 2))			\
		{						\
			return (I) - (D);	\
		}						\
	}

uint32_t string_to_case(uint32_t input, uint8_t required_case)
{
	static const uint16_t upper[][2] =
	{
		{ 181, 924 }, { 215, 215 }, { 247, 247 }, { 255, 376 }, { 305, 73 }, { 383, 83 },
		{ 384, 579 }, { 405, 502 }, { 410, 573 }, { 414, 544 }, { 447, 503 }, { 453, 452 },
		{ 454, 452 }, { 456, 455 }, { 457, 455 }, { 459, 458 }, { 460, 458 }, { 477, 398 },
		{ 498, 497 }, { 499, 497 }, { 501, 500 }, { 572, 571 }, { 578, 577 }, { 592, 11375 },
		{ 593, 11373 }, { 595, 385 }, { 596, 390 }, { 598, 393 }, { 599, 394 }, { 601, 399 },
		{ 603, 400 }, { 608, 403 }, { 611, 404 }, { 616, 407 }, { 617, 406 }, { 619, 11362 },
		{ 623, 412 }, { 625, 11374 }, { 626, 413 }, { 629, 415 }, { 637, 11364 }, { 640, 422 },
		{ 643, 425 }, { 648, 430 }, { 649, 580 }, { 650, 433 }, { 651, 434 }, { 652, 581 },
		{ 658, 439 }, { 837, 921 }, { 887, 886 }, { 891, 1021 }, { 892, 1022 }, { 893, 1023 },
		{ 940, 902 }, { 941, 904 }, { 942, 905 }, { 943, 906 }, { 945, 913 }, { 946, 914 },
		{ 947, 915 }, { 948, 916 }, { 949, 917 }, { 950, 918 }, { 951, 919 }, { 952, 920 },
		{ 953, 921 }, { 954, 922 }, { 955, 923 }, { 956, 924 }, { 957, 925 }, { 958, 926 },
		{ 959, 927 }, { 960, 928 }, { 961, 929 }, { 962, 931 }, { 963, 931 }, { 964, 932 },
		{ 965, 933 }, { 966, 934 }, { 967, 935 }, { 968, 936 }, { 969, 937 }, { 970, 938 },
		{ 971, 939 }, { 972, 908 }, { 973, 910 }, { 974, 911 }, { 976, 914 }, { 977, 920 },
		{ 981, 934 }, { 982, 928 }, { 983, 975 }, { 1008, 922 }, { 1009, 929 }, { 1010, 1017 },
		{ 1013, 917 }, { 1016, 1015 }, { 1019, 1018 }, { 1072, 1040 }, { 1073, 1041 }, { 1074, 1042 },
		{ 1075, 1043 }, { 1076, 1044 }, { 1077, 1045 }, { 1078, 1046 }, { 1079, 1047 }, { 1080, 1048 },
		{ 1081, 1049 }, { 1082, 1050 }, { 1083, 1051 }, { 1084, 1052 }, { 1085, 1053 }, { 1086, 1054 },
		{ 1087, 1055 }, { 1088, 1056 }, { 1089, 1057 }, { 1090, 1058 }, { 1091, 1059 }, { 1092, 1060 },
		{ 1093, 1061 }, { 1094, 1062 }, { 1095, 1063 }, { 1096, 1064 }, { 1097, 1065 }, { 1098, 1066 },
		{ 1099, 1067 }, { 1100, 1068 }, { 1101, 1069 }, { 1102, 1070 }, { 1103, 1071 }, { 1104, 1024 },
		{ 1105, 1025 }, { 1106, 1026 }, { 1107, 1027 }, { 1108, 1028 }, { 1109, 1029 }, { 1110, 1030 },
		{ 1111, 1031 }, { 1112, 1032 }, { 1113, 1033 }, { 1114, 1034 }, { 1115, 1035 }, { 1116, 1036 },
		{ 1117, 1037 }, { 1118, 1038 }, { 1119, 1039 }, { 1231, 1216 }, { 1377, 1329 }, { 1378, 1330 },
		{ 1379, 1331 }, { 1380, 1332 }, { 1381, 1333 }, { 1382, 1334 }, { 1383, 1335 }, { 1384, 1336 },
		{ 1385, 1337 }, { 1386, 1338 }, { 1387, 1339 }, { 1388, 1340 }, { 1389, 1341 }, { 1390, 1342 },
		{ 1391, 1343 }, { 1392, 1344 }, { 1393, 1345 }, { 1394, 1346 }, { 1395, 1347 }, { 1396, 1348 },
		{ 1397, 1349 }, { 1398, 1350 }, { 1399, 1351 }, { 1400, 1352 }, { 1401, 1353 }, { 1402, 1354 },
		{ 1403, 1355 }, { 1404, 1356 }, { 1405, 1357 }, { 1406, 1358 }, { 1407, 1359 }, { 1408, 1360 },
		{ 1409, 1361 }, { 1410, 1362 }, { 1411, 1363 }, { 1412, 1364 }, { 1413, 1365 }, { 1414, 1366 },
		{ 7545, 42877 }, { 7549, 11363 }, { 7835, 7776 }, { 7936, 7944 }, { 7937, 7945 }, { 7938, 7946 },
		{ 7939, 7947 }, { 7940, 7948 }, { 7941, 7949 }, { 7942, 7950 }, { 7943, 7951 }, { 7952, 7960 },
		{ 7953, 7961 }, { 7954, 7962 }, { 7955, 7963 }, { 7956, 7964 }, { 7957, 7965 }, { 7968, 7976 },
		{ 7969, 7977 }, { 7970, 7978 }, { 7971, 7979 }, { 7972, 7980 }, { 7973, 7981 }, { 7974, 7982 },
		{ 7975, 7983 }, { 7984, 7992 }, { 7985, 7993 }, { 7986, 7994 }, { 7987, 7995 }, { 7988, 7996 },
		{ 7989, 7997 }, { 7990, 7998 }, { 7991, 7999 }, { 8000, 8008 }, { 8001, 8009 }, { 8002, 8010 },
		{ 8003, 8011 }, { 8004, 8012 }, { 8005, 8013 }, { 8017, 8025 }, { 8019, 8027 }, { 8021, 8029 },
		{ 8023, 8031 }, { 8032, 8040 }, { 8033, 8041 }, { 8034, 8042 }, { 8035, 8043 }, { 8036, 8044 },
		{ 8037, 8045 }, { 8038, 8046 }, { 8039, 8047 }, { 8048, 8122 }, { 8049, 8123 }, { 8050, 8136 },
		{ 8051, 8137 }, { 8052, 8138 }, { 8053, 8139 }, { 8054, 8154 }, { 8055, 8155 }, { 8056, 8184 },
		{ 8057, 8185 }, { 8058, 8170 }, { 8059, 8171 }, { 8060, 8186 }, { 8061, 8187 }, { 8064, 8072 },
		{ 8065, 8073 }, { 8066, 8074 }, { 8067, 8075 }, { 8068, 8076 }, { 8069, 8077 }, { 8070, 8078 },
		{ 8071, 8079 }, { 8080, 8088 }, { 8081, 8089 }, { 8082, 8090 }, { 8083, 8091 }, { 8084, 8092 },
		{ 8085, 8093 }, { 8086, 8094 }, { 8087, 8095 }, { 8096, 8104 }, { 8097, 8105 }, { 8098, 8106 },
		{ 8099, 8107 }, { 8100, 8108 }, { 8101, 8109 }, { 8102, 8110 }, { 8103, 8111 }, { 8112, 8120 },
		{ 8113, 8121 }, { 8115, 8124 }, { 8126, 921 }, { 8131, 8140 }, { 8144, 8152 }, { 8145, 8153 },
		{ 8160, 8168 }, { 8161, 8169 }, { 8165, 8172 }, { 8179, 8188 }, { 8526, 8498 }, { 8560, 8544 },
		{ 8561, 8545 }, { 8562, 8546 }, { 8563, 8547 }, { 8564, 8548 }, { 8565, 8549 }, { 8566, 8550 },
		{ 8567, 8551 }, { 8568, 8552 }, { 8569, 8553 }, { 8570, 8554 }, { 8571, 8555 }, { 8572, 8556 },
		{ 8573, 8557 }, { 8574, 8558 }, { 8575, 8559 }, { 8580, 8579 }, { 9424, 9398 }, { 9425, 9399 },
		{ 9426, 9400 }, { 9427, 9401 }, { 9428, 9402 }, { 9429, 9403 }, { 9430, 9404 }, { 9431, 9405 },
		{ 9432, 9406 }, { 9433, 9407 }, { 9434, 9408 }, { 9435, 9409 }, { 9436, 9410 }, { 9437, 9411 },
		{ 9438, 9412 }, { 9439, 9413 }, { 9440, 9414 }, { 9441, 9415 }, { 9442, 9416 }, { 9443, 9417 },
		{ 9444, 9418 }, { 9445, 9419 }, { 9446, 9420 }, { 9447, 9421 }, { 9448, 9422 }, { 9449, 9423 },
		{ 11312, 11264 }, { 11313, 11265 }, { 11314, 11266 }, { 11315, 11267 }, { 11316, 11268 }, { 11317, 11269 },
		{ 11318, 11270 }, { 11319, 11271 }, { 11320, 11272 }, { 11321, 11273 }, { 11322, 11274 }, { 11323, 11275 },
		{ 11324, 11276 }, { 11325, 11277 }, { 11326, 11278 }, { 11327, 11279 }, { 11328, 11280 }, { 11329, 11281 },
		{ 11330, 11282 }, { 11331, 11283 }, { 11332, 11284 }, { 11333, 11285 }, { 11334, 11286 }, { 11335, 11287 },
		{ 11336, 11288 }, { 11337, 11289 }, { 11338, 11290 }, { 11339, 11291 }, { 11340, 11292 }, { 11341, 11293 },
		{ 11342, 11294 }, { 11343, 11295 }, { 11344, 11296 }, { 11345, 11297 }, { 11346, 11298 }, { 11347, 11299 },
		{ 11348, 11300 }, { 11349, 11301 }, { 11350, 11302 }, { 11351, 11303 }, { 11352, 11304 }, { 11353, 11305 },
		{ 11354, 11306 }, { 11355, 11307 }, { 11356, 11308 }, { 11357, 11309 }, { 11358, 11310 }, { 11361, 11360 },
		{ 11365, 570 }, { 11366, 574 }, { 11379, 11378 }, { 11382, 11381 }, { 11520, 4256 }, { 11521, 4257 },
		{ 11522, 4258 }, { 11523, 4259 }, { 11524, 4260 }, { 11525, 4261 }, { 11526, 4262 }, { 11527, 4263 },
		{ 11528, 4264 }, { 11529, 4265 }, { 11530, 4266 }, { 11531, 4267 }, { 11532, 4268 }, { 11533, 4269 },
		{ 11534, 4270 }, { 11535, 4271 }, { 11536, 4272 }, { 11537, 4273 }, { 11538, 4274 }, { 11539, 4275 },
		{ 11540, 4276 }, { 11541, 4277 }, { 11542, 4278 }, { 11543, 4279 }, { 11544, 4280 }, { 11545, 4281 },
		{ 11546, 4282 }, { 11547, 4283 }, { 11548, 4284 }, { 11549, 4285 }, { 11550, 4286 }, { 11551, 4287 },
		{ 11552, 4288 }, { 11553, 4289 }, { 11554, 4290 }, { 11555, 4291 }, { 11556, 4292 }, { 11557, 4293 },
		{ 42892, 42891 }, { 65345, 65313 }, { 65346, 65314 }, { 65347, 65315 }, { 65348, 65316 }, { 65349, 65317 },
		{ 65350, 65318 }, { 65351, 65319 }, { 65352, 65320 }, { 65353, 65321 }, { 65354, 65322 }, { 65355, 65323 },
		{ 65356, 65324 }, { 65357, 65325 }, { 65358, 65326 }, { 65359, 65327 }, { 65360, 65328 }, { 65361, 65329 },
		{ 65362, 65330 }, { 65363, 65331 }, { 65364, 65332 }, { 65365, 65333 }, { 65366, 65334 }, { 65367, 65335 },
		{ 65368, 65336 }, { 65369, 65337 }, { 65370, 65338 }
	};
	/**/
	static const uint16_t lower[][2] =
	{
		{ 215, 215 }, { 247, 247 }, { 376, 255 }, { 304, 105 }, { 376, 255 }, { 385, 595 },
		{ 390, 596 }, { 393, 598 }, { 394, 599 }, { 398, 477 }, { 399, 601 }, { 400, 603 },
		{ 403, 608 }, { 404, 611 }, { 406, 617 }, { 407, 616 }, { 412, 623 }, { 413, 626 },
		{ 415, 629 }, { 422, 640 }, { 425, 643 }, { 430, 648 }, { 433, 650 }, { 434, 651 },
		{ 439, 658 }, { 452, 454 }, { 453, 454 }, { 455, 457 }, { 456, 457 }, { 458, 460 },
		{ 459, 460 }, { 497, 499 }, { 498, 499 }, { 500, 501 }, { 502, 405 }, { 503, 447 },
		{ 544, 414 }, { 570, 11365 }, { 571, 572 }, { 573, 410 }, { 574, 11366 }, { 577, 578 },
		{ 579, 384 }, { 580, 649 }, { 581, 652 }, { 886, 887 }, { 902, 940 }, { 904, 941 },
		{ 905, 942 }, { 906, 943 }, { 908, 972 }, { 910, 973 }, { 911, 974 }, { 913, 945 },
		{ 914, 946 }, { 915, 947 }, { 916, 948 }, { 917, 949 }, { 918, 950 }, { 919, 951 },
		{ 920, 952 }, { 921, 953 }, { 922, 954 }, { 923, 955 }, { 924, 956 }, { 925, 957 },
		{ 926, 958 }, { 927, 959 }, { 928, 960 }, { 929, 961 }, { 931, 963 }, { 932, 964 },
		{ 933, 965 }, { 934, 966 }, { 935, 967 }, { 936, 968 }, { 937, 969 }, { 938, 970 },
		{ 939, 971 }, { 975, 983 }, { 978, 965 }, { 979, 973 }, { 980, 971 }, { 1012, 952 },
		{ 1015, 1016 }, { 1017, 1010 }, { 1018, 1019 }, { 1021, 891 }, { 1022, 892 }, { 1023, 893 },
		{ 1024, 1104 }, { 1025, 1105 }, { 1026, 1106 }, { 1027, 1107 }, { 1028, 1108 }, { 1029, 1109 },
		{ 1030, 1110 }, { 1031, 1111 }, { 1032, 1112 }, { 1033, 1113 }, { 1034, 1114 }, { 1035, 1115 },
		{ 1036, 1116 }, { 1037, 1117 }, { 1038, 1118 }, { 1039, 1119 }, { 1040, 1072 }, { 1041, 1073 },
		{ 1042, 1074 }, { 1043, 1075 }, { 1044, 1076 }, { 1045, 1077 }, { 1046, 1078 }, { 1047, 1079 },
		{ 1048, 1080 }, { 1049, 1081 }, { 1050, 1082 }, { 1051, 1083 }, { 1052, 1084 }, { 1053, 1085 },
		{ 1054, 1086 }, { 1055, 1087 }, { 1056, 1088 }, { 1057, 1089 }, { 1058, 1090 }, { 1059, 1091 },
		{ 1060, 1092 }, { 1061, 1093 }, { 1062, 1094 }, { 1063, 1095 }, { 1064, 1096 }, { 1065, 1097 },
		{ 1066, 1098 }, { 1067, 1099 }, { 1068, 1100 }, { 1069, 1101 }, { 1070, 1102 }, { 1071, 1103 },
		{ 1216, 1231 }, { 1329, 1377 }, { 1330, 1378 }, { 1331, 1379 }, { 1332, 1380 }, { 1333, 1381 },
		{ 1334, 1382 }, { 1335, 1383 }, { 1336, 1384 }, { 1337, 1385 }, { 1338, 1386 }, { 1339, 1387 },
		{ 1340, 1388 }, { 1341, 1389 }, { 1342, 1390 }, { 1343, 1391 }, { 1344, 1392 }, { 1345, 1393 },
		{ 1346, 1394 }, { 1347, 1395 }, { 1348, 1396 }, { 1349, 1397 }, { 1350, 1398 }, { 1351, 1399 },
		{ 1352, 1400 }, { 1353, 1401 }, { 1354, 1402 }, { 1355, 1403 }, { 1356, 1404 }, { 1357, 1405 },
		{ 1358, 1406 }, { 1359, 1407 }, { 1360, 1408 }, { 1361, 1409 }, { 1362, 1410 }, { 1363, 1411 },
		{ 1364, 1412 }, { 1365, 1413 }, { 1366, 1414 }, { 4256, 11520 }, { 4257, 11521 }, { 4258, 11522 },
		{ 4259, 11523 }, { 4260, 11524 }, { 4261, 11525 }, { 4262, 11526 }, { 4263, 11527 }, { 4264, 11528 },
		{ 4265, 11529 }, { 4266, 11530 }, { 4267, 11531 }, { 4268, 11532 }, { 4269, 11533 }, { 4270, 11534 },
		{ 4271, 11535 }, { 4272, 11536 }, { 4273, 11537 }, { 4274, 11538 }, { 4275, 11539 }, { 4276, 11540 },
		{ 4277, 11541 }, { 4278, 11542 }, { 4279, 11543 }, { 4280, 11544 }, { 4281, 11545 }, { 4282, 11546 },
		{ 4283, 11547 }, { 4284, 11548 }, { 4285, 11549 }, { 4286, 11550 }, { 4287, 11551 }, { 4288, 11552 },
		{ 4289, 11553 }, { 4290, 11554 }, { 4291, 11555 }, { 4292, 11556 }, { 4293, 11557 }, { 7838, 223 },
		{ 7944, 7936 }, { 7945, 7937 }, { 7946, 7938 }, { 7947, 7939 }, { 7948, 7940 }, { 7949, 7941 },
		{ 7950, 7942 }, { 7951, 7943 }, { 7960, 7952 }, { 7961, 7953 }, { 7962, 7954 }, { 7963, 7955 },
		{ 7964, 7956 }, { 7965, 7957 }, { 7976, 7968 }, { 7977, 7969 }, { 7978, 7970 }, { 7979, 7971 },
		{ 7980, 7972 }, { 7981, 7973 }, { 7982, 7974 }, { 7983, 7975 }, { 7992, 7984 }, { 7993, 7985 },
		{ 7994, 7986 }, { 7995, 7987 }, { 7996, 7988 }, { 7997, 7989 }, { 7998, 7990 }, { 7999, 7991 },
		{ 8008, 8000 }, { 8009, 8001 }, { 8010, 8002 }, { 8011, 8003 }, { 8012, 8004 }, { 8013, 8005 },
		{ 8025, 8017 }, { 8027, 8019 }, { 8029, 8021 }, { 8031, 8023 }, { 8040, 8032 }, { 8041, 8033 },
		{ 8042, 8034 }, { 8043, 8035 }, { 8044, 8036 }, { 8045, 8037 }, { 8046, 8038 }, { 8047, 8039 },
		{ 8072, 8064 }, { 8073, 8065 }, { 8074, 8066 }, { 8075, 8067 }, { 8076, 8068 }, { 8077, 8069 },
		{ 8078, 8070 }, { 8079, 8071 }, { 8088, 8080 }, { 8089, 8081 }, { 8090, 8082 }, { 8091, 8083 },
		{ 8092, 8084 }, { 8093, 8085 }, { 8094, 8086 }, { 8095, 8087 }, { 8104, 8096 }, { 8105, 8097 },
		{ 8106, 8098 }, { 8107, 8099 }, { 8108, 8100 }, { 8109, 8101 }, { 8110, 8102 }, { 8111, 8103 },
		{ 8120, 8112 }, { 8121, 8113 }, { 8122, 8048 }, { 8123, 8049 }, { 8124, 8115 }, { 8136, 8050 },
		{ 8137, 8051 }, { 8138, 8052 }, { 8139, 8053 }, { 8140, 8131 }, { 8152, 8144 }, { 8153, 8145 },
		{ 8154, 8054 }, { 8155, 8055 }, { 8168, 8160 }, { 8169, 8161 }, { 8170, 8058 }, { 8171, 8059 },
		{ 8172, 8165 }, { 8184, 8056 }, { 8185, 8057 }, { 8186, 8060 }, { 8187, 8061 }, { 8188, 8179 },
		{ 8486, 969 }, { 8490, 107 }, { 8491, 229 }, { 8498, 8526 }, { 8544, 8560 }, { 8545, 8561 },
		{ 8546, 8562 }, { 8547, 8563 }, { 8548, 8564 }, { 8549, 8565 }, { 8550, 8566 }, { 8551, 8567 },
		{ 8552, 8568 }, { 8553, 8569 }, { 8554, 8570 }, { 8555, 8571 }, { 8556, 8572 }, { 8557, 8573 },
		{ 8558, 8574 }, { 8559, 8575 }, { 8579, 8580 }, { 9398, 9424 }, { 9399, 9425 }, { 9400, 9426 },
		{ 9401, 9427 }, { 9402, 9428 }, { 9403, 9429 }, { 9404, 9430 }, { 9405, 9431 }, { 9406, 9432 },
		{ 9407, 9433 }, { 9408, 9434 }, { 9409, 9435 }, { 9410, 9436 }, { 9411, 9437 }, { 9412, 9438 },
		{ 9413, 9439 }, { 9414, 9440 }, { 9415, 9441 }, { 9416, 9442 }, { 9417, 9443 }, { 9418, 9444 },
		{ 9419, 9445 }, { 9420, 9446 }, { 9421, 9447 }, { 9422, 9448 }, { 9423, 9449 }, { 11264, 11312 },
		{ 11265, 11313 }, { 11266, 11314 }, { 11267, 11315 }, { 11268, 11316 }, { 11269, 11317 },
		{ 11270, 11318 }, { 11271, 11319 }, { 11272, 11320 }, { 11273, 11321 }, { 11274, 11322 },
		{ 11275, 11323 }, { 11276, 11324 }, { 11277, 11325 }, { 11278, 11326 }, { 11279, 11327 },
		{ 11280, 11328 }, { 11281, 11329 }, { 11282, 11330 }, { 11283, 11331 }, { 11284, 11332 },
		{ 11285, 11333 }, { 11286, 11334 }, { 11287, 11335 }, { 11288, 11336 }, { 11289, 11337 },
		{ 11290, 11338 }, { 11291, 11339 }, { 11292, 11340 }, { 11293, 11341 }, { 11294, 11342 },
		{ 11295, 11343 }, { 11296, 11344 }, { 11297, 11345 }, { 11298, 11346 }, { 11299, 11347 },
		{ 11300, 11348 }, { 11301, 11349 }, { 11302, 11350 }, { 11303, 11351 }, { 11304, 11352 },
		{ 11305, 11353 }, { 11306, 11354 }, { 11307, 11355 }, { 11308, 11356 }, { 11309, 11357 },
		{ 11310, 11358 }, { 11360, 11361 }, { 11362, 619 }, { 11363, 7549 }, { 11364, 637 }, { 11373, 593 },
		{ 11374, 625 }, { 11375, 592 }, { 11378, 11379 }, { 11381, 11382 }, { 42877, 7545 }, { 42891, 42892 },
		{ 65313, 65345 }, { 65314, 65346 }, { 65315, 65347 }, { 65316, 65348 }, { 65317, 65349 }, { 65318, 65350 },
		{ 65319, 65351 }, { 65320, 65352 }, { 65321, 65353 }, { 65322, 65354 }, { 65323, 65355 }, { 65324, 65356 },
		{ 65325, 65357 }, { 65326, 65358 }, { 65327, 65359 }, { 65328, 65360 }, { 65329, 65361 }, { 65330, 65362 },
		{ 65331, 65363 }, { 65332, 65364 }, { 65333, 65365 }, { 65334, 65366 }, { 65335, 65367 }, { 65336, 65368 },
		{ 65337, 65369 }, { 65338, 65370 }
	};

	if (input < INT8_MAX)
	{
		return string_get_id_of_to_lower_function() == required_case ? tolower(input) : toupper(input);
	}

	if (string_get_id_of_to_upper_function() == required_case)
	{
		for (uint16_t i = 0, count = COUNT_OF(upper); i < count; ++i)
		{
			if (input == upper[i][0])
			{
				return upper[i][1];
			}
		}

		if (224 <= input && input <= 254)
		{
			return input - 32;
		}
	}
	else
	{
		for (uint16_t i = 0, count = COUNT_OF(lower); i < count; ++i)
		{
			if (input == lower[i][0])
			{
				return lower[i][1];
			}
		}

		if (192 <= input && input <= 222)
		{
			return input + 32;
		}
	}

	if ((256 <= input && input <= 311) ||
		(330 <= input && input <= 375) ||
		(386 <= input && input <= 389) ||
		(408 <= input && input <= 409) ||
		(416 <= input && input <= 422) ||
		(428 <= input && input <= 429) ||
		(440 <= input && input <= 441) ||
		(444 <= input && input <= 445) ||
		(478 <= input && input <= 495) ||
		(504 <= input && input <= 543) ||
		(546 <= input && input <= 563) ||
		(582 <= input && input <= 591) ||
		(880 <= input && input <= 883) ||
		(984 <= input && input <= 1007) ||
		(1120 <= input && input <= 1215) ||
		(1232 <= input && input <= 1315) ||
		(7680 <= input && input <= 7829) ||
		(7840 <= input && input <= 7935) ||
		(11392 <= input && input <= 11491) ||
		(42560 <= input && input <= 42591) ||
		(42594 <= input && input <= 42605) ||
		(42624 <= input && input <= 42647) ||
		(42786 <= input && input <= 42799) ||
		(42802 <= input && input <= 42863) ||
		(42878 <= input && input <= 42887))
	{
		CHANGE_CASE_A(input, required_case, 1);
	}

	if ((313 <= input && input <= 328) ||
		(377 <= input && input <= 382) ||
		(391 <= input && input <= 392) ||
		(395 <= input && input <= 396) ||
		(401 <= input && input <= 402) ||
		(423 <= input && input <= 427) ||
		(431 <= input && input <= 433) ||
		(435 <= input && input <= 438) ||
		(461 <= input && input <= 476) ||
		(1217 <= input && input <= 1230) ||
		(11367 <= input && input <= 11372) ||
		(42873 <= input && input <= 42876))
	{
		CHANGE_CASE_B(input, required_case, 1);
	}

	return input;
}

uint8_t string_transform_to_case(const uint8_t* input_start, const uint8_t* input_finish,
								 struct buffer* output, uint8_t required_case)
{
	if (range_in_parts_is_null_or_empty(input_start, input_finish) ||
		NULL == output ||
		(string_get_id_of_to_lower_function() != required_case &&
		 string_get_id_of_to_upper_function() != required_case))
	{
		return 0;
	}

	ptrdiff_t size = buffer_size(output);

	if (!buffer_append(output, NULL, 2 * (input_finish - input_start)))
	{
		return 0;
	}

	uint8_t offset;
	uint32_t input;

	while (0 < (offset = text_encoding_decode_UTF8_single(input_start, input_finish, &input)))
	{
		input_start += offset;
		input = string_to_case(input, required_case);

		if (!buffer_append(output, NULL, 4) ||
			(offset = text_encoding_encode_UTF8_single(input, buffer_data(output, size))) < 1 ||
			!buffer_resize(output, size + offset))
		{
			return 0;
		}

		size += offset;
	}

	return 1;
}

uint8_t string_to_lower(const uint8_t* input_start, const uint8_t* input_finish, struct buffer* output)
{
	return string_transform_to_case(input_start, input_finish, output, string_get_id_of_to_lower_function());
}

uint8_t string_to_upper(const uint8_t* input_start, const uint8_t* input_finish, struct buffer* output)
{
	return string_transform_to_case(input_start, input_finish, output, string_get_id_of_to_upper_function());
}

uint8_t string_trim_any(struct range* input_output, uint8_t mode, const uint16_t* trim_symbols,
						uint8_t count_of_symbols)
{
	static const uint16_t internal_trim_symbols[] =
	{
		'\0', '\t', '\n', 11, 12, '\r', ' ', 133, 160,
		5760, 8192, 8193, 8194, 8195, 8196, 8197, 8198,
		8199, 8200, 8201, 8202, 8232, 8233, 8239, 8287,
		12288
	};

	if (NULL == trim_symbols)
	{
		trim_symbols = internal_trim_symbols;
		count_of_symbols = COUNT_OF(internal_trim_symbols);
	}

	if (NULL != input_output && input_output->start == input_output->finish)
	{
		return 1;
	}

	if (NULL == input_output ||
		NULL == input_output->start ||
		NULL == input_output->finish ||
		input_output->finish < input_output->start ||
		(string_get_id_of_trim_function() != mode &&
		 string_get_id_of_trim_end_function() != mode &&
		 string_get_id_of_trim_start_function() != mode))
	{
		return 0;
	}

	uint32_t out;
	const uint32_t* finish = &out + 1;

	if (string_get_id_of_trim_function() == mode ||
		string_get_id_of_trim_start_function() == mode)
	{
		const uint8_t* pos = NULL;

		while (NULL != (pos = string_enumerate(input_output->start, input_output->finish, &out)))
		{
			if (finish == find_any_symbol_like_or_not_like_that_UTF32LE(
					&out, finish, trim_symbols, count_of_symbols, 1, 1))
			{
				break;
			}
			else
			{
				input_output->start = pos;
			}
		}
	}

	if (input_output->start < input_output->finish &&
		(string_get_id_of_trim_function() == mode ||
		 string_get_id_of_trim_end_function() == mode))
	{
		mode = 0;
		/**/
		const uint8_t* expected_finish = input_output->finish;
		const uint8_t* pos = input_output->start;
		const uint8_t* prev_pos = pos;

		while (NULL != (pos = string_enumerate(prev_pos, input_output->finish, &out)))
		{
			if (finish == find_any_symbol_like_or_not_like_that_UTF32LE(
					&out, finish, trim_symbols, count_of_symbols, 1, 1))
			{
				mode = 0;
				expected_finish = input_output->finish;
			}
			else
			{
				if (!mode)
				{
					expected_finish = prev_pos;
				}

				mode = 1;
			}

			prev_pos = pos;
		}

		if (expected_finish != input_output->finish)
		{
			input_output->finish = expected_finish;
		}
	}

	return 1;
}

uint8_t string_trim(struct range* input_output)
{
	return string_trim_any(input_output, string_get_id_of_trim_function(), NULL, 0);
}

uint8_t string_trim_end(struct range* input_output)
{
	return string_trim_any(input_output, string_get_id_of_trim_end_function(), NULL, 0);
}

uint8_t string_trim_start(struct range* input_output)
{
	return string_trim_any(input_output, string_get_id_of_trim_start_function(), NULL, 0);
}

uint8_t string_quote(const uint8_t* input_start, const uint8_t* input_finish,
					 struct buffer* output)
{
	if (NULL == output)
	{
		return 0;
	}

	if (!buffer_push_back(output, quote_symbols[0]))
	{
		return 0;
	}

	if (!range_in_parts_is_null_or_empty(input_start, input_finish) &&
		!buffer_append(output, input_start, input_finish - input_start))
	{
		return 0;
	}

	return buffer_push_back(output, quote_symbols[0]);
}

uint8_t string_un_quote(struct range* input_output)
{
	if (NULL == input_output ||
		NULL == input_output->start ||
		NULL == input_output->finish ||
		input_output->finish < input_output->start)
	{
		return 0;
	}

	input_output->start = find_any_symbol_like_or_not_like_that(
							  input_output->start, input_output->finish, quote_symbols, 2, 0, 1);

	if (input_output->finish == input_output->start)
	{
		return 1;
	}

	input_output->finish = 1 + find_any_symbol_like_or_not_like_that(
							   input_output->finish - 1, input_output->start, quote_symbols, 2, 0, -1);
	return 1;
}

uint8_t string_equal(const uint8_t* input_1_start, const uint8_t* input_1_finish,
					 const uint8_t* input_2_start, const uint8_t* input_2_finish)
{
	if (NULL == input_1_start || NULL == input_1_finish ||
		NULL == input_2_start || NULL == input_2_finish ||
		input_1_finish < input_1_start || input_2_finish < input_2_start ||
		input_1_finish - input_1_start != input_2_finish - input_2_start)
	{
		return 0;
	}

	if (0 == input_1_finish - input_1_start)
	{
		return 1;
	}

	if (input_1_start == input_2_start)
	{
		return 1;
	}

	return 0 == memcmp(input_1_start, input_2_start, input_1_finish - input_1_start);
}
