if(MSVC)
  add_library(framework_gate
    "${CMAKE_CURRENT_SOURCE_DIR}/clr_control.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/clr_control.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/host_controller.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/host_controller.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/meta_host.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/meta_host.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/runtime_host.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/runtime_host.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/runtime_info.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/runtime_info.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/unknown_structure.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/unknown_structure.h")

  target_include_directories(framework_gate PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

  add_library(ant4c.net.framework.module SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.framework.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.framework.h")

  target_link_libraries(ant4c.net.framework.module ant4c framework_gate)

  target_compile_options(framework_gate PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(framework_gate PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)

  target_compile_options(ant4c.net.framework.module PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(ant4c.net.framework.module PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)
endif()