
include("${CMAKE_CURRENT_LIST_DIR}/gtest.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/pugixml.cmake")

enable_testing()

# tests_base

set(TESTS_BASE_HEADERS
  "${CMAKE_CURRENT_LIST_DIR}/tests_base_xml.h"
  "${CMAKE_CURRENT_LIST_DIR}/tests_argument_parser.h"
)

add_library(tests_base STATIC
  "${CMAKE_CURRENT_LIST_DIR}/tests.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/gtest.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/pugixml.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/tests_base.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/tests_base_xml.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/text_encoding.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tests_argument_parser.get_properties.cpp"
  ${TESTS_BASE_HEADERS}
)

target_compile_options(tests_base PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

if(${GTEST_FOUND})
  list(APPEND libraries "gtest")
else()
  list(APPEND libraries "GTest::gtest")
endif()

if(${PUGIXML_FOUND})
  list(APPEND libraries "pugixml")
else()
  list(APPEND libraries "Pugixml::pugixml")
endif()

if(DOTNET_RUNTIME_ISSUE_43036)
# https://github.com/dotnet/runtime/issues/43036
if((NOT MSVC) AND (NOT MINGW))
  find_package(Threads REQUIRED)
  list(APPEND libraries "Threads::Threads")
  target_compile_definitions(
    tests_base PRIVATE DOTNET_RUNTIME_ISSUE_43036=1)
endif()
endif()

target_link_libraries(tests_base
  PUBLIC
  ${libraries}
  Ant4C::ant4c
)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
    target_compile_features(tests_base
      PRIVATE
      cxx_std_11)
  else()
    set_property(TARGET tests_base PROPERTY CXX_STANDARD 11)
  endif()
else()
  if(CMAKE_VERSION VERSION_LESS 3.13)
    file(READ "${CMAKE_CURRENT_LIST_DIR}/tests_base_xml.h" file_contents)
    string(REPLACE "#define _TESTS_BASE_XML_H_" "#define _TESTS_BASE_XML_H_\n\n#pragma comment(linker, \"/entry:wmainCRTStartup\")" file_contents "${file_contents}")
    file(WRITE "${CMAKE_CURRENT_LIST_DIR}/tests_base_xml.h" "${file_contents}")
  else()
    target_link_options(tests_base INTERFACE "/entry:wmainCRTStartup")
  endif()
endif()

# Install tests_base
if(INSTALL_TESTS_BASE)
#TODO:
endif()