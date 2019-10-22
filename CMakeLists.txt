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

# tests_exec_app

add_executable(tests_exec_app
            "${CMAKE_SOURCE_DIR}/tests_exec_app.c")

# tests

set(SOURCES_OF_TESTS
            "${CMAKE_SOURCE_DIR}/tests_argument_parser.cpp"
            "${CMAKE_SOURCE_DIR}/tests_base_xml.cpp"
            "${CMAKE_SOURCE_DIR}/tests_base_xml.h"
            "${CMAKE_SOURCE_DIR}/tests_conversion.cpp"
            "${CMAKE_SOURCE_DIR}/tests_date_time.cpp"
            "${CMAKE_SOURCE_DIR}/tests_environment.cpp"
            "${CMAKE_SOURCE_DIR}/tests_exec.cpp"
            "${CMAKE_SOURCE_DIR}/tests_file_system.cpp"
            "${CMAKE_SOURCE_DIR}/tests_interpreter.cpp"
            "${CMAKE_SOURCE_DIR}/tests_math_unit.cpp"
            "${CMAKE_SOURCE_DIR}/tests_path.cpp"
            "${CMAKE_SOURCE_DIR}/tests_project.cpp"
            "${CMAKE_SOURCE_DIR}/tests_property.cpp"
            "${CMAKE_SOURCE_DIR}/tests_string_unit.cpp"
            "${CMAKE_SOURCE_DIR}/tests_xml.cpp")

set(pugixml_FOUND 0)
set(GTest_FOUND False)

find_package(pugixml)
find_package(GTest)

if((1 EQUAL ${pugixml_FOUND}) AND ${GTest_FOUND})
  add_executable(ant4c_tests ${SOURCES_OF_TESTS})
  target_link_libraries(ant4c_tests ant4c pugixml GTest::GTest)# GTest::Main)
  if(NOT MSVC)
    target_link_libraries(ant4c_tests m)
  endif()
  if((NOT MSVC) AND (${CMAKE_VERSION} GREATER 3.0))
    set_target_properties(ant4c_tests PROPERTIES CXX_STANDARD 11)
  endif()
  add_dependencies(ant4c_tests tests_exec_app)
  if(EXISTS ${CMAKE_SOURCE_DIR}/tests.xml)
    add_custom_command(TARGET ant4c_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")
  endif()
  add_custom_command(TARGET ant4c_tests POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Path to tests_exec_app is - $<TARGET_FILE:tests_exec_app>")
else()
  message(STATUS "Search pugixml and GTest via defined PUGIXML_PATH and/or GTEST_PATH if such exists.")
  if(DEFINED ENV{PUGIXML_PATH})
    string(REPLACE "\\" "/" pugixml_Path $ENV{PUGIXML_PATH})
  elseif(DEFINED PUGIXML_PATH)
    string(REPLACE "\\" "/" pugixml_Path ${PUGIXML_PATH})
  else()
    message(WARNING "PUGIXML_PATH not set. Targets that use pugixml will not be avaliable.")
  endif()

  if(pugixml_Path)
    if (EXISTS ${pugixml_Path}/CMakeLists.txt)
      add_subdirectory(${pugixml_Path} ${CMAKE_BINARY_DIR}/pugixml)
    else()
      add_subdirectory(${pugixml_Path}/scripts ${CMAKE_BINARY_DIR}/pugixml)
    endif()

    # tests

    if(DEFINED ENV{GTEST_PATH})
      string(REPLACE "\\" "/" gtest_Path $ENV{GTEST_PATH})
    elseif(DEFINED GTEST_PATH)
      string(REPLACE "\\" "/" gtest_Path ${GTEST_PATH})
    else()
      message(WARNING "GTEST_PATH not set. Tests will not be avaliable.")
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

      # ant4c_tests

      add_executable(ant4c_tests ${SOURCES_OF_TESTS})
      target_include_directories(ant4c_tests SYSTEM PRIVATE ${pugixml_Path}/src ${GTEST_INCLUDE_DIR})
      target_link_libraries(ant4c_tests ant4c pugixml ${GTEST_MAIN_LIBRARY})

      if(NOT MSVC)
        target_link_libraries(ant4c_tests m)
      endif()

      if((NOT MSVC) AND (${CMAKE_VERSION} GREATER 3.0))
        set_target_properties(${GTEST_MAIN_LIBRARY} PROPERTIES CXX_STANDARD 11)
        set_target_properties(ant4c_tests PROPERTIES CXX_STANDARD 11)
      endif()

      add_dependencies(ant4c_tests tests_exec_app)
      if(EXISTS ${CMAKE_SOURCE_DIR}/tests.xml)
        add_custom_command(TARGET ant4c_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")
      endif()
      add_custom_command(TARGET ant4c_tests POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E echo "Path to tests_exec_app is - $<TARGET_FILE:tests_exec_app>")
      message(STATUS "pugixml and GTest was found.")
    endif()
  endif()
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
