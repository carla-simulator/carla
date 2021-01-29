---
title: Installing/Linux Build
permalink: /Installing/Linux_Build/
---

This document describes how to install SUMO on Linux from sources. If
you don't want to **extend** SUMO, but merely **use** it, you might want
to [download one of our pre-built binary
packages](../Installing.md) instead.

To be able to run SUMO on Linux, just follow these steps:

1.  Install all of the required tools and libraries
2.  Get the source code
3.  Build the SUMO binaries

For ubuntu this boils down to

```
 sudo apt-get install cmake python g++ libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev libgl2ps-dev swig
 git clone --recursive https://github.com/eclipse/sumo
 export SUMO_HOME="$PWD/sumo"
 mkdir sumo/build/cmake-build && cd sumo/build/cmake-build
 cmake ../..
 make -j$(nproc)
```

Each of these steps is described in more detail and with possible
alternatives below.

## Installing required tools and libraries

- For the build infrastructure you will need cmake together with a moderately
  recent g++ (4.8 will do) or clang++ (or any other C++11 enabled compiler).
- The library Xerces-C is always needed. To use
  [sumo-gui](../sumo-gui.md) you also need Fox Toolkit in version
  1.6.x. It is highly recommended to also install Proj to have support
  for geo-conversion and referencing. Another common requirement is
  network import from shapefile (arcgis). This requires the GDAL
  library. To compile you will need the devel versions of all packages.
  For openSUSE this means installing libxerces-c-devel, libproj-devel,
  libgdal-devel, and fox16-devel. There are some [platform specific
  and manual build instructions for the
  libraries](../Installing/Linux_Build_Libraries.md)
- Optionally you may want to add ffmpeg-devel (for video output),
  libOpenSceneGraph-devel (for the experimental 3D GUI) and
  python-devel and swig (for running TraCI pythons scripts without a socket
  connection)

## Getting the source code

For the correct setting of SUMO_HOME you have to remember the correct
path, where you build your SUMO, the SUMO build path. This path is shown
with pwd at the end of getting the source code. If you want to develop
actively on sumo we strongly recommend to use the git repository. Please
build the tarball version if you need for some reason a specific version
of sumo.

### repository checkout (recommended)

The following commands should be issued:

```
git clone --recursive https://github.com/eclipse/sumo
cd sumo
git fetch origin refs/replace/*:refs/replace/*
pwd
```

The additional fetch of the replacements is necessary to get a full
local project history.

### release version or nightly tarball

Download
[sumo-src-{{Version}}.tar.gz](https://sumo.dlr.de/releases/{{Version}}/sumo-src-{{Version}}.tar.gz) or <http://sumo.dlr.de/daily/sumo-src-git.tar.gz>

```
tar xzf sumo-src-<version>.tar.gz
cd sumo-<version>/
pwd
```

## Definition of SUMO_HOME

Before compiling is advisable to
define the environment variable SUMO_HOME. SUMO_HOME must be set to
the SUMO build path from the previous step. Assuming that you placed
SUMO in the folder "*/home/<user\>/sumo-<version\>*", if you want to
define only for the current session, type in the console

```
export SUMO_HOME="/home/<user>/sumo-<version>"
```

If you want to define for all sessions (i.e. for every time that you run
your Linux distribution), go to your HOME folder, and find one of the
next three files (depending of your Linux distribution):
**.bash_profile**, **.bash_login** or **.profile** (Note that these
files can be hidden). Then edit the file, add the line from above at the
end and restart your session.

You can check that SUMO_HOME was successfully set if you type

```
echo $SUMO_HOME
```

and console shows "/home/<user\>/sumo-<version\>"

## Building the SUMO binaries with cmake

To build with cmake version 3 or higher is required.

Create a build folder for cmake (in the sumo root folder)

```
mkdir build/cmake-build
cd build/cmake-build
```

to build sumo with the full set of available options just like GDAL and
OpenSceneGraph support (if the libraries are installed) just run:

```
cmake ../..
```

to build the debug version just use

```
cmake -D CMAKE_BUILD_TYPE=Debug ../..
```

!!! note
    On some platforms the required cmake executable is called *cmake3*.

after this is finished, run

```
make -j $(nproc)
```

The `nproc` command gives you the number of logical cores on your
computer, so that make will start parallel build jobs which makes the
build a lot faster. If `nproc` is not available on your system, insert a
fixed number here or leave the option out. You may also try

```
make -j $(grep -c ^processor /proc/cpuinfo)
```

Other useful cmake options:

- `-D PROFILING=ON` enable profiling instrumentation for gprof (gcc
  build only)
- `-D COVERAGE=ON` enable coverage instrumentation for lcov (gcc build
  only)
- `-D CHECK_OPTIONAL_LIBS=OFF` disable all optional libraries (only
  include EPL compatible licensed code)
- `-D PROJ_LIBRARY=` disable PROJ
- `-D FOX_CONFIG=` disable FOX toolkit (GUI and multithreading)

## Building with clang

If you want to use a different compiler (just for the fun of it or
because it has additional features) you can enable it at configure time.
Our current clang configuration for additional static code checking
enables the following CXXFLAGS:

```
-stdlib=libstdc++ -fsanitize=undefined,address,integer,unsigned-integer-overflow -fno-omit-frame-pointer -fsanitize-blacklist=$SUMO_HOME/build/clang_sanitize_blacklist.txt
```

You may of course leave out all the sanitizer-checks you don't want but
the stdlib option has to be set. The blacklist filters out a known bug
in the cstdlib. For details see the clang documentation.

The current CMake configuration already contains this for the debug
build, so for building with CMake and clang just change to your build
dir and use

```
CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ../..
```

## Installing the SUMO binaries

This (optional) step will copy the SUMO binaries to another path, so
that you can delete all source and intermediate files afterwards. If you
do not want (or need) to do that, you can simply skip this step and run
SUMO from the bin subfolder (bin/sumo-gui and bin/sumo).

If you want to install the SUMO binaries into your system, run
```
sudo make install
```

You have to adjust your SUMO_HOME variable to the install dir (usually
/usr/local/share/sumo)
```
export SUMO_HOME=/usr/local/share/sumo
```

## Uninstalling

CMake provides no `make uninstall` so if you ever want to uninstall, run
```
sudo xargs rm < install_manifest.txt
```
from the same folder you ran `make install`. This will leave some empty
directories, so if you want to remove them as well, double check that
$SUMO_HOME points to the right directory (see above) and run
```
sudo xargs rm -r $SUMO_HOME
```

## Troubleshooting

### Problems with the socket subsystem

Problem:

```
recv ./foreign/tcpip/libtcpip.a(socket.o) (symbol belongs to implicit dependency /usr/lib/libsocket.so.1)
```

Solution:
<http://lists.danga.com/pipermail/memcached/2005-September/001611.html>

### ld cannot find an existing library (Fedora-23)

Problem:

```
/usr/bin/ld: cannot find -lfreetype
ls -lah /usr/lib64/libfreetype*
 lrwxrwxrwx. 1 root root   21 Jul 28 15:54 /usr/lib64/libfreetype.so.6 -> libfreetype.so.6.12.0
 lrwxr-xr-x. 1 root root 689K Jul 28 15:54 /usr/lib64/libfreetype.so.6.12.0
```

Solution: Install the dev package; for fedora:

```
sudo yum install freetype-devel
```

For details see
[stackoverflow](http://stackoverflow.com/questions/335928/ld-cannot-find-an-existing-library)
discussion.

### Additional notes for Cygwin users

At the moment GUI building is still troublesome. It depends whether you
want to use the X-Server or native Windows GUI. We tried native Windows
GUI only and had to change the installed libFOX-1.4.la such that it
contains

```
dependency_libs=' -lgdi32 -lglaux -ldl -lcomctl32 -lwsock32 -lwinspool -lmpr
-lpthread -lpng /usr/lib/libtiff.la /usr/lib/libjpeg.la -lz -lbz2 -lopengl32 -lglu32'
```

Your mileage may vary.
