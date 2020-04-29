C++ Client Example
==================

This example creates an application using CARLA's C++ API to connect and control
the simulator from C++.

Compile and run
---------------

Use the Makefile provided (Linux only), to compile and run the example. Note
that it expects to have a simulator running at port 2000.

```
make run
```

How it works
------------

In order to link our application against LibCarla, we need to compile LibCarla
with the same compiler and configuration we are using with our application. To
do so, we generate a CMake tool-chain file specifying the compiler and flags we
want

```cmake
# Example ToolChain.cmake
set(CMAKE_C_COMPILER /usr/bin/clang-8)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -DNDEBUG" CACHE STRING "" FORCE)
```

We pass this file to CMake when compiling LibCarla.client

```sh
cd /path/to/carla-root-folder

make setup

cd /path/to/build-folder

cmake \
    -G "Ninja" \
    -DCMAKE_BUILD_TYPE=Client \
    -DLIBCARLA_BUILD_RELEASE=ON \
    -DLIBCARLA_BUILD_DEBUG=OFF \
    -DLIBCARLA_BUILD_TEST=OFF \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/ToolChain.cmake \
    -DCMAKE_INSTALL_PREFIX=/path/to/install-folder \
    /path/to/carla-root-folder

ninja
ninja install
```

This will generate the following structure at the provided install path

```
libcarla-install
├── include
│   ├── carla
│   ├── cephes
│   ├── pugixml
|   ├── ...
│   └── system
│       ├── boost
│       ├── recast
│       └── rpc
└── lib
    ├── libcarla_client.a
    ├── librpc.a
    ├── libboost_filesystem.a
    └── ...
```

Our application needs to be linked at minimum against `libcarla_client.a`,
`librpc.a`, `libRecast.a`, and `libDetour*.a`. If we make use of IO
functionality and/or image processing we would need to link against
`boost_filesystem`, `png`, `tiff`, and/or `jpeg`.

For more details take a look at the Makefile provided.
