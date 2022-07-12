
include("${CMAKE_CURRENT_LIST_DIR}/gtest.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/pugixml.cmake")

# tests_base

set(TESTS_BASE_HEADERS
  "${CMAKE_CURRENT_LIST_DIR}/tests_base_xml.h"
  "${CMAKE_CURRENT_LIST_DIR}/tests_argument_parser.h")

add_library(tests_base STATIC
  "${CMAKE_CURRENT_LIST_DIR}/tests.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/gtest.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/pugixml.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/tests_base.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/tests_base_xml.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/text_encoding.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tests_argument_parser.get_properties.cpp"
  ${TESTS_BASE_HEADERS})

target_compile_options(tests_base PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)

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

target_link_libraries(tests_base
  PUBLIC
  ${libraries}
  Ant4C::ant4c)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
    target_compile_features(tests_base
      PRIVATE
      cxx_std_11)
  else()
    set_property(TARGET tests_base PROPERTY CXX_STANDARD 11)
  endif()
else()
  target_link_options(tests_base INTERFACE "/entry:wmainCRTStartup")
endif()

# Install tests_base
if(INSTALL_TESTS_BASE)
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif()
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR bin)
endif()
if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR include)
endif()

install(
  TARGETS tests_base gtest pugixml
  EXPORT TestsBaseTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT TestsBaseTargets
  FILE TestsBaseTargets.cmake
  NAMESPACE TestsBase::
  DESTINATION lib/cmake/TestsBase)

file(
  GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TestsBaseConfig.cmake"
  CONTENT
"include(CMakeFindDependencyMacro)
if(NOT MSVC)
  find_dependency(dl)
  find_dependency(m)
endif()
include(\"\${CMAKE_CURRENT_LIST_DIR}/TestsBaseTargets.cmake\")")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TestsBaseConfig.cmake" DESTINATION lib/cmake/TestsBase)

get_target_property(gtest_include_directories gtest INCLUDE_DIRECTORIES)
install(DIRECTORY "${gtest_include_directories}" DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.h")

if(DEFINED PROGRAM_VERSION)
  include(CMakePackageConfigHelpers)

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/TestsBaseConfigVersion.cmake"
    VERSION ${PROGRAM_VERSION}
    COMPATIBILITY SameMajorVersion)

  install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TestsBaseConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TestsBaseConfigVersion.cmake"
    DESTINATION lib/cmake/TestsBase)
endif()
endif()