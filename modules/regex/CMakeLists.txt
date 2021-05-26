
add_library(ant4c.regex SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/regex.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/regex.h"
  "${CMAKE_SOURCE_DIR}/text_encoding.c"
  "${CMAKE_SOURCE_DIR}/text_encoding.cpp"
  "${CMAKE_SOURCE_DIR}/text_encoding.h")

if(${CMAKE_VERSION} VERSION_GREATER "3.2.9")
if(MSVC)
  target_compile_options(ant4c.regex PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(ant4c.regex PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)
else()
  set_target_properties(ant4c.regex PROPERTIES CXX_STANDARD 11)
  target_compile_options(ant4c.regex PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(ant4c.regex PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
endif()
else()
if(NOT MSVC)
  set_target_properties(ant4c.regex PROPERTIES CXX_STANDARD 11)
endif()
endif()

target_compile_definitions(ant4c.regex PUBLIC NO_BUFFER_UNIT)
target_compile_definitions(ant4c.regex PUBLIC NO_COMMON_UNIT)

find_package(Boost COMPONENTS regex)

if(Boost_FOUND)
  target_include_directories(ant4c.regex SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})

  if(MSVC)
    target_link_directories(ant4c.regex PRIVATE ${Boost_LIBRARY_DIRS})
    set_target_properties(ant4c.regex PROPERTIES PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set_target_properties(ant4c.regex PROPERTIES PDB_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
  else()
    target_link_libraries(ant4c.regex ${Boost_REGEX_LIBRARY})
  endif()
else()
  if(BOOST_ROOT)
    file(TO_CMAKE_PATH "${BOOST_ROOT}" BOOST_ROOT)

    file(GLOB CPP_FILES ${BOOST_ROOT} ${BOOST_ROOT}/libs/regex/src/*.cpp)
    add_library(boost_regex ${CPP_FILES} ${H_FILES})

    if(EXISTS ${BOOST_ROOT}/boost)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT})
    endif()

    if(EXISTS ${BOOST_ROOT}/libs)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/assert/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/config/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/container_hash/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/core/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/detail/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/integer/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/mpl/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/predef/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/preprocessor/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/regex/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/smart_ptr/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/static_assert/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/throw_exception/include)
      target_include_directories(boost_regex SYSTEM PUBLIC ${BOOST_ROOT}/libs/type_traits/include)
    endif()

    if(NOT MSVC)
      set_target_properties(boost_regex PROPERTIES CXX_STANDARD 11)
      target_compile_options(boost_regex PRIVATE "-fPIC")
    endif()

    target_link_libraries(ant4c.regex boost_regex)
    #target_compile_definitions(ant4c.regex PRIVATE BOOST_LIB_NAME=regex)
    target_compile_definitions(ant4c.regex PRIVATE BOOST_AUTO_LINK_NOMANGLE)
  endif()
endif()

if((NOT MSVC) AND (NOT MINGW) AND (NOT (CMAKE_HOST_SYSTEM_NAME STREQUAL "OpenBSD")))
  #target_compile_options(ant4c.regex PRIVATE "-fPIE")
  if(APPLE)
    #set_target_properties(ant4c.regex PROPERTIES LINK_FLAGS "-pie")
  else()
    set_target_properties(ant4c.regex PROPERTIES LINK_FLAGS "-pie -Wl,-z,now")
  endif()
endif()
