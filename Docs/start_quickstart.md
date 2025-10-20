# Quick start package installation

This guide shows how to download and install the packaged version of CARLA. The package includes the CARLA server and two options for the client library. There are additional assets that can be downloaded and imported into the package. Advanced customization and development options that require use of the Unreal Engine editor are not available but these can be accessed by using the build version of CARLA for either [Windows](build_windows_ue5.md) or [Linux](build_linux_ue5.md).

* __[Before you begin](#before-you-begin)__  
* __[CARLA installation](#carla-installation)__  
* __[Install client library](#install-client-library)__
* __[Running CARLA](#running-carla)__    
* __[Running CARLA with a Docker container](#running-carla-using-a-docker-container)__   
* __[Follow-up](#follow-up)__ 
---
## Before you begin

The following requirements should be fulfilled before installing CARLA:

* __System requirements__: CARLA is built for Windows and Linux systems.
* __Operating system__: CARLA Unreal Engine 5 version requires a minimum of **Ubuntu 22.04** or **Windows 11**.
* __An adequate GPU__: CARLA aims for realistic simulations that require a lot of compute power. We recommend at minimum an NVIDIA RTX 3000 series or better with at least **16 Gb of VRAM**. A dedicated GPU, separate from the GPU used for CARLA, is highly recommended to handle large machine learning workloads. 
* __GPU drivers__: CARLA Unreal Engine 5 version requires NVIDIA RTX driver release **550 or later for Ubuntu** and NVIDIA RTX driver release **560 or later for Windows**
* __Disk space__: CARLA requires **130 GB** of hard disk (or SSD) space.
* __Python__: [Python]((https://www.python.org/downloads/)) is the main scripting language in CARLA. CARLA supports Python 3 on Linux and Windows.
* __Pip__: The CARLA Python client requires __pip3__ version 20.3 or higher for installation. To check your __pip__ version:

>>      # For Python 3
>>      pip3 -V

>If you need to upgrade:

>>      # For Python 3
>>      pip3 install --upgrade pip

* __Two TCP ports and a good internet connection.__ 2000 and 2001 by default. Make sure that these ports are not blocked by firewalls or any other applications. 
* __Other requirements.__  CARLA requires some Python dependencies. Install the dependencies according to your operating system:

### Windows

```sh
pip3 install --user pygame numpy
```

### Linux

```sh
pip3 install --user pygame numpy
```

---
## CARLA package installation

Download the desired CARLA package from GitHub:

- [Download the package from GitHub here](https://github.com/carla-simulator/carla/releases/tag/0.10.0)

This repository contains different versions of CARLA. You will find options to download the __current release__ with all the most recent fixes and features, __previous releases__ and a __nightly build__ with all the developmental fixes and features (the nightly build is the most unstable version of CARLA).

The package is a compressed file named __CARLA_version.number__. Download and extract the release file. It contains a precompiled version of the simulator, the Python API module and some scripts to be used as examples. 

!!! note
    The documentation you are currently viewing is for the Unreal Engine 5 versions of CARLA. If you are intending to use the Unreal Engine 4 version of CARLA, you should refer the the [correct documentation](https://carla.readthedocs.io/en/latest/).

## Install client library

Install the CARLA client library from the wheel provided with the package:

```sh
cd CARLA_ROOT/PythonAPI/dist/

pip3 install carla-*.*.*-cp3**-linux_x86_64.whl
```

The characters denoted by wildcards depend upon the version of the simulator you are using and your Python version.

__It is recommended to install the CARLA client library in a virtual environment to avoid conflicts when working with multiple versions.__

You may also wish to install the wheel directly from the package, to ensure version consistency. Locate the CARLA Python wheel in `PythonAPI/dist` and install it with PIP.

---
## Running CARLA

Linux:

```sh
cd path/to/carla/root
./CarlaUnreal.sh
```
 
Windows:

```sh
cd path/to/carla/root
CarlaUnreal.exe
```

A window containing a view over Town 10 will pop up. This is the _spectator view_. To fly around the city use the mouse and `WASD` keys, holding down the right mouse button to control the direction. 

![town_10_default](../img/catalogue/maps/town10/town10default.png)

This window is driven by the simulator server which is now running and waiting for a client to connect and interact with the world. You can try some of the example scripts to create traffic in the city and drive a vehicle around the map. Open up two separate terminals and run the following commands:

```sh
# Terminal A 
cd PythonAPI\examples

pip3 install -r requirements.txt

python3 generate_traffic.py  

# Terminal B
cd PythonAPI\examples

python3 manual_control.py 
```

## Running CARLA using a Docker container

For Linux users, CARLA is available in a Docker image, which you may use if you need to run it on the cloud or a cluster or if your machine does not meet the software or operating system prerequisites. 

Firstly, install Docker by following the [installation instructions](https://docs.docker.com/engine/install/ubuntu/). You may want to follow the [post installation instructions](https://docs.docker.com/engine/install/linux-postinstall/) to avoid the need for `sudo` commands. 

Next, install the NVIDIA container toolkit using the [installation instructions](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html#installing-with-apt) and then follow the [configuration instructions](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html#configuring-docker).

Then you may run CARLA in the Docker container using the following commands:

**Running the Docker image without display**:

```sh
docker run \
    --runtime=nvidia \
    --net=host \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    carlasim/carla:0.10.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
```

---

**Running the Docker image with a display**:

To run the Docker image with a display, you will need the `x11` display protocol:

```sh
docker run \
    --runtime=nvidia \
    --net=host \
    --user=$(id -u):$(id -g) \
    --env=DISPLAY=$DISPLAY \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    carlasim/carla:0.10.0 bash CarlaUnreal.sh -nosound
```

---

## Follow-up

By now you should have a packaged version of CARLA up and running. If you came across any difficulties during the installation process, feel free to post in the [CARLA forum](https://github.com/carla-simulator/carla/discussions/) or in the [Discord](https://discord.gg/8kqACuC) channel.

The next step is to learn more about the core concepts in CARLA. Start with the [__First steps__](tuto_first_steps.md) tutorial to start learning and then refer to the [Foundations](foundations.md), [Actors](core_actors.md), [Maps](core_map.md) and [Sensors](core_sensors.md) sections to form a deeper understanding of CARLA's functionality. You should also familiarize yourself with the Python API classes and methods in the [Python API reference](python_api.md).
