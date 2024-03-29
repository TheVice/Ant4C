cmake_minimum_required(VERSION 2.8.12)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Configuration process cannot start from project source directory.")
endif()

project("ant4c.net.framework.module")

if(MSVC)
  include("${CMAKE_SOURCE_DIR}/../../common.cmake")

  append_to_flags_from_outside()
  set_position_independent_code()

  # Find Ant4C
  find_package(ant4c PATHS "${ANT4C_PATH}" REQUIRED)

  # framework_gate
  add_library(framework_gate
    "${CMAKE_SOURCE_DIR}/clr_control.c"
    "${CMAKE_SOURCE_DIR}/clr_control.h"
    "${CMAKE_SOURCE_DIR}/host_controller.c"
    "${CMAKE_SOURCE_DIR}/host_controller.h"
    "${CMAKE_SOURCE_DIR}/meta_host.c"
    "${CMAKE_SOURCE_DIR}/meta_host.h"
    "${CMAKE_SOURCE_DIR}/runtime_host.c"
    "${CMAKE_SOURCE_DIR}/runtime_host.h"
    "${CMAKE_SOURCE_DIR}/runtime_info.c"
    "${CMAKE_SOURCE_DIR}/runtime_info.h"
    "${CMAKE_SOURCE_DIR}/unknown_structure.c"
    "${CMAKE_SOURCE_DIR}/unknown_structure.h")

  target_include_directories(framework_gate INTERFACE ${CMAKE_SOURCE_DIR})
  target_compile_options(framework_gate PRIVATE /W4)
  target_link_libraries(framework_gate PUBLIC Ant4C::ant4c)

  if(NOT DEFINED NETFXSDK)
    set(NETFXSDK "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8")
  endif()

  if(MSVC_VERSION VERSION_LESS 1910)
    target_include_directories(framework_gate PUBLIC SYSTEM "${NETFXSDK}/Include/um")

    if(CMAKE_CL_64)
      target_link_directories(framework_gate INTERFACE SYSTEM "${NETFXSDK}/Lib/um/x64")
    else()
      target_link_directories(framework_gate INTERFACE SYSTEM "${NETFXSDK}/Lib/um/x86")
    endif()
  endif()

  add_library(Ant4C::framework_gate ALIAS framework_gate)

  # ant4c.net.framework.module
  add_library(ant4c.net.framework.module SHARED
    "${CMAKE_SOURCE_DIR}/ant4c.net.framework.adoc"
    "${CMAKE_SOURCE_DIR}/ant4c.net.framework.c"
    "${CMAKE_SOURCE_DIR}/ant4c.net.framework.h"
    "${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.clr.csproj"
    "${CMAKE_SOURCE_DIR}/ant4c.net.framework.xml"
    "${CMAKE_SOURCE_DIR}/CustomAppDomainManager.cs"
    "${CMAKE_SOURCE_DIR}/IFrameworkNamespace.cs"
    "${CMAKE_SOURCE_DIR}/VersionDetector.cs")

  target_compile_options(ant4c.net.framework.module PRIVATE /W4 /WX)
  target_link_libraries(ant4c.net.framework.module framework_gate)

  add_library(Ant4C::ant4c.net.framework.module ALIAS ant4c.net.framework.module)

  # Tests
  if(EXISTS "${CMAKE_SOURCE_DIR}/../../tests_base.cmake")
  # Find tests_base
  # find_package(TestsBase PATHS REQUIRED)
  include("${CMAKE_SOURCE_DIR}/../../tests_base.cmake")

  # tests_ant4c.net.framework.module
  add_executable(tests_ant4c.net.framework.module
    "${CMAKE_SOURCE_DIR}/../../tests_ant4c.net.framework.module.cpp")

  target_compile_options(tests_ant4c.net.framework.module PRIVATE /W4 /WX)

  target_link_libraries(tests_ant4c.net.framework.module
    Ant4C::framework_gate
    tests_base)

  add_dependencies(tests_ant4c.net.framework.module
    Ant4C::ant4c.net.framework.module)

  add_test(
    NAME tests_ant4c.net.framework.module
    COMMAND "$<TARGET_FILE:tests_ant4c.net.framework.module>")
  endif()
else()
  message(FATAL_ERROR "This module can be build only by MSVC compiler.")
endif()
