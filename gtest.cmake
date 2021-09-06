if(DEFINED LIBRARY_BINARY_DIR)
  add_library(gtest INTERFACE)

  if(CMAKE_VERSION VERSION_EQUAL 3.19 OR CMAKE_VERSION VERSION_GREATER 3.19)
    file(REAL_PATH "${LIBRARY_BINARY_DIR}/../googletest/include" gtest_Path)
  else()
    set(gtest_Path "${LIBRARY_BINARY_DIR}/../googletest/include")
  endif()

  if(NOT IS_DIRECTORY "${gtest_Path}")
    message(FATAL_ERROR "gtest_Path '${gtest_Path}' is not a directory.")
  endif()

  target_include_directories(
    gtest SYSTEM INTERFACE "${gtest_Path}"
  )

  if(MSVC)
    target_link_libraries(
      gtest
      INTERFACE
      debug "${LIBRARY_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/Debug/gtest.lib"
    )
    target_link_libraries(
      gtest
      INTERFACE
      optimized "${LIBRARY_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/Release/gtest.lib"
    )
  elseif(MINGW)
    target_link_libraries(
      gtest
      INTERFACE
      debug "${LIBRARY_BINARY_DIR}/x64/MinGW-W64-Debug/libgtest.a"
    )
    target_link_libraries(
      gtest
      INTERFACE
      optimized "${LIBRARY_BINARY_DIR}/x64/MinGW-W64-Release/libgtest.a"
    )
  else()
    find_library(
      GTest
      NAMES gtest
      PATHS "${LIBRARY_BINARY_DIR}"
    )
    target_link_libraries(gtest INTERFACE ${GTest})
  endif()
  add_library(GTest::gtest ALIAS gtest)
else()
  set(GTest_FOUND False)
  find_package(GTest)

  if(${GTest_FOUND})
    if(CMAKE_VERSION VERSION_EQUAL 3.20 OR CMAKE_VERSION VERSION_GREATER 3.20)
    else()
      #add_library(GTest::gtest ALIAS GTest::GTest)
      add_library(gtest INTERFACE)
      target_link_libraries(gtest INTERFACE GTest::GTest)
      add_library(GTest::gtest ALIAS gtest)
    endif()
  else()
    message(STATUS "Search GTest via defined GTEST_PATH if such exists.")

    if(DEFINED ENV{GTEST_PATH})
      file(TO_CMAKE_PATH "$ENV{GTEST_PATH}" gtest_Path)
    elseif(DEFINED GTEST_PATH)
      file(TO_CMAKE_PATH "${GTEST_PATH}" gtest_Path)
    else()
      message(FATAL_ERROR "GTEST_PATH not set.")
    endif()

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

    add_library(gtest STATIC ${GTEST_FILES})
    target_include_directories(gtest SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
    target_compile_definitions(gtest PRIVATE GTEST_HAS_PTHREAD=0)

    if(NOT MSVC)
      if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
      target_compile_features(
        gtest
        PRIVATE
        cxx_std_11
      )
      else()
      set_property(TARGET gtest PROPERTY CXX_STANDARD 11)
      endif()
    endif()

    message(STATUS "GTest was found.")
    add_library(GTest::gtest ALIAS gtest)
  endif()
endif()
