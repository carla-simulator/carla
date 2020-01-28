
<h1>Quickstart</h1>

* [Requirements](#requirements)
* [Downloading CARLA](#downloading-carla)
* [Running CARLA](#running-carla)
* [Updating CARLA](#updating-carla)
* [Summary](#summary)
---------------
##Requirements

The quickstart installation uses a pre-packaged version of CARLA. This comprises the content in a boundle that can run automatically with no build needed. The API can be accesseded fully but in exchange, advanced customization and developing options are unavailable.  
However, some requirements are still needed.  

* __Server side:__ A good GPU will be needed to run a highly realistic environment (4GB minimum). A dedicated GPU is highly advised for machine learning. 
* __Client side:__ it consists of a command line to get in touch with the simulator using [Python](https://www.python.org/downloads/). No graphics at all. A good internet connection and two TCP ports (2000 and 1 by default) will be needed. 
* __System requirements:__ Any 64-bits OS should run the corresponding version of CARLA.
* __Other requirements:__  Only two specific Python modules: [Pygame](https://www.pygame.org/download.shtml), to create graphics directly with Python and [Numpy](https://pypi.org/project/numpy/) for great calculus.  

If you have [pip](https://pip.pypa.io/en/stable/installing/) in your system, you can geth both libraries simply by: 
```sh
 pip install --user pygame numpy
```    
---------------
##Downloading CARLA

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/blob/master/Docs/download.md" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>

The repository contains the different versions of the simulator available. The _nightly build_ is the current development version as today. It is the most unstable and requires build installation. The packaged versions of interest for this quickstart can be found in _development_ and _stable_ sections, which contain the different official releases. The later the version the more experimental it is. By the time this documentation is written, __CARLA 0.9.7__ is the latest containing all new features (such as traffic manager), but __CARLA 0.8.2__ is the stable one, recommended for those who are looking for a neat run.  

!!! note
    Latest Windows release is __CARLA 0.9.5__, but this is to be updated soon. 

For each release there may be many files. The package is named as __CARLA_version.number__ (compressed file format _.tar.gz_ for Linux and _.zip_ for Windows). Other elements such as __Town06_0.9.5.tar.gz__ are additional assets for Linux releases.

Download and extract the release in a folder of your choice. It contains a precompiled version of the simulator, the Python API module and some scripts to be used as examples.  
If you downloaded any additional assets in Linux, move them to the _Import_ folder that has appeared (there is no need to extract them). Open a terminal window in the folder where you extracted CARLA and run the script _ImportAssets_ to get the additional content automatically: 

```sh
./ImportAssets.sh
```

---------------
##Running CARLA

Open a terminal window in the folder where you extracted CARLA. The following command will execute the package file and start the simulation:

```sh
Linux:
./CarlaUE4.sh

Windows:
CarlaUE4.exe
```

A window will open, containing a view over the city. This is the "spectator"
view, you can fly around the city using the mouse and WASD keys, but you cannot
interact with the world. The simulator is now running as a server,
waiting for a client app to connect and interact with the world.

!!! note
    If your firewall or any other application are blocking the TCP ports needed, you can manually change them by adding to the previous command the argument: `-carla-port=N`, being `N` your desired port. The second will be automatically set to `N+1`.

---------------
##Updating CARLA

The packaged version requires no updates. The content is bundled and thus, tied to a specific version of CARLA. Everytime there is a release, the repository will be updated. To run this latest or any other version, delete the previous one, visit the CARLA repository on GitHub and repeat the installation steps with the desired version. 

---------------
##Summary

That concludes the quickstart installation process. In case any unexpected error or issue occurs, the CARLA forumm is open to everybody. There is an **Installation issues** category to post this kind of problems and doubts. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
Visit the forum</a>
</p>
</div>

So far, CARLA should be operative in your desired system. A terminal window will be used to contact the server via scripts. Thus will access all of the capabilities that CARLA provides. The next step should be to visiting the _First steps_ section to learn more about this. However, all the information about the Python API regarding classes and its methods can be accessed in this [reference](../python_api.md).

<div class="build-buttons">
<p>
<a href="../../python_api_tutorial" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
Go to: First steps</a>
</p>
</div>





