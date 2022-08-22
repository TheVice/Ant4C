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
    c_std_11)
  endif()
endif()

# tests_base

include("${CMAKE_SOURCE_DIR}/tests_base.cmake")

# ant4c_tests

add_executable(ant4c_tests
  "${CMAKE_SOURCE_DIR}/tests.xml"
  "${CMAKE_SOURCE_DIR}/tests.cmake"
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

target_link_libraries(ant4c_tests Ant4C::ant4c tests_base)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
  target_compile_features(ant4c_tests
    PRIVATE
    cxx_std_11)
  else()
  set_property(TARGET ant4c_tests PROPERTY CXX_STANDARD 11)
  endif()
endif()

add_custom_command(TARGET ant4c_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")

add_custom_command(TARGET ant4c_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests_exec_app is - $<TARGET_FILE:tests_exec_app>")

add_dependencies(ant4c_tests tests_exec_app)

add_test(
  NAME ant4c_tests
  COMMAND "$<TARGET_FILE:ant4c_tests>"
    --tests_xml=${CMAKE_SOURCE_DIR}/tests.xml
    --tests_exec_app=$<TARGET_FILE:tests_exec_app>
    --tests_base_directory=.
    --gtest_output=xml:$<TARGET_FILE_DIR:ant4c_tests>/ant4c_tests-report.xml
)
