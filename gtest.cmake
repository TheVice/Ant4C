
if(DEFINED GTEST_BINARY_PATH)
  if(CMAKE_HOST_WIN32)
  string(REPLACE "\\" "/" gtest_Path ${GTEST_BINARY_PATH}/../../googletest)
  else()
  string(REPLACE "\\" "/" gtest_Path ${GTEST_BINARY_PATH}/../googletest)
  endif()

  set(GTEST_INCLUDE_DIR ${gtest_Path}/include)
  set(GTEST_INCLUDE_DIRS ${gtest_Path} ${GTEST_INCLUDE_DIR})

  set(GTEST_MAIN_LIBRARY gtest)

  find_library(GTest ${GTEST_MAIN_LIBRARY} ${GTEST_BINARY_PATH})

  add_library(${GTEST_MAIN_LIBRARY} STATIC IMPORTED)
  #target_include_directories(${GTEST_MAIN_LIBRARY} SYSTEM INTERFACE ${GTEST_INCLUDE_DIRS})
  include_directories(${GTEST_INCLUDE_DIRS})
  set_target_properties(${GTEST_MAIN_LIBRARY} PROPERTIES IMPORTED_LOCATION ${GTest})

  list(APPEND LIBRARIES4TESTING ${GTEST_MAIN_LIBRARY})
else()
set(GTest_FOUND False)
find_package(GTest)

if(${GTest_FOUND})
  list(APPEND LIBRARIES4TESTING GTest::GTest)
else()
  message(STATUS "Search GTest via defined GTEST_PATH if such exists.")

  if(DEFINED ENV{GTEST_PATH})
    string(REPLACE "\\" "/" gtest_Path $ENV{GTEST_PATH})
  elseif(DEFINED GTEST_PATH)
    string(REPLACE "\\" "/" gtest_Path ${GTEST_PATH})
  else()
    message(FATAL_ERROR "GTEST_PATH not set.")
  endif()

  set(GTEST_MAIN_LIBRARY gtest)

  if(EXISTS ${gtest_Path}/include)
    set(GTEST_INCLUDE_DIR ${gtest_Path}/include)
    set(GTEST_INCLUDE_DIRS ${gtest_Path} ${GTEST_INCLUDE_DIR})
  else()
    set(GTEST_INCLUDE_DIR ${gtest_Path}/googletest/include)
    set(GTEST_INCLUDE_DIRS ${gtest_Path}/googletest ${GTEST_INCLUDE_DIR})
  endif()

  if((EXISTS ${gtest_Path}/src/gtest-all.cc) AND (EXISTS ${gtest_Path}/src/gtest_main.cc))
    set(GTEST_FILES ${gtest_Path}/src/gtest-all.cc)
  elseif((EXISTS ${gtest_Path}/gtest/gtest-all.cc) AND (EXISTS ${gtest_Path}/gtest/gtest_main.cc))
    set(GTEST_FILES ${gtest_Path}/gtest/gtest-all.cc)
  elseif((EXISTS ${gtest_Path}/googletest/src/gtest-all.cc) AND (EXISTS ${gtest_Path}/googletest/src/gtest_main.cc))
    set(GTEST_FILES ${gtest_Path}/googletest/src/gtest-all.cc)
  else()
    message(FATAL_ERROR "Could not find Google Test library at ${gtest_Path}, please check it at https://github.com/google/googletest/releases")
  endif()

  add_library(${GTEST_MAIN_LIBRARY} STATIC ${GTEST_FILES})
  target_include_directories(${GTEST_MAIN_LIBRARY} SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
  target_compile_definitions(${GTEST_MAIN_LIBRARY} PRIVATE GTEST_HAS_PTHREAD=0)

  if(NOT MSVC)
    set_target_properties(${GTEST_MAIN_LIBRARY} PROPERTIES CXX_STANDARD 11)
  endif()

  if(("GNU" STREQUAL CMAKE_CXX_COMPILER_ID) AND (NOT MINGW))
    target_compile_options(${GTEST_MAIN_LIBRARY} PRIVATE "-fPIE")
  endif()

  list(APPEND LIBRARIES4TESTING ${GTEST_MAIN_LIBRARY})

  message(STATUS "GTest was found.")
endif()
endif()
