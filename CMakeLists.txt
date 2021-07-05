cmake_minimum_required(VERSION 2.8.12)

project("sample")

add_executable(
  tests
  "${CMAKE_SOURCE_DIR}/buffer.c"
  "${CMAKE_SOURCE_DIR}/buffer.h"
  "${CMAKE_SOURCE_DIR}/common.c"
  "${CMAKE_SOURCE_DIR}/common.h"
  "${CMAKE_SOURCE_DIR}/conversion.c"
  "${CMAKE_SOURCE_DIR}/conversion.h"
  "${CMAKE_SOURCE_DIR}/host_fxr.c"
  "${CMAKE_SOURCE_DIR}/host_fxr.h"
  "${CMAKE_SOURCE_DIR}/main.cpp"
  "${CMAKE_SOURCE_DIR}/net.common.h"
  "${CMAKE_SOURCE_DIR}/range.c"
  "${CMAKE_SOURCE_DIR}/range.h"
  "${CMAKE_SOURCE_DIR}/shared_object.c"
  "${CMAKE_SOURCE_DIR}/shared_object.h"
  "${CMAKE_SOURCE_DIR}/stdc_secure_api.h"
  "${CMAKE_SOURCE_DIR}/string_unit.c"
  "${CMAKE_SOURCE_DIR}/string_unit.h"
  "${CMAKE_SOURCE_DIR}/text_encoding.c"
  "${CMAKE_SOURCE_DIR}/text_encoding.h"
)

find_library(DL_LIB dl)

if("DL_LIB-NOTFOUND" STREQUAL DL_LIB)
  set(DL_LIB "")
endif()

target_link_libraries(
  tests
  ${DL_LIB}
)
