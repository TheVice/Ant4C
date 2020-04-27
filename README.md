# Ant4C
Tool that interprets scenarios. Similar to Apache Ant and NAnt. Written in C.

## Overview
Ant for C is C-based script tool. Source of script should be written in XML-based code.
For understanding why tools with such vision was written read [Apache Ant Introduction](http://jakarta.apache.org/ant/manual/) or/and [NAnt help](http://nant.sourceforge.net/).

Name of program just reference to language on which source was written.
Developing was started in August of 2019 and first present to the public in October of 2019.
The second public release available on April of 2020.

## Downloads
Binaries for Windows, Ubuntu, OpenBSD and FreeBSD are available on [release page](https://github.com/TheVice/Ant4C/releases).
Also library available to use ant4c with other C/C++ projects.

## Developing
Tests (at the develop branch) written in C++ and required [Google Test](https://github.com/google/googletest) and [pugixml](https://github.com/zeux/pugixml/) libraries.
On systems with missed this libraries in packages repositories or if unable to install it, PUGIXML_PATH and GTEST_PATH definition can be used while configure project with CMake.
For example:
* cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-j$(nproc) -s" -DPUGIXML_PATH=<full path>/pugixml-1.9/ -DGTEST_PATH=<full path>/googletest-release-1.8.1/ <full path>/Ant4C

## Building
Build can be done by one of C compilers - MSVC, MinGW, GCC or CLang, after configuring with [CMake](http://www.cmake.org/download/).
For release configuration, without version information, Ant4C script can be used.
* ant4c_app -buildfile:build.build -D:cmake=cmake
* ant4c_app -buildfile:build.build -D:VS2019="" -D:cmake=cmake
* ant4c_app -buildfile:build.build -D:VS2019="" -D:cmake_tool="ClangCL" -D:cmake=cmake
* ant4c_app -buildfile:build.build -D:VS2017="" -D:cmake=cmake
* ant4c_app -buildfile:build.build -D:VS2017="" -D:cmake_arch="Win64" -D:cmake=cmake
* ant4c_app -buildfile:build.build -D:VS2017="" -D:cmake_tool="v141_clang_c2" -D:cmake=cmake
* ant4c_app -buildfile:build.build -D:MinGW="" -D:cmake=cmake

Of course full path to cmake (for example -D:cmake=/usr/local/bin/cmake or -D:cmake="C:\Program Files\cmake\bin\cmake.exe") and build file should be provided.

## License
This project available under terms of [MIT License](LICENSE).
