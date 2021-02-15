---
title: Installing/macOS Build
permalink: /Installing/MacOS_Build/
---

This document describes how to install and build SUMO on macOS from its source code. If you don't want to **extend** SUMO, but just **use** it, you may want to simply follow the [installation instructions for macOS](../Installing.md#macos) instead.

You may use one of two ways to build and install SUMO on macOS: **Homebrew** (recommended) and **MacPorts**.

# The Homebrew Approach

## Prerequisites

The installation requires [Homebrew](http://brew.sh). If you did not already install homebrew, do so by invoking
```
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```
and make sure your homebrew db is up-to-date.
```
brew update
```

In order to compile the C++ source code files of SUMO, a C++ compiler is needed. On macOS the default C/C++ compiler is Clang. If you want to install the Clang compilers, please use the following command:
```
xcode-select --install
```
After the successful installation, you can test Clang with the following command:
```
clang --version
```

SUMO uses [CMake](https://cmake.org/) to manage the software compilation process. You can install CMake with homebrew easily.
```
brew install cmake
```

## Dependencies
In order to compile and execute SUMO, there are several libraries that need to be installed. You can install these dependencies with homebrew with the following commands:
```
brew cask install xquartz
brew install xerces-c fox proj gdal gl2ps
```
Depending on the SUMO features you want to enable during compilation, you may want to additional libraries. Most libraries are available in homebrew and should be recognized with CMake.

## Git Cloning and Building
The source code of SUMO can be cloned with the following command to the directory `./sumo`. The environment variable `SUMO_HOME` should also be set to this directory.
```
git clone --recursive https://github.com/eclipse/sumo
export SUMO_HOME="$PWD/sumo"
```
SUMO is usually build as an out-of-source build. You need to create a directory for your build and invoke CMake to trigger the configuration from there. 
```
cd $SUMO_HOME
mkdir build/cmake-build
cd build/cmake-build
cmake ../..
```
The output of the CMake configuration process will show you which libraries have been found on your system and which SUMO features have been enabled accordingly.
The build process can now be triggered with the following command
```
cd $SUMO_HOME/build/cmake-build
cmake --build . --parallel $(sysctl -n hw.ncpu)
```
## Optional Steps

### TraCI as a Service (TraaS) 
TraaS is a java library for working with TraCI. Building TraaS can be triggered with the following commands.
```
cd $SUMO_HOME/build/cmake-build
cmake --build . --target traas --parallel
```

### Examples and Unit Tests
SUMO provides unit tests to be used with Google's Testing and Mocking Framework - Googletest. In order to execute these tests you need to install Googletest first.
```
git clone https://github.com/google/googletest
cd googletest
git checkout release-1.10.0
mkdir build
cd build
cmake ..
make
make install
```

The creation of the examples and the execution of the tests can be triggered as follows
```
cd $SUMO_HOME/build/cmake-build
make CTEST_OUTPUT_ON_FAILURE=1 examples test
```

More information is provided [here](../Developer/Unit_Tests.md).

### Integration Tests with TextTest
SUMO uses an application called TextTest to manage and execute and extensive set of integration tests. If you plan to extend SUMO with new features, we would like to encourage you to also add tests for your code to the SUMO testsuite and to make sure that existing functionality is not affected. 

The installation of TextTest on macOS is documented [here](../Developer/Tests.md).

### Code Editor
Finally, you may also want to use a code editor or integrated development environment. There is a great variety of suitable tools available. If you are unsure which tool to pick, we would suggest to have a look at [Visual Studio Code](https://code.visualstudio.com/) for macOS. The configuration of Visual Studio Code for the CMake setup is documented [here](../Developer/VisualStudioCode.md).

# The Macports Approach (legacy)

!!! note
    This uses a pre-packaged version of sumo which is convenient but may lag behind the latest official release of SUMO.

You should start by [installing Macports](https://www.macports.org/install.php). Afterwards start a terminal session and run

```
sudo port install sumo
```

While this will install a SUMO version you maybe do not want to use, it will pull in all dependencies you need.

If you want to build from a repository checkout you should additionally do

```
sudo port install automake autoconf
```

After obtaining the [required libraries](../Installing/Linux_Build.md#installing_required_tools_and_libraries)
you can follow the build steps of [building under Linux](../Installing/Linux_Build.md#getting_the_source_code), you
might want to add another --prefix=/opt/sumo to the configure line.

If you wish to use clang rather than gcc for compilation do:

```
./configure CXX=clang++ CXXFLAGS="-stdlib=libstdc++"
```
