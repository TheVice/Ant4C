# Ant4C
Tool that interprets scenarios. Similar to Apache Ant and NAnt. Written in C.

# Overview
Ant for C is C-based script tool. Source of script should be written in XML-based code.
For understanding why tools with such vision was written read [Apache Ant Introduction](http://jakarta.apache.org/ant/manual/) or/and [NAnt help](http://nant.sourceforge.net/).

Name of program just reference to language on which source was written.

For first initial release echo and exec tasks support. Targets are not support. Functions from name spaces bool, cygpath, datetime, double, environment, int, int64, long, math, operating_system, path, platform, program, project, property, string, timespan and version are available.

Developing was started in August of 2019 and first present to the public in October of 2019.

# Downloads
Binaries for Windows, Ubuntu 16.04, OpenBSD 6.5 and FreeBSD 12.0 are available on [release page](https://github.com/TheVice/Ant4C/releases).
Also library available to use ant4c with other C/C++ projects.
For Windows binaries present after compiled with MinGW and Visual Studio 2019.

# Differences from NAnt
Some name space have addition functions (like is64bit-operating-system from environment) and addition version (like substring with two arguments from string addition to version with three arguments) comparing to NAnt implementation. Some functions are missed.

# Developing
Tests written in C++ and required [Google Test](https://github.com/google/googletest) and [pugixml](https://github.com/zeux/pugixml/) libraries.
On systems with missed this libraries in packages repositories or if unable to install it, PUGIXML_PATH and GTEST_PATH definition can be used while configure project with CMake.
For example:
* cmake -DCMAKE_BUILD_TYPE=Debug -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-j$(nproc) -s" -DPUGIXML_PATH=/home/user/Downloads/pugixml-1.9/ -DGTEST_PATH=/home/user/Downloads/googletest-release-1.8.1/ /home/user/Downloads/Ant4C

# Building
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

# List of functions
```
bool::parse
bool::to-string
```
```
double::parse
double::to-string
```
```
int::parse
int::to-string
```
```
long::parse
long::to-string
```
```
datetime::format-to-string
datetime::parse
datetime::to-string
```
```
datetime::get-day
datetime::get-day-of-year
datetime::get-days-in-month
datetime::get-hour
datetime::get-minute
datetime::get-month
datetime::get-second
datetime::get-year
datetime::is-leap-year
datetime::now
datetime::from-input
```
```
timespan::parse
timespan::to-string
```
```
timespan::from-days
timespan::from-hours
timespan::from-minutes
timespan::from-seconds
timespan::get-days
timespan::get-hours
timespan::get-minutes
timespan::get-seconds
timespan::get-total-days
timespan::get-total-hours
timespan::get-total-minutes
timespan::get-total-seconds
```
```
environment::get-folder-path
environment::get-machine-name
environment::get-operating-system
environment::get-user-name
environment::get-variable
environment::get-version
environment::newline
environment::variable-exists
```
```
math::abs
math::ceiling
math::floor
math::round
math::acos
math::asin
math::atan
math::atan2
math::cos
math::cosh
math::exp
math::log
math::log10
math::max
math::min
math::pow
math::sign
math::sin
math::sinh
math::sqrt
math::tan
math::tanh
math::cot
math::coth
math::truncate
math::PI
math::E
math::degrees
math::radians
math::addition
math::subtraction
math::multiplication
math::division
math::near
math::less
math::greater
```
```
program::version
program::current_directory
```
```
platform::get-name
platform::is-unix
platform::is-windows
```
```
project::get-base-directory
project::get-buildfile-path
project::get-buildfile-uri
project::get-default-target
project::get-name
```
```
property::exists
property::get-value
property::is-dynamic
property::is-readonly
```
```
operating-system::get-platform
operating-system::get-version
operating-system::to-string
```
```
path::change-extension
path::combine
path::get-directory-name
path::get-extension
path::get-file-name
path::get-file-name-without-extension
path::get-full-path
path::get-path-root
path::get-temp-file-name
path::get-temp-path
path::has-extension
path::is-path-rooted
```
```
string::contains
string::empty
string::ends-with
string::equal
string::get-length
string::index-of
string::last-index-of
string::pad-left
string::pad-right
string::quote
string::replace
string::starts-with
string::substring
string::to-lower
string::to-upper
string::trim
string::trim-end
string::trim-start
string::un-quote
```
```
cygpath::get-dos-path
cygpath::get-unix-path
cygpath::get-windows-path
```
```
version::parse
version::to-string
version::get-build
version::get-major
version::get-minor
version::get-revision
```
