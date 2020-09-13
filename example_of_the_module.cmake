
add_library(example_of_the_module SHARED
            "${CMAKE_SOURCE_DIR}/buffer.c"
            "${CMAKE_SOURCE_DIR}/buffer.h"
            "${CMAKE_SOURCE_DIR}/example_of_the_module.c"
            "${CMAKE_SOURCE_DIR}/example_of_the_module.h")
#target_link_libraries(example_of_the_module ant4c)
