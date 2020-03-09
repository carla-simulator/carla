# Quick start

  * [Requirements](#requirements)
  * [Downloading CARLA](#downloading-carla)
	* a) apt-get installation  
	* b) Repository download  
  * [Running CARLA](#running-carla)
	* Command-line options  
  * [Updating CARLA](#updating-carla)
  * [Summary](#summary)

---
## Requirements

The quickstart installation uses a pre-packaged version of CARLA. This comprises the content in a boundle that can run automatically with no build installation needed. The API can be accesseded fully but in exchange, advanced customization and developing options are unavailable.  
However, some requirements are still a must.  

  * __Server side:__ A good GPU will be needed to run a highly realistic environment (4GB minimum). A dedicated GPU is highly advised for machine learning. 
  * __Client side:__ The API is accessed via command line. To do so, [Python](https://www.python.org/downloads/) is necessary, and also a good internet connection and two TCP ports (2000 and 2001 by default). 
  * __System requirements:__ Any 64-bits OS should run the corresponding version of CARLA.
  * __Other requirements:__  Only two specific Python modules: [Pygame](https://www.pygame.org/download.shtml), to create graphics directly with Python and [Numpy](https://pypi.org/project/numpy/) for great calculus.  

If you have [pip](https://pip.pypa.io/en/stable/installing/) in your system, you can geth both modules simply by running the following commands: 
```sh
 pip install --user pygame numpy
```    
---
## Downloading CARLA

The easiest way to get the latest release in Linux is using the __apt repository__.  
To get either a specific release or get the Windows version of CARLA: __download the repository__.  
Both methods will set CARLA ready to run. 

<h4>a) apt-get CARLA 0.9.7</h4>

First, add the repository to the system: 
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys DB53A429E64554FC &&
sudo add-apt-repository "deb [trusted=yes] http://dist.carla.org/carla-0.9.7/ bionic main"
```
Then, simply install it. CARLA will be in the `/opt/` folder, where other software such as ROS install themselves: 
```sh
sudo apt-get update
sudo apt-get install carla
```

<h4>b) Downloading the repository</h4>

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/blob/master/Docs/download.md" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>

The repository contains the different versions of the simulator available. The _development_ and _stable_ sections, contain the packages for the different official releases. The later the version the more experimental it is. The _nightly build_ is the current development version as today and so, the most unstable (developers are currently working with this build). If you want a more robust version, you may search for the latest tagged version.

!!! note
    Latest Windows release is __CARLA 0.9.5__, but this is to be updated soon. 

There may be many files per release. The package is named as __CARLA_version.number__ (compressed file format _.tar.gz_ for Linux and _.zip_ for Windows). Other elements such as __Town06_0.9.5.tar.gz__ are additional assets for Linux releases.

Download and extract the release in a folder of your choice. It contains a precompiled version of the simulator, the Python API module and some scripts to be used as examples.  
If you downloaded any additional assets in Linux, move them to the _Import_ folder that has appeared (there is no need to extract them). Open a terminal in the folder where you extracted CARLA and run the script _ImportAssets_ to get the additional content automatically: 

```sh
./ImportAssets.sh
```

---
## Running CARLA

Open a terminal in the folder where CARLA was extracted. The following command will execute the package file and start the simulation:

```sh
Linux:
./CarlaUE4.sh

Windows:
CarlaUE4.exe
```
!!! Important
    In the __apt-get__ installation, `CarlaUE4.sh` can be found in `/opt/carla/bin/`, instead of the main `/carla/` folder where it normally is. 

A window will open, containing a view over the city. This is the "spectator" view. To fly around the city use the mouse and WASD keys (while clicking). The simulator is now running as a server, waiting for a client app to connect and interact with the world.

!!! note
    If the firewall or any other application are blocking the TCP ports needed, these can be manually changed by adding to the previous command the argument: `-carla-port=N`, being `N` the desired port. The second will be automatically set to `N+1`.

#### Command-line options

There are some configuration options available when launching CARLA:  

  * `-carla-rpc-port=N` Listen for client connections at port N, streaming port is set to N+1 by default.
  * `-carla-streaming-port=N` Specify the port for sensor data streaming, use 0 to get a random unused port.
  * `-quality-level={Low,Epic}` Change graphics quality level.
  * [Full list of UE4 command-line arguments][ue4clilink] (note that many of these won't work in the release version).

[ue4clilink]: https://docs.unrealengine.com/en-US/Programming/Basics/CommandLineArguments
```sh
> ./CarlaUE4.sh -carla-rpc-port=3000
```
However, some may not be available (especially those provided by UE). For said reason, the script in `PythonAPI/util/config.py` provides for some more configuration options: 
```sh
> ./config.py --no-rendering      # Disable rendering
> ./config.py --map Town05        # Change map
> ./config.py --weather ClearNoon # Change weather
```

To check all the available configurations, run the following command:

```sh
> ./config.py --help
```

---
## Updating CARLA

The packaged version requires no updates. The content is bundled and thus, tied to a specific version of CARLA. Everytime there is a release, the repository will be updated. To run this latest or any other version, delete the previous one and repeat the installation steps with the desired. 

---
## Summary

That concludes the quickstart installation process. In case any unexpected error or issue occurs, the [CARLA forum](https://forum.carla.org/) is open to everybody. There is an _Installation issues_ category to post this kind of problems and doubts. 

So far, CARLA should be operative in the desired system. Terminals will be used to contact the server via script and retrieve data. Thus will access all of the capabilities that CARLA provides. Next step should be  visiting the __First steps__ section to learn more about this. However, all the information about the Python API regarding classes and its methods can be accessed in the [Python API reference](python_api.md).

<div class="build-buttons">
<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Go to first steps">
Go to: First steps</a>
</p>
</div>
