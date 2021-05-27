cmake_minimum_required(VERSION 2.8.12)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Configuration process cannot start from project source directory.")
endif()

project("Ant4C")

if(CMAKE_VERSION VERSION_LESS "3.0")
if(DEFINED ENV{CFLAGS})
  list(APPEND CMAKE_C_FLAGS $ENV{CFLAGS})
endif()

if(DEFINED ENV{CPPFLAGS})
  list(APPEND CMAKE_CXX_FLAGS $ENV{CPPFLAGS})
endif()

if(DEFINED ENV{CXXFLAGS})
  list(APPEND CMAKE_CXX_FLAGS $ENV{CXXFLAGS})
endif()

if(DEFINED ENV{LDFLAGS})
  list(APPEND LINK_FLAGS $ENV{LDFLAGS})
endif()

if(DEFINED CFLAGS)
  list(APPEND CMAKE_C_FLAGS ${CFLAGS})
endif()

if(DEFINED CPPFLAGS)
  list(APPEND CMAKE_CXX_FLAGS ${CPPFLAGS})
endif()

if(DEFINED CXXFLAGS)
  list(APPEND CMAKE_CXX_FLAGS ${CXXFLAGS})
endif()

if(DEFINED LDFLAGS)
  list(APPEND LINK_FLAGS ${LDFLAGS})
endif()
endif()

if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

find_library(DL_LIB dl)
find_library(M_LIB m)

if("DL_LIB-NOTFOUND" STREQUAL DL_LIB)
  set(DL_LIB "")
endif()

if("M_LIB-NOTFOUND" STREQUAL M_LIB)
  set(M_LIB "")
endif()

set(
  ANT4C_SOURCES
  "${CMAKE_SOURCE_DIR}/argument_parser.c"
  "${CMAKE_SOURCE_DIR}/argument_parser.h"
  "${CMAKE_SOURCE_DIR}/buffer.c"
  "${CMAKE_SOURCE_DIR}/buffer.h"
  "${CMAKE_SOURCE_DIR}/choose_task.c"
  "${CMAKE_SOURCE_DIR}/choose_task.h"
  "${CMAKE_SOURCE_DIR}/common.c"
  "${CMAKE_SOURCE_DIR}/common.h"
  "${CMAKE_SOURCE_DIR}/conversion.c"
  "${CMAKE_SOURCE_DIR}/conversion.h"
  "${CMAKE_SOURCE_DIR}/copy_move.c"
  "${CMAKE_SOURCE_DIR}/copy_move.h"
  "${CMAKE_SOURCE_DIR}/date_time.c"
  "${CMAKE_SOURCE_DIR}/date_time.h"
  "${CMAKE_SOURCE_DIR}/echo.c"
  "${CMAKE_SOURCE_DIR}/echo.h"
  "${CMAKE_SOURCE_DIR}/environment.c"
  "${CMAKE_SOURCE_DIR}/environment.h"
  "${CMAKE_SOURCE_DIR}/exec.c"
  "${CMAKE_SOURCE_DIR}/exec.h"
  "${CMAKE_SOURCE_DIR}/fail_task.c"
  "${CMAKE_SOURCE_DIR}/fail_task.h"
  "${CMAKE_SOURCE_DIR}/file_system.c"
  "${CMAKE_SOURCE_DIR}/file_system.h"
  "${CMAKE_SOURCE_DIR}/for_each.c"
  "${CMAKE_SOURCE_DIR}/for_each.h"
  "${CMAKE_SOURCE_DIR}/hash.blake2.c"
  "${CMAKE_SOURCE_DIR}/hash.blake3.c"
  "${CMAKE_SOURCE_DIR}/hash.c"
  "${CMAKE_SOURCE_DIR}/hash.crc32.c"
  "${CMAKE_SOURCE_DIR}/hash.h"
  "${CMAKE_SOURCE_DIR}/hash.sha3.c"
  "${CMAKE_SOURCE_DIR}/hash.xxhash.c"
  "${CMAKE_SOURCE_DIR}/if_task.c"
  "${CMAKE_SOURCE_DIR}/if_task.h"
  "${CMAKE_SOURCE_DIR}/interpreter.c"
  "${CMAKE_SOURCE_DIR}/interpreter.h"
  "${CMAKE_SOURCE_DIR}/listener.c"
  "${CMAKE_SOURCE_DIR}/listener.h"
  "${CMAKE_SOURCE_DIR}/load_file.c"
  "${CMAKE_SOURCE_DIR}/load_file.h"
  "${CMAKE_SOURCE_DIR}/load_tasks.c"
  "${CMAKE_SOURCE_DIR}/load_tasks.h"
  "${CMAKE_SOURCE_DIR}/math_unit.c"
  "${CMAKE_SOURCE_DIR}/math_unit.h"
  "${CMAKE_SOURCE_DIR}/operating_system.c"
  "${CMAKE_SOURCE_DIR}/operating_system.h"
  "${CMAKE_SOURCE_DIR}/path.c"
  "${CMAKE_SOURCE_DIR}/path.h"
  "${CMAKE_SOURCE_DIR}/project.c"
  "${CMAKE_SOURCE_DIR}/project.h"
  "${CMAKE_SOURCE_DIR}/property.c"
  "${CMAKE_SOURCE_DIR}/property.h"
  "${CMAKE_SOURCE_DIR}/range.c"
  "${CMAKE_SOURCE_DIR}/range.h"
  "${CMAKE_SOURCE_DIR}/shared_object.c"
  "${CMAKE_SOURCE_DIR}/shared_object.h"
  "${CMAKE_SOURCE_DIR}/sleep_unit.c"
  "${CMAKE_SOURCE_DIR}/sleep_unit.h"
  "${CMAKE_SOURCE_DIR}/stdc_secure_api.h"
  "${CMAKE_SOURCE_DIR}/string_unit.c"
  "${CMAKE_SOURCE_DIR}/string_unit.h"
  "${CMAKE_SOURCE_DIR}/target.c"
  "${CMAKE_SOURCE_DIR}/target.h"
  "${CMAKE_SOURCE_DIR}/task.c"
  "${CMAKE_SOURCE_DIR}/task.h"
  "${CMAKE_SOURCE_DIR}/text_encoding.c"
  "${CMAKE_SOURCE_DIR}/text_encoding.h"
  "${CMAKE_SOURCE_DIR}/try_catch.c"
  "${CMAKE_SOURCE_DIR}/try_catch.h"
  "${CMAKE_SOURCE_DIR}/version.c"
  "${CMAKE_SOURCE_DIR}/version.h"
  "${CMAKE_SOURCE_DIR}/xml.c"
  "${CMAKE_SOURCE_DIR}/xml.h"
)

# ant4c

add_library(ant4c STATIC ${ANT4C_SOURCES})

if(NOT MSVC)
  set_property(TARGET ant4c PROPERTY C_STANDARD 11)
endif()

target_compile_options(ant4c PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

# ant4c_shared

add_library(ant4c_shared SHARED ${ANT4C_SOURCES})
set_property(TARGET ant4c_shared APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/ant4c_shared")
set_property(TARGET ant4c_shared APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/ant4c_shared")
set_property(TARGET ant4c_shared APPEND PROPERTY OUTPUT_NAME ant4c)

if(NOT MSVC)
  set_property(TARGET ant4c_shared PROPERTY C_STANDARD 11)
  target_link_libraries(ant4c_shared ${DL_LIB} ${M_LIB})
endif()

target_compile_options(ant4c_shared PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

# ant4c_app

add_executable(
  ant4c_app
  "${CMAKE_SOURCE_DIR}/CHANGELOG"
  "${CMAKE_SOURCE_DIR}/LICENSE"
  "${CMAKE_SOURCE_DIR}/MD5SUMS"
  "${CMAKE_SOURCE_DIR}/README.md"
  "${CMAKE_SOURCE_DIR}/SHA3-224SUMS"
  "${CMAKE_SOURCE_DIR}/help.adoc"
  "${CMAKE_SOURCE_DIR}/main.c"
)

target_link_libraries(ant4c_app ant4c
  ${DL_LIB} ${M_LIB}
)
set_property(TARGET ant4c_app APPEND PROPERTY OUTPUT_NAME ant4c)

if(NOT MSVC)
  set_property(TARGET ant4c_app PROPERTY C_STANDARD 11)
endif()

target_compile_options(ant4c_app PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

# Set program version

if(DEFINED PROGRAM_VERSION)
  target_compile_definitions(ant4c PRIVATE -DPROGRAM_VERSION=${PROGRAM_VERSION})
  target_compile_definitions(ant4c_app PRIVATE -DPROGRAM_VERSION=${PROGRAM_VERSION})
  target_compile_definitions(ant4c_shared PRIVATE -DPROGRAM_VERSION=${PROGRAM_VERSION})
endif()

# Modules

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/ant4c.net.framework")

if(DEFINED USE_BOOST)
  add_subdirectory("${CMAKE_SOURCE_DIR}/modules/dns")
  add_subdirectory("${CMAKE_SOURCE_DIR}/modules/regex")
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/net")
include("${CMAKE_SOURCE_DIR}/modules/example.cmake")

# Listener

include("${CMAKE_SOURCE_DIR}/default_listener.cmake")

# Tests

include("${CMAKE_SOURCE_DIR}/tests.cmake")
