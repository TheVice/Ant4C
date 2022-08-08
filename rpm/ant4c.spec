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

%package     -n %{name}-devel
Summary:        Development files for %{name}
%description -n %{name}-devel
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
%{_bindir}/%{name}
%{_libdir}/libdefault_listener.so
%{_mandir}/man1/%{name}.1.gz
%doc help.html

%files -n %{name}-devel
%license LICENSE
%{_includedir}/%{name}/
%{_libdir}/cmake/%{name}/
%{_libdir}/lib%{name}.a

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
