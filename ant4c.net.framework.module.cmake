if(MSVC)
  add_library(framework_gate
     "${CMAKE_SOURCE_DIR}/clr_control.c"
     "${CMAKE_SOURCE_DIR}/clr_control.h"
     "${CMAKE_SOURCE_DIR}/meta_host.c"
     "${CMAKE_SOURCE_DIR}/meta_host.h"
     "${CMAKE_SOURCE_DIR}/runtime_host.c"
     "${CMAKE_SOURCE_DIR}/runtime_host.h"
     "${CMAKE_SOURCE_DIR}/runtime_info.c"
     "${CMAKE_SOURCE_DIR}/runtime_info.h"
     "${CMAKE_SOURCE_DIR}/unknown_structure.c"
     "${CMAKE_SOURCE_DIR}/unknown_structure.h")

  target_include_directories(framework_gate SYSTEM PRIVATE ${CMAKE_SOURCE_DIR})

  project("Ant4C" LANGUAGES CSharp)

  include(CSharpUtilities)

  add_library(ant4c.net.framework.module.clr
      "${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.AssemblyInfo.cs"
      "${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.CustomAppDomainManager.cs"
      "${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.IFrameworkNamespace.cs"
      "${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.VersionDetector.cs")

  add_library(ant4c.net.framework.module SHARED
      "${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.c"
      "${CMAKE_SOURCE_DIR}/ant4c.net.framework.module.h"
      "${CMAKE_SOURCE_DIR}/host_controller.c"
      "${CMAKE_SOURCE_DIR}/host_controller.h")

  target_include_directories(ant4c.net.framework.module SYSTEM PRIVATE ${CMAKE_SOURCE_DIR})

  add_dependencies(ant4c.net.framework.module ant4c.net.framework.module.clr)
  target_link_libraries(ant4c.net.framework.module ant4c framework_gate)

  list(APPEND SOURCES_OF_TESTS
            "${CMAKE_SOURCE_DIR}/tests_ant4c.net.framework.module.cpp")
  list(APPEND LIBRARIES4TESTING framework_gate)
endif()