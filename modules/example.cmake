
add_library(example_of_the_module SHARED
  "${CMAKE_SOURCE_DIR}/modules/example.cmake"
  "${CMAKE_SOURCE_DIR}/buffer.c"
  "${CMAKE_SOURCE_DIR}/buffer.h"
  "${CMAKE_SOURCE_DIR}/modules/example.c"
  "${CMAKE_SOURCE_DIR}/modules/example.h")
#target_link_libraries(example_of_the_module ant4c)

target_include_directories(example_of_the_module PUBLIC ${CMAKE_SOURCE_DIR})

if(NOT MSVC)
  set_property(TARGET example_of_the_module PROPERTY C_STANDARD 11)
endif()

target_compile_options(example_of_the_module PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)
