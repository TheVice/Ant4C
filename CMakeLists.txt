cmake_minimum_required(VERSION 2.8.12)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Configuration process cannot start from project source directory.")
endif()

project("Ant4C")

# ant4c

add_library(ant4c STATIC
            "${CMAKE_SOURCE_DIR}/argument_parser.c"
            "${CMAKE_SOURCE_DIR}/argument_parser.h"
            "${CMAKE_SOURCE_DIR}/buffer.c"
            "${CMAKE_SOURCE_DIR}/buffer.h"
            "${CMAKE_SOURCE_DIR}/common.c"
            "${CMAKE_SOURCE_DIR}/common.h"
            "${CMAKE_SOURCE_DIR}/conversion.c"
            "${CMAKE_SOURCE_DIR}/conversion.h"
            "${CMAKE_SOURCE_DIR}/date_time.c"
            "${CMAKE_SOURCE_DIR}/date_time.h"
            "${CMAKE_SOURCE_DIR}/echo.c"
            "${CMAKE_SOURCE_DIR}/echo.h"
            "${CMAKE_SOURCE_DIR}/environment.c"
            "${CMAKE_SOURCE_DIR}/environment.h"
            "${CMAKE_SOURCE_DIR}/exec.c"
            "${CMAKE_SOURCE_DIR}/exec.h"
            "${CMAKE_SOURCE_DIR}/file_system.c"
            "${CMAKE_SOURCE_DIR}/file_system.h"
            "${CMAKE_SOURCE_DIR}/interpreter.c"
            "${CMAKE_SOURCE_DIR}/interpreter.h"
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
            "${CMAKE_SOURCE_DIR}/string_unit.c"
            "${CMAKE_SOURCE_DIR}/string_unit.h"
            "${CMAKE_SOURCE_DIR}/target.c"
            "${CMAKE_SOURCE_DIR}/target.h"
            "${CMAKE_SOURCE_DIR}/version.c"
            "${CMAKE_SOURCE_DIR}/version.h"
            "${CMAKE_SOURCE_DIR}/xml.c"
            "${CMAKE_SOURCE_DIR}/xml.h")

# ant4c_app

add_executable(ant4c_app
            "${CMAKE_SOURCE_DIR}/main.c")

target_link_libraries(ant4c_app ant4c)

if(DEFINED PROGRAM_VERSION AND DEFINED PROGRAM_VERSION_LENGTH)
  target_compile_definitions(ant4c PRIVATE -DPROGRAM_VERSION=${PROGRAM_VERSION})
  target_compile_definitions(ant4c PRIVATE -DPROGRAM_VERSION_LENGTH=${PROGRAM_VERSION_LENGTH})

  target_compile_definitions(ant4c_app PRIVATE -DPROGRAM_VERSION=${PROGRAM_VERSION})
  target_compile_definitions(ant4c_app PRIVATE -DPROGRAM_VERSION_LENGTH=${PROGRAM_VERSION_LENGTH})
endif()

if(NOT MSVC)
  target_link_libraries(ant4c_app m)
endif()

if(MSVC)
  # (${MSVC_VERSION} LESS 1901)
  # (${MSVC_TOOLSET_VERSION} LESS 141(142))
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
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
    else()
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
endif()
