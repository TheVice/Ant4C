
if(DEFINED LIBRARY_BINARY_DIR)
  add_library(gtest INTERFACE)

  if(CMAKE_VERSION VERSION_EQUAL 3.19 OR CMAKE_VERSION VERSION_GREATER 3.19)
    file(REAL_PATH "${LIBRARY_BINARY_DIR}/../googletest/include" gtest_Path)
    file(REAL_PATH "${LIBRARY_BINARY_DIR}" LIBRARY_BINARY_DIR)
  else()
    set(gtest_Path "${LIBRARY_BINARY_DIR}/../googletest/include")
  endif()

  if(NOT IS_DIRECTORY "${gtest_Path}")
    message(FATAL_ERROR "gtest_Path '${gtest_Path}' is not a directory.")
  endif()

  target_include_directories(gtest SYSTEM INTERFACE "${gtest_Path}")

  if(MSVC)
    target_link_libraries(
      gtest
      INTERFACE
      debug "${LIBRARY_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/Debug/gtest.lib")
    target_link_libraries(
      gtest
      INTERFACE
      optimized "${LIBRARY_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/Release/gtest.lib")
  elseif(MINGW)
    target_link_libraries(
      gtest
      INTERFACE
      debug "${LIBRARY_BINARY_DIR}/x64/MinGW-W64-Debug/libgtest.a")
    target_link_libraries(
      gtest
      INTERFACE
      optimized "${LIBRARY_BINARY_DIR}/x64/MinGW-W64-Release/libgtest.a")
  else()
    find_library(
      GTest
      NAMES gtest
      PATHS "${LIBRARY_BINARY_DIR}")

    target_link_libraries(gtest INTERFACE ${GTest})
  endif()
else()
  find_package(GTest)

  if(${GTest_FOUND})
    if(CMAKE_VERSION VERSION_EQUAL 3.20 OR CMAKE_VERSION VERSION_GREATER 3.20)
    else()
      add_library(gtest INTERFACE)
      target_link_libraries(gtest INTERFACE GTest::GTest)
    endif()
  elseif(${GTEST_FOUND})
  else()
    message(STATUS "Search GTest via defined GTEST_PATH if such exists.")

    if(DEFINED ENV{GTEST_PATH})
      file(TO_CMAKE_PATH "$ENV{GTEST_PATH}" gtest_Path)
    elseif(DEFINED GTEST_PATH)
      file(TO_CMAKE_PATH "${GTEST_PATH}" gtest_Path)
    else()
      message(FATAL_ERROR "GTEST_PATH not set. Please check it at https://github.com/google/googletest/releases")
    endif()

    set(BUILD_GMOCK OFF)
    set(INSTALL_GTEST OFF)
    set(gtest_force_shared_crt ON) # CACHE BOOL "" FORCE)
    add_subdirectory(${gtest_Path} ${CMAKE_BINARY_DIR}/googletest)

    if(NOT MSVC)
    # DEBUG
    if(${DO_NOT_DISMISS_WARNING_AS_ERROR_AT_GTEST})
    else()
      get_target_property(COMPILE_FLAGS gtest COMPILE_FLAGS)
      string(REPLACE "-Werror" "" COMPILE_FLAGS "${COMPILE_FLAGS}")
      set_target_properties(gtest PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS})
    endif()
    endif()

    message(STATUS "GTest was found at the ${gtest_Path}.")
  endif()
endif()

if(TARGET gtest)
  add_library(GTest::gtest ALIAS gtest)
endif()
