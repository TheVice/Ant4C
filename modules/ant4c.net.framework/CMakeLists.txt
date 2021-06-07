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
    "${CMAKE_CURRENT_SOURCE_DIR}/unknown_structure.h"
  )

  target_include_directories(framework_gate PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
  target_compile_options(framework_gate PRIVATE /W4)

  if(MSVC_VERSION VERSION_LESS 1910)
    target_include_directories(framework_gate PUBLIC SYSTEM "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/Include/um")
    if(CMAKE_CL_64)
      target_link_directories(framework_gate INTERFACE SYSTEM "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/Lib/um/x64")
    else()
      target_link_directories(framework_gate INTERFACE SYSTEM "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/Lib/um/x86")
    endif()
  endif()

  add_library(ant4c.net.framework.module SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.framework.adoc"
    "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.framework.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.framework.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.framework.module.clr.csproj"
    "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.framework.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/CustomAppDomainManager.cs"
    "${CMAKE_CURRENT_SOURCE_DIR}/IFrameworkNamespace.cs"
    "${CMAKE_CURRENT_SOURCE_DIR}/VersionDetector.cs"
  )

  target_link_libraries(ant4c.net.framework.module ant4c framework_gate)
  target_compile_options(ant4c.net.framework.module PRIVATE /W4 /WX)
endif()