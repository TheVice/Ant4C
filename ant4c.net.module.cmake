add_library(net_gate
  "${CMAKE_SOURCE_DIR}/host_fxr.c"
  "${CMAKE_SOURCE_DIR}/host_fxr.h")

add_library(ant4c.net.module SHARED
  "${CMAKE_SOURCE_DIR}/ant4c.net.module.c"
  "${CMAKE_SOURCE_DIR}/ant4c.net.module.h")
if(${CMAKE_VERSION} VERSION_GREATER "3.2.9")
if(MSVC)
  target_compile_options(net_gate PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(net_gate PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)

  target_compile_options(ant4c.net.module PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(ant4c.net.module PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)
else()
  set_target_properties(net_gate PROPERTIES C_STANDARD 11)
  target_compile_options(net_gate PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(net_gate PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)

  set_target_properties(ant4c.net.module PROPERTIES C_STANDARD 11)
  target_compile_options(ant4c.net.module PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(ant4c.net.module PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
endif()
endif()
if((NOT MSVC) AND (NOT MINGW))
  target_compile_options(net_gate PRIVATE "-fPIC")
  target_compile_options(ant4c.net.module PRIVATE "-fPIC")
  set_target_properties(ant4c.net.module PROPERTIES LINK_FLAGS "-Wl,-z,now")
endif()

target_link_libraries(ant4c.net.module ant4c net_gate)
