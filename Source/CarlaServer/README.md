CarlaServer
===========

Library for socket communications.

Building
--------

#### Linux

Install boost, protobuf, cmake and ninja.

    $ sudo apt-get install libprotobuf-dev protobuf-compiler libboost-all-dev cmake ninja-build

Requires to manually compile
[TurboJPEG](https://github.com/libjpeg-turbo/libjpeg-turbo).
Follow the instructions
[here](https://github.com/libjpeg-turbo/libjpeg-turbo/blob/master/BUILDING.md),
but at the configure step make sure to use the compile option -fPIC otherwise
Unreal is not able to link

    $ {source_directory}/configure CFLAGS='-fPIC'
    $ make

Most probably libraries are generated under `.lib` folder.

Set the environment variable `TURBOJPEG_LIB_PATH` to a folder containing the
generated `libturbojpeg.a` (e.g., add
`export TURBOJPEG_LIB_PATH=<path-to-the-folder>` to your `~/.bashrc`). This way
we can find the right version of the library from Unreal Build Tool.

Back to CarlaServer folder, run make

    $ make

#### Windows

Note: JPEG compression is not implemented on Windows, the server won't send any
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
