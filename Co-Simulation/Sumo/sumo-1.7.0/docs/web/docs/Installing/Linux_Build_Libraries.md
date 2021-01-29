---
title: Installing/Linux Build Libraries
permalink: /Installing/Linux_Build_Libraries/
---

This document contains instructions for two basic ways of installing
tools and libraries.

- Either install them manually for the current user (no root
  privileges required)
- Or let your distribution install them system-wide (root privileges
  required)

You can follow either way.

### Manual installation

This document describes how to build and install (higher level)
libraries SUMO depends on from scratch under Linux. There is no need for
package management (RPM or such) or root access. All you need is a
working GCC (version 3.3 and up should suffice) and probably some basic
libraries Fox depends on.

It has been tested successfully on several SuSE Linux installations and
with a recent [cygwin](http://www.cygwin.org). Everything that needs
extra work on cygwin is described in the [cygwin
section](#cygwin).

- First check whether your Linux distribution comes with a Xerces-C
  package and install it together with the header files (for SuSE
  users this is the -devel package). Version 3.1 is known to work with
  the current repository version. You should also check whether Proj,
  GDAL and Fox already come with your distribution and (if so) install
  them together with the header files (devel packages). Every package
  installed this way you will not need to build by yourself.
- Second download the sources of Proj, GDAL, Fox (if you want a GUI),
  and Xerces-C (if it does not come with your distribution).
- If you are building a fresh repository checkout and don't need
  precise geocoordinates you can leave out Proj and GDAL.

#### The tools

We use the GNU autotools for generating configure and Makefiles for
Unix-like environments. Documentation can be found in the

- [GNU autoconf pages](http://www.gnu.org/software/autoconf/),
- [Manual for autoconf](http://www.gnu.org/software/autoconf/manual/),
- [Manual for
  automake](https://www.gnu.org/software/automake/),
- [Manual for
  libtool](https://www.gnu.org/software/libtool/manual/libtool.html), and
- the
  [Autobook](http://sources.redhat.com/autobook/autobook/autobook_toc.html).

#### The Libraries

- Build and install the libraries (if you don't have root access
  choose a different target for the libs and includes such as $HOME).
  For GDAL, Fox and Proj this is more or less straightforward:

```
tar xzf fox-1.6.36.tar.gz
cd fox-1.6.36
./configure --prefix=$HOME && make install
cd ..
tar xzf gdal-1.5.1.tar.gz
cd gdal-1.5.1
./configure --prefix=$HOME && make install
cd ..
tar xzf proj-4.6.0.tar.gz
cd proj-4.6.0
./configure --prefix=$HOME && make install
```

- Note: The "make install" for GDAL may fail due to the Python
  bindings which it wants to install into some only-root-writable
  directory. You can safely ignore this.
- It is strongly recommended to use a user writable installation
  directory and do "make install" instead of trying to specify
  directories inside the directory trees of GDAL, Proj and Fox when
  building SUMO later on.
- Note: It seems like some distributions of Fox are built with
  disabled openGl-support. If you get unresolved references to methods
  such as "glColor...", "glVertex3f...", etc. during compilation of
  [sumo-gui](../sumo-gui.md) you have to enable openGL-support
  before compiling the FOX-library using "./configure
  --with-opengl=yes --prefix=$HOME && make install"; Still, this is
  the default normally.

<!-- end list -->

- In contrast to the other libraries, it is very likely that Xerces-C
  also comes with your Linux distribution (at least it does with a
  recent SuSE). If so, please first try to install it from there and
  do not forget to install the developer package as well.
- The [Xerces
  build](http://xerces.apache.org/xerces-c/build-3.html) is
  somewhat more involved:

```
tar xzf xerces-c-current.tar.gz
export XERCESCROOT=${HOME}/xerces-c-src_3_0_1
cd $XERCESCROOT/src/xercesc
autoconf
./runConfigure -plinux -cgcc -xg++ -minmem -nsocket -tnative -rpthread -P$HOME
make
make install
```

#### Concluding Remarks

After installing all of the required libraries and tools in this
fashion, note that, when building SUMO, the following parameters will be
required for the "./configure" run:

```
./configure --with-fox-config=$HOME/bin/fox-config --with-proj-gdal=$HOME --with-xerces=$HOME
```

### Distribution-specific instructions

If you want to build and optionally install SUMO on Ubuntu systems, you
may want to follow the instructions below.

#### Ubuntu 12.04 (tested with SUMO 0.22.0)

```
sudo apt-get install autoconf
sudo apt-get install proj  libtool libgdal1-dev libxerces-c2-dev libfox-1.6-0 libfox-1.6-dev
cd /usr/lib; sudo ln -s libgdal1.7.0.so libgdal.so cd /usr/local/src/sumo-0.22.0
tar xzf sumo-src-0.22.0.tar.gz
sudo mv -v sumo-0.22.0 /usr/local/src
cd /usr/local/src/sumo-0.22.0
./configure --with-fox-includes=/usr/include/fox-1.6 --with-gdal-includes=/usr/include/gdal --with-proj-libraries=/usr --with-gdal-libraries=/usr --with-proj-gdal
make
cd bin
sudo mv activitygen emissionsDrivingCycle netconvert polyconvert TraCITestClient dfrouter emissionsMap netgenerate  sumo duarouter jtrrouter od2trips sumo-gui /usr/local/bin
```

#### Ubuntu 14.04 "Trusty Tahr" (tested with SUMO 0.22.0)

```
sudo apt-get install autoconf
sudo apt-get install libproj-dev proj-bin proj-data  libtool libgdal1-dev libxerces-c3-dev libfox-1.6-0 libfox-1.6-dev
cd /usr/lib;
```

create this link ...if it complains that it exists, even better

```
sudo ln -s libgdal1.7.0.so libgdal.so
sudo ln -s libproj.so.0.7.0 proj-lib.so
```

```
cd sumo-0.22.0
tar xzf sumo-src-0.22.0.tar.gz
open file "configure" and  change: am__api_version='1.13' to am__api_version='1.14'
cd ..
sudo mv -v sumo-0.22.0 /usr/local/src
cd /usr/local/src/sumo-0.22.0
sudo aclocal
sudo automake --add-missing
sudo ./configure --with-fox-includes=/usr/include/fox-1.6 --with-gdal-includes=/usr/include/gdal --with-proj-libraries=/usr/lib --with-gdal-libraries=/usr --with-proj-gdal
sudo make
cd bin
sudo mv activitygen emissionsDrivingCycle netconvert polyconvert TraCITestClient dfrouter emissionsMap netgenerate  sumo duarouter jtrrouter od2trips sumo-gui /usr/local/bin
cd /usr/local/share
sudo mkdir sumo-0.22.0
sudo mv /usr/local/src/sumo-0.22.0/tools /usr/local/share/sumo-0.22.0/
sudo mv  /usr/local/src/sumo-0.22.0/data /usr/local/share/sumo-0.22.0/
```

if exist...

```
sudo mv  /usr/local/src/sumo-0.22.0/doc /usr/local/share/sumo-0.22.0/
```

#### Ubuntu 18.04 (tested with SUMO 0.32.0)

The required libraries are libxerces-c-dev, libgdal-dev, libproj-dev and
libfox-1.6-dev.

#### Cygwin

Cygwin comes with a quite old xerces-c (version 2.5). After installing
it you need to make a symbolic link in /usr/lib such that the linker
finds the correct library:

```
cd /usr/lib; ln -s /usr/lib/libxerces-c25.dll.a /usr/lib/libxerces-c.dll.a
```

Now everything (but the GUI) should build fine.