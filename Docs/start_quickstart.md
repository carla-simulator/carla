# Quick start package installation

This guide shows how to download and install the packaged version of CARLA. The package includes the CARLA server and the Python client library. There are [additional assets](content_authoring_packs.md) that can be downloaded and imported into the package. Advanced customization and development options that require use of the Unreal Engine editor are not available but these can be accessed by using the build version of CARLA for either [Windows](build_windows_ue5.md) or [Linux](build_linux_ue5.md).

- [__Before you begin__](#before-you-begin)
- [__CARLA package installation__](#carla-package-installation)
- [__Install client library__](#install-client-library)
- [__Running CARLA__](#running-carla)
- [__Running CARLA using a Docker container__](#running-carla-using-a-docker-container)
- [__Follow-up__](#follow-up)

---
## Before you begin

The following requirements should be fulfilled before installing CARLA:

* __System requirements__: CARLA is built for Windows and Linux systems.
* __Operating system__: CARLA Unreal Engine 5 version requires **Ubuntu 22.04/24.04** or **Windows 11**.
* __An adequate GPU__: CARLA aims for realistic simulations that require a lot of compute power. We recommend at minimum an NVIDIA RTX 3000 series or better with at least **16 Gb of VRAM**. NVIDIA RTX 40 and 50 series GPUs are also supported. A dedicated GPU, separate from the GPU used for CARLA, is highly recommended to handle large machine learning workloads. To run the simulation without a GPU, for example for planning, start the server with `-nullrhi`.
* __GPU drivers__: CARLA Unreal Engine 5 version requires NVIDIA RTX driver release **550 or later for Ubuntu** and NVIDIA RTX driver release **560 or later for Windows**. For NVIDIA RTX 50 series (Blackwell) GPUs, driver release **570 or later** is required.
* __Disk space__: CARLA requires **130 GB** of hard disk (or SSD) space.
* __Python__: [Python](https://www.python.org/downloads/) is the main scripting language in CARLA. CARLA supports Python 3 on Linux and Windows. It is recommended to work in a [virtual environment](https://docs.python.org/3/library/venv.html) to avoid conflicts when working with multiple versions of CARLA. Create and activate one before installing anything:

```sh
python3 -m venv carla-env
source carla-env/bin/activate    # On Windows: carla-env\Scripts\activate
```

* __Pip__: The CARLA Python client requires __pip3__ version 20.3 or higher for installation. To check your __pip__ version, and upgrade it if needed:

```sh
pip3 -V
pip3 install --upgrade pip
```

* __Three TCP ports and a good internet connection.__ 2000, 2001 and 2002 by default. Make sure that these ports are not blocked by firewalls or any other applications.
* __Other requirements.__ On Windows, CARLA requires the **Microsoft Visual C++ Redistributable**. Without it, the simulator will fail to launch without any error message. Download and install it from the [Microsoft website](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist). Choose the **x64** version under "Visual Studio 2015, 2017, 2019, and 2022".

---
## CARLA package installation

Download the desired CARLA package from GitHub:

- Download the package from GitHub here __(TODO: add the link to the 1.0.0 release once it is published)__

This repository contains different versions of CARLA. You will find options to download the __current release__ with all the most recent fixes and features, __previous releases__ and a __nightly build__ with all the developmental fixes and features (the nightly build is the most unstable version of CARLA).

The package is a compressed file named __CARLA_version.number__. Download and extract the release file. It contains a precompiled version of the simulator, the Python API module and some scripts to be used as examples.

!!! note
    The documentation you are currently viewing is for the Unreal Engine 5 versions of CARLA. If you are intending to use the Unreal Engine 4 version of CARLA, you should refer to the [correct documentation](https://carla.readthedocs.io/en/latest/).

---
## Install client library

Install the CARLA client library from the wheel provided with the package. The client and the server must be the same version, and the wheel in the package always matches its server.

### Linux

```sh
cd CARLA_ROOT/PythonAPI/carla/dist/
pip3 install carla-*.whl
```

### Windows

```sh
cd CARLA_ROOT\PythonAPI\carla\dist\
pip3 install carla-<version>-cp310-cp310-win_amd64.whl
```

Only one wheel ships with a package, built for one Python minor version and one platform, for example `carla-<version>-cp310-cp310-linux_x86_64.whl` for Python 3.10 on Linux. Installing it on another Python version fails, so use the interpreter the wheel was built for.

!!! Warning
    Do not install the client with `pip3 install carla`. The `carla` package on PyPI is the client for the Unreal Engine 4 version of CARLA and cannot connect to this server.

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

![town_10_default](img/catalogue/maps/town10/town10default.webp)

This window is driven by the simulator server which is now running and waiting for a client to connect and interact with the world. You can try some of the example scripts to create traffic in the city and drive a vehicle around the map. Open up two separate terminals and run the following commands:

```sh
# Terminal A: populate the city with traffic, and leave it running
cd PythonAPI/examples
pip3 install -r requirements.txt
python3 generate_traffic.py

# Terminal B: drive a vehicle around
cd PythonAPI/examples
python3 manual_control.py
```

On Windows, use `cd PythonAPI\examples`.

---
## Running CARLA using a Docker container

For Linux users, CARLA is available in a Docker image, which you may use if you need to run it on the cloud or a cluster or if your machine does not meet the software or operating system prerequisites.

Firstly, install Docker by following the [installation instructions](https://docs.docker.com/engine/install/ubuntu/). You may want to follow the [post installation instructions](https://docs.docker.com/engine/install/linux-postinstall/) to avoid the need for `sudo` commands.

Next, install the NVIDIA Container Toolkit using the [installation guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). After installation, configure CDI (Container Device Interface) support:

```sh
sudo nvidia-ctk cdi generate --output=/etc/cdi/nvidia.yaml
sudo systemctl restart docker
```

Then pull the CARLA image __(TODO: confirm the image tag and the `CarlaUnreal.sh` path once the 1.0.0 image is published)__:

```sh
docker pull carlasim/carla:1.0.0
```

### Without a display

```sh
docker run \
    --device=nvidia.com/gpu=all \
    --net=host \
    carlasim/carla:1.0.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
```

### With a display

To run the Docker image with a display, you will need the `x11` display protocol:

```sh
docker run \
    --device=nvidia.com/gpu=all \
    --net=host \
    --user=$(id -u):$(id -g) \
    --env=DISPLAY=$DISPLAY \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    carlasim/carla:1.0.0 bash CarlaUnreal.sh -nosound
```

See [CARLA in Docker](build_docker.md) for the legacy NVIDIA runtime, getting the client library out of the image and running several servers on different GPUs.

---
## Follow-up

By now you should have a packaged version of CARLA up and running. If you came across any difficulties during the installation process, feel free to post in the [GitHub Discussions](https://github.com/carla-simulator/carla/discussions) or in the [Discord](https://discord.gg/8kqACuC) channel.

The next step is to learn more about the core concepts in CARLA. Start with the [__First steps__](start_first_steps.md) tutorial to start learning and then refer to the [Foundations](core_foundations.md), [Actors](core_actors.md), [Maps](core_map.md) and [Sensors](core_sensors.md) sections to form a deeper understanding of CARLA's functionality. You should also familiarize yourself with the Python API classes and methods in the [Python API reference](python_api.md).
