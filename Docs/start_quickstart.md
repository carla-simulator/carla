# Quick start package installation

This guide demonstrates how to download and install the packaged version of CARLA. The package includes the CARLA server and the CARLA Python client library.

* __[Before you begin](#before-you-begin)__  
* __[CARLA installation](#carla-installation)__  
    * [Download a CARLA package](#download-and-extract-a-carla-package)
    * [Additional assets](#additional-assets)
    * [Install the Python client library](#install-the-python-client-library)
* __[Running CARLA](#running-carla)__ 
    * [Launch the CARLA server](#launch-the-carla-server)
    * [Run a Python client example](#run-a-python-client-example-script) 
* __[Next steps](#next-steps)__ 

The packaged version of CARLA allows you to get started quickly with the CARLA simulator. If you wish to modify CARLA's engine code or create custom maps or assets for CARLA, you should [build CARLA from source](build_carla.md) following the instructions appropriate for your operating system.

---
## Before you begin

The following requirements should be fulfilled before installing CARLA:

* __Operating System__: CARLA is built for Windows **10** and **11** and Ubuntu **20.04** and **22.04**. 
* __An adequate GPU__ :We recommend a dedicated GPU equivalent to an NVIDIA 2070 or better with at least 8Gb of VRAM or more.
* __Disk space__: CARLA will use about 20 GB of disk space.
* __Two TCP ports__: Ports 2000 and 2001 by default. Make sure that these ports are not blocked by firewalls or any other applications.
* __Python__: The CARLA API uses [Python]((https://www.python.org/downloads/)) for scripting. CARLA supports Python 3.7 to 3.12 on Ubuntu, and Python 3.7 to 3.12 on Windows.
* __PIP__: Python package manager version 20.3 or higher. 

Check your __PIP__ version:

```sh
python3 -m pip -V
```
If you need to upgrade:

```sh
# For Python 3
python3 -m pip install --upgrade pip
```

---
## CARLA installation

### Download and extract a CARLA package

Download the desired CARLA package from GitHub using the link provided below. We recommend downloading the Package for the latest release, which is currently [0.9.15](https://github.com/carla-simulator/carla/releases/tag/0.9.15/).

<div class="build-buttons">
<p>
<a href="https://carla.readthedocs.io/en/latest/download/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
<span class="icon icon-github"></span> Download CARLA packages</a>
</p>
</div>

This repository contains multiple versions of CARLA. You will find options to download the __current release__ with all the most recent fixes and features, __previous releases__ and a __nightly build__ with all the developmental fixes and features (the nightly build may be unstable, use it with caution).

The package is a compressed file named with the version number. Download and extract the archive in a convenient location (e.g. the `home` or `Documents` directory).

#### Additional assets

Each CARLA package includes pre-installed maps that you can use out-of-the-box. Some additional maps can be downloaded for each version of CARLA, please follow [these instructions](ext_quickstart.md#import-additional-assets) to download and install the additional maps.

### Install the Python client library

Install the Python client library from [PyPi](https://pypi.org/project/carla/) using PIP for an officially released version:

```sh
python3 -m pip install carla # carla==0.9.14 <= For a specific version
```

CARLA packages also have Python wheels included inside `PythonAPI/carla/dist`. **If you are using a custom package built from source or the CARLA-latest package, you need to install the wheels included in the package, not the wheels from PyPi**. From inside this directory, install the relevant wheel with PIP:

```sh
cd PythonAPI/carla/dist
python3 -m pip install carla-0.9.15-cp37m-manylinux_2_27_x86_64.whl # For CARLA v0.9.15 with Python 3.7
```

The Python client example scripts rely on some pre-requisite packages, install these using PIP in the `PythonAPI/examples` directory:

```sh
cd PythonAPI/examples
python3 -m pip install -r requirements.txt
```

---
## Running CARLA

### Launch the CARLA server

Start the CARLA server with the launch script in the top level of the package:

- Ubuntu:

```sh
cd path/to/carla/root

./CarlaUE4.sh
```

- Windows package:

```sh
cd path/to/carla/root

CarlaUE4.exe
```

A window containing a view over the default city map will open. This is the _spectator view_ of the CARLA server that allows you to inspect your simulation. To fly around the city in the spectator view use the mouse and `WASD` keys, holding down the right mouse button to gimbal the view direction. 

![spectator_view](/img/build_install/town10_spectator_view.png)

Additional arguments can be added on the command line for special modes or configuration, please see the [command-line options documentation](ext_quickstart.md#command-line-options) for details. There is also an auxiliary Python config script to launch the 

### Run a Python client example script

Now that Python API client library is installed and the CARLA server is running, you can try the example scripts and start writing your own. Navigate to the `PythonAPI/examples` directory, open 2 command-line terminals then run the `generate_traffic.py` script followed by `manual_control.py`.

```sh
# Run generate Terminal A 
cd PythonAPI\examples
python3 generate_traffic.py  

# Terminal B
cd PythonAPI\examples
python3 manual_control.py 
```

A new Pygame window will open allowing you to control a vehicle in the CARLA simulator with the arrow keys on the keyboard. When writing your own scripts for CARLA, you should import the CARLA Python API, connect to the server (which needs to be launched first) and access the simulation [`world` Object](python_api.md#carlaworld) with the following code:

```py
import carla

client = carla.Client()
world = client.get_world()
```

---

## Next steps

By now you should have a packaged version of CARLA up and running. If you came across any difficulties during the installation process, please post your issues in the [CARLA forum](https://github.com/carla-simulator/carla/discussions/) or in the [Discord](https://discord.gg/8kqACuC) channel.

The next step is to start creating your own simulations with Python scripts using the CARLA Python API:

* Take a look at the [__First Steps tutorial__](tuto_first_steps.md)
* Learn the [core CARLA concepts](core_concepts.md):
    * [Foundations](foundations.md)
    * [Actors](core_actors.md)
    * [Maps](core_map.md)
    * [Sensors and data](core_sensors.md)
    * [Traffic](ts_traffic_simulation_overview.md)
* Browse the Python API with the [API reference](python_api.md)

---
