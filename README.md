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

### For Linux distributions
* [Ubuntu 14.04](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_2019.10.21-1trusty1.0_amd64.deb)
* [Ubuntu 16.04](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_2019.10.21-1xenial1.0_amd64.deb)
* [Ubuntu 18.04](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_2019.10.21-1bionic1.0_amd64.deb)
* [Ubuntu 20.04](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_2019.10.21-1focal1.0_amd64.deb)

### For BSD distributions
* [FreeBSD 12.0](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_app_2019.10.21_clang_6.0.1_FreeBSD12.0_x64.tar.bz2)
* [OpenBSD 6.5](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_app_2019.10.21_clang_7.0.1_OpenBSD6.5_x64.tar.bz2)

### For Windows
* [MSVC 12 x64](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_app_2019.10.21_MSVC_12_x64.zip)
* [MSVC 12 x86](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_app_2019.10.21_MSVC_12_x86.zip)
* [MSVC 14.23 x64](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_app_2019.10.21_MSVC_1423_x64.zip)
* [MSVC 14.23 x86](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_app_2019.10.21_MSVC_1423_x86.zip)
* [MinGW-w64 8.1.0 x64](https://github.com/TheVice/Ant4C/releases/download/v2019.10.21/ant4c_app_2019.10.21_MinGW-w64_8.1.0_x64.zip)

### MD5SUMS
* ant4c_2019.10.21-1trusty1.0_amd64.deb 9af03bc7ef5a831d3e7c1227f09999f7
* ant4c_2019.10.21-1xenial1.0_amd64.deb 9bdcb84044235e1f28831ef3ef2197f3
* ant4c_2019.10.21-1bionic1.0_amd64.deb 6c3da2043ee97c2e651ce15c48218801
* ant4c_2019.10.21-1focal1.0_amd64.deb da9ddaf488ef4bdc9d46fa3d12b2b163
* ant4c_app_2019.10.21_clang_6.0.1_FreeBSD12.0_x64.tar.bz2 baa9652333cb2bb9f025114a43fcbe8d
* ant4c_app_2019.10.21_clang_7.0.1_OpenBSD6.5_x64.tar.bz2 eed7b24da7f8f32390bd89361132d9e7
* ant4c_app_2019.10.21_MinGW-w64_8.1.0_x64.zip 655dfd71eff0ca996e047c8dae5fddcf
* ant4c_app_2019.10.21_MSVC_12_x64.zip e50d8efed6d6dd7ed60e44262f1506b3
* ant4c_app_2019.10.21_MSVC_12_x86.zip f4d2829b6c5ecb3020564d91df22d9e8
* ant4c_app_2019.10.21_MSVC_1423_x64.zip 7422c041574bb3613cf36f107dc1a06e
* ant4c_app_2019.10.21_MSVC_1423_x86.zip 595bbbf18525ea112cff3c9c7bc0d90e
* ant4c_lib_2019.10.21_clang_6.0.1_FreeBSD12.0_x64.tar.bz2 59ce8cf0ee700ce4cc2abdb00c9cc5c6
* ant4c_lib_2019.10.21_clang_7.0.1_OpenBSD6.5_x64.tar.bz2 91b7539d2a55f256a310e58c9a53e6bb
* ant4c_lib_2019.10.21_gcc5.4.0-Ubuntu_16.04_x64.tar.bz2 e54a640d4cf7d958100e56fa83266e2
* ant4c_lib_2019.10.21_MinGW-w64_8.1.0_x64.zip 56a49b3552878bc0ed04ece1508c03e7
* ant4c_lib_2019.10.21_MSVC_12_x64.zip 3c82bc3c38d4c98d2ac086a30fd19698
* ant4c_lib_2019.10.21_MSVC_12_x86.zip 3dae54997ae5bc41fe9ca7eb2d243a39
* ant4c_lib_2019.10.21_MSVC_1423_x64.zip bfa731eee86a808ee1025fd5016ba1f1
* ant4c_lib_2019.10.21_MSVC_1423_x86.zip bd495143f69da952a2389f32035979c5

### SHA3-224SUMS
* ant4c_2019.10.21-1trusty1.0_amd64.deb f1d8aa5d0ae37803d5dbb9c03cb772eb822ed5f09f398710c0ee7425
* ant4c_2019.10.21-1xenial1.0_amd64.deb d9bd021f55d0714226c99a56f2155a74a36e468ef3ff3e9997d0f223
* ant4c_2019.10.21-1bionic1.0_amd64.deb 4f56ee4404138292ef1df224f0bd32ac23440a8e88b5c90afdb947f2
* ant4c_2019.10.21-1focal1.0_amd64.deb a9b317377e01c1482b3686bbc4e7b46235e582b69ca3cef4067703a2
* ant4c_app_2019.10.21_clang_6.0.1_FreeBSD12.0_x64.tar.bz2 140f4f747bef98730c864cf4216d9890e64c147e6775776cc6c11db6
* ant4c_app_2019.10.21_clang_7.0.1_OpenBSD6.5_x64.tar.bz2 0cdf2c94266d736174bbdab6eed2d5e86aba9dfd2a446633b3c485d3
* ant4c_app_2019.10.21_MinGW-w64_8.1.0_x64.zip 596688df845f8e3aba54f46c5b74edef4643dfe65c29cb5d7e3185b2
* ant4c_app_2019.10.21_MSVC_12_x64.zip bef6ecb51b4d8eb117258f665657c21b217e04cd0f8cadcc5ec28b97
* ant4c_app_2019.10.21_MSVC_12_x86.zip a7e5ee79d28bb8ff3a32c6282db45c6a85a0440a7d3cea3d967a550c
* ant4c_app_2019.10.21_MSVC_1423_x64.zip 947e00d7b8b274a2907e383b199a489d13a01c12af0e247445387b1f
* ant4c_app_2019.10.21_MSVC_1423_x86.zip dceaac7edc87f757859db33e536b8c45f06d236801c9175294d11926
* ant4c_lib_2019.10.21_clang_6.0.1_FreeBSD12.0_x64.tar.bz2 167ffffff47c7d4c04a7ab76f43830c0f5b3a9b41dba433b27c97dd4
* ant4c_lib_2019.10.21_clang_7.0.1_OpenBSD6.5_x64.tar.bz2 d451e2b089a87b2c5fb8df4b9a1eaabc2e33060acf0c4a2ff6718d37
* ant4c_lib_2019.10.21_gcc5.4.0-Ubuntu_16.04_x64.tar.bz2 c70aae2b9cf024cde2cb42c05554fcb93f30121dc5a05024970b4edd
* ant4c_lib_2019.10.21_MinGW-w64_8.1.0_x64.zip 88abc3c9562357a1ed6063850964cc80f3056b4435233bd44c5a5443
* ant4c_lib_2019.10.21_MSVC_12_x64.zip f99b68b6da764ee8a869513b9f7c38a6c785c87178a5b86e51af1b01
* ant4c_lib_2019.10.21_MSVC_12_x86.zip 6e8f5277ef59dc4dfe94bf7be72c34e71645e709486d79abbe2eb25b
* ant4c_lib_2019.10.21_MSVC_1423_x64.zip e354d111f08642e4042ab220a573f4d50a1e268981270f506c5da29e
* ant4c_lib_2019.10.21_MSVC_1423_x86.zip 4c2e2af1c776dc59a0a6d70034e6bcebc05abd5f63a43a8eeee8be0e

## Developing
Tests (at the develop branch) written in C++ and required [Google Test](https://github.com/google/googletest) and [pugixml](https://github.com/zeux/pugixml/) libraries.
On systems with missed this libraries in packages repositories or if unable to install it, PUGIXML_PATH and GTEST_PATH definition can be used while configure project with CMake.
For example:
* cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-s" -DPUGIXML_PATH=<full path>/pugixml-1.9/ -DGTEST_PATH=<full path>/googletest-release-1.8.1/ <full path>/Ant4C
Optional -j$(nproc) or -j`sysctl -n hw.ncpu` can be added to the CMAKE_ECLIPSE_MAKE_ARGUMENTS depend on your operation system.

## Building
Build can be done by one of C compilers - MSVC, MinGW, GCC or CLang after configuring was done with [CMake](http://www.cmake.org/download/).

## License
This project available under terms of [MIT License](LICENSE).
