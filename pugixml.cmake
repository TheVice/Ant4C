
set(pugixml_FOUND 0)

if((DEFINED ENV{pugixml_issues_390}) OR (DEFINED pugixml_issues_390))# https://github.com/zeux/pugixml/issues/390
else()
  find_package(pugixml)
endif()

if(1 EQUAL ${pugixml_FOUND})
else()
  message(STATUS "Search pugixml via defined PUGIXML_PATH if such exists.")

  if(DEFINED ENV{PUGIXML_PATH})
    string(REPLACE "\\" "/" pugixml_Path $ENV{PUGIXML_PATH})
  elseif(DEFINED PUGIXML_PATH)
    string(REPLACE "\\" "/" pugixml_Path ${PUGIXML_PATH})
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

  if(NOT MSVC)
    set_target_properties(pugixml PROPERTIES CXX_STANDARD 11)
  endif()

  if(("GNU" STREQUAL CMAKE_CXX_COMPILER_ID) AND (NOT MINGW))
    target_compile_options(pugixml PRIVATE "-fPIE")
  endif()

  message(STATUS "pugixml was found.")
endif()
