
add_library(default_listener SHARED
  "${CMAKE_SOURCE_DIR}/default_listener.cmake"
  "${CMAKE_SOURCE_DIR}/default_listener.c"
  "${CMAKE_SOURCE_DIR}/default_listener.h"
)

if(NOT MSVC)
  target_compile_features(default_listener
    PRIVATE
    c_std_11
  )
endif()

target_compile_options(default_listener PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)
