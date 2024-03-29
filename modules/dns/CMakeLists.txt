
add_library(ant4c.dns SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/dns.adoc"
  "${CMAKE_CURRENT_SOURCE_DIR}/dns.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/dns.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/dns.xml"
)

# find_package(Boost COMPONENTS asio)
find_package(Boost COMPONENTS system)

if(Boost_FOUND)
  target_include_directories(ant4c.dns SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})

  if(MSVC)
    target_link_directories(ant4c.dns PRIVATE ${Boost_LIBRARY_DIRS})
  endif()
else()
  if(BOOST_ROOT)
    file(TO_CMAKE_PATH "${BOOST_ROOT}" BOOST_ROOT)

    if(EXISTS ${BOOST_ROOT}/boost)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT})
      target_compile_definitions(ant4c.dns PRIVATE BOOST_DATE_TIME_NO_LIB)
      target_compile_definitions(ant4c.dns PRIVATE BOOST_REGEX_NO_LIB)
      #target_compile_definitions(ant4c.dns PRIVATE BOOST_AUTO_LINK_NOMANGLE)
    endif()

    if(EXISTS ${BOOST_ROOT}/libs)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/date_time/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/throw_exception/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/asio/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/assert/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/bind/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/config/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/core/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/mpl/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/numeric/conversion/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/predef/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/preprocessor/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/regex/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/smart_ptr/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/static_assert/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/system/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/type_traits/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/utility/include)
      target_include_directories(ant4c.dns SYSTEM PRIVATE ${BOOST_ROOT}/libs/winapi/include)
    endif()

    if(MINGW)
      target_link_libraries(ant4c.dns ws2_32)
    endif()

    if(USE_PTHREAD)
      target_link_libraries(ant4c.dns pthread)
    endif()
  endif()
endif()

if(CMAKE_HOST_WIN32 AND (NOT (DEFINED _WIN32_WINNT)))
  set(WindowsVersion ${CMAKE_SYSTEM_VERSION})
  string(FIND ${WindowsVersion} "10.0." pos)

  if(-1 EQUAL ${pos})
    string(FIND ${WindowsVersion} "6.3" pos)

    if(-1 EQUAL ${pos})
      string(FIND ${WindowsVersion} "6.2" pos)

      if(-1 EQUAL ${pos})
        string(FIND ${WindowsVersion} "6.1" pos)

        if(-1 EQUAL ${pos})
          string(FIND ${WindowsVersion} "6.0" pos)

          if(-1 EQUAL ${pos})
            string(FIND ${WindowsVersion} "5.2" pos)

            if(-1 EQUAL ${pos})
              string(FIND ${WindowsVersion} "5.1" pos)

              if(-1 EQUAL ${pos})
                string(FIND ${WindowsVersion} "5.0" pos)

                if(-1 EQUAL ${pos})
                  # message("Unknown Windows")
                else()
                  # message("Windows 2000")
                endif()
                target_compile_definitions(ant4c.dns PRIVATE _WIN32_WINNT=0x0500)
              else()
                # message("Windows XP")
                target_compile_definitions(ant4c.dns PRIVATE _WIN32_WINNT=0x0501)
              endif()
            else()
              # message("Windows Server 2003")
              target_compile_definitions(ant4c.dns PRIVATE _WIN32_WINNT=0x0502)
            endif()
          else()
            # message("Windows Vista")
            target_compile_definitions(ant4c.dns PRIVATE _WIN32_WINNT=0x0600)
          endif()
        else()
          # message("Windows 7")
          target_compile_definitions(ant4c.dns PRIVATE _WIN32_WINNT=0x0601)
        endif()
      else()
        # message("Windows 8")
        target_compile_definitions(ant4c.dns PRIVATE _WIN32_WINNT=0x0602)
      endif()
    else()
      # message("Windows 8.1")
      target_compile_definitions(ant4c.dns PRIVATE _WIN32_WINNT=0x0603)
    endif()
  else()
    # message("Windows 10")
    target_compile_definitions(ant4c.dns PRIVATE _WIN32_WINNT=0x0A00)
  endif()
endif()

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_std_11;")
  target_compile_features(ant4c.dns
    PRIVATE
    cxx_std_11
  )
  else()
  set_property(TARGET ant4c.dns PROPERTY CXX_STANDARD 11)
  endif()
endif()

target_compile_options(ant4c.dns PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)
