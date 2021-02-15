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
  if(NOT MINGW)
    target_compile_options(ant4c PRIVATE "-fPIE")
    target_compile_options(ant4c PRIVATE "-fPIC")
    target_compile_options(ant4c_app PRIVATE "-fPIE")
    set_target_properties(ant4c_app PROPERTIES LINK_FLAGS "-pie -Wl,-z,now")
  endif()

  target_link_libraries(ant4c_app m)

  if((NOT MINGW) AND (NOT (CMAKE_HOST_SYSTEM_NAME STREQUAL "OpenBSD")))
    target_link_libraries(ant4c_app dl)
  endif()
endif()

set_target_properties(ant4c_app PROPERTIES OUTPUT_NAME ant4c)

set(SOURCES_OF_TESTS)
set(LIBRARIES4TESTING ant4c)

include("${CMAKE_SOURCE_DIR}/dns-CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/regex-CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/default_listener.cmake")
include("${CMAKE_SOURCE_DIR}/example_of_the_module.cmake")

if(CMAKE_HOST_WIN32)
  include("${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.cmake")
endif()

set(SOURCES_OF_EXEC_APP "${CMAKE_SOURCE_DIR}/tests_exec_app.c")

list(APPEND SOURCES_OF_TESTS
            "${CMAKE_SOURCE_DIR}/tests_argument_parser.cpp"
            "${CMAKE_SOURCE_DIR}/tests_base_xml.cpp"
            "${CMAKE_SOURCE_DIR}/tests_base_xml.h"
            "${CMAKE_SOURCE_DIR}/tests_conversion.cpp"
            "${CMAKE_SOURCE_DIR}/tests_date_time.cpp"
            "${CMAKE_SOURCE_DIR}/tests_environment.cpp"
            "${CMAKE_SOURCE_DIR}/tests_exec.cpp"
            "${CMAKE_SOURCE_DIR}/tests_file_system.cpp"
            "${CMAKE_SOURCE_DIR}/tests_hash.cpp"
            "${CMAKE_SOURCE_DIR}/tests_interpreter.cpp"
            "${CMAKE_SOURCE_DIR}/tests_load_file.cpp"
            "${CMAKE_SOURCE_DIR}/tests_load_tasks.cpp"
            "${CMAKE_SOURCE_DIR}/tests_math_unit.cpp"
            "${CMAKE_SOURCE_DIR}/tests_path.cpp"
            "${CMAKE_SOURCE_DIR}/tests_project.cpp"
            "${CMAKE_SOURCE_DIR}/tests_property.cpp"
            "${CMAKE_SOURCE_DIR}/tests_string_unit.cpp"
            "${CMAKE_SOURCE_DIR}/tests_text_encoding.cpp"
            "${CMAKE_SOURCE_DIR}/tests_xml.cpp"
            "${CMAKE_SOURCE_DIR}/text_encoding.cpp")

set(pugixml_FOUND 0)
set(GTest_FOUND False)

if((DEFINED ENV{pugixml_issues_390}) OR (DEFINED pugixml_issues_390))# https://github.com/zeux/pugixml/issues/390
else()
  find_package(pugixml)
endif()

find_package(GTest)

if((1 EQUAL ${pugixml_FOUND}) AND ${GTest_FOUND})
  add_executable(tests_exec_app ${SOURCES_OF_EXEC_APP})
  add_executable(ant4c_tests ${SOURCES_OF_TESTS})

  target_link_libraries(ant4c_tests ${LIBRARIES4TESTING} pugixml GTest::GTest)# GTest::Main)

  if(NOT MSVC)
    target_link_libraries(ant4c_tests m)

    if((NOT MINGW) AND (NOT (CMAKE_HOST_SYSTEM_NAME STREQUAL "OpenBSD")))
      target_link_libraries(ant4c_tests dl)
    endif()
  endif()

  if((NOT MSVC) AND (${CMAKE_VERSION} GREATER 3.0))
    set_target_properties(ant4c_tests PROPERTIES CXX_STANDARD 11)
  endif()

  add_dependencies(ant4c_tests tests_exec_app)
  add_dependencies(ant4c_tests ant4c.dns)
  add_dependencies(ant4c_tests ant4c.regex)
  add_dependencies(ant4c_tests example_of_the_module)

  if((CMAKE_HOST_WIN32) AND (MSVC))
    add_dependencies(ant4c_tests ant4c.net.framework.module)
  endif()

  if(EXISTS ${CMAKE_SOURCE_DIR}/tests.xml)
    add_custom_command(TARGET ant4c_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")
  endif()

  add_custom_command(TARGET ant4c_tests POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Path to tests_exec_app is - $<TARGET_FILE:tests_exec_app>")

  if(DEFINED PUGIXML_HEADER_ONLY)
    target_compile_definitions(ant4c_tests PRIVATE -DPUGIXML_HEADER_ONLY=${PUGIXML_HEADER_ONLY})
  endif()
else()
  message(STATUS "Search pugixml and GTest via defined PUGIXML_PATH and/or GTEST_PATH if such exists.")

  if(DEFINED ENV{PUGIXML_PATH})
    string(REPLACE "\\" "/" pugixml_Path $ENV{PUGIXML_PATH})
  elseif(DEFINED PUGIXML_PATH)
    string(REPLACE "\\" "/" pugixml_Path ${PUGIXML_PATH})
  else()
    message(WARNING "PUGIXML_PATH not set. Targets that use pugixml will not be available.")
  endif()

  if(pugixml_Path)
    if (EXISTS ${pugixml_Path}/CMakeLists.txt)
      add_subdirectory(${pugixml_Path} ${CMAKE_BINARY_DIR}/pugixml)
    else()
      add_subdirectory(${pugixml_Path}/scripts ${CMAKE_BINARY_DIR}/pugixml)
    endif()

    if(DEFINED ENV{GTEST_PATH})
      string(REPLACE "\\" "/" gtest_Path $ENV{GTEST_PATH})
    elseif(DEFINED GTEST_PATH)
      string(REPLACE "\\" "/" gtest_Path ${GTEST_PATH})
    else()
      message(WARNING "GTEST_PATH not set. Tests will not be available.")
    endif()

    if(gtest_Path)
      set(GTEST_MAIN_LIBRARY gtest)

      if (EXISTS ${gtest_Path}/include)
        set(GTEST_INCLUDE_DIR ${gtest_Path}/include)
        set(GTEST_INCLUDE_DIRS ${gtest_Path} ${GTEST_INCLUDE_DIR})
      else()
        set(GTEST_INCLUDE_DIR ${gtest_Path}/googletest/include)
        set(GTEST_INCLUDE_DIRS ${gtest_Path}/googletest ${GTEST_INCLUDE_DIR})
      endif()

      if((EXISTS ${gtest_Path}/src/gtest-all.cc) AND (EXISTS ${gtest_Path}/src/gtest_main.cc))
        set(GTEST_FILES ${gtest_Path}/src/gtest-all.cc)# ${gtest_Path}/src/gtest_main.cc)
      elseif((EXISTS ${gtest_Path}/gtest/gtest-all.cc) AND (EXISTS ${gtest_Path}/gtest/gtest_main.cc))
        set(GTEST_FILES ${gtest_Path}/gtest/gtest-all.cc)# ${gtest_Path}/gtest/gtest_main.cc)
      elseif((EXISTS ${gtest_Path}/googletest/src/gtest-all.cc) AND (EXISTS ${gtest_Path}/googletest/src/gtest_main.cc))
        set(GTEST_FILES ${gtest_Path}/googletest/src/gtest-all.cc)# ${gtest_Path}/googletest/src/gtest_main.cc)
      else()
        message(FATAL_ERROR "Could not find Google Test library at ${gtest_Path}, please check it at https://github.com/google/googletest/releases")
      endif()

      add_library(${GTEST_MAIN_LIBRARY} STATIC ${GTEST_FILES})
      target_include_directories(${GTEST_MAIN_LIBRARY} SYSTEM PRIVATE ${GTEST_INCLUDE_DIRS})
      target_compile_definitions(${GTEST_MAIN_LIBRARY} PRIVATE GTEST_HAS_PTHREAD=0)

      add_executable(tests_exec_app ${SOURCES_OF_EXEC_APP})
      add_executable(ant4c_tests ${SOURCES_OF_TESTS})
      target_include_directories(ant4c_tests SYSTEM PRIVATE ${pugixml_Path}/src ${GTEST_INCLUDE_DIR})
      target_link_libraries(ant4c_tests ${LIBRARIES4TESTING} pugixml ${GTEST_MAIN_LIBRARY})

      if(NOT MSVC)
        target_link_libraries(ant4c_tests m)
        if((NOT MINGW) AND (NOT (CMAKE_HOST_SYSTEM_NAME STREQUAL "OpenBSD")))
          target_link_libraries(ant4c_tests dl)
        endif()
      endif()

      if((NOT MSVC) AND (${CMAKE_VERSION} GREATER 3.0))
        set_target_properties(${GTEST_MAIN_LIBRARY} PROPERTIES CXX_STANDARD 11)
        set_target_properties(ant4c_tests PROPERTIES CXX_STANDARD 11)
      endif()

      add_dependencies(ant4c_tests tests_exec_app)
      add_dependencies(ant4c_tests ant4c.dns)
      add_dependencies(ant4c_tests ant4c.regex)
      add_dependencies(ant4c_tests example_of_the_module)

      if((CMAKE_HOST_WIN32) AND (MSVC))
        add_dependencies(ant4c_tests ant4c.net.framework.module)
      endif()

      if(EXISTS ${CMAKE_SOURCE_DIR}/tests.xml)
        add_custom_command(TARGET ant4c_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")
      endif()

      add_custom_command(TARGET ant4c_tests POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E echo "Path to tests_exec_app is - $<TARGET_FILE:tests_exec_app>")

      if(DEFINED PUGIXML_HEADER_ONLY)
        target_compile_definitions(ant4c_tests PRIVATE -DPUGIXML_HEADER_ONLY=${PUGIXML_HEADER_ONLY})
      endif()

      if(("GNU" STREQUAL "${CMAKE_CXX_COMPILER_ID}") AND (NOT MINGW))
        target_compile_options(pugixml PRIVATE "-fPIE")
        target_compile_options(${GTEST_MAIN_LIBRARY} PRIVATE "-fPIE")
      endif()

      message(STATUS "pugixml and GTest was found.")
    endif()
  endif()
endif()
