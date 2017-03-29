CarlaServer
===========

Library for socket communications.

Building
--------

#### Linux

Install boost, protobuf, cmake and ninja.

    $ sudo apt-get install libprotobuf-dev protobuf-compiler libboost-all-dev cmake ninja-build

Run make

    $ make

Requires to manually compile
[TurboJPEG](https://github.com/libjpeg-turbo/libjpeg-turbo).
Follow the instructions
[here](https://github.com/libjpeg-turbo/libjpeg-turbo/blob/master/BUILDING.md).
At the configure step add PIC compile option

    $ {source_directory}/configure CFLAGS='-fPIC'
    $ make

#### Windows

Warning: Outdated

Install and compile [boost](http://www.boost.org/).

Install and compile [protobuf](https://developers.google.com/protocol-buffers/).
While compiling protobuf, use `-Dprotobuf_MSVC_STATIC_RUNTIME=OFF` when calling
CMake in order to use the static runtime library, otherwise may give errors
while trying to link with CarlaServer.

CMake looks at the following environment variables

  * `CMAKE_INCLUDE_PATH` should contain the protobuf include folder.
  * `CMAKE_LIBRARY_PATH` should contain "libprotobuf" "libprotobuf-lite" "liteprotoc" .lib files.
  * `BOOST_ROOT` root of the boost folder.

To generate the Visual Studio solution

    $ make vsproject

The solution gets generated at `./build/CarlaServer.sln`. Change the solution
configuration to match protobuf (release, most probably). Don't forget to build
the project INSTALL, it doesn't build when building the solution.
