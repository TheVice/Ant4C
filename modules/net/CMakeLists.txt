cmake_minimum_required(VERSION 2.8.12)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Configuration process cannot start from project source directory.")
endif()

project("ant4c.net.module")

include("${CMAKE_SOURCE_DIR}/../../common.cmake")

append_to_flags_from_outside()
set_position_independent_code()

# Find Ant4C
find_package(ant4c PATHS "${ANT4C_PATH}" REQUIRED)

# net_gate
add_library(net_gate
  "${CMAKE_SOURCE_DIR}/arguments.c"
  "${CMAKE_SOURCE_DIR}/arguments.h"
  "${CMAKE_SOURCE_DIR}/core_host_context_contract.c"
  "${CMAKE_SOURCE_DIR}/core_host_context_contract.h"
  "${CMAKE_SOURCE_DIR}/core_host_initialize_request.c"
  "${CMAKE_SOURCE_DIR}/core_host_initialize_request.h"
  "${CMAKE_SOURCE_DIR}/error_writer.c"
  "${CMAKE_SOURCE_DIR}/error_writer.h"
  "${CMAKE_SOURCE_DIR}/host_fxr.c"
  "${CMAKE_SOURCE_DIR}/host_fxr.h"
  "${CMAKE_SOURCE_DIR}/host_interface.c"
  "${CMAKE_SOURCE_DIR}/host_interface.h"
  "${CMAKE_SOURCE_DIR}/host_policy.c"
  "${CMAKE_SOURCE_DIR}/host_policy.h"
  "${CMAKE_SOURCE_DIR}/net.common.c"
  "${CMAKE_SOURCE_DIR}/net.common.h"
  "${CMAKE_SOURCE_DIR}/net.file.c"
  "${CMAKE_SOURCE_DIR}/net.file.h"
  "${CMAKE_SOURCE_DIR}/net.host.c"
  "${CMAKE_SOURCE_DIR}/net.host.h"
  "${CMAKE_SOURCE_DIR}/net.host_fxr.c"
  "${CMAKE_SOURCE_DIR}/net.host_fxr.h"
  "${CMAKE_SOURCE_DIR}/net.host_policy.c"
  "${CMAKE_SOURCE_DIR}/net.host_policy.h"
  "${CMAKE_SOURCE_DIR}/net_delegate.h")

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(net_gate
    PRIVATE
    c_std_11)
  endif()
endif()

target_compile_options(net_gate PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)

target_link_libraries(net_gate Ant4C::ant4c)

# ant4c.net.module
add_library(ant4c.net.module SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.module.cs"
  "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.module.csproj"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.adoc"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.h")

target_link_libraries(ant4c.net.module net_gate)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(ant4c.net.module
    PRIVATE
    c_std_11)
  endif()
endif()

target_compile_options(ant4c.net.module PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)

add_library(Ant4C::net_gate ALIAS net_gate)
add_library(Ant4C::ant4c.net.module ALIAS ant4c.net.module)

# Tests
if(EXISTS "${CMAKE_SOURCE_DIR}/../../tests_base.cmake")
# Find tests_base
# find_package(TestsBase PATHS REQUIRED)
include("${CMAKE_SOURCE_DIR}/../../tests_base.cmake")

# tests_ant4c.net.module
add_executable(tests_ant4c.net.module
  "${CMAKE_SOURCE_DIR}/../../tests_net.module.cpp"
  "${CMAKE_SOURCE_DIR}/net.xml")

target_compile_options(tests_ant4c.net.module PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)

target_link_libraries(tests_ant4c.net.module
  Ant4C::net_gate
  tests_base)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
  target_compile_features(tests_ant4c.net.module
    PRIVATE
    cxx_std_11
  )
  else()
  set_property(TARGET tests_ant4c.net.module PROPERTY CXX_STANDARD 11)
  endif()
endif()

add_custom_command(TARGET tests_ant4c.net.module POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/../../tests.xml")

add_custom_command(TARGET tests_ant4c.net.module POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to build_file is - ${CMAKE_SOURCE_DIR}/net.xml")

add_dependencies(tests_ant4c.net.module
  Ant4C::ant4c.net.module)
endif()
