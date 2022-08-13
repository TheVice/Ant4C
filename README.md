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
  * FreeBSD: *pkg install \*.txz*.
  * OpenBSD: *pkg_add -Dunsigned \*.tgz*.
  * RPM based: *rpm -Uvh \*.rpm*.

### Modules
Starting from version 2020.09 program support modules - binary files that increase functional by adding functions and/or tasks to the program.

* [ant4c.net.framework.module](modules/ant4c.net.framework). Module with namespaces **framework**, **metahost** and added to exists one **file** function *is-assembly*. Available only for Windows platform.
* [dns](modules/dns). Module with **dns** namespace that contain function *get-host-name*.
* [net.module](modules/net). Module to interact with installed .NET Core via namespaces **nethost**, **hostfxr** and function *is-assembly* from **file** namespace.
* [regex](modules/regex). Module that increase functional by adding ***regex*** task.

### Help

Help available at the [help.html file](https://github.com/TheVice/Ant4C/releases/download/v2020.09/help.html).

## Developing
Tests, at the develop branch, written in C++ and required [Google Test](https://github.com/google/googletest/) and [pugixml](https://github.com/zeux/pugixml/) libraries.
Also some of modules require third-party components.

## Building

### Supported Platforms

* Apple macOS
* FreeBSD
* Linux
* Microsoft Windows
* OpenBSD

Build can be done by one of C compilers - MSVC, MinGW, GCC or Clang after configuring was done with [CMake](https://www.cmake.org/download/).
Alternative 'make' tool or 'meson' can be used to build from source.

## License
This project available under terms of [MIT License](LICENSE).

Some of modules use third-party components that available by own license. See help files of modules to find out that terms.
