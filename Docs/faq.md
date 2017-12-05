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

#### I downloaded CARLA source from GitHub, where is the "CarlaUE4.sh" script?

There is no "CarlaUE4.sh" script in the source version of CARLA, you need to
follow the instructions in the [documentation](http://carla.readthedocs.io) on
building CARLA from source.

Once you open the project in the Unreal Editor, you can hit Play to test CARLA.

#### How can I create a binary version of CARLA?

To compile a binary (packaged) version of CARLA, open the CarlaUE4 project with
Unreal Editor, go to the menu “File -> Package Project”, and select your
platform. This takes a while, but in the end it should generate a packaged
version of CARLA to execute without Unreal Editor.
