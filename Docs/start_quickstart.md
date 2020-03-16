# Quick start

* __[Installation summary](#installation-summary)__  
* __[Requirements](#requirements)__  
* __[Downloading CARLA](#downloading-carla)__  
	* a) deb CARLA installation  
	* b) GitHub repository installation  
* __[Running CARLA](#running-carla)__  
	* Command-line options  
* __[Updating CARLA](#updating-carla)__  
* __[Follow-up](#follow-up)__  

---
## Installation summary

<details>
   <summary>
    Show command line summary for the quick start installation
   </summary>

```sh
# Install required modules Pygame and Numpy. 
 pip install --user pygame numpy

# Option A) deb package installation (only Linux)
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys DB53A429E64554FC &&
sudo add-apt-repository "deb [trusted=yes] http://dist.carla.org/carla-0.9.7/ bionic main"
sudo apt-get update
sudo apt-get install carla
cd /opt/carla/bin
./CarlaUE4.sh

# Option B) GitHub repository installation
#   Go to: https://github.com/carla-simulator/carla/blob/master/Docs/download.md
#   Download the desired package and additional assets. 
#   Extract the package. 
#   Extract the additional assets in `/Import`
#   Run CARLA (Linux)
./CarlaUE.sh
#   Run CARLA (Windows)
> CarlaUE4.exe

# Run a script to test CARLA
cd PythonAPI/examples
python3 spawn_npc.py

```
</details>

---
## Requirements

The quick start installation uses a pre-packaged version of CARLA. The content is comprised in a boundle that can run automatically with no build installation needed. The API can be accesseded fully but advanced customization and development options are unavailable.  
The requirements are simpler than those for the build installation.  

* __Server side.__ A 4GB minimum GPU will be needed to run a highly realistic environment. A dedicated GPU is highly advised for machine learning.  
* __Client side.__ [Python](https://www.python.org/downloads/) is necessary to access the API via command line. Also, a good internet connection and two TCP ports (2000 and 2001 by default).  
* __System requirements.__ Any 64-bits OS should run CARLA.  
* __Other requirements.__  Two Python modules: [Pygame](https://www.pygame.org/download.shtml) to create graphics directly with Python, and [Numpy](https://pypi.org/project/numpy/) for great calculus.  

To install both modules using [pip](https://pip.pypa.io/en/stable/installing/), run the following commands. 
```sh
 pip install --user pygame numpy
```    
---
## Downloading CARLA

The __deb installation__ is the easiest way to get the latest release in Linux.  
__Download the GitHub repository__ to get either a specific release or the Windows version of CARLA.  

### a) deb CARLA installation

Add the repository to the system. 
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys DB53A429E64554FC &&
sudo add-apt-repository "deb [trusted=yes] http://dist.carla.org/carla-0.9.7/ bionic main"
```
Install CARLA and check for the installation in the `/opt/` folder.
```sh
sudo apt-get update
sudo apt-get install carla
cd /opt/carla
```

### b) GitHub repository installation

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/blob/master/Docs/download.md" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>

The repository contains the different versions of the simulator available. _Development_ and _stable_ sections list the packages for the different official releases. The later the version the more experimental it is. The _nightly build_ is the current development version as today and so, the most unstable.  

There may be many files per release. The package is a compressed file named as __CARLA_version.number__. Other elements such as __Town06_0.9.5.tar.gz__ are additional assets.

Download and extract the release file. It contains a precompiled version of the simulator, the Python API module and some scripts to be used as examples.  
Move any additional assets to the _Import_ folder in the release and extract them. The script _ImportAssets_ in the main CARLA folder should extract these automatically.  

```sh
> cd ~/carla
> ./ImportAssets.sh
```

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
    In the __deb installation__, `CarlaUE4.sh` will be in `/opt/carla/bin/`, instead of the main `/carla/` folder where it normally is. 

A window containing a view over the city will pop up. This is the _spectator view_. To fly around the city use the mouse and `WASD` keys (while clicking). The server simulator is now running and waiting for a client to connect and interact with the world.  
Now it is time to start running scripts. The following example will spawn some life into the city: 

```sh
# Go to the folder containing example scripts
> cd PythonAPI/examples

> python3 spawn_npc.py
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

> ./config.py --help # Check all the available configuration options. 
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
