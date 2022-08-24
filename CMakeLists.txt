cmake_minimum_required(VERSION 2.8.12)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Configuration process cannot start from project source directory.")
endif()

project("Ant4C")

include("${CMAKE_SOURCE_DIR}/common.cmake")

append_to_flags_from_outside()
set_position_independent_code()

if(DEFINED PROGRAM_VERSION)
  get_compiler_version()
endif()

find_library(DL_LIB dl)
find_library(M_LIB m)

if("DL_LIB-NOTFOUND" STREQUAL DL_LIB)
  set(DL_LIB "")
endif()

if("M_LIB-NOTFOUND" STREQUAL M_LIB)
  set(M_LIB "")
endif()

set(
  ANT4C_SOURCES
  "${CMAKE_SOURCE_DIR}/argument_parser.c"
  "${CMAKE_SOURCE_DIR}/buffer.c"
  "${CMAKE_SOURCE_DIR}/choose_task.c"
  "${CMAKE_SOURCE_DIR}/common.c"
  "${CMAKE_SOURCE_DIR}/conversion.c"
  "${CMAKE_SOURCE_DIR}/copy_move.c"
  "${CMAKE_SOURCE_DIR}/date_time.c"
  "${CMAKE_SOURCE_DIR}/echo.c"
  "${CMAKE_SOURCE_DIR}/environment.c"
  "${CMAKE_SOURCE_DIR}/exec.c"
  "${CMAKE_SOURCE_DIR}/fail_task.c"
  "${CMAKE_SOURCE_DIR}/file_system.c"
  "${CMAKE_SOURCE_DIR}/for_each.c"
  "${CMAKE_SOURCE_DIR}/hash.blake2.c"
  "${CMAKE_SOURCE_DIR}/hash.blake3.c"
  "${CMAKE_SOURCE_DIR}/hash.c"
  "${CMAKE_SOURCE_DIR}/hash.crc32.c"
  "${CMAKE_SOURCE_DIR}/hash.sha3.c"
  "${CMAKE_SOURCE_DIR}/hash.xxhash.c"
  "${CMAKE_SOURCE_DIR}/if_task.c"
  "${CMAKE_SOURCE_DIR}/interpreter.c"
  "${CMAKE_SOURCE_DIR}/interpreter.conversion.c"
  "${CMAKE_SOURCE_DIR}/interpreter.date_time.c"
  "${CMAKE_SOURCE_DIR}/interpreter.echo.c"
  "${CMAKE_SOURCE_DIR}/interpreter.environment.c"
  "${CMAKE_SOURCE_DIR}/interpreter.exec.c"
  "${CMAKE_SOURCE_DIR}/interpreter.file_system.c"
  "${CMAKE_SOURCE_DIR}/interpreter.hash.c"
  "${CMAKE_SOURCE_DIR}/interpreter.load_file.c"
  "${CMAKE_SOURCE_DIR}/interpreter.math_unit.c"
  "${CMAKE_SOURCE_DIR}/interpreter.operating_system.c"
  "${CMAKE_SOURCE_DIR}/interpreter.path.c"
  "${CMAKE_SOURCE_DIR}/interpreter.property.c"
  "${CMAKE_SOURCE_DIR}/interpreter.sleep_unit.c"
  "${CMAKE_SOURCE_DIR}/interpreter.string_unit.c"
  "${CMAKE_SOURCE_DIR}/interpreter.target.c"
  "${CMAKE_SOURCE_DIR}/interpreter.version.c"
  "${CMAKE_SOURCE_DIR}/listener.c"
  "${CMAKE_SOURCE_DIR}/load_file.c"
  "${CMAKE_SOURCE_DIR}/load_tasks.c"
  "${CMAKE_SOURCE_DIR}/math_unit.c"
  "${CMAKE_SOURCE_DIR}/operating_system.c"
  "${CMAKE_SOURCE_DIR}/path.c"
  "${CMAKE_SOURCE_DIR}/project.c"
  "${CMAKE_SOURCE_DIR}/property.c"
  "${CMAKE_SOURCE_DIR}/range.c"
  "${CMAKE_SOURCE_DIR}/shared_object.c"
  "${CMAKE_SOURCE_DIR}/sleep_unit.c"
  "${CMAKE_SOURCE_DIR}/string_unit.c"
  "${CMAKE_SOURCE_DIR}/target.c"
  "${CMAKE_SOURCE_DIR}/task.c"
  "${CMAKE_SOURCE_DIR}/text_encoding.c"
  "${CMAKE_SOURCE_DIR}/try_catch.c"
  "${CMAKE_SOURCE_DIR}/version.c"
  "${CMAKE_SOURCE_DIR}/xml.c"
)

set(ANT4C_HEADERS
  "${CMAKE_SOURCE_DIR}/argument_parser.h"
  "${CMAKE_SOURCE_DIR}/buffer.h"
  "${CMAKE_SOURCE_DIR}/choose_task.h"
  "${CMAKE_SOURCE_DIR}/common.h"
  "${CMAKE_SOURCE_DIR}/conversion.h"
  "${CMAKE_SOURCE_DIR}/copy_move.h"
  "${CMAKE_SOURCE_DIR}/date_time.h"
  "${CMAKE_SOURCE_DIR}/echo.h"
  "${CMAKE_SOURCE_DIR}/environment.h"
  "${CMAKE_SOURCE_DIR}/exec.h"
  "${CMAKE_SOURCE_DIR}/fail_task.h"
  "${CMAKE_SOURCE_DIR}/file_system.h"
  "${CMAKE_SOURCE_DIR}/for_each.h"
  "${CMAKE_SOURCE_DIR}/hash.h"
  "${CMAKE_SOURCE_DIR}/if_task.h"
  "${CMAKE_SOURCE_DIR}/interpreter.h"
  "${CMAKE_SOURCE_DIR}/interpreter.exec.h"
  "${CMAKE_SOURCE_DIR}/interpreter.file_system.h"
  "${CMAKE_SOURCE_DIR}/interpreter.load_file.h"
  "${CMAKE_SOURCE_DIR}/interpreter.property.h"
  "${CMAKE_SOURCE_DIR}/interpreter.string_unit.h"
  "${CMAKE_SOURCE_DIR}/listener.h"
  "${CMAKE_SOURCE_DIR}/load_file.h"
  "${CMAKE_SOURCE_DIR}/load_tasks.h"
  "${CMAKE_SOURCE_DIR}/math_unit.h"
  "${CMAKE_SOURCE_DIR}/operating_system.h"
  "${CMAKE_SOURCE_DIR}/path.h"
  "${CMAKE_SOURCE_DIR}/project.h"
  "${CMAKE_SOURCE_DIR}/property.h"
  "${CMAKE_SOURCE_DIR}/range.h"
  "${CMAKE_SOURCE_DIR}/shared_object.h"
  "${CMAKE_SOURCE_DIR}/sleep_unit.h"
  "${CMAKE_SOURCE_DIR}/stdc_secure_api.h"
  "${CMAKE_SOURCE_DIR}/string_unit.h"
  "${CMAKE_SOURCE_DIR}/target.h"
  "${CMAKE_SOURCE_DIR}/task.h"
  "${CMAKE_SOURCE_DIR}/text_encoding.h"
  "${CMAKE_SOURCE_DIR}/try_catch.h"
  "${CMAKE_SOURCE_DIR}/version.h"
  "${CMAKE_SOURCE_DIR}/xml.h"
)

# ant4c

add_library(ant4c STATIC ${ANT4C_SOURCES} ${ANT4C_HEADERS})
set_target_properties(ant4c PROPERTIES PUBLIC_HEADER "${ANT4C_HEADERS}")
target_include_directories(ant4c SYSTEM INTERFACE
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(ant4c
    PRIVATE
    c_std_11)
  endif()
endif()

target_compile_options(ant4c PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(ant4c INTERFACE ${DL_LIB} ${M_LIB})

if(DEFINED PROGRAM_VERSION)
  target_compile_definitions(ant4c PRIVATE PROGRAM_VERSION="${PROGRAM_VERSION} ${COMPILER_VERSION}")
endif()

add_library(Ant4C::ant4c ALIAS ant4c)

# ant4c_shared
if(BUILD_SHARED)
add_library(ant4c_shared SHARED ${ANT4C_SOURCES} ${ANT4C_HEADERS})
# set_target_properties(ant4c_shared PROPERTIES PUBLIC_HEADER ${ANT4C_HEADERS})

set_property(TARGET ant4c_shared APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/ant4c_shared")
set_property(TARGET ant4c_shared APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/ant4c_shared")
set_property(TARGET ant4c_shared APPEND PROPERTY OUTPUT_NAME ant4c)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(ant4c_shared
    PRIVATE
    c_std_11)
  endif()
endif()

target_compile_options(ant4c_shared PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(ant4c_shared
  ${DL_LIB}
  ${M_LIB}
)

if(DEFINED PROGRAM_VERSION)
  target_compile_definitions(ant4c_shared PRIVATE PROGRAM_VERSION="${PROGRAM_VERSION} ${COMPILER_VERSION}")
endif()

add_library(Ant4C::ant4c_shared ALIAS ant4c_shared)
endif()

# ant4c_app

add_executable(
  ant4c_app
  "${CMAKE_SOURCE_DIR}/CHANGELOG"
  "${CMAKE_SOURCE_DIR}/LICENSE"
  "${CMAKE_SOURCE_DIR}/MD5SUMS"
  "${CMAKE_SOURCE_DIR}/README.md"
  "${CMAKE_SOURCE_DIR}/SHA3-224SUMS"
  "${CMAKE_SOURCE_DIR}/help.adoc"
  "${CMAKE_SOURCE_DIR}/main.c"
)

target_link_libraries(ant4c_app Ant4C::ant4c)

set_property(TARGET ant4c_app APPEND PROPERTY OUTPUT_NAME ant4c)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(ant4c_app
    PRIVATE
    c_std_11)
  endif()
endif()

target_compile_options(ant4c_app PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

if(DEFINED PROGRAM_VERSION)
  target_compile_definitions(ant4c_app PRIVATE PROGRAM_VERSION="${PROGRAM_VERSION} ${COMPILER_VERSION}")
endif()

add_executable(Ant4C::ant4c_app ALIAS ant4c_app)

# Listener

include("${CMAKE_SOURCE_DIR}/default_listener.cmake")

# Install ant4c

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(cmake_package_name ant4c)
set(cmake_files_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${cmake_package_name}")
set(config_file "${CMAKE_CURRENT_BINARY_DIR}/${cmake_package_name}Config.cmake")

install(
  TARGETS ant4c ant4c_app default_listener
  EXPORT ant4cTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${cmake_package_name}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${cmake_package_name})

install(
  EXPORT ant4cTargets
  NAMESPACE Ant4C::
  DESTINATION ${cmake_files_install_dir})

file(
  GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ant4cConfig.cmake"
  CONTENT
"include(CMakeFindDependencyMacro)
if((NOT MSVC) AND (NOT MINGW))
  #find_dependency(dl)
  #find_dependency(m)
endif()
include(\"\${CMAKE_CURRENT_LIST_DIR}/ant4cTargets.cmake\")"
)

install(FILES ${config_file} DESTINATION ${cmake_files_install_dir})

if(DEFINED PROGRAM_VERSION)
  set(
    version_file "${CMAKE_CURRENT_BINARY_DIR}/${cmake_package_name}ConfigVersion.cmake")
  write_basic_package_version_file(
    ${version_file} VERSION ${PROGRAM_VERSION} COMPATIBILITY AnyNewerVersion)
  install(
    FILES ${version_file} ${config_file}
    DESTINATION ${cmake_files_install_dir})
endif()
# Example module

include("${CMAKE_SOURCE_DIR}/modules/example.cmake")

