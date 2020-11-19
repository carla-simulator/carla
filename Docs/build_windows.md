# Windows build

*   [__Windows build command summary__](#windows-build-command-summary)  
*   [__Requirements__](#requirements)  
	*   [System specifics](#system-specifics)  
*   [__Necessary software__](#necessary-software)  
	*   [Minor installations (CMake, git, make, Python3 x64)](#minor-installations)  
	*   [Visual Studio 2017](#visual-studio-2017)  
	*   [Unreal Engine (4.24)](#unreal-engine)  
*   [__CARLA build__](#carla-build)  
	*   [Clone repository](#clone-repository)  
	*   [Get assets](#get-assets)  
	*   [Set the environment variable](#set-the-environment-variable)  
	*   [make CARLA](#make-carla)  

The build process can be quite long and tedious. The **[F.A.Q.](build_faq.md)** page offers solution for the most common complications. Alternatively, use the [CARLA forum](https://forum.carla.org/c/installation-issues/linux) to post any unexpected issues that may occur.  

---
## Windows build command summary 

<details>
<summary> Show command lines to build on Windows</summary>
<br>
To execute the <code>make</code> commands below, you <b>must</b> use the Visual Studio 2017 native console x64 <b>with</b> administrator rights, otherwise you may be getting permission errors.

!!! Important
    To execute the ```make``` commands below, you **must** use the Visual Studio 2017 native console x64 **with** administrator rights, otherwise you may be getting permission errors.
	
```sh
# Make sure to meet the minimum requirements

# Necessary software: 
#   CMake
#   Git
#   Make
#   Python3 x64
#   Unreal Engine 4.24
#   Visual Studio 2017 with Windows 8.1 SDK and x64 Visual C++ Toolset

# Set environment variables for the software

# Clone the CARLA repository
git clone https://github.com/carla-simulator/carla

# make the CARLA client and the CARLA server
make PythonAPI
make launch

# Press play in the Editor to initialize the server
# Run example scripts to test CARLA
# Terminal A 
cd PythonAPI/examples
python3 spawn_npc.py 
# Terminal B
cd PythonAPI/examples
python3 dynamic_weather.py 
# The PythonAPI will be built based on the installed Python version, but the docs will use Python3 by default.

```
</details>

---
## Requirements

### System specifics

* __x64 system.__ The simulator should run in any 64 bits Windows system.  
* __30GB disk space.__ Installing all the software needed and CARLA will require quite a lot of space. Make sure to have around 30/50GB of free disk space.  
* __An adequate GPU.__ CARLA aims for realistic simulations, so the server needs at least a 4GB GPU. A dedicated GPU is highly recommended for machine learning. 
* __Two TCP ports and good internet connection.__ 2000 and 2001 by default. Be sure neither the firewall nor any other application are blocking these. 

---
## Necessary software
### Minor installations

* [__CMake__](https://cmake.org/download/) generates standard build files from simple configuration files.  
* [__Git__](https://git-scm.com/downloads) is a version control system to manage CARLA repositories.  
* [__Make__](http://gnuwin32.sourceforge.net/packages/make.htm) generates the executables.  
* [__Python3 x64__](https://www.python.org/downloads/) is the main script language in CARLA. Having a x32 version installed may cause conflict, so it is highly advisable to have it uninstalled.  

!!! Important
    Be sure that these programs are added to the [environment path](https://www.java.com/en/download/help/path.xml). Remember that the path added leads to the _bin_ directory.  


### Visual Studio 2017

Get the 2017 version from [here](https://developerinsider.co/download-visual-studio-2017-web-installer-iso-community-professional-enterprise/). __Community__ is the free version. Use the _Visual Studio Installer_ to install two additional elements. 

* __Windows 8.1 SDK.__ Select it in the _Installation details_ section on the right.  
* __x64 Visual C++ Toolset.__ In the _Workloads_ section, choose __Desktop development with C++__. This will enable a x64 command prompt that will be used for the build. Check it up by pressing the `Win` button and searching for `x64`. Be careful to __not open a `x86_x64` prompt__.  

!!! Important
    Other Visual Studio versions may cause conflict. Even if these have been uninstalled, some registers may persist. To completely clean Visual Studio from the computer, go to `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout` and run `.\InstallCleanup.exe -full`  

### Unreal Engine

Go to [Unreal Engine](https://www.unrealengine.com/download) and download the _Epic Games Launcher_. In `Engine versions/Library`, download __Unreal Engine 4.24.x__. Make sure to run it in order to check that everything was properly installed.  

!!! Note 
    Having VS2017 and UE4.24 installed, a __Generate Visual Studio project files__ option should appear when doing right-click on __.uproject__ files. If this is not available, something went wrong whith the UE4.24 installation. Create a UE project to check it out and reinstall if necessary. 

---
## CARLA build

!!! Important
    Lots of things have happened so far. It is highly advisable to restart the computer.  

### Clone repository

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla" target="_blank" class="btn btn-neutral" title="Go to the CARLA repository">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>

The official repository of the project. Either download and extract it or clone it using the following command line in a **x64 terminal**.

```cmd
git clone https://github.com/carla-simulator/carla
```

Now the latest content for the project, known as `master` branch in the repository, has been copied in local. 

!!! Note
    The `master` branch contains the latest fixes and features. Stable code is inside the `stable` and previous CARLA versions have their own branch. Always remember to check the current branch in git with the command `git branch`. 

### Get assets

Only the assets package is yet to be downloaded. `\Util\ContentVersions.txt` contains the links to the assets for CARLA releases. These must be extracted in `Unreal\CarlaUE4\Content\Carla`. If the path doesn't exist, create it.  

Download the __latest__ assets to work with the current version of CARLA.

### Set the environment variable

This is necessary for CARLA to find the Unreal Engine installation folder. By doing so, users can choose which specific version of Unreal Engine is to be used. If no environment variable is specified, the CARLA will look up in the directories and use the last version in search order.  

__1.   Open the Windows Control Panel__ and go to `Advanced System Settings`.  
__2.   On the `Advanced` panel__ open `Environment Variables...`.  
__3.   Click `New...`__ to create the variable.  
__4.   Name the variable as `UE4_ROOT`__ and choose the path to the installation folder of the desire UE4 installation.  


### make CARLA

The last step is to finally build CARLA. There are different `make` commands to build the different modules. All of them run in the root CARLA folder.

!!! Warning
    Make sure to run `make PythonAPI` to prepare the client and `make launch` for the server.
    Alternatively `make LibCarla` will prepare the CARLA library to be imported anywhere.

* __make PythonAPI__ compiles the API client, necessary to grant control over the simulation. It is only needed the first time. Remember to run it again when updating CARLA. Scripts will be able to run after this command is executed.  
```sh
make PythonAPI
```

* __make launch__ compiles the server simulator and launches Unreal Engine. Press **Play** to start the spectator view and close the editor window to exit. Camera can be moved with `WASD` keys and rotated by clicking the scene while moving the mouse around.  
```sh
make launch
```
The project may ask to build other instances such as `UE4Editor-Carla.dll` the first time. Agree in order to open the project. During the first launch, the editor may show warnings regarding shaders and mesh distance fields. These take some time to be loaded and the city will not show properly until then.


Finally, let's test the simulator. Inside `PythonAPI/examples` and `PythonAPI/util` there are some example scripts that may be especially useful for starters. The following commands will spawn some life into the town, and create a weather cycle. Each script should be run in one terminal 

```sh
# Terminal A 
cd PythonAPI/examples
python3 spawn_npc.py  
# Terminal B
cd PythonAPI/examples
python3 dynamic_weather.py 
# The PythonAPI will be built based on the installed Python version, but the docs will use Python3 by default.
```

!!! Important
    If the simulation is running at very low FPS rates, go to `Edit/Editor preferences/Performance` in the UE editor and disable __Use less CPU when in background__.

Now CARLA is ready to go. Here is a brief summary of the most useful `make` commands available.  

<table class ="defTable">
<thead>
<th>Command</th>
<th>Description</th>
</thead>
<tbody>
<td><code>make help</code> </td>
<td>Prints all available commands.</td>
</tr>
<tr>
<td><code>make launch</code></td>
<td>Launches CARLA server in Editor window.</td>
</tr>
<tr>
<td><code>make PythonAPI</code></td>
<td>Builds the CARLA client.</td>
</tr>
<tr>
<td><code>make package</code></td>
<td>Builds CARLA and creates a packaged version for distribution.</td>
</tr>
<tr>
<td><code>make clean</code></td>
<td>Deletes all the binaries and temporals generated by the build system.</td>
</tr>
<tr>
<td><code>make rebuild</code></td>
<td><code>make clean</code> and <code>make launch</code> both in one command.</td>
</tbody>
</table>

---

Read the **[F.A.Q.](build_faq.md)** page or post in the [CARLA forum](https://forum.carla.org/c/installation-issues/linux) for any issues regarding this guide.  

Some recommendations after finishing the build. Learn how to update the CARLA build or take your first steps in the simulation, and learn some core concepts.  
<div class="build-buttons">

<p>
<a href="../build_update" target="_blank" class="btn btn-neutral" title="Learn how to update the build">
Update CARLA</a>
</p>

<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Learn about CARLA core concepts">
First steps</a>
</p>

</div>



