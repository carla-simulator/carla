---
title: Developer/Unit Tests
permalink: /Developer/Unit_Tests/
---

# Introduction

The Unit Tests for SUMO are carried out with the help of the Framework
[Google Test](http://code.google.com/p/googletest/). With *Google Test*
new Unit Tests can be simple added or existing tests extended. All
available tests are executed daily and it is checked whether these tests
run through successfully. This installation guide is for the version
1.4.0 to 1.6.0 of *Google Test*.

# Windows Setup

Please clone the
[SUMOLibraries](https://github.com/DLR-TS/SUMOLibraries) and set the
environment variable `SUMO_LIBRARIES` to the checkout directory. Google
Test should be found and set up automatically then.

# Linux setup

## Ubuntu and the CMake build

For newer Ubuntu and GTest versions follow [this
recipe](https://stackoverflow.com/questions/24295876/cmake-cannot-find-googletest-required-library-in-ubuntu),
which boils down to:

```
sudo apt install libgtest-dev build-essential cmake
cd /usr/src/googletest
sudo cmake .
sudo cmake --build . --target install
```

At least the cmake build from sumo will now pick up Google Test
correctly.

## Legacy

After downloading you have to executed the following commands in the
Google Test directory:

```
./configure
make
sudo make install
```

Now all libraries are installed in the system directory /usr/local or
/usr/bin. Before the Unit Tests can be executed, you must build SUMO
with an additional input parameter:

```
./configure --with-gtest=DIR (DIR=installation directory of GoogleTest)
```

If new Unit Test files are added under *unittest/src*, the Makefiles
must be updated in order to run all tests.