cmake_minimum_required(VERSION 2.8.12)

# project(dns)

# find_package(Boost COMPONENTS asio)
find_package(Boost COMPONENTS system)

add_library(ant4c.dns SHARED
  "${CMAKE_SOURCE_DIR}/dns.cpp"
  "${CMAKE_SOURCE_DIR}/dns.h")

if(${CMAKE_VERSION} VERSION_GREATER "3.2.9")
if(MSVC)
  target_compile_options(ant4c.dns PUBLIC $<$<CONFIG:DEBUG>:/W4 /GS>)
  target_compile_options(ant4c.dns PUBLIC $<$<CONFIG:RELEASE>:/W4 /GS>)
else()
  set_target_properties(ant4c.dns PROPERTIES CXX_STANDARD 11)
  target_compile_options(ant4c.dns PUBLIC $<$<CONFIG:DEBUG>:-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
  target_compile_options(ant4c.dns PUBLIC $<$<CONFIG:RELEASE>:-O3 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas>)
endif()
endif()

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

if(CMAKE_HOST_WIN32)
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

if(("GNU" STREQUAL CMAKE_CXX_COMPILER_ID) AND (NOT MINGW))
  #target_compile_options(ant4c.dns PRIVATE "-fPIE")
  set_target_properties(ant4c.dns PROPERTIES LINK_FLAGS "-pie -Wl,-z,now")
endif()
