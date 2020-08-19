# Ant4C
Tool that interprets scenarios. Similar to Apache Ant and NAnt. Written in C.

## Overview
Ant for C is C-based script tool. Source of script should be written in XML-based code.
For understanding why tools with such vision was written read [Apache Ant Introduction](http://jakarta.apache.org/ant/manual/) or/and [NAnt help](http://nant.sourceforge.net/).
For different and specific possibility read the [help file](https://github.com/TheVice/Ant4C/releases/download/v2020.05/help.html).

Name of program just reference to language on which source was written.
Developing was started in the summer of 2019 and first present to the public in October of 2019.
The second public release available on April of 2020.
Third on May of 2020.

## Downloads
All binaries available on the [release page](https://github.com/TheVice/Ant4C/releases/).

### For Linux distributions
* [Ubuntu 14.04 (Trusty Tahr)](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_2020.05-1trusty1.0_amd64.deb)
* [Ubuntu 16.04 (Xenial Xerus)](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_2020.05-1xenial1.0_amd64.deb)
* [Ubuntu 18.04 (Bionic Beaver)](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_2020.05-1bionic1.0_amd64.deb)
* [Ubuntu 20.04 (Focal Fossa)](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_2020.05-1focal1.0_amd64.deb)

### For BSD distributions
* [FreeBSD 11](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c-2020.05_freebsd_11.txz)
* [FreeBSD 12](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c-2020.05_freebsd_12.txz)
* [OpenBSD 6.4](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c-2020.05_openbsd_6.4.tgz)
* [OpenBSD 6.5](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c-2020.05_openbsd_6.5.tgz)
* [OpenBSD 6.6](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c-2020.05_openbsd_6.6.tgz)
* [OpenBSD 6.7](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c-2020.05_openbsd_6.7.tgz)

### For Windows
* [MinGW-w64 8.1.0 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_app_2020.05_MinGW-w64_8.1.0_x64.zip)
* [MSVC 12 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_app_2020.05_MSVC_12_x64.zip)
* [MSVC 12 x86](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_app_2020.05_MSVC_12_x86.zip)
* [MSVC 14.25 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_app_2020.05_MSVC_14.25_x64.zip)

#### Checksums

* [MD5SUMS](MD5SUMS)
* [SHA3-224SUMS](SHA3-224SUMS)

#### Help

Help available at the [help.html file](https://github.com/TheVice/Ant4C/releases/download/v2020.05/help.html).

## Developing
Tests (at the develop branch) written in C++ and required [Google Test](https://github.com/google/googletest/) and [pugixml](https://github.com/zeux/pugixml/) libraries.
On systems with missed this libraries in packages repositories or if unable to install it, PUGIXML_PATH and GTEST_PATH definition can be used while configure project with CMake.
Also for ant4c.regex module, used at testing, required [Boost regex](https://github.com/boostorg/regex/) library.
For example:
* cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-s" -DPUGIXML_PATH=<full path>/pugixml-1.9/ -DGTEST_PATH=<full path>/googletest-release-1.8.1/ -DBOOST_ROOT=<full path>/boost_1_73_0 <full path>/Ant4C
```
Optional -j$(nproc) or -j`sysctl -n hw.ncpu` can be added to the CMAKE_ECLIPSE_MAKE_ARGUMENTS depend on your operation system.
```

## Building
Build can be done by one of C compilers - MSVC, MinGW, GCC or CLang after configuring was done with [CMake](http://www.cmake.org/download/).

## License
This project available under terms of [MIT License](LICENSE).

### ant4c.dns and ant4c.regex modules
Modules available by same, MIT License, but internally they use Boost libraries - that available by [Boost Software License](https://github.com/boostorg/boost/blob/7dd85823c436b0a858c2f97f29b6a44beea71dfb/LICENSE_1_0.txt).
