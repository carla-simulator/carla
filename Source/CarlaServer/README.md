CarlaServer
===========

Library for socket communications.

Building
--------

#### Linux

Install boost, protobuf, and ninja.

    $ sudo apt-get install libprotobuf-dev protobuf-compiler libboost-all-dev ninja-build

Run make

    $ make

#### Windows

Install boost and protobuf.

To generate the Visual Studio solution

    $ make vsproject

The solution gets generated at `./build/CarlaServer.sln`.
