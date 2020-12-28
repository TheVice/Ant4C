cmake_minimum_required(VERSION 2.8.12)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Configuration process cannot start from project source directory.")
endif()

project("Ant4C")

if(MSVC)
  if (("v140_clang_c2" STREQUAL CMAKE_GENERATOR_TOOLSET) OR ("v141_clang_c2" STREQUAL CMAKE_GENERATOR_TOOLSET))
    set(FLAGS "${FLAGS} -Wall /GS")
  else()
    set(FLAGS "${FLAGS} /W4 /GS")
  endif()

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEFAULT_CMAKE_C_FLAGS} ${FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEFAULT_CMAKE_CXX_FLAGS} ${FLAGS}")

  if(CMAKE_CL_64)
    set(LINK_FLAGS "${LINK_FLAGS} /DynamicBase /NXCompat")
  else()
    set(LINK_FLAGS "${LINK_FLAGS} /SafeSEH /DynamicBase /NXCompat")
  endif()
else()
  set(FLAGS "${FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEFAULT_CMAKE_C_FLAGS} ${FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEFAULT_CMAKE_CXX_FLAGS} ${FLAGS}")

  if(${CMAKE_VERSION} LESS 3.0)
    if(("GNU" STREQUAL "${CMAKE_CXX_COMPILER_ID}") AND ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "6.0"))
      if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "5.0")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
      else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
      endif()
    else()
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
    endif()
    if(("GNU" STREQUAL "${CMAKE_CXX_COMPILER_ID}") AND ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.8"))
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
  endif()
endif()

if(DEFINED ENV{CFLAGS})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{CFLAGS}")
endif()

if(DEFINED ENV{CPPFLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CPPFLAGS}")
endif()

if(DEFINED ENV{CXXFLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS}")
endif()

if(DEFINED ENV{LDFLAGS})
  set(LINK_FLAGS "${LINK_FLAGS} $ENV{LDFLAGS}")
endif()

if(DEFINED CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS}")
endif()

if(DEFINED CPPFLAGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPPFLAGS}")
endif()

if(DEFINED CXXFLAGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXFLAGS}")
endif()

if(DEFINED LDFLAGS)
  set(LINK_FLAGS "${LINK_FLAGS} ${LDFLAGS}")
endif()

# ant4c

add_library(ant4c STATIC
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
            "${CMAKE_SOURCE_DIR}/xml.h")

# ant4c_app

add_executable(ant4c_app
            "${CMAKE_SOURCE_DIR}/main.c")

target_link_libraries(ant4c_app ant4c)

if(DEFINED PROGRAM_VERSION)
  target_compile_definitions(ant4c PRIVATE -DPROGRAM_VERSION=${PROGRAM_VERSION})
  target_compile_definitions(ant4c_app PRIVATE -DPROGRAM_VERSION=${PROGRAM_VERSION})
endif()

if(NOT MSVC)
  if(("GNU" STREQUAL "${CMAKE_CXX_COMPILER_ID}") AND (NOT MINGW))
    target_compile_options(ant4c PRIVATE "-fPIE")
    target_compile_options(ant4c_app PRIVATE "-fPIE")
    set_target_properties(ant4c_app PROPERTIES LINK_FLAGS "-pie -Wl,-z,now")
  endif()

  target_link_libraries(ant4c_app m)

  if((NOT MINGW) AND (NOT (CMAKE_HOST_SYSTEM_NAME STREQUAL "OpenBSD")))
    target_link_libraries(ant4c_app dl)
  endif()
endif()

set_target_properties(ant4c_app PROPERTIES OUTPUT_NAME ant4c)

include("${CMAKE_SOURCE_DIR}/dns-CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/regex-CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/default_listener.cmake")
include("${CMAKE_SOURCE_DIR}/example_of_the_module.cmake")

# if(CMAKE_HOST_WIN32)
#  include("${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.cmake")
# endif()

# add_dependencies(ant4c ant4c.dns)
# add_dependencies(ant4c ant4c.regex)
add_dependencies(ant4c default_listener)
add_dependencies(ant4c example_of_the_module)

# if((CMAKE_HOST_WIN32) AND (MSVC))
#  add_dependencies(ant4c ant4c.net.framework.module)
# endif()
