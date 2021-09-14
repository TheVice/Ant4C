
add_library(net_gate
  "${CMAKE_CURRENT_SOURCE_DIR}/arguments.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/arguments.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/core_host_context_contract.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/core_host_context_contract.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/core_host_initialize_request.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/core_host_initialize_request.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/error_writer.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/error_writer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/host_fxr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/host_fxr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/host_interface.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/host_interface.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/host_policy.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/host_policy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.common.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.common.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.file.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.file.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.host.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.host.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.host_fxr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.host_fxr.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.host_policy.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.host_policy.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/net_delegate.h"
)

target_include_directories(net_gate PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(net_gate
    PRIVATE
    c_std_11
  )
  endif()
endif()

target_compile_options(net_gate PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

add_library(ant4c.net.module SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.module.cs"
  "${CMAKE_CURRENT_SOURCE_DIR}/ant4c.net.module.csproj"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.adoc"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/net.h"
)

target_link_libraries(ant4c.net.module net_gate Ant4C::ant4c)

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_C_COMPILE_FEATURES};" MATCHES ";c_std_11;")
  target_compile_features(ant4c.net.module
    PRIVATE
    c_std_11
  )
  endif()
endif()

target_compile_options(ant4c.net.module PRIVATE
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(net_gate INTERFACE Ant4C::ant4c)
add_library(Ant4C::net_gate ALIAS net_gate)
add_library(Ant4C::ant4c.net.module ALIAS ant4c.net.module)
