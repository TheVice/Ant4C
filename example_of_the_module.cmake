
add_library(example_of_the_module SHARED
            "${CMAKE_SOURCE_DIR}/buffer.c"
            "${CMAKE_SOURCE_DIR}/buffer.h"
            "${CMAKE_SOURCE_DIR}/example_of_the_module.c"
            "${CMAKE_SOURCE_DIR}/example_of_the_module.h")
#target_link_libraries(example_of_the_module ant4c)

if(${CMAKE_VERSION} VERSION_GREATER "3.2.9")
if(MSVC)
  target_compile_options(example_of_the_module PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(example_of_the_module PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)
else()
  set_target_properties(example_of_the_module PROPERTIES C_STANDARD 11)
  target_compile_options(example_of_the_module PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(example_of_the_module PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
endif()
endif()
