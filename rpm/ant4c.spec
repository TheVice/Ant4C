Summary:        Execute XML based scripts
Name:           ant4c
Version:        2020.09
Release:        1%{?dist}
# Group:        Development/Tools
# Group:        Development/Tools/Other

License:        MIT
URL:            https://github.com/TheVice/%{name}
Source0:        https://github.com/TheVice/%{name}/archive/v%{version}.tar.gz
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  pugixml-devel
BuildRequires:  gtest-devel
%description
Cross-platform, open-source script tool.
Ant4C is the program that interprets XML based scenarios.
There are tasks and name spaces with functions that
can be used for writing scripts.

%package     -n ant4c-devel
Summary:        Development files for %{name}
%description -n ant4c-devel
This package contains library of the %{name}.

%prep
%autosetup -p1 -n v%{version}

%build
%cmake -DPROGRAM_VERSION="%{version}"
%cmake_build

%install
%cmake_install
mkdir -p %{buildroot}/%{_mandir}/man1
install -m 0644 %{name}.1.gz %{buildroot}/%{_mandir}/man1/%{name}.1.gz

%check
%ctest

%files
%license LICENSE
%{_bindir}/ant4c
%{_libdir}/libdefault_listener.so
%{_mandir}/man1/%{name}.1.gz
%doc help.html

%files -n ant4c-devel
%{_includedir}/%{name}/argument_parser.h
%{_includedir}/%{name}/buffer.h
%{_includedir}/%{name}/choose_task.h
%{_includedir}/%{name}/common.h
%{_includedir}/%{name}/conversion.h
%{_includedir}/%{name}/copy_move.h
%{_includedir}/%{name}/date_time.h
%{_includedir}/%{name}/echo.h
%{_includedir}/%{name}/environment.h
%{_includedir}/%{name}/exec.h
%{_includedir}/%{name}/fail_task.h
%{_includedir}/%{name}/file_system.h
%{_includedir}/%{name}/for_each.h
%{_includedir}/%{name}/hash.h
%{_includedir}/%{name}/if_task.h
%{_includedir}/%{name}/interpreter.exec.h
%{_includedir}/%{name}/interpreter.file_system.h
%{_includedir}/%{name}/interpreter.h
%{_includedir}/%{name}/interpreter.string_unit.h
%{_includedir}/%{name}/listener.h
%{_includedir}/%{name}/load_file.h
%{_includedir}/%{name}/load_tasks.h
%{_includedir}/%{name}/math_unit.h
%{_includedir}/%{name}/operating_system.h
%{_includedir}/%{name}/path.h
%{_includedir}/%{name}/project.h
%{_includedir}/%{name}/property.h
%{_includedir}/%{name}/range.h
%{_includedir}/%{name}/shared_object.h
%{_includedir}/%{name}/sleep_unit.h
%{_includedir}/%{name}/stdc_secure_api.h
%{_includedir}/%{name}/string_unit.h
%{_includedir}/%{name}/target.h
%{_includedir}/%{name}/task.h
%{_includedir}/%{name}/text_encoding.h
%{_includedir}/%{name}/try_catch.h
%{_includedir}/%{name}/version.h
%{_includedir}/%{name}/xml.h
%{_libdir}/cmake/ant4c/ant4cConfig.cmake
%{_libdir}/cmake/ant4c/ant4cTargets-noconfig.cmake
%{_libdir}/cmake/ant4c/ant4cTargets.cmake
%{_libdir}/cmake/ant4c/ant4cConfigVersion.cmake
%{_libdir}/libant4c.a

%changelog
* Mon Sep 14 2020 TheVice <TheVice> - 2020.09-1
- Introduced support of modules.
- Functions from the path unit allowed to operate with empty paths.
    Function 'combine' can be used for normalizing path.
- Increased list of symbols to trim from string at 'string::trim'
    function.
- Replaced script hash functions with hash length at the names with
    'blake2b', 'blake3', 'keccak' and 'sha3'.
- Added support of 'blake3' and 'keccak' algorithms at the script
    function 'file::get-checksum'.
- Added argument for configure algorithm - byte order, hash length,
    to the 'file::get-checksum' function.
- Added possibility to use external events listener.
- Added 'loadtasks' task.
- For POSIX systems added searching of files with '.build' extension,
    if no file provided at command prompt.
- Added 'path::glob' function.
- Allowed to call executable from the directory that was set
    at the PATH environment variable.

* Sun May 24 2020 TheVice <TheVice> - 2020.05-1
- Attempt to set value for read only property will not fail
    evaluation of script.
- Fixed enumeration of target dependencies if space or/and tabs
    was used between names.
- Fixed enumeration of strings with non ASCII chars at 'for_each' task.
- WIN32: fixed print of non ASCII chars at the terminal.

* Mon Apr 27 2020 TheVice <TheVice> - 2020.04-1
- Added support of UTF8 encoding at the string unit.
- Added new tasks 'attrib', 'call', 'choose', 'copy',
    'delete', 'description', 'fail', 'foreach', 'if',
    'loadfile', 'mkdir', 'move', 'program', 'property',
    'sleep', 'touch' and 'trycatch' to early exists 'echo' and 'exec'.
- Added hash unit with 'crc32', 'BLAKE2b', 'BLAKE3',
    'SHA3/Keccak' algorithms. Algorithms 'crc32' and 'BLAKE2b'
    also available for 'file::get-checksum' script function.
- Tag elements 'project' and 'target' internally also marked as tasks.

* Tue Oct 22 2019 TheVice <TheVice> - 2019.10.21-1
- Available tasks 'echo', 'exec' and 'property'.
- Available functions from name spaces 'bool', 'cygpath, datetime',
    'double', 'environment', 'int', 'int64', 'long', 'math',
    'operating-system', 'path', 'platform', 'program', 'project',
    'property, 'string', 'timespan' and 'version'.
