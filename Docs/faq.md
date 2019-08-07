<h1>CARLA F.A.Q.</h1>

For more issues related to F.A.Q. see [question list](https://github.com/carla-simulator/carla/issues?utf8=%E2%9C%93&q=label%3Aquestion+).

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    What is the recommended hardware to run CARLA?
    </h5></summary>

  CARLA is a very performance demanding software, at the very minimum you would
  need a computer with a dedicated GPU capable of running Unreal Engine. See
  [Unreal Engine's recommended hardware](https://wiki.unrealengine.com/Recommended_Hardware).

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    What is the expected disk space needed for building CARLA?
    </h5></summary>

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
    </h5></summary>

  There is no "CarlaUE4.sh" script in the source version of CARLA, you need to
  follow the instructions in the [documentation](http://carla.readthedocs.io) for
  building CARLA from source.

  Once you open the project in the Unreal Editor, you can hit Play to test CARLA.

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Can I connect to the simulator while running within Unreal Editor?
    </h5></summary>

  Yes, you can connect a Python client to a simulator running within Unreal
  Editor. Press the "Play" button and wait until the scene is loaded, at that
  point you can connect as you would with the standalone simulator.

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    How can I create a binary version of CARLA?
    </h5></summary>

  In Linux, the recommended way is to run `make package` in the project folder.
  This method makes a packaged version of the project, including the Python API
  modules. This is the method we use to make a release of CARLA for Linux.

  Alternatively, it is possible to compile a binary version of CARLA within Unreal
  Editor, open the CarlaUE4 project, go to the menu "File -> Package Project", and
  select your platform. This takes a while, but in the end it should generate a
  packaged version of CARLA to execute without Unreal Editor.

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Why do I have very low FPS when running the server in Unreal Editor?
    </h5></summary>

  UE4 Editor goes to a low performance mode when out of focus. It can be disabled
  in the editor preferences. Go to "Edit->Editor Preferences->Performance" and
  disable the "Use Less CPU When in Background" option.

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Fatal error: 'version.h' has been modified since the precompiled header.
    </h5></summary>

  This happens from time to time due to Linux updates, and for that we have a
  special target in our Makefile

      $ make hard-clean
      $ make CarlaUE4Editor

  It takes a long time but fixes the issue.

  </details>
