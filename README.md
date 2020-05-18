# Ant4C
Tool that interprets scenarios. Similar to Apache Ant and NAnt. Written in C.

## Overview
Ant for C is C-based script tool. Source of script should be written in XML-based code.
For understanding why tools with such vision was written read [Apache Ant Introduction](http://jakarta.apache.org/ant/manual/) or/and [NAnt help](http://nant.sourceforge.net/).

Name of program just reference to language on which source was written.

For first initial release echo and exec tasks support. Targets are not support. Functions from name spaces bool, cygpath, datetime, double, environment, int, int64, long, math, operating_system, path, platform, program, project, property, string, timespan and version are available.

Developing was started in August of 2019 and present to the public in October of 2019.

## Downloads
Binaries for Windows, Ubuntu, OpenBSD and FreeBSD are available on [release page](https://github.com/TheVice/Ant4C/releases/).
Also library available to use ant4c with other C/C++ projects.

### Linux distributions
* [Ubuntu 14.04](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_2019.10.21-1trusty1.0_amd64.deb)
* [Ubuntu 16.04](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_2019.10.21-1xenial1.0_amd64.deb)
* [Ubuntu 18.04](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_2019.10.21-1bionic1.0_amd64.deb)
* [Ubuntu 20.04](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_2019.10.21-1focal1.0_amd64.deb)

#### MD5SUMS
* ant4c_2019.10.21-1trusty1.0_amd64.deb 9af03bc7ef5a831d3e7c1227f09999f7
* ant4c_2019.10.21-1xenial1.0_amd64.deb 9bdcb84044235e1f28831ef3ef2197f3
* ant4c_2019.10.21-1bionic1.0_amd64.deb 6c3da2043ee97c2e651ce15c48218801
* ant4c_2019.10.21-1focal1.0_amd64.deb da9ddaf488ef4bdc9d46fa3d12b2b163

#### SHA3-224SUMS
* ant4c_2019.10.21-1trusty1.0_amd64.deb f1d8aa5d0ae37803d5dbb9c03cb772eb822ed5f09f398710c0ee7425
* ant4c_2019.10.21-1xenial1.0_amd64.deb d9bd021f55d0714226c99a56f2155a74a36e468ef3ff3e9997d0f223
* ant4c_2019.10.21-1bionic1.0_amd64.deb 4f56ee4404138292ef1df224f0bd32ac23440a8e88b5c90afdb947f2
* ant4c_2019.10.21-1focal1.0_amd64.deb a9b317377e01c1482b3686bbc4e7b46235e582b69ca3cef4067703a2

## Developing
Tests (at the develop branch) written in C++ and required [Google Test](https://github.com/google/googletest) and [pugixml](https://github.com/zeux/pugixml/) libraries.
On systems with missed this libraries in packages repositories or if unable to install it, PUGIXML_PATH and GTEST_PATH definition can be used while configure project with CMake.
For example:
* cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-j$(nproc) -s" -DPUGIXML_PATH=<full path>/pugixml-1.9/ -DGTEST_PATH=<full path>/googletest-release-1.8.1/ <full path>/Ant4C

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
