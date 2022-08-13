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
set(NET_GATE_SOURCES
  "${CMAKE_SOURCE_DIR}/arguments.c"
  "${CMAKE_SOURCE_DIR}/core_host_context_contract.c"
  "${CMAKE_SOURCE_DIR}/core_host_initialize_request.c"
  "${CMAKE_SOURCE_DIR}/error_writer.c"
  "${CMAKE_SOURCE_DIR}/host_fxr.c"
  "${CMAKE_SOURCE_DIR}/host_interface.c"
  "${CMAKE_SOURCE_DIR}/host_policy.c"
  "${CMAKE_SOURCE_DIR}/net.common.c"
  "${CMAKE_SOURCE_DIR}/net.file.c"
  "${CMAKE_SOURCE_DIR}/net.host.c"
  "${CMAKE_SOURCE_DIR}/net.host_fxr.c"
  "${CMAKE_SOURCE_DIR}/net.host_policy.c"
)

set(NET_GATE_HEADERS
  "${CMAKE_SOURCE_DIR}/arguments.h"
  "${CMAKE_SOURCE_DIR}/core_host_context_contract.h"
  "${CMAKE_SOURCE_DIR}/core_host_initialize_request.h"
  "${CMAKE_SOURCE_DIR}/error_writer.h"
  "${CMAKE_SOURCE_DIR}/host_fxr.h"
  "${CMAKE_SOURCE_DIR}/host_interface.h"
  "${CMAKE_SOURCE_DIR}/host_policy.h"
  "${CMAKE_SOURCE_DIR}/net.common.h"
  "${CMAKE_SOURCE_DIR}/net.file.h"
  "${CMAKE_SOURCE_DIR}/net.host.h"
  "${CMAKE_SOURCE_DIR}/net.host_fxr.h"
  "${CMAKE_SOURCE_DIR}/net.host_policy.h"
  "${CMAKE_SOURCE_DIR}/net_delegate.h"
)

add_library(net_gate STATIC ${NET_GATE_SOURCES} ${NET_GATE_HEADERS})

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
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(net_gate Ant4C::ant4c)

# ant4c.net.module
add_library(ant4c.net.module SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.module.cs"
  "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.module.csproj"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.adoc"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.h"
)

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
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

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
  "${CMAKE_SOURCE_DIR}/net.xml"
)

target_compile_options(tests_ant4c.net.module PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(tests_ant4c.net.module
  Ant4C::net_gate
  tests_base
)

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
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/../../tests.xml"
)

add_custom_command(TARGET tests_ant4c.net.module POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to build_file is - ${CMAKE_SOURCE_DIR}/net.xml"
)

add_dependencies(tests_ant4c.net.module
  Ant4C::ant4c.net.module
)

if(NOT DEFINED dotnet_root)
  if(WIN32)
    if((MINGW) OR (CMAKE_CL_64))
      set(dotnet_root "$ENV{ProgramFiles}\\dotnet")
    else()
      set(variable_name "ProgramFiles(x86)")
      set(dotnet_root "$ENV{${variable_name}}\\dotnet")
    endif()
  else()
    set(dotnet_root "/usr/share/dotnet")
  endif()
else()
  if(DEFINED ENV{DOTNET_ROOT})
    set(dotnet_root "$ENV{DOTNET_ROOT}")
  endif()
endif()

add_test(
  NAME tests_ant4c.net.module
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module-report.xml
)
if(ALLOW_ALL_TEST)
add_test(
  NAME tests_ant4c.net.module_hostfxr_resolve_sdk
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:hostfxr_resolve_sdk="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_hostfxr_resolve_sdk-report.xml
)

add_test(
  NAME tests_ant4c.net.module_hostfxr_get_available_sdks
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:hostfxr_get_available_sdks="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_hostfxr_get_available_sdks-report.xml
)

add_test(
  NAME tests_ant4c.net.module_hostfxr_get_native_search_directories
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:hostfxr_get_native_search_directories="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_hostfxr_get_native_search_directories-report.xml
)

add_test(
  NAME tests_ant4c.net.module_hostfxr_resolve_sdk2
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:hostfxr_resolve_sdk2="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_hostfxr_resolve_sdk2-report.xml
)
endif()
add_test(
  NAME tests_ant4c.net.module_test_hostfxr_initialize_for_dotnet_command_line
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_initialize_for_dotnet_command_line="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_initialize_for_dotnet_command_line-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_hostfxr_initialize_for_runtime_config
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_initialize_for_runtime_config="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_initialize_for_runtime_config-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_hostfxr_run_app
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_run_app="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_run_app-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_hostfxr_get_runtime_properties
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_get_runtime_properties="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_get_runtime_properties-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_hostfxr_get_runtime_property_value
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_get_runtime_property_value="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_get_runtime_property_value-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_hostfxr_set_runtime_property_value
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_set_runtime_property_value="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_set_runtime_property_value-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_hostfxr_get_runtime_delegate
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_get_runtime_delegate="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_get_runtime_delegate-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_hostfxr_set_error_writer
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_set_error_writer="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_set_error_writer-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_hostfxr_main_bundle_startupinfo
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_hostfxr_main_bundle_startupinfo="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_hostfxr_main_bundle_startupinfo-report.xml
)

add_test(
  NAME tests_ant4c.net.module_test_file_is_assembly
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:test_file_is_assembly="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_test_file_is_assembly-report.xml
)

add_test(
  NAME tests_ant4c.net.module_core_host_main
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:core_host_main="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_core_host_main-report.xml
)

add_test(
  NAME tests_ant4c.net.module_core_host_main_with_output_buffer
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:core_host_main_with_output_buffer="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_core_host_main_with_output_buffer-report.xml
)

add_test(
  NAME tests_ant4c.net.module_core_host_initialize
  COMMAND "$<TARGET_FILE:tests_ant4c.net.module>"
    --tests_xml=${CMAKE_SOURCE_DIR}/../../tests.xml
    --build_file=${CMAKE_SOURCE_DIR}/net.xml
    --dotnet_root=${dotnet_root}
    -D:DOTNET_ROOT=${dotnet_root}
    -D:test_hostfxr_="1"
    -D:core_host_initialize="1"
    --gtest_filter=TestNetModuleViaBuildFile*
    --gtest_output=xml:$<TARGET_FILE_DIR:tests_ant4c.net.module>/tests_ant4c.net.module_core_host_initialize-report.xml
)

endif()
