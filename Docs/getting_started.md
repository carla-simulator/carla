<h1>Getting started with CARLA</h1>

![Welcome to CARLA](img/welcome.png)

!!! important
    This tutorial refers to the latest development versions of CARLA, 0.9.0 or
    later. For the documentation of the stable version please switch to the
    [stable branch](https://carla.readthedocs.io/en/stable/getting_started/).

Welcome to CARLA! This tutorial provides the basic steps for getting started
using CARLA.

<!-- Latest release button -->
<p align="middle"><a href="https://github.com/carla-simulator/carla/blob/master/Docs/download.md" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release"><span class="icon icon-github"></span> Get the latest release</a></p>

Download the latest release from our GitHub page and extract all the contents of
the package in a folder of your choice.

The release package contains the following

  * The CARLA simulator.
  * The "carla" Python API module.
  * A few Python scripts with usage examples.

The simulator can be started by running `CarlaUE4.sh` on Linux, or
`CarlaUE4.exe` on Windows. Unlike previous versions, now the simulator
automatically starts in "server mode". That is, you can already start connecting
your Python scripts to control the actors in the simulation.

CARLA requires two available TCP ports on your computer, by default 2000 and
2001. Make sure you don't have a firewall or another application blocking those
ports. Alternatively, you can manually change the port CARLA uses by launching
the simulator with the command-line argument `-carla-port=N`, the second port
will be automatically set to `N+1`.

!!! tip
    You can launch the simulator in windowed mode by using the argument
    `-windowed`, and control the window size with `-ResX=N` and `-ResY=N`.

#### Running the example script

Run the example script with

```sh
python example.py
```

If everything went well you should start seeing cars appearing in the scene.

_We strongly recommend taking a look at the example code to understand how it
works, and modify it at will. We'll have soon tutorials for writing your own
scripts, but for now the examples is all we have._

#### Changing the map

By default, the simulator starts up in our _"Town01"_ map. The second map can be
started by passing the path to the map as first argument when launching the
simulator

```sh
# On Linux
$ ./CarlaUE4.sh /Game/Carla/Maps/Town02
```

```cmd
rem On Windows
> CarlaUE4.exe /Game/Carla/Maps/Town02
```
