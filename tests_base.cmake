
include("${CMAKE_SOURCE_DIR}/gtest.cmake")
include("${CMAKE_SOURCE_DIR}/pugixml.cmake")

# tests_base

add_library(tests_base STATIC
  "${CMAKE_SOURCE_DIR}/tests.cmake"
  "${CMAKE_SOURCE_DIR}/gtest.cmake"
  "${CMAKE_SOURCE_DIR}/pugixml.cmake"
  "${CMAKE_SOURCE_DIR}/tests_base_xml.cpp"
  "${CMAKE_SOURCE_DIR}/tests_base_xml.h"
  "${CMAKE_SOURCE_DIR}/text_encoding.cpp"
  "${CMAKE_SOURCE_DIR}/tests_argument_parser.get_properties.cpp"
  "${CMAKE_SOURCE_DIR}/tests_argument_parser.h"
)

target_compile_options(tests_base PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(tests_base
  PUBLIC
  GTest::gtest
  Pugixml::pugixml
)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
  target_compile_features(tests_base
    PRIVATE
    cxx_std_11
  )
  else()
  set_property(TARGET tests_base PROPERTY CXX_STANDARD 11)
  endif()
else()
  target_link_options(tests_base INTERFACE "/entry:wmainCRTStartup")
endif()

