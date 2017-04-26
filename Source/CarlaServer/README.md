CarlaServer
===========

Library for socket communications.

Building
--------

#### Linux

Install boost, protobuf, cmake and ninja.

    $ sudo apt-get install libpng16-dev libprotobuf-dev protobuf-compiler libboost-all-dev cmake ninja-build

Run make in this folder

    $ make

#### Windows

Note: PNG compression is not implemented on Windows, the server won't send any
images.

Compile and install [boost](http://www.boost.org/).

Compile [protobuf](https://developers.google.com/protocol-buffers/). While
compiling protobuf, use `-Dprotobuf_MSVC_STATIC_RUNTIME=OFF` when calling CMake
in order to use the static runtime library, otherwise may give errors while
trying to link with CarlaServer. Generate both debug and release and install
under `$PROTOBUF_ROOT/Debug` and `$PROTOBUF_ROOT/Release` respectively.

The Makefile uses cmake to generate project files. Under Windows, it requires
the following environment variables set

  * `BOOST_ROOT` root of the boost folder.
  * `PROTOBUF_ROOT` contains two folder `Debug` and `Release`, each of them with subfolders `lib` and `include`.

Before calling make you need to set up the environment calling `vcvarsall.bat`,
usually located at the Visual Studio installation folder. Call it (don't forget
the amd64 at the end)

    $ "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

Then build either debug or release

    $ make debug

or

    $ make release

To generate the Visual Studio project

    $ make vsproject

The solution gets generated at `./build/visualstudio/CarlaServer.sln`. Change
the solution configuration to match protobuf (release, most probably). Don't
forget to build the project INSTALL, it doesn't build when building the
solution.
