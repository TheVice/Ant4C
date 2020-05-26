# Ant4C
Tool that interprets scenarios. Similar to Apache Ant and NAnt. Written in C.

## Overview
Ant for C is C-based script tool. Source of script should be written in XML-based code.
For understanding why tools with such vision was written read [Apache Ant Introduction](http://jakarta.apache.org/ant/manual/) or/and [NAnt help](http://nant.sourceforge.net/).

Name of program just reference to language on which source was written.
Developing was started in August of 2019 and first present to the public in October of 2019.
The second public release available on April of 2020.

## Downloads
All binaries available on [release page](https://github.com/TheVice/Ant4C/releases/).
Also static libraries available to use ant4c with other C/C++ projects.

### For Linux distributions
* [Ubuntu 16.04](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_app_2020.04_gcc5.4.0_Ubuntu.tar.bz2)

### For BSD distributions
* [FreeBSD 12.1](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_app_2020.04_clang_8_FreeBSD.tar.bz2)
* [OpenBSD 6.5](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_app_2020.04_clang_7_OpenBSD.tar.bz2)

### For Windows
* [MSVC 14.25 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_app_2020.04_MSVC_14.25_x64.zip)
* [MinGW-w64 8.1.0 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_app_2020.04_MinGW-w64_8.1.0_x64.zip)

### MD5SUMS
* ant4c_app_2020.04_clang_7_OpenBSD.tar.bz2 83f6fcc7fbd7aa040ab2773b602e7673
* ant4c_app_2020.04_clang_8_FreeBSD.tar.bz2 4431ced87ce5eebcdc7dd4e28df1b41c
* ant4c_app_2020.04_gcc5.4.0_Ubuntu.tar.bz2 04a3d7855fa779f7afd67923b183d96b
* ant4c_app_2020.04_MinGW-w64_8.1.0_x64.zip 32c56e572b9cfbdef81c725564190d20
* ant4c_app_2020.04_MSVC_14.25_x64.zip 2f14c043dabcf682b340b53e317cc746
* ant4c_lib_2020.04_clang_7_OpenBSD.tar.bz2 f22f4a7f27301037b0e0837739fb20da
* ant4c_lib_2020.04_clang_8_FreeBSD.tar.bz2 29defd11bc0c6bb11d6b1dd2c686cfe7
* ant4c_lib_2020.04_gcc5.4.0_Ubuntu.tar.bz2 52095ecbeaa40d8230625b832e92dae2
* ant4c_lib_2020.04_MinGW-w64_8.1.0_x64.zip 0b7848a9af23fa3a8d5dec05b67d22a9
* ant4c_lib_2020.04_MSVC_14.25_x64.zip 1f18de4f57d4a8c928d02747480f8903

### SHA3-224SUMS
* ant4c_app_2020.04_clang_7_OpenBSD.tar.bz2 c9f7c6d1f76e75b7488772f6161dc1b94a88e82a6501e2644fa366ea
* ant4c_app_2020.04_clang_8_FreeBSD.tar.bz2 2b0da59268a59961dcd65f05841d84f1a92f7cd29fcd65e7a3a84fca
* ant4c_app_2020.04_gcc5.4.0_Ubuntu.tar.bz2 c96a97b100091f8fa7a044858a59b3d27803e9f6c92a24d4acda3aa1
* ant4c_app_2020.04_MinGW-w64_8.1.0_x64.zip eaf2fda8c180d58e1c7cfe9a1886d10273f66b42d557a6134c5858b1
* ant4c_app_2020.04_MSVC_14.25_x64.zip 211fa7b384f4adc4e38fafc0a3b718162de38f264146b034482e1f2c
* ant4c_lib_2020.04_clang_7_OpenBSD.tar.bz2 47473e09285bc1573820f0bd46ae7044373bb08a9d1f29e0af961763
* ant4c_lib_2020.04_clang_8_FreeBSD.tar.bz2 9a1b7d1ec2e96088995fee73884a87aca8428a97cc859052a4a50aa2
* ant4c_lib_2020.04_gcc5.4.0_Ubuntu.tar.bz2 fe60a3cb3c4acbac7cfbe9c14069cb05229a465b2f24dea469e36110
* ant4c_lib_2020.04_MinGW-w64_8.1.0_x64.zip 1dabb6f78f30f1960a71e21627f477ab3c3644454ac3021bdbe5e70a
* ant4c_lib_2020.04_MSVC_14.25_x64.zip b59b9b0d80384786356622d6ac5b9d84fb5792e396bf5c6365ee46f1

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
