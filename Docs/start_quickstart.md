# Quick start

* __[Installation summary](#installation-summary)__  
* __[Requirements](#requirements)__  
* __[CARLA installation](#carla-installation)__  
	* [A. Debian CARLA installation](#a-debian-carla-installation)  
	* [B. Package installation](#b-package-installation)  
* __[Import additional assets](#import-additional-assets)__  
* __[Running CARLA](#running-carla)__  
	* [Command-line options](#command-line-options)  
* __[Updating CARLA](#updating-carla)__  
* __[Follow-up](#follow-up)__  

---
## Installation summary

<details>
   <summary>
    Show command line summary for the quick start installation
   </summary>

```sh
# Install required modules Pygame and Numpy
 pip install --user pygame numpy

# There are two different ways to install CARLA

# Option A) Debian package installation
# This repository contains CARLA 0.9.10 and later. To install previous CARLA versions, change to a previous version of the docs using the pannel in the bottom right part of the window
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
> CarlaUE4.exe

# Run a script to test CARLA.
cd PythonAPI/examples
python3 spawn_npc.py # Support for Python2 is provided in the CARLA release packages

```
</details>

---
## Requirements

The quick start installation uses a pre-packaged version of CARLA. The content is comprised in a boundle that can run automatically with no build installation needed. The API can be accesseded fully but advanced customization and development options are unavailable.  
The requirements are simpler than those for the build installation.  

* __Server side.__ A 4GB minimum GPU will be needed to run a highly realistic environment. A dedicated GPU is highly advised for machine learning.  
* __Client side.__ [Python](https://www.python.org/downloads/) is necessary to access the API via command line. Also, a good internet connection and two TCP ports (2000 and 2001 by default).  
* __System requirements.__ Any 64-bits OS should run CARLA. However, since release 0.9.9, __CARLA cannot run in 16.04 Linux systems with default compilers__. These should be upgraded to work with CARLA.  
* __Other requirements.__  Two Python modules: [Pygame](https://pypi.org/project/pygame/) to create graphics directly with Python, and [Numpy](https://pypi.org/project/numpy/) for great calculus.  

To install both modules using [pip](https://pip.pypa.io/en/stable/installing/), run the following commands. 
```sh
 pip install --user pygame numpy
```    
---
## CARLA installation

The __Debian installation__ is the easiest way to get the latest release in Linux.  
__Download the GitHub repository__ to get either a specific release or the Windows version of CARLA.  

### A. Debian CARLA installation

Set up the Debian repository in the system.
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1AF1527DE64CB8D9
sudo add-apt-repository "deb [arch=amd64] http://dist.carla.org/carla $(lsb_release -sc) main"
```
Install CARLA and check for the installation in the `/opt/` folder.
```sh
sudo apt-get update # Update the Debian package index
sudo apt-get install carla-simulator # Install the latest CARLA version, or update the current installation
cd /opt/carla-simulator # Open the folder where CARLA is installed
```

This repository contains CARLA 0.9.10 and later versions. To install a specific version add the version tag to the installation command.  
```sh
sudo apt-get install carla-simulator=0.9.10-1 # In this case, "0.9.10" refers to a CARLA version, and "1" to the Debian revision
```

!!! Important
    To install CARLA versions prior to 0.9.10, change to a previous version of the documentation using the pannel in the bottom right corner of the window, and follow the old instructions.  

### B. Package installation

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/blob/master/Docs/download.md" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>

The repository contains different versions of the simulator available. _Development_ and _stable_ sections list the packages for the different official releases. The later the version the more experimental it is. The _nightly build_ is the current development version as today and so, the most unstable.  

There may be many files per release. The package is a compressed file named as __CARLA_version.number__.  

Download and extract the release file. It contains a precompiled version of the simulator, the Python API module and some scripts to be used as examples. 

---
## Import additional assets 

For every release there are other packages containing additional assets and maps, such as __Additional_Maps_0.9.9.2__ for CARLA 0.9.9.2, which contains __Town06__, __Town07__, and __Town10__. These are stored separatedly to reduce the size of the build, so they can only be run after these packages are imported.  

Download and move the package to the _Import_ folder, and run the following script to extract them.  

```sh
> cd ~/carla
> ./ImportAssets.sh
```

!!! Note
    On Windows, directly extract the package on the root folder. 

---
## Running CARLA

Open a terminal in the main CARLA folder. Run the following command to execute the package file and start the simulation:

```sh
# Linux:
> ./CarlaUE4.sh
# Windows:
> CarlaUE4.exe
```
!!! Important
    In the __deb installation__, `CarlaUE4.sh` will be in `/opt/carla-simulator/bin/`, instead of the main `carla/` folder where it normally is. 

A window containing a view over the city will pop up. This is the _spectator view_. To fly around the city use the mouse and `WASD` keys (while clicking). The server simulator is now running and waiting for a client to connect and interact with the world.  
Now it is time to start running scripts. The following example will spawn some life into the city: 

```sh
# Go to the folder containing example scripts
> cd PythonAPI/examples

> python3 spawn_npc.py # Support for Python2 is provided in the CARLA release packages
```


#### Command-line options

There are some configuration options available when launching CARLA.  

* `-carla-rpc-port=N` Listen for client connections at port `N`. Streaming port is set to `N+1` by default.  
* `-carla-streaming-port=N` Specify the port for sensor data streaming. Use 0 to get a random unused port. The second port will be automatically set to `N+1`.  
* `-quality-level={Low,Epic}` Change graphics quality level. Find out more in [rendering options](adv_rendering_options.md).  
* __[Full list of UE4 command-line arguments][ue4clilink].__ There is a lot of options provided by UE. However, not all of these will be available in CARLA.  

[ue4clilink]: https://docs.unrealengine.com/en-US/Programming/Basics/CommandLineArguments
```sh
> ./CarlaUE4.sh -carla-rpc-port=3000
```
The script `PythonAPI/util/config.py` provides for more configuration options. 

```sh
> ./config.py --no-rendering      # Disable rendering
> ./config.py --map Town05        # Change map
> ./config.py --weather ClearNoon # Change weather

> ./config.py --help # Check all the available configuration options
```

---
## Updating CARLA

The packaged version requires no updates. The content is bundled and thus, tied to a specific version of CARLA. Everytime there is a release, the repository will be updated. To run this latest or any other version, delete the previous and install the one desired.  

---
## Follow-up

Thus concludes the quick start installation process. In case any unexpected error or issue occurs, the [CARLA forum](https://forum.carla.org/) is open to everybody. There is an _Installation issues_ category to post this kind of problems and doubts. 

So far, CARLA should be operative in the desired system. Terminals will be used to contact the server via script, interact with the simulation and retrieve data. To do so, it is essential to understand the core concepts in CARLA. Read the __First steps__ section to learn on those. Additionally, all the information about the Python API regarding classes and its methods can be accessed in the [Python API reference](python_api.md).

<div class="build-buttons">
<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Go to first steps">
Go to: First steps</a>
</p>
</div>
