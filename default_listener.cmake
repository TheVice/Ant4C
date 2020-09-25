
add_library(default_listener SHARED
            "${CMAKE_SOURCE_DIR}/default_listener.c"
            "${CMAKE_SOURCE_DIR}/default_listener.h")

if(("GNU" STREQUAL "${CMAKE_CXX_COMPILER_ID}") AND (NOT MINGW))
  target_compile_options(default_listener PRIVATE "-fPIE")
  set_target_properties(default_listener PROPERTIES LINK_FLAGS "-pie -Wl,-z,now")
endif()
