# Ant4C
Tool that interprets scenarios. Similar to Apache Ant and NAnt. Written in C.

## Overview
Ant for C is C-based script tool. Source of script should be written in XML-based code.
For understanding why tools with such vision was written read [Apache Ant Introduction](http://jakarta.apache.org/ant/manual/) or/and [NAnt help](http://nant.sourceforge.net/).

Name of program just reference to language on which source was written.
Developing was started in August of 2019 and first present to the public in October of 2019.
The second public release available on April of 2020.
Third on May of 2020.

## Downloads
All binaries available on [release page](https://github.com/TheVice/Ant4C/releases/).
Also static libraries available to use ant4c with other C/C++ projects.

### Windows
* [ant4c_app_2020.05_MinGW-w64_8.1.0_x64.zip](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_app_2020.05_MinGW-w64_8.1.0_x64.zip)
* [ant4c_app_2020.05_MSVC_14.25_x64.zip](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_app_2020.05_MSVC_14.25_x64.zip)
* [ant4c_lib_2020.05_MinGW-w64_8.1.0_x64.zip](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_lib_2020.05_MinGW-w64_8.1.0_x64.zip)
* [ant4c_lib_2020.05_MSVC_14.25_x64.zip](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_lib_2020.05_MSVC_14.25_x64.zip)

#### Checksums

##### MD5SUMS
* ant4c_app_2020.05_MinGW-w64_8.1.0_x64.zip 80b2361d9a0fd7a0dd900e7e068d9c9e
* ant4c_app_2020.05_MSVC_14.25_x64.zip 602f6636bf05e2a907d8032ebcdcf780
* ant4c_lib_2020.05_MinGW-w64_8.1.0_x64.zip c49a0ea3d2f419443abbfc4200d84d0b
* ant4c_lib_2020.05_MSVC_14.25_x64.zip d5901e1a794cf6cd256d3539484d930b

##### SHA3-224SUMS
* ant4c_app_2020.05_MinGW-w64_8.1.0_x64.zip  02a130753d634049d0e43f92e77e399f5eeb081c2e80656ee0e01240
* ant4c_app_2020.05_MSVC_14.25_x64.zip  ab1d274422f2f615dbffd6728a9cc67aa0c24b68dee91a11ff69700c
* ant4c_lib_2020.05_MinGW-w64_8.1.0_x64.zip  0485abf3a5260e5276e75bb3aee947d33d6f3e4b117ea7dc6e31c372
* ant4c_lib_2020.05_MSVC_14.25_x64.zip  822665b2f6fe2ab5c677a593a053e762c2818249dd79ea6bdb01856c

## Developing
Tests (at the develop branch) written in C++ and required [Google Test](https://github.com/google/googletest) and [pugixml](https://github.com/zeux/pugixml/) libraries.
On systems with missed this libraries in packages repositories or if unable to install it, PUGIXML_PATH and GTEST_PATH definition can be used while configure project with CMake.
For example:
* cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-s" -DPUGIXML_PATH=<full path>/pugixml-1.9/ -DGTEST_PATH=<full path>/googletest-release-1.8.1/ <full path>/Ant4C
Optional -j$(nproc) or -j`sysctl -n hw.ncpu` can be added to the CMAKE_ECLIPSE_MAKE_ARGUMENTS depend on your operation system.

## Building
Build can be done by one of C compilers - MSVC, MinGW, GCC or CLang after configuring was done with [CMake](http://www.cmake.org/download/).
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
