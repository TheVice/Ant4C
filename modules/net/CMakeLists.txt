add_library(net_gate
  "${CMAKE_CURRENT_SOURCE_DIR}/host_fxr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/host_fxr.h")

target_include_directories(net_gate PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

add_library(ant4c.net.module SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/net.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.h")
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
  if(APPLE)
  else()
    set_target_properties(ant4c.net.module PROPERTIES LINK_FLAGS "-Wl,-z,now")
  endif()
endif()

target_link_libraries(ant4c.net.module ant4c net_gate)
