# Windows build

* [__Windows build command summary__](#windows-build-command-summary)  
* [__Requirements__](#requirements)  
	* System specifics
* [__Necessary software__](#necessary-software)  
	* Minor installations: CMake, git, make, Python3 x64  
	* Visual Studio 2017
	* Unreal Engine 4.22 
* [__CARLA build__](#carla-build)  
	* Clone repository  
	* Get assets  
	* make CARLA  

The build process can be quite long and tedious. The **[F.A.Q.](build_faq.md)** section contains the most common issues and solutions that appear during the installation. However, the CARLA forum is open for anybody to post unexpected issues, doubts or suggestions. There is a specific section for installation issues on Linux. Feel free to login and become part of the community. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>

---
## Windows build command summary 

<details>
<summary> Show command lines to build on Windows</summary>

```sh
# Make sure to meet the minimum requirements.

# Necessary software: 
#   CMake
#   Git
#   Make
#   Python3 x64
#   Unreal Engine 4.22
#   Visual Studio 2017 with Windows 8.1 SDK and x64 Visual C++ Toolset.

# Set environment variables for the software

# Clone the CARLA repository
git clone https://github.com/carla-simulator/carla

# make the CARLA server and the CARLA client
make launch
make PythonAPI

# Run an example script to test CARLA. 
cd PythonAPI/Examples && python3 spawn_npc.py
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

### Unreal Engine 4.22

Go to [Unreal Engine](https://www.unrealengine.com/download) and download the _Epic Games Launcher_. In `Engine versions/Library`, download __Unreal Engine 4.22.x__. Make sure to run it in order to check that everything was properly installed.  

!!! Note 
    Having VS2017 and UE4.22 installed, a __Generate Visual Studio project files__ option should appear when doing right-click on __.uproject__ files. If this is not available, something went wrong whith the UE4.22 installation. Create a UE project to check it out and reinstall if necessary. 

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

### make CARLA

Go to the root CARLA folder to make the build. The process may take a while, around 20-40 minutes, it will download and install the necessary libraries. There are different commands to build the different modules.  

!!! Warning
    Make sure to run __make launch__ to prepare the server and __make PythonAPI__ for the client.  
    Alternatively __make libcarla__ will prepare the CARLA library to be imported anywhere. 


### make CARLA

The last step is to finally build CARLA. There are different `make` commands to build the different modules. All of them run in the root CARLA folder.  

!!! Warning
    Make sure to run __make launch__ to prepare the server and __make PythonAPI__ for the client.  
    Alternatively __make libcarla__ will prepare the CARLA library to be imported anywhere. 

* __make launch__ compiles the server simulator and launches Unreal Engine. Press **Play** to start the spectator view and close the editor window to exit. Camera can be moved with `WASD` keys and rotated by clicking the scene while moving the mouse around.  
```sh
make launch
```  
The project may ask to build other instances such as `UE4Editor-Carla.dll` the first time. Agree in order to open the project. During the first launch, the editor may show warnings regarding shaders and mesh distance fields. These take some time to be loaded and the city will not show properly until then.  

* __make PythonAPI__ compiles the API client, necessary to grant control over the simulation. It is only needed the first time. Remember to run it again when updating CARLA. Scripts will be able to run after this command is executed. The following example will spawn some life into the town.  

```sh
make PythonAPI && cd PythonAPI/examples && python3 spawn_npc.py
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
<td>
<code>make help</code> </td>
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
<td>make clean and make launch both in one command.</td>
</tbody>
</table>

---
<br>
Keep reading this section to learn how to update the build or take some first steps in CARLA. 
<div class="build-buttons">
<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Learn about CARLA core concepts">
Go to __First steps__</a>
</p>

<p>
<a href="../build_update" target="_blank" class="btn btn-neutral" title="Learn how to update the build">
Update CARLA</a>
</p>
</div>




