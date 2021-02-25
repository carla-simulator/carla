# Quick start package installation

This guide shows how to download and install the packaged version of CARLA. The package includes the CARLA server and the Python API client library ready to run out of the box and does not require any build process. There are additional assets that can be downloaded and imported into the package. Advanced customization and development options that require use of the Unreal Engine editor are not available but these can be accessed by using the build version of CARLA for either [Windows](build_windows.md) or [Linux](build_linux.md).

* __[Before you begin](#before-you-begin)__  
* __[CARLA installation](#carla-installation)__  
	* [A. Debian CARLA installation](#a-debian-carla-installation)  
	* [B. Binary installation](#b-binary-installation)  
* __[Import additional assets](#import-additional-assets)__  
* __[Running CARLA](#running-carla)__  
	* [Command-line options](#command-line-options)  
* __[Updating CARLA](#updating-carla)__   
* __[Installation summary](#installation-summary)__  
* __[Follow-up](#follow-up)__ 
---
## Before you begin

The following requirements should be fulfilled before installing CARLA:

* __System requirements.__ Any 64-bits OS should run CARLA. However, since release 0.9.9, __CARLA cannot run in 16.04 Linux systems with default compilers__. These should be upgraded to work with CARLA.  
* __An adequate GPU.__ CARLA aims for realistic simulations, so the server needs at least a 6 GB GPU although we would recommend 8 GB. A dedicated GPU is highly recommended for machine learning. 
* __Disk space.__ CARLA will use about 20 GB of space.
* __Python.__ [Python]((https://www.python.org/downloads/)) is the main scripting language in CARLA. CARLA supports both Python 2 and Python 3 although Python 3 is recommended.
* __Two TCP ports and good internet connection.__ 2000 and 2001 by default. Make sure that these ports are not blocked by firewalls or any other applications. 
* __Other requirements.__  Two Python modules: [Pygame](https://pypi.org/project/pygame/) to create graphics directly with Python, and [Numpy](https://pypi.org/project/numpy/) for calculus. Install them using:

```sh
        python3 -m pip install --user pygame numpy  # Support for Python2 is provided in the CARLA release packages
```    
---
## CARLA installation

There are two methods to download and install CARLA as a package:

__A)__ [Download the Debian package.](#a-debian-carla-installation)

__B)__ [Download the binary from GitHub.](#b-binary-installation) 

### A. Debian CARLA installation

The Debain package is available for both Ubuntu 18.04 and Ubuntu 20.04, however __the officially supported platform is Ubuntu 18.04__.

__1.__ Set up the Debian repository in the system:
```sh
    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1AF1527DE64CB8D9
    sudo add-apt-repository "deb [arch=amd64] http://dist.carla.org/carla $(lsb_release -sc) main"
```

__2.__ Install CARLA and check for the installation in the `/opt/` folder:
```sh
    sudo apt-get update # Update the Debian package index
    sudo apt-get install carla-simulator # Install the latest CARLA version, or update the current installation
    cd /opt/carla-simulator # Open the folder where CARLA is installed
```

This repository contains CARLA 0.9.10 and later versions. To install a specific version add the version tag to the installation command:  
```sh
    apt-cache madison carla-simulator # List the available versions of Carla
    sudo apt-get install carla-simulator=0.9.10-1 # In this case, "0.9.10" refers to a CARLA version, and "1" to the Debian revision
```

!!! Important
    To install CARLA versions prior to 0.9.10, change to a previous version of the documentation using the panel in the bottom right corner of the window, and follow the old instructions.  

### B. Binary installation

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/blob/master/Docs/download.md" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>

This repository contains different versions of CARLA. You will find options to download the __current release__ with all the most recent fixes and features, __previous releases__ and a __nightly build__ with all the developmental fixes and features (the nightly build is the most unstable version of CARLA).

The package is a compressed file named __CARLA_version.number__. Download and extract the release file. It contains a precompiled version of the simulator, the Python API module and some scripts to be used as examples. 

---
## Import additional assets 

Each release has it's own additional package of extra assets and maps. This additional package includes the maps __Town06__, __Town07__, and __Town10__. These are stored separately to reduce the size of the build, so they can only be imported after the main package has been installed. 

__1.__ [Download](https://github.com/carla-simulator/carla/blob/master/Docs/download.md) the appropriate package for your desired version of CARLA.

__2.__ Extract the package:

- __On Linux__:

    - move the package to the _Import_ folder and run the following script to extract the contents:  

```sh
        cd path/to/carla/root
        ./ImportAssets.sh
```

- __On Windows__:

    - Extract the contents directly in the root folder. 

---
## Running CARLA

The method to start a CARLA server depends on the installation method you used and your operating system:

- Debian installation:

```sh
    cd /opt/carla-simulator/bin/
    ./CarlaUE4.sh
```

- Linux binary installation:

```sh
    cd path/to/carla/root
    ./CarlaUE4.sh
```

- Windows binary installation:

```sh
    cd path/to/carla/root
    CarlaUE4.exe
```

A window containing a view over the city will pop up. This is the _spectator view_. To fly around the city use the mouse and `WASD` keys, holding down the right mouse button to control the direction. 

This is the server simulator which is now running and waiting for a client to connect and interact with the world. You can try some of the example scripts to spawn life into the city and drive a car:

```sh
        # Terminal A 
        cd PythonAPI\examples
        python3 -m pip install -r requirements.txt # Support for Python2 is provided in the CARLA release packages
        python3 spawn_npc.py  

        # Terminal B
        cd PythonAPI\examples
        python3 manual_control.py 
```

#### Command-line options

There are some configuration options available when launching CARLA and they can be used as follows:

```sh
    ./CarlaUE4.sh -carla-rpc-port=3000
```

* `-carla-rpc-port=N` Listen for client connections at port `N`. Streaming port is set to `N+1` by default.  
* `-carla-streaming-port=N` Specify the port for sensor data streaming. Use 0 to get a random unused port. The second port will be automatically set to `N+1`.  
* `-quality-level={Low,Epic}` Change graphics quality level. Find out more in [rendering options](adv_rendering_options.md).  
* __[List of Unreal Engine 4 command-line arguments][ue4clilink].__ There are a lot of options provided by Unreal Engine however not all of these are available in CARLA.  

[ue4clilink]: https://docs.unrealengine.com/en-US/Programming/Basics/CommandLineArguments

The script [`PythonAPI/util/config.py`][config] provides more configuration options and should be run when the server has been started up:

[config]: https://github.com/carla-simulator/carla/blob/master/PythonAPI/util/config.py

```sh
    ./config.py --no-rendering      # Disable rendering
    ./config.py --map Town05        # Change map
    ./config.py --weather ClearNoon # Change weather

    ./config.py --help # Check all the available configuration options
```

---
## Updating CARLA

There is no way to update the packaged version of CARLA. When a new version is released, the repository is updated and you will need to delete the previous version and install the new version.

---

## Installation summary

```sh
# Install required modules Pygame and Numpy
python3 -m pip install --user pygame numpy # Support for Python 2 is provided in the CARLA release packages

# There are two different ways to install CARLA

# Option A) Debian package installation
# This repository contains CARLA 0.9.10 and later. To install previous CARLA versions, change to a previous version of the docs using the panel in the bottom right part of the window
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1AF1527DE64CB8D9
sudo add-apt-repository "deb [arch=amd64] http://dist.carla.org/carla $(lsb_release -sc) main"
sudo apt-get update
sudo apt-get install carla-simulator # Install the latest CARLA version or update the current installation
sudo apt-get install carla-simulator=0.9.10-1 # install a specific CARLA version
cd /opt/carla-simulator
./CarlaUE4.sh

# Option B) Package installation
#   Go to: https://github.com/carla-simulator/carla/blob/master/Docs/download.md
#   Download the desired package and additional assets
#   Extract the package
#   Extract the additional assets in `/Import`
#   Run CARLA (Linux).
./CarlaUE.sh
#   Run CARLA (Windows)
CarlaUE4.exe

# Run a script to test CARLA.
cd PythonAPI/examples
python3 -m pip install -r requirements.txt
python3 spawn_npc.py # Support for Python 2 is provided in the CARLA release packages

```

---
## Follow-up

By now you should have a packaged version of CARLA up and running. If you came across any difficulties during the installation process, feel free to post in the [CARLA forum](https://forum.carla.org/) or in the [Discord](https://discord.gg/8kqACuC) channel.

The next step is to learn more about the core concepts in CARLA. Read the __First steps__ section to start learning. You can also find all the information about the Python API classes and methods in the [Python API reference](python_api.md).

<div class="build-buttons">
<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Go to first steps">
Go to: First steps</a>
</p>
</div>
