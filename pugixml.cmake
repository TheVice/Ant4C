
if(DEFINED LIBRARY_BINARY_DIR)
  add_library(pugixml INTERFACE)

  if(CMAKE_VERSION VERSION_EQUAL 3.19 OR CMAKE_VERSION VERSION_GREATER 3.19)
    file(REAL_PATH "${LIBRARY_BINARY_DIR}/../pugixml/src" pugixml_Path)
  else()
    set(pugixml_Path "${LIBRARY_BINARY_DIR}/../pugixml/src")
  endif()

  if(NOT IS_DIRECTORY "${pugixml_Path}")
    message(FATAL_ERROR "pugixml_Path '${pugixml_Path}' is not a directory.")
  endif()

  target_include_directories(
    pugixml SYSTEM INTERFACE
    "${pugixml_Path}"
  )

  if(MSVC)
    target_link_libraries(
      pugixml
      INTERFACE
      debug "${LIBRARY_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/Debug/pugixml.lib"
    )
    target_link_libraries(
      pugixml
      INTERFACE
      optimized "${LIBRARY_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/Release/pugixml.lib"
    )
  elseif(MINGW)
    target_link_libraries(
      pugixml
      INTERFACE
      debug "${LIBRARY_BINARY_DIR}/x64/MinGW-W64-Debug/libpugixml.a"
    )
    target_link_libraries(
      pugixml
      INTERFACE
      optimized "${LIBRARY_BINARY_DIR}/x64/MinGW-W64-Release/libpugixml.a"
    )
  else()
    find_library(
      pugixml_full_path
      NAMES pugixml
      PATHS ${LIBRARY_BINARY_DIR}
    )
    target_link_libraries(pugixml INTERFACE "${pugixml_full_path}")
  endif()
else()
set(pugixml_FOUND False)

if((DEFINED ENV{pugixml_issues_390}) OR (DEFINED pugixml_issues_390))# https://github.com/zeux/pugixml/issues/390
else()
  find_package(pugixml)
endif()

if(${pugixml_FOUND})
else()
  message(STATUS "Search pugixml via defined PUGIXML_PATH if such exists.")

  if(DEFINED ENV{PUGIXML_PATH})
    file(TO_CMAKE_PATH "$ENV{PUGIXML_PATH}" pugixml_Path)
  elseif(DEFINED PUGIXML_PATH)
    file(TO_CMAKE_PATH "${PUGIXML_PATH}" pugixml_Path)
  else()
    message(FATAL_ERROR "PUGIXML_PATH not set.")
  endif()

  if(EXISTS ${pugixml_Path}/CMakeLists.txt)
    add_subdirectory(${pugixml_Path} ${CMAKE_BINARY_DIR}/pugixml)
  elseif(EXISTS ${pugixml_Path}/scripts/CMakeLists.txt)
    add_subdirectory(${pugixml_Path}/scripts ${CMAKE_BINARY_DIR}/pugixml)
  else()
    message(FATAL_ERROR "Could not find pugixml library at ${pugixml_Path}, please check it at https://github.com/zeus/pugixml/releases")
  endif()

  message(STATUS "pugixml was found.")
endif()
endif()
