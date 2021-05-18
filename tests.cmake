
set(SOURCES_OF_EXEC_APP "${CMAKE_SOURCE_DIR}/tests_exec_app.c")

list(APPEND SOURCES_OF_TESTS
  "${CMAKE_SOURCE_DIR}/tests_argument_parser.cpp"
  "${CMAKE_SOURCE_DIR}/tests_base_xml.cpp"
  "${CMAKE_SOURCE_DIR}/tests_base_xml.h"
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
  "${CMAKE_SOURCE_DIR}/tests_xml.cpp"
  "${CMAKE_SOURCE_DIR}/text_encoding.cpp")

add_executable(tests_exec_app ${SOURCES_OF_EXEC_APP})
add_executable(ant4c_tests ${SOURCES_OF_TESTS})

target_include_directories(ant4c_tests SYSTEM PRIVATE ${pugixml_Path}/src ${GTEST_INCLUDE_DIR})
target_link_libraries(ant4c_tests ${LIBRARIES4TESTING} pugixml)

if(MSVC)
  target_compile_options(tests_exec_app PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(tests_exec_app PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)

  target_compile_options(ant4c_tests PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(ant4c_tests PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)
else()
  set_target_properties(tests_exec_app PROPERTIES CXX_STANDARD 11)
  target_compile_options(tests_exec_app PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(tests_exec_app PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)

  set_target_properties(ant4c_tests PROPERTIES CXX_STANDARD 11)
  target_compile_options(ant4c_tests PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(ant4c_tests PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)

  target_link_libraries(ant4c_tests m)

  if((NOT MINGW) AND (NOT (CMAKE_HOST_SYSTEM_NAME STREQUAL "OpenBSD")))
    target_link_libraries(ant4c_tests dl)
  endif()
endif()

if(DEFINED PUGIXML_HEADER_ONLY)
  target_compile_definitions(ant4c_tests PRIVATE -DPUGIXML_HEADER_ONLY=${PUGIXML_HEADER_ONLY})
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/tests.xml)
  add_custom_command(TARGET ant4c_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")
endif()

add_custom_command(TARGET ant4c_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests_exec_app is - $<TARGET_FILE:tests_exec_app>")
