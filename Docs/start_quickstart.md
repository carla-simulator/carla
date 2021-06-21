# Quick start package installation

This guide shows how to download and install the packaged version of CARLA. The package includes the CARLA server and a Python wheel for the client library that needs to be installed with __pip__. There are additional assets that can be downloaded and imported into the package. Advanced customization and development options that require use of the Unreal Engine editor are not available but these can be accessed by using the build version of CARLA for either [Windows](build_windows.md) or [Linux](build_linux.md).

* __[Before you begin](#before-you-begin)__  
* __[CARLA installation](#carla-installation)__  
	* [A. Debian CARLA installation](#a-debian-carla-installation)  
	* [B. Package installation](#b-package-installation)  
* __[Import additional assets](#import-additional-assets)__  
* __[Running CARLA](#running-carla)__  
	* [Command-line options](#command-line-options)  
* __[Updating CARLA](#updating-carla)__   
* __[Installation summary](#installation-summary)__  
* __[Follow-up](#follow-up)__ 
---
## Before you begin

The following requirements should be fulfilled before installing CARLA:

* __System requirements.__ CARLA is built for Windows and Linux systems. If running on Linux, the OS must support __glibc >= 2.27__ (Ubuntu 18.04+)
* __An adequate GPU.__ CARLA aims for realistic simulations, so the server needs at least a 6 GB GPU although we would recommend 8 GB. A dedicated GPU is highly recommended for machine learning. 
* __Disk space.__ CARLA will use about 20 GB of space.
* __Python.__ [Python]((https://www.python.org/downloads/)) is the main scripting language in CARLA. CARLA supports Python 2.7, 3.6, 3.7 and 3.8.
* __Two TCP ports and good internet connection.__ 2000 and 2001 by default. Make sure that these ports are not blocked by firewalls or any other applications. 
* __Other requirements.__  CARLA requires some dependencies for the client library. These dependencies must be installed using __pip>=20.3__. To check your __pip__ version:

```sh
pip -V
```

If you need to upgrade:

```sh
pip install --upgrade pip
```

Once you have the correct version of __pip__, you can install the dependencies according to your operating system:

### Windows

```sh
pip install --user pygame numpy &&
pip3 install --user pygame numpy &&
pip install --user wheel &&
pip3 install --user wheel
```

### Linux

```sh
pip install --user pygame numpy &&
pip3 install --user pygame numpy &&
pip install --user wheel &&
pip3 install --user wheel auditwheel
```

---
## CARLA installation

There are two methods to download and install CARLA as a package:

__A)__ [Download the Debian package.](#a-debian-carla-installation)

__B)__ [Download the package from GitHub.](#b-package-installation) 

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

### B. Package installation

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

## Install Python library

### CARLA versions prior to 0.9.12

Previous versions of CARLA did not require the Python library to be installed, they came with an `.egg` file that was ready to use out of the box. __CARLA versions 0.9.12+ change this behaviour significantly; `.egg` files are no longer supported.__ If you are using a version of CARLA prior to 0.9.12, please select that version in the bottom right corner of the screen to see the relevant documentation.

### CARLA 0.9.12+

CARLA provides `.whl` files for Python versions 2.7, 3.6, 3.7 and 3.8. To be able to use the Python client library you will need to install the `.whl` file. The `.whl` file is found in `PythonAPI/carla/dist/`. There is one file per supported Python version, indicated by the file name (e.g., carla-0.9.12-__cp36__-cp36m-manylinux_2_27_x86_64.whl indicates Python 3.6).

__It is recommended to install the CARLA client library in a virtual environment to avoid conflicts when working with muliple versions.__

To install the CARLA client library, run the following command, choosing the file appropriate to your desired Python version:

```sh
pip install <wheel-file-name>.whl
```


---
## Running CARLA

The method to start a CARLA server depends on the installation method you used and your operating system:

- Debian installation:

```sh
    cd /opt/carla-simulator/bin/

    ./CarlaUE4.sh
```

- Linux package installation:

```sh
    cd path/to/carla/root

    ./CarlaUE4.sh
```

- Windows package installation:

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

The script [`PythonAPI/util/config.py`][config] provides more configuration options and should be run when the server has been started:

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
# Make sure you are running pip>=20.3
pip install --upgrade pip

# Install required modules Pygame and Numpy
python3 -m pip install --user pygame numpy # Support for Python 2 is provided in the CARLA release packages

# Install client library requirements - Windows
pip install --user wheel &&
pip3 install --user wheel

# Install client library requirements - Linux
pip install --user wheel &&
pip3 install --user wheel auditwheel

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

# Install the client library
pip install <PythonAPI/carla/dist/wheel-file>.whl

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

By now you should have a packaged version of CARLA up and running. If you came across any difficulties during the installation process, feel free to post in the [CARLA forum](https://github.com/carla-simulator/carla/discussions/) or in the [Discord](https://discord.gg/8kqACuC) channel.

The next step is to learn more about the core concepts in CARLA. Read the __First steps__ section to start learning. You can also find all the information about the Python API classes and methods in the [Python API reference](python_api.md).

<div class="build-buttons">
<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Go to first steps">
Go to: First steps</a>
</p>
</div>
