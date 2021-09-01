
add_library(example_of_the_module SHARED
  "${CMAKE_SOURCE_DIR}/modules/example.cmake"
  "${CMAKE_SOURCE_DIR}/modules/example.c"
  "${CMAKE_SOURCE_DIR}/modules/example.h")
target_link_libraries(example_of_the_module ant4c)

target_include_directories(example_of_the_module PUBLIC ${CMAKE_SOURCE_DIR})

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(example_of_the_module
    PRIVATE
    c_std_11
  )
  endif()
endif()

target_compile_options(example_of_the_module PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)
