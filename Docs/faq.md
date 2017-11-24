CARLA F.A.Q.
============

#### What is the expected disk space needed for building CARLA?

Building CARLA from source requires about 15GB of disk space, not counting
Unreal Engine installation.

However, you will also need to build and install Unreal Engine, which on Linux
requires much more disk space as it keeps all the intermediate files,
[see this thread](https://answers.unrealengine.com/questions/430541/linux-engine-size.html).

#### Is it possible to dump images from the CARLA server view?

Yes, this is an Unreal Engine feature. You can dump the images of the server
camera by running CARLA with

    $ ./CarlaUE4.sh -benchmark -fps=30 -dumpmovie

Images are saved to "CarlaUE4/Saved/Screenshots/LinuxNoEditor".
