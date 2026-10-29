# Quick start with the CARLA binary

This guide shows how to download and run the CARLA binary, the precompiled package of each release. The package includes the CARLA server and the Python client library. There are [additional assets](content_authoring_packs.md) that can be downloaded and imported into the package. Advanced customization and development options that require use of the Unreal Engine editor are not available but these can be accessed by using the build version of CARLA for either [Windows](build_windows_ue5.md) or [Linux](build_linux_ue5.md).

!!! note
    This documentation is for CARLA 1.0. The Unreal Engine 4 versions of CARLA (0.9.x) have [their own documentation](https://carla.readthedocs.io/en/latest/).

- [__Before you begin__](#before-you-begin)
- [__Install with your agent__](#install-with-your-agent)
- [__CARLA package installation__](#carla-package-installation)
- [__Install client library__](#install-client-library)
- [__Running CARLA__](#running-carla)
- [__Running CARLA using a Docker container__](#running-carla-using-a-docker-container)
- [__Follow-up__](#follow-up)

---
## Before you begin

The following requirements should be fulfilled before installing CARLA:

* __Operating system__: CARLA is built for Linux and Windows systems. CARLA requires **Ubuntu 22.04/24.04** or **Windows 11**.
* __An adequate GPU__: CARLA aims for realistic simulations that require a lot of compute power. We recommend at minimum an NVIDIA RTX 3000 series or better with at least **16 GB of VRAM**. NVIDIA RTX 40 and 50 series GPUs are also supported. A dedicated GPU, separate from the GPU used for CARLA, is highly recommended to handle large machine learning workloads. To run the simulation without a GPU, for example for planning, start the server with `-nullrhi`.
* __GPU drivers__: CARLA requires NVIDIA RTX driver release **550 or later for Ubuntu** and NVIDIA RTX driver release **560 or later for Windows**. For NVIDIA RTX 50 series (Blackwell) GPUs, driver release **570 or later** is required.
* __Disk space__: CARLA requires **225 GB** of hard disk (or SSD) space.
* __Three TCP ports and a good internet connection.__ 2000, 2001 and 2002 by default. Make sure that these ports are not blocked by firewalls or any other applications.
* __Other requirements.__ On Windows, CARLA requires the **Microsoft Visual C++ Redistributable**. Without it, the simulator will fail to launch without any error message. Download and install it from the [Microsoft website](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist). Choose the **x64** version for Visual Studio 2022.
* __Python__: [Python](https://www.python.org/downloads/) is the main scripting language in CARLA. CARLA supports Python 3 on Linux and Windows. It is recommended to work in a [virtual environment](https://docs.python.org/3/library/venv.html) to avoid conflicts when working with multiple versions of CARLA. Create and activate one before installing anything.

On Linux:

```sh
python3 -m venv carla-env
source carla-env/bin/activate
```

On Windows:

```sh
python -m venv carla-env
carla-env\Scripts\activate
```

* __Pip__: The CARLA Python client requires __pip3__ version 20.3 or higher for installation. To check your __pip__ version, and upgrade it if needed:

```sh
pip3 -V
pip3 install --upgrade pip
```

---
## Install with your agent

The quickest way to install CARLA is to let your AI coding agent do it. With the [CARLA skills connected](agent_skills.md#connect-your-agent), ask it in plain words and it downloads the release, installs the matching Python client, starts the server and checks that a client can reach it, following procedures tested by the CARLA team:

!!! agent "Ask your agent"
    *"Download CARLA 1.0, install its Python client in a virtual environment and start a server on Town10."*

    Skills: `download-carla`, `install-python-api`, `run-carla-ue58-server`. [Connect your agent](agent_skills.md#connect-your-agent).

The same works for everything that comes after: *"spawn heavy traffic and make it rain at dusk"*, *"mount a front camera and a lidar and save their data"*. Throughout this documentation, a box like the one above marks the procedures your agent can run for you.

If you prefer to install CARLA by hand, follow the rest of this page.

---
## CARLA package installation

Download the desired CARLA package from GitHub:

- [CARLA 1.0 on GitHub](https://github.com/carla-simulator/carla/releases/tag/1.0), with the release notes and every download.
- Direct downloads: [Linux](https://downloads.carlasim.com/Linux/Carla-1.0-Linux-Shipping.tar.gz) and [Windows](https://downloads.carlasim.com/Windows/Carla-1.0-Windows-Shipping.zip). The [additional maps package](map_content_packs.md) of each platform is listed on the release page.

This repository contains different versions of CARLA. You will find options to download the __current release__ with all the most recent fixes and features and __previous releases__. A __nightly build__ for Linux with all the developmental fixes and features is available at [downloads.carlasim.com](https://downloads.carlasim.com/Linux/Dev/CARLA_UE5_Latest.tar.gz) (the nightly build is the most unstable version of CARLA).

The package is a compressed file named `Carla-1.0-Linux-Shipping.tar.gz` on Linux and `Carla-1.0-Windows-Shipping.zip` on Windows. Download and extract the release file. It contains a precompiled version of the simulator, the Python API module and some scripts to be used as examples.

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

### Linux

```sh
cd CARLA_ROOT
./CarlaUnreal.sh
```

### Windows

```sh
cd CARLA_ROOT
CarlaUnreal.exe
```

A window containing a view over Town 10 will pop up. This is the _spectator view_. To fly around the city use the mouse and `WASD` keys, holding down the right mouse button to control the direction.

![town_10_default](img/catalogue/maps/town10/town10default.webp)

This window is driven by the simulator server which is now running and waiting for a client to connect and interact with the world. You can try some of the example scripts to create traffic in the city and drive a vehicle around the map. Open up two separate terminals and run the following commands:

__Linux__:

```sh
# Terminal A: populate the city with traffic, and leave it running
cd CARLA_ROOT/PythonAPI/examples
pip3 install -r requirements.txt
python3 generate_traffic.py

# Terminal B: drive a vehicle around
cd CARLA_ROOT/PythonAPI/examples
python3 manual_control.py
```

__Windows__:

```sh
# Terminal A: populate the city with traffic, and leave it running
cd CARLA_ROOT\PythonAPI\examples
pip3 install -r requirements.txt
python3 generate_traffic.py

# Terminal B: drive a vehicle around
cd CARLA_ROOT\PythonAPI\examples
python3 manual_control.py
```

---
## Running CARLA using a Docker container

For Linux users, CARLA is available in a Docker image, which you may use if you need to run it on the cloud or a cluster or if your machine does not meet the software or operating system prerequisites.

Firstly, install Docker by following the [installation instructions](https://docs.docker.com/engine/install/ubuntu/). You may want to follow the [post installation instructions](https://docs.docker.com/engine/install/linux-postinstall/) to avoid the need for `sudo` commands.

Next, install the NVIDIA Container Toolkit using the [installation guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). After installation, configure CDI (Container Device Interface) support:

```sh
sudo nvidia-ctk cdi generate --output=/etc/cdi/nvidia.yaml
sudo systemctl restart docker
```

Then pull the CARLA image, tag `1.0` (see the [CARLA 1.0 release](https://github.com/carla-simulator/carla/releases/tag/1.0)):

```sh
docker pull carlasim/carla:1.0
```

### Without a display

```sh
docker run \
    --device=nvidia.com/gpu=all \
    --net=host \
    carlasim/carla:1.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
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
    carlasim/carla:1.0 bash CarlaUnreal.sh -nosound
```

See [CARLA in Docker](build_docker.md) for the legacy NVIDIA runtime, getting the client library out of the image and running several servers on different GPUs.

---
## Follow-up

By now you should have CARLA up and running. If something did not work, or as you go further, the CARLA community and team are there to help:

* __Found a bug?__ Open an [issue on GitHub](https://github.com/carla-simulator/carla/issues). Search first in case it is already reported, then include your CARLA version, operating system, GPU and the steps or script that reproduce it.
* __Have a question?__ Ask the community and the CARLA team in [GitHub Discussions](https://github.com/carla-simulator/carla/discussions).
* __Want to talk with other users?__ Join the CARLA [Discord](https://discord.gg/8kqACuC), where users and developers share their work and help each other.
* __Want to contribute?__ Pull requests are welcome; the [contribution guidelines](cont_contribution_guidelines.md) explain how.

The next step is to learn more about the core concepts in CARLA. Start with the [__First steps__](start_first_steps.md) tutorial to start learning and then refer to the [Foundations](core_foundations.md), [Actors](core_actors.md), [Maps](core_map.md) and [Sensors](core_sensors.md) sections to form a deeper understanding of CARLA's functionality. You should also familiarize yourself with the Python API classes and methods in the [Python API reference](python_api.md).
