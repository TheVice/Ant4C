
include("${CMAKE_SOURCE_DIR}/gtest.cmake")
include("${CMAKE_SOURCE_DIR}/pugixml.cmake")

add_executable(tests_exec_app
  "${CMAKE_SOURCE_DIR}/tests_exec_app.c"
)

target_compile_options(tests_exec_app PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

add_executable(ant4c_tests
  "${CMAKE_SOURCE_DIR}/tests.cmake"
  "${CMAKE_SOURCE_DIR}/gtest.cmake"
  "${CMAKE_SOURCE_DIR}/pugixml.cmake"
  "${CMAKE_SOURCE_DIR}/tests.xml"
  "${CMAKE_SOURCE_DIR}/tests_argument_parser.h"
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
  "${CMAKE_SOURCE_DIR}/tests_net.module.cpp"
  "${CMAKE_SOURCE_DIR}/tests_path.cpp"
  "${CMAKE_SOURCE_DIR}/tests_project.cpp"
  "${CMAKE_SOURCE_DIR}/tests_property.cpp"
  "${CMAKE_SOURCE_DIR}/tests_string_unit.cpp"
  "${CMAKE_SOURCE_DIR}/tests_text_encoding.cpp"
  "${CMAKE_SOURCE_DIR}/tests_version.cpp"
  "${CMAKE_SOURCE_DIR}/tests_xml.cpp"
  "${CMAKE_SOURCE_DIR}/text_encoding.cpp"
)

if(MSVC)
  target_sources(ant4c_tests PRIVATE
    "${CMAKE_SOURCE_DIR}/tests_ant4c.net.framework.module.cpp"
  )
endif()

target_compile_options(ant4c_tests PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

target_include_directories(ant4c_tests
  SYSTEM PRIVATE ${pugixml_Path}/src ${GTEST_INCLUDE_DIR})
target_link_libraries(ant4c_tests
  ant4c
  ${DL_LIB}
  ${M_LIB}
  ${GTEST_MAIN_LIBRARY}
  pugixml
  $<$<C_COMPILER_ID:MSVC>:framework_gate>
)

if(NOT MSVC)
  target_compile_features(tests_exec_app
    PRIVATE
    c_std_11
  )
  target_compile_features(ant4c_tests
    PRIVATE
    cxx_std_11
  )
endif()

if(DEFINED PUGIXML_HEADER_ONLY)
  target_compile_definitions(ant4c_tests
    PRIVATE -DPUGIXML_HEADER_ONLY=${PUGIXML_HEADER_ONLY}
  )
endif()

add_dependencies(ant4c_tests
  tests_exec_app
  ant4c.net.module
  example_of_the_module
  default_listener
)

if(MSVC)
  add_dependencies(ant4c_tests
    ant4c.net.framework.module
  )
endif()

if(DEFINED USE_BOOST)
  add_dependencies(ant4c_tests
    ant4c.dns
    ant4c.regex
  )
endif()

add_custom_command(TARGET ant4c_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests.xml is - ${CMAKE_SOURCE_DIR}/tests.xml")

add_custom_command(TARGET ant4c_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Path to tests_exec_app is - $<TARGET_FILE:tests_exec_app>")

if(DEFINED NET_MODULE_TESTS)
add_executable(net.module_tests
  "${CMAKE_SOURCE_DIR}/tests.cmake"
  "${CMAKE_SOURCE_DIR}/gtest.cmake"
  "${CMAKE_SOURCE_DIR}/pugixml.cmake"
  "${CMAKE_SOURCE_DIR}/tests.xml"
  "${CMAKE_SOURCE_DIR}/tests_argument_parser.h"
  "${CMAKE_SOURCE_DIR}/tests_argument_parser.cpp"
  "${CMAKE_SOURCE_DIR}/tests_base_xml.cpp"
  "${CMAKE_SOURCE_DIR}/tests_base_xml.h"
  "${CMAKE_SOURCE_DIR}/tests_net.module.cpp"
  "${CMAKE_SOURCE_DIR}/text_encoding.cpp"
  "${CMAKE_SOURCE_DIR}/modules/net/net.xml"
)

target_compile_definitions(net.module_tests PRIVATE NET_MODULE_TESTS)

target_compile_options(net.module_tests PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

target_include_directories(net.module_tests
  SYSTEM PRIVATE ${pugixml_Path}/src ${GTEST_INCLUDE_DIR})
target_link_libraries(net.module_tests
  ant4c
  ${DL_LIB}
  ${M_LIB}
  ${GTEST_MAIN_LIBRARY}
  pugixml
)

if(NOT MSVC)
  target_compile_features(net.module_tests
    PRIVATE
    cxx_std_11
  )
endif()

if(DEFINED PUGIXML_HEADER_ONLY)
  target_compile_definitions(net.module_tests
    PRIVATE -DPUGIXML_HEADER_ONLY=${PUGIXML_HEADER_ONLY}
  )
endif()

add_dependencies(net.module_tests
  ant4c.net.module
)
endif()