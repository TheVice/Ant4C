
if(CMAKE_VERSION VERSION_LESS 3.1)
  function(target_compile_features one two three)
    string(FIND ${ARGV2} c_ position)

    if(0 EQUAL position)
      set(STANDARD C_STANDARD)
    endif()

    string(FIND ${ARGV2} cxx_ position)

    if(0 EQUAL position)
      set(STANDARD CXX_STANDARD)
    endif()

    string(FIND ${ARGV2} _ position REVERSE)
    math(EXPR position "${position} + 1")
    string(SUBSTRING ${ARGV2} ${position} -1 STANDARD_VERSION)

    set_property(TARGET ${ARGV0} PROPERTY ${STANDARD} ${STANDARD_VERSION})
  endfunction()
endif()

macro(get_compiler_version)
  if(MSVC)
    if(MSVC14)
      string(SUBSTRING ${MSVC_TOOLSET_VERSION} 0 2 VERSION_PREFIX)
      string(SUBSTRING ${MSVC_TOOLSET_VERSION} 2 -1 VERSION_SUFFIX)
      string(FIND "${CMAKE_C_COMPILER}" "${VERSION_PREFIX}.${VERSION_SUFFIX}" position REVERSE)
      if(-1 EQUAL position)
        set(COMPILER_VERSION "${VERSION_PREFIX}.${VERSION_SUFFIX}")
      else()
        string(SUBSTRING "${CMAKE_C_COMPILER}" ${position} -1 COMPILER_VERSION)
        string(FIND "${COMPILER_VERSION}" "/" position)
        string(SUBSTRING "${COMPILER_VERSION}" 0 ${position} COMPILER_VERSION)
      endif()
    else()
      string(SUBSTRING ${MSVC_TOOLSET_VERSION} 0 2 COMPILER_VERSION)
    endif()
    set(COMPILER_VERSION "MSVC ${COMPILER_VERSION}")
    if(CMAKE_CL_64)
      set(COMPILER_VERSION "${COMPILER_VERSION} x64")
    else()
      set(COMPILER_VERSION "${COMPILER_VERSION} x86")
    endif()
  elseif(MINGW)
    set(COMPILER_VERSION "MinGW-w64 ${CMAKE_C_COMPILER_VERSION} x64")
  else()
    if("GNU" STREQUAL "${CMAKE_C_COMPILER_ID}")
      set(COMPILER_VERSION "gcc")
    elseif("Clang" STREQUAL "${CMAKE_C_COMPILER_ID}")
      set(COMPILER_VERSION "clang")
    else()
      message(WARNING "CMAKE_C_COMPILER_ID -> ${CMAKE_C_COMPILER_ID}")
      set(COMPILER_VERSION "${CMAKE_C_COMPILER_ID}")
    endif()
    if(ANDROID)
      set(COMPILER_VERSION "${COMPILER_VERSION} ${CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION} ${CMAKE_ANDROID_ARCH} ${CMAKE_ANDROID_ARCH_ABI} ${CMAKE_SYSTEM_PROCESSOR}")
    else()
      set(COMPILER_VERSION "${COMPILER_VERSION} ${CMAKE_C_COMPILER_VERSION} x64")
    endif()
  endif()
endmacro()

macro(append_to_flags_from_outside)
  if(CMAKE_VERSION VERSION_LESS 3.0)
    if(DEFINED ENV{CFLAGS})
      list(APPEND CMAKE_C_FLAGS $ENV{CFLAGS})
    endif()

    if(DEFINED ENV{CPPFLAGS})
      list(APPEND CMAKE_CXX_FLAGS $ENV{CPPFLAGS})
    endif()

    if(DEFINED ENV{CXXFLAGS})
      list(APPEND CMAKE_CXX_FLAGS $ENV{CXXFLAGS})
    endif()

    if(DEFINED ENV{LDFLAGS})
      list(APPEND LINK_FLAGS $ENV{LDFLAGS})
    endif()

    if(DEFINED CFLAGS)
      list(APPEND CMAKE_C_FLAGS ${CFLAGS})
    endif()

    if(DEFINED CPPFLAGS)
      list(APPEND CMAKE_CXX_FLAGS ${CPPFLAGS})
    endif()

    if(DEFINED CXXFLAGS)
      list(APPEND CMAKE_CXX_FLAGS ${CXXFLAGS})
    endif()

    if(DEFINED LDFLAGS)
      list(APPEND LINK_FLAGS ${LDFLAGS})
    endif()

    string(REPLACE ";" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE ";" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE ";" "" LINK_FLAGS "${LINK_FLAGS}")
  endif()
endmacro()

macro(set_position_independent_code)
  if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()
endmacro()