
add_library(default_listener SHARED
  "${CMAKE_SOURCE_DIR}/default_listener.cmake"
  "${CMAKE_SOURCE_DIR}/default_listener.c"
  "${CMAKE_SOURCE_DIR}/default_listener.h"
)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(default_listener
    PRIVATE
    c_std_11
  )
  endif()
endif()

target_compile_options(default_listener PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)
