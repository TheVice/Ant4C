# Ant4C
Tool that interprets scenarios. Similar to [Apache Ant](http://jakarta.apache.org/ant/manual/) and [NAnt](http://nant.sourceforge.net/). Written in C.

## Overview
Ant for C is C-based script tool. Source of script should be written in XML-based code.
Name of program just reference to language on which source was written.

Developing was started in the summer of 2019.

## Downloads
All binaries available on the [release page](https://github.com/TheVice/Ant4C/releases/).

### Installing
Depend on variant available for your system:
* Unpack *zip* archive to prefer location.
* For packages - install program using the system package manager.
  * Alpine Linux: *apk add --allow-untrusted \*.apk*.
  * Debian based: *dpkg --install \*.deb*.
  * RPM based: *rpm -Uvh \*.rpm*.
  * FreeBSD: *pkg install \*.txz*.
  * OpenBSD: *pkg_add -Dunsigned \*.tgz*.

### Help

Help available at the [help.html file](https://github.com/TheVice/Ant4C/releases/download/v2020.09/help.html).

## Developing
Tests, at the develop branch, written in C++ and required [Google Test](https://github.com/google/googletest/) and [pugixml](https://github.com/zeux/pugixml/) libraries.
On systems with missed this libraries in packages repositories or if unable to install it, PUGIXML_PATH and GTEST_PATH definition can be used while configure project with CMake.
To build *ant4c.regex* and *ant4c.regex* modules, also used by tests, required [Boost](https://github.com/boostorg/) libraries.

* cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-s" -DPUGIXML_PATH=\<full path\>/pugixml-1.9/ -DGTEST_PATH=\<full path\>/googletest-release-1.8.1/ -DBOOST_ROOT=\<full path\>/boost_1_73_0 \<full path\>/Ant4C

Optional for [Eclipse IDE](https://www.eclipse.org/downloads/) users:

```
-j$(nproc) or -j`sysctl -n hw.ncpu` be added to the CMAKE_ECLIPSE_MAKE_ARGUMENTS depend on your operation system.
```

## Building
Build can be done by one of C compilers - MSVC, MinGW, GCC or CLang after configuring was done with [CMake](https://www.cmake.org/download/).

## License
This project available under terms of [MIT License](LICENSE).

### [ant4c.dns](https://github.com/TheVice/Ant4C/releases/download/v2020.09/ant4c.dns.zip) and [ant4c.regex](https://github.com/TheVice/Ant4C/releases/download/v2020.09/ant4c.regex.zip) modules
Modules available by same, MIT License, but internally they use Boost libraries - that available by [Boost Software License](https://github.com/boostorg/boost/blob/7dd85823c436b0a858c2f97f29b6a44beea71dfb/LICENSE_1_0.txt).
