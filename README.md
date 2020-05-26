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

### For Linux distributions
 * [Ubuntu 14.04](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_2020.05-1trusty1.0_amd64.deb)
 * [Ubuntu 16.04](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_2020.05-1xenial1.0_amd64.deb)
 * [Ubuntu 18.04](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_2020.05-1bionic1.0_amd64.deb)
 * [Ubuntu 20.04](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_2020.05-1focal1.0_amd64.deb)

### Windows
* [MinGW-w64 8.1.0 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_app_2020.05_MinGW-w64_8.1.0_x64.zip)
* [MSVC 14.25 x64](https://github.com/TheVice/Ant4C/releases/download/v2020.05/ant4c_app_2020.05_MSVC_14.25_x64.zip)

#### Checksums

##### MD5SUMS
* ant4c_2020.05-1trusty1.0_amd64.deb e95861e59325ac2bf17affacf6fe5211
* ant4c_2020.05-1xenial1.0_amd64.deb 962fcbe412b1c6370e3a6eadf7775dc7
* ant4c_2020.05-1bionic1.0_amd64.deb 4f35a2c4ea2b821ebf4a8760fc5daca2
* ant4c_2020.05-1focal1.0_amd64.deb c51edd9aa41392eb9f4eddebc7c3a9d2
* ant4c_lib_2020.05_gcc_Ubuntu.tar.bz2 7a3a4b349e3e38eb18b322da138ae9ad
* ant4c_app_2020.05_MinGW-w64_8.1.0_x64.zip 80b2361d9a0fd7a0dd900e7e068d9c9e
* ant4c_app_2020.05_MSVC_14.25_x64.zip 602f6636bf05e2a907d8032ebcdcf780
* ant4c_lib_2020.05_MinGW-w64_8.1.0_x64.zip c49a0ea3d2f419443abbfc4200d84d0b
* ant4c_lib_2020.05_MSVC_14.25_x64.zip d5901e1a794cf6cd256d3539484d930b

##### SHA3-224SUMS
* ant4c_2020.05-1trusty1.0_amd64.deb 75be4da11f2c89c1d9c66213ddd270034986ce4888b782b0f58c3f3b
* ant4c_2020.05-1xenial1.0_amd64.deb 2021ad0f34286cd8c4af107b4366a9f056df20b53286e367e52688be
* ant4c_2020.05-1bionic1.0_amd64.deb 489e36182d2905d8101ed783440bb75362385833439866acc98c4452
* ant4c_2020.05-1focal1.0_amd64.deb 21022d65492c82a6dbcf4d6713151207afcc26ec58a105a0be5fabbe
* ant4c_lib_2020.05_gcc_Ubuntu.tar.bz2 566d12ad7b2279142b17550fadaa87274549dce6a0a126b4b42e09c2
* ant4c_app_2020.05_MinGW-w64_8.1.0_x64.zip 02a130753d634049d0e43f92e77e399f5eeb081c2e80656ee0e01240
* ant4c_app_2020.05_MSVC_14.25_x64.zip ab1d274422f2f615dbffd6728a9cc67aa0c24b68dee91a11ff69700c
* ant4c_lib_2020.05_MinGW-w64_8.1.0_x64.zip 0485abf3a5260e5276e75bb3aee947d33d6f3e4b117ea7dc6e31c372
* ant4c_lib_2020.05_MSVC_14.25_x64.zip 822665b2f6fe2ab5c677a593a053e762c2818249dd79ea6bdb01856c

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
