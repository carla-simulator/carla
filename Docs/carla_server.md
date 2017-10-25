CARLA Server
============

Build
-----

Some scripts are provided for building and testing CarlaServer on Linux

    $ Setup.sh
    $ make
    $ make check

The setup script downloads and compiles all the required dependencies. The
Makefile calls CMake to build CarlaServer and installs it under "Util/Install".

Protocol
--------

All the messages are prepended by a 32 bits unsigned integer (little-endian)
indicating the size of the coming message.

Three consecutive ports are used,

  * world-port (default 2000)
  * measurements-port = world-port + 1
  * control-port = world-port + 2

each of these ports has an associated thread that sends/reads data
asynchronuosly.

###### World thread

Server reads one, writes one. Always protobuf messages.

    [client] RequestNewEpisode
    [server] SceneDescription
    [client] EpisodeStart
    [server] EpisodeReady
    ...repeat...

###### Measurements thread

Server only writes, first measurements message then the bulk of raw images.

    [server] Measurements
    [server] raw images
    ...repeat...

Every image is an array of uint32's

    [width, height, type, color[0], color[1],...]

where each color is an [FColor][fcolorlink] (BGRA) as stored in Unreal Engine,
and the possible types of images are

    type = 0  None                  (RGB without any post-processing)
    type = 1  SceneFinal            (RGB with post-processing present at the scene)
    type = 2  Depth                 (Depth Map)
    type = 3  SemanticSegmentation  (Semantic Segmentation)

[fcolorlink]: https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Math/FColor/index.html "FColor API Documentation"

###### Control thread

Server only reads, client sends Control message every frame.

    [client] Control
    ...repeat...

In the synchronous mode, the server halts execution each frame until the Control
message is received.

C API
-----

The library is encapsulated behind a single include file in C,
["carla/carla_server.h"][carlaserverhlink].

This file contains the basic interface for reading and writing messages to the
client, hiding the networking and multi-threading part. Most of the functions
have a time-out parameter and block  until the corresponding asynchronous
operation is completed or the time-out is met. Set a time-out of 0 to get a
non-blocking call.

A CarlaServer instance is created with `carla_make_server()` and should be
destroyed after use with `carla_server_free(ptr)`.

[carlaserverhlink]: ../Util/CarlaServer/include/carla/carla_server.h

Design
------

The C API takes care of dispatching the request to the corresponding server.
There are three asynchronous servers each of them running on its own thread.

![CarlaServer design](img/carlaserver.svg)

Conceptually there are two servers, the _World Server_ and the _Agent Server_.
The _World Server_ controls the initialization of episodes. A new episode is
started every time it is requested to the World Server by a RequestNewEpisode
message. Once the episode is ready, the World Server launches the Agent Server.
The _Agent Server_ has two threads, one for sending the streaming of the
measurements and another for receiving the control. Both agent threads
communicate with the main thread through a lock-free double-buffer to speed up
the streaming of messages and images.

The encoding of the messages (protobuf) and the networking operations are
executed asynchronously.
