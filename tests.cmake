
include("${CMAKE_SOURCE_DIR}/gtest.cmake")
include("${CMAKE_SOURCE_DIR}/pugixml.cmake")

# tests_exec_app

add_executable(tests_exec_app
  "${CMAKE_SOURCE_DIR}/tests_exec_app.c"
)

target_compile_options(tests_exec_app PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(tests_exec_app
    PRIVATE
    c_std_11
  )
  endif()
endif()

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

# ant4c_tests

add_executable(ant4c_tests
  "${CMAKE_SOURCE_DIR}/tests.xml"
  "${CMAKE_SOURCE_DIR}/tests_argument_parser.cpp"
  "${CMAKE_SOURCE_DIR}/tests_conversion.cpp"
  "${CMAKE_SOURCE_DIR}/tests_date_time.cpp"
  "${CMAKE_SOURCE_DIR}/tests_environment.cpp"
  "${CMAKE_SOURCE_DIR}/tests_exec.cpp"
  "${CMAKE_SOURCE_DIR}/tests_exec.h"
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
  "${CMAKE_SOURCE_DIR}/tests_version.cpp"
  "${CMAKE_SOURCE_DIR}/tests_xml.cpp"
)

target_compile_options(ant4c_tests PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(ant4c_tests
  Ant4C::ant4c
  tests_base
)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
  target_compile_features(ant4c_tests
    PRIVATE
    cxx_std_11
  )
  else()
  set_property(TARGET ant4c_tests PROPERTY CXX_STANDARD 11)
  endif()
endif()

add_custom_command(TARGET ant4c_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")

add_custom_command(TARGET ant4c_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests_exec_app is - $<TARGET_FILE:tests_exec_app>")

add_dependencies(ant4c_tests
  tests_exec_app
)

# tests_ant4c.net.framework.module

if(MSVC)
  add_executable(tests_ant4c.net.framework.module
    "${CMAKE_SOURCE_DIR}/tests_ant4c.net.framework.module.cpp"
  )

  target_compile_options(tests_ant4c.net.framework.module PRIVATE /W4 /WX)

  target_link_libraries(tests_ant4c.net.framework.module
    Ant4C::framework_gate
    tests_base
  )

  add_dependencies(tests_ant4c.net.framework.module
    Ant4C::ant4c.net.framework.module
  )
endif()

# tests_net.module

add_executable(tests_net.module
  "${CMAKE_SOURCE_DIR}/tests_net.module.cpp"
  "${CMAKE_SOURCE_DIR}/modules/net/net.xml"
)

target_compile_options(tests_net.module PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(tests_net.module
  Ant4C::net_gate
  tests_base
)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
  target_compile_features(tests_net.module
    PRIVATE
    cxx_std_11
  )
  else()
  set_property(TARGET tests_net.module PROPERTY CXX_STANDARD 11)
  endif()
endif()

add_custom_command(TARGET tests_net.module POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")

add_custom_command(TARGET tests_net.module POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to build_file is - ${CMAKE_SOURCE_DIR}/modules/net/net.xml")

add_dependencies(tests_net.module
  Ant4C::ant4c.net.module
)