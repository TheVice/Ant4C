
add_library(default_listener SHARED
            "${CMAKE_SOURCE_DIR}/default_listener.c"
            "${CMAKE_SOURCE_DIR}/default_listener.h")

if(${CMAKE_VERSION} VERSION_GREATER "3.2.9")
if(MSVC)
  target_compile_options(default_listener PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(default_listener PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)
else()
  set_target_properties(default_listener PROPERTIES C_STANDARD 11)
  target_compile_options(default_listener PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(default_listener PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
endif()
endif()

if(("GNU" STREQUAL CMAKE_CXX_COMPILER_ID) AND (NOT MINGW))
  target_compile_options(default_listener PRIVATE "-fPIE")
  set_target_properties(default_listener PROPERTIES LINK_FLAGS "-pie -Wl,-z,now")
endif()
