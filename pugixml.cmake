
if(DEFINED LIBRARY_BINARY_DIR)
  add_library(pugixml INTERFACE)

  if(CMAKE_VERSION VERSION_EQUAL 3.19 OR CMAKE_VERSION VERSION_GREATER 3.19)
    file(REAL_PATH "${LIBRARY_BINARY_DIR}/../pugixml/src" pugixml_DIR)
    file(REAL_PATH "${LIBRARY_BINARY_DIR}" LIBRARY_BINARY_DIR)
  else()
    set(pugixml_DIR "${LIBRARY_BINARY_DIR}/../pugixml/src")
  endif()

  if(NOT IS_DIRECTORY "${pugixml_DIR}")
    message(FATAL_ERROR "pugixml_DIR '${pugixml_DIR}' is not a directory.")
  endif()

  target_include_directories(pugixml SYSTEM INTERFACE "${pugixml_DIR}")

  if(MSVC)
    target_link_libraries(
      pugixml
      INTERFACE
      debug "${LIBRARY_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/Debug/pugixml.lib")
    target_link_libraries(
      pugixml
      INTERFACE
      optimized "${LIBRARY_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/Release/pugixml.lib")
  elseif(MINGW)
    target_link_libraries(
      pugixml
      INTERFACE
      debug "${LIBRARY_BINARY_DIR}/x64/MinGW-W64-Debug/libpugixml.a")
    target_link_libraries(
      pugixml
      INTERFACE
      optimized "${LIBRARY_BINARY_DIR}/x64/MinGW-W64-Release/libpugixml.a")
  else()
    find_library(
      pugixml_full_path
      NAMES pugixml
      PATHS "${LIBRARY_BINARY_DIR}")

    target_link_libraries(pugixml INTERFACE "${pugixml_full_path}")
  endif()

  set_target_properties(pugixml PROPERTIES PUBLIC_HEADER "${pugixml_DIR}/pugixml.hpp")

  add_library(Pugixml::pugixml ALIAS pugixml)
else()
  set(pugixml_FOUND False)

  if((DEFINED ENV{pugixml_issues_390}) OR (DEFINED pugixml_issues_390))# https://github.com/zeux/pugixml/issues/390
  else()
    find_package(pugixml)
  endif()

  if(${pugixml_FOUND})
    if("${pugixml_SOURCE_DIR}" STREQUAL "")
      foreach(include ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
        file(GLOB files ${include}/pugi*.h*)

        if(NOT("${files}" STREQUAL ""))
          break()
        endif()
      endforeach()

      if("${files}" STREQUAL "")
        message(WARNING "Public headers of the pugixml target could not be find at the includes - '${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}'.")
      else()
        set_target_properties(pugixml PROPERTIES PUBLIC_HEADER "${files}")
      endif()
    else()
      # target_include_directories(pugixml SYSTEM INTERFACE ${pugixml_SOURCE_DIR}/src)
      set_target_properties(pugixml PROPERTIES PUBLIC_HEADER "${pugixml_SOURCE_DIR}/src/pugixml.hpp")
    endif()

    if(CMAKE_VERSION VERSION_GREATER 3.10)
      add_library(Pugixml::pugixml ALIAS pugixml)
    endif()
  elseif(("" STREQUAL "$ENV{PUGIXML_PATH}") AND ("" STREQUAL "${PUGIXML_PATH}") AND
         (EXISTS "/usr/include/pugixml.hpp") AND (EXISTS "/usr/lib64/libpugixml.so"))
    message(STATUS "pugixml was found by directly checking of file on exists.")
    set(PUGIXML_FOUND True)
  else()
    message(STATUS "Search pugixml via defined PUGIXML_PATH if such exists.")

    if(DEFINED ENV{PUGIXML_PATH})
      file(TO_CMAKE_PATH "$ENV{PUGIXML_PATH}" pugixml_DIR)
    elseif(DEFINED PUGIXML_PATH)
      file(TO_CMAKE_PATH "${PUGIXML_PATH}" pugixml_DIR)
    else()
      message(FATAL_ERROR "PUGIXML_PATH not set.")
    endif()

    if(EXISTS ${pugixml_DIR}/CMakeLists.txt)
      add_subdirectory(${pugixml_DIR} ${CMAKE_BINARY_DIR}/pugixml)
    elseif(EXISTS ${pugixml_DIR}/scripts/CMakeLists.txt)
      add_subdirectory(${pugixml_DIR}/scripts ${CMAKE_BINARY_DIR}/pugixml)
    else()
      message(FATAL_ERROR "Could not find pugixml library at ${pugixml_DIR}, please check it at https://github.com/zeux/pugixml/releases")
    endif()

    target_include_directories(pugixml SYSTEM INTERFACE ${pugixml_DIR}/src)
    set_target_properties(pugixml PROPERTIES PUBLIC_HEADER "${pugixml_DIR}/src/pugixml.hpp")

    add_library(Pugixml::pugixml ALIAS pugixml)
    message(STATUS "pugixml was found at the ${pugixml_DIR}.")
  endif()
endif()
