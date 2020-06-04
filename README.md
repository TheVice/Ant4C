# Ant4C
Tool that interprets scenarios. Similar to Apache Ant and NAnt. Written in C.

## Overview
Ant for C is C-based script tool. Source of script should be written in XML-based code.
For understanding why tools with such vision was written read [Apache Ant Introduction](http://jakarta.apache.org/ant/manual/) or/and [NAnt help](http://nant.sourceforge.net/).

Name of program just reference to language on which source was written.
Developing was started in August of 2019 and first present to the public in October of 2019.
The second public release available on April of 2020.

## Downloads
All binaries available on the [release page](https://github.com/TheVice/Ant4C/releases/).

### For Linux distributions
* [Ubuntu 14.04 (Trusty Tahr)](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_2020.04-1trusty1.0_amd64.deb)
* [Ubuntu 16.04 (Xenial Xerus)](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_2020.04-1xenial1.0_amd64.deb)
* [Ubuntu 18.04 (Bionic Beaver)](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_2020.04-1bionic1.0_amd64.deb)
* [Ubuntu 20.04 (Focal Fossa)](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_2020.04-1focal1.0_amd64.deb)

### For BSD distributions
* [FreeBSD 11](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c-2020.04_freebsd_11.txz)
* [FreeBSD 12](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c-2020.04_freebsd_12.txz)
* [OpenBSD 6.4](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c-2020.04_openbsd_6.4.tgz)
* [OpenBSD 6.5](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c-2020.04_openbsd_6.5.tgz)
* [OpenBSD 6.6](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c-2020.04_openbsd_6.6.tgz)
* [OpenBSD 6.7](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c-2020.04_openbsd_6.7.tgz)

### For Windows
* [MSVC 14.25 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_app_2020.04_MSVC_14.25_x64.zip)
* [MinGW-w64 8.1.0 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.04/ant4c_app_2020.04_MinGW-w64_8.1.0_x64.zip)

#### Checksums

* [MD5SUMS](MD5SUMS)
* [SHA3-224SUMS](SHA3-224SUMS)

## Developing
Tests (at the develop branch) written in C++ and required [Google Test](https://github.com/google/googletest) and [pugixml](https://github.com/zeux/pugixml/) libraries.
On systems with missed this libraries in packages repositories or if unable to install it, PUGIXML_PATH and GTEST_PATH definition can be used while configure project with CMake.
For example:
* cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-s" -DPUGIXML_PATH=<full path>/pugixml-1.9/ -DGTEST_PATH=<full path>/googletest-release-1.8.1/ <full path>/Ant4C
```
Optional -j$(nproc) or -j`sysctl -n hw.ncpu` can be added to the CMAKE_ECLIPSE_MAKE_ARGUMENTS depend on your operation system.
```

## Building
Build can be done by one of C compilers - MSVC, MinGW, GCC or CLang after configuring was done with [CMake](http://www.cmake.org/download/).

## License
This project available under terms of [MIT License](LICENSE).
