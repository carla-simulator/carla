<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  What is the recommended hardware to run CARLA?
  </h4></summary>

CARLA is a very performance demanding software, at the very minimum you would
need a computer with a dedicated GPU capable of running Unreal Engine. See
[Unreal Engine's recommended hardware](https://wiki.unrealengine.com/Recommended_Hardware).

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  What is the expected disk space needed for building CARLA?
  </h4></summary>

Building CARLA from source requires about 15GB of disk space, not counting
Unreal Engine installation.

However, you will also need to build and install Unreal Engine, which on Linux
requires much more disk space as it keeps all the intermediate files,
[see this thread](https://answers.unrealengine.com/questions/430541/linux-engine-size.html).

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  I downloaded CARLA source from GitHub, where is the "CarlaUE4.sh" script?
  </h4></summary>

There is no "CarlaUE4.sh" script in the source version of CARLA, you need to
follow the instructions in the [documentation](http://carla.readthedocs.io) for
building CARLA from source.

Once you open the project in the Unreal Editor, you can hit Play to test CARLA.

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  Setup.sh fails to download content, can I skip this step?
  </h4></summary>

It is possible to skip the download step by passing the `-s` argument to the
setup script

    $ ./Setup.sh -s

Bear in mind that if you do so, you are supposed to manually download and
extract the content package yourself, check out the last output of the Setup.sh
for instructions or run

    $ ./Update.sh -s

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  Can I run the server from within Unreal Editor?
  </h4></summary>

Yes, you can connect the Python client to a server running within Unreal Editor
as if it was the standalone server.

Go to **"Unreal/CarlaUE4/Config/CarlaSettings.ini"** (this file should have been
created by the Setup.sh) and enable networking. If for whatever reason you don't
have this file, just create it and add the following

```ini
[CARLA/Server]
UseNetworking=true
```

Now when you hit Play the editor will hang until a client connects.

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  Why Unreal Editor hangs after hitting Play?
  </h4></summary>

This is most probably happening because CARLA is starting in server mode. Check
your **"Unreal/CarlaUE4/Config/CarlaSettings.ini"** and set

```ini
[CARLA/Server]
UseNetworking=false
```

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  How can I create a binary version of CARLA?
  </h4></summary>

To compile a binary (packaged) version of CARLA, open the CarlaUE4 project with
Unreal Editor, go to the menu "File -> Package Project", and select your
platform. This takes a while, but in the end it should generate a packaged
version of CARLA to execute without Unreal Editor.

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  Why do I have very low FPS when running the server in Unreal Editor?
  </h4></summary>

UE4 Editor goes to a low performance mode when out of focus. It can be disabled
in the editor preferences. Go to "Edit->Editor Preferences->Performance" and
disable the "Use Less CPU When in Background" option.

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  Is it possible to dump images from the CARLA server view?
  </h4></summary>

Yes, this is an Unreal Engine feature. You can dump the images of the server
camera by running CARLA with

    $ ./CarlaUE4.sh -benchmark -fps=30 -dumpmovie

Images are saved to "CarlaUE4/Saved/Screenshots/LinuxNoEditor".

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  Fatal error: 'version.h' has been modified since the precompiled header.
  </h4></summary>

This happens from time to time due to Linux updates. It is possible to force a
rebuild of all the project files with

    $ cd Unreal/CarlaUE4/
    $ make CarlaUE4Editor ARGS=-clean
    $ make CarlaUE4Editor

It takes a long time but fixes the issue. Sometimes a reboot is also needed.

</details>

<!-- ======================================================================= -->
<details>
  <summary><h5 style="display:inline">
  Fatal error: 'carla/carla_server.h' file not found.
  </h4></summary>

This indicates that the CarlaServer dependency failed to compile.

Please follow the instructions at
[How to build on Linux](http://carla.readthedocs.io/en/latest/how_to_build_on_linux/).

Make sure that the Setup script does print _"Success!"_ at the end

    $ ./Setup.sh
    ...
    ...
    ****************
    *** Success! ***
    ****************

Then check if CarlaServer compiles without errors running make

    $ make

It should end printing something like

```
[1/1] Install the project...
-- Install configuration: "Release"
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/shared/libc++abi.so.1
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/shared/libc++abi.so.1.0
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/shared/libc++.so.1
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/shared/libc++.so.1.0
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/shared/libc++.so
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/shared/libc++abi.so
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/lib/libc++abi.a
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/lib/libboost_system.a
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/lib/libprotobuf.a
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/include/carla
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/include/carla/carla_server.h
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/lib/libcarlaserver.a
-- Installing: Unreal/CarlaUE4/Plugins/Carla/CarlaServer/bin/test_carlaserver
-- Set runtime path of "Unreal/CarlaUE4/Plugins/Carla/CarlaServer/bin/test_carlaserver" to ""
```

If so you can safely run Rebuild.sh.

</details>
