
set(ANT4C_SOURCE_FILES
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

add_library(ant4c STATIC ${ANT4C_SOURCE_FILES})

if(MSVC)
  target_compile_options(ant4c PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(ant4c PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)
else()
  set_target_properties(ant4c PROPERTIES C_STANDARD 11)
  target_compile_options(ant4c PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(ant4c PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
endif()

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
