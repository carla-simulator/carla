CARLA Server
============

Protocol
--------

All the messages are prepended by a 32 bits unsigned integer (assumed little-
endian) indicating the size of the coming message.

Three consecutive ports are used,

  * world-port (default 2000)
  * stream-port = world-port + 1
  * control-port = world-port + 2

###### World thread

Server reads one, writes one. Always protobuf messages.

    [client] RequestNewEpisode
    [server] SceneDescription
    [client] EpisodeStart
    [server] EpisodeReady
    ...repeat...

###### Stream thread

Server only writes, first measurements then the raw images.

    [server] Measurements
    [server] raw images
    ...repeat...

Every image is an array of uint32's

    [width, height, type, color[0], color[1],...]

where each color is an [FColor](fcolorlink) (BGRA) as stored in Unreal Engine.

[fcolorlink]: https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Math/FColor/index.html

###### Control thread

Server only reads, client sends Control every frame.

    [client] Control
    ...repeat...

In the synchronous mode, the server waits each frame for a Control message.
