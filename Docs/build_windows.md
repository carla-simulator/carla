# Windows build
  * [__Requirements__](#requirements):  
	* System specifics
  * [__Necessary software__](#necessary-software):  
	* Minor installations: CMake, git, make, Python3 x64  
	* Visual Studio 2017
	* Unreal Engine 4.22 
  * [__CARLA build__](#carla-build): 
	* Clone repository  
	* Get assets  
	* make CARLA

The build process can be quite long and tedious. This documentation tries to make things clear and provides for a **[F.A.Q.](build_faq.md)** with solutions for the most common starting issues. However, the CARLA forum is open for anybody to post unexpected issues, doubts or suggestions. There is a specific section for installation issues on Windows. Feel free to login and become part of the community. 

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</p>
</div>

---
## Requirements
#### System specifics

  * __x64 system:__ The simulator should run in any Windows system currently available as long as it is a 64 bits OS. 
  * __30GB disk space:__ Installing all the software needed and CARLA itself will require quite a lot of space, especially Unreal Engine. Make sure to have around 30/50GB of free disk space.  
  * __An adequate GPU:__ CARLA aims for realistic simulations, so the server needs at least a 4GB GPU. A dedicated GPU is highly recommended for machine learning. 
  * __Two TCP ports and good internet connection:__ 2000 and 2001 by default. Be sure neither firewall nor any other application are blocking these. 

---
## Necessary software
#### Minor installations

Some software is needed for the build process the installation of which is quite straightforward.  

* [CMake](https://cmake.org/download/): Generates standard build files from simple configuration files.
* [Git](https://git-scm.com/downloads): Version control system to manage CARLA repositories. 
* [Make](http://gnuwin32.sourceforge.net/packages/make.htm): Executable generator. 
* [Python3 x64](https://www.python.org/downloads/): Main script language that CARLA provides support to. Having a x32 version installed may cause conflict even if the x64 is also there, so it is highly advisable to uninstall these.  

!!! Important
    Be sure that these programs are added to your [environment path](https://www.java.com/en/download/help/path.xml), so you can use them from your command prompt. The path values to add lead to the _bin_ directories for each software. 


#### Visual Studio 2017

Get the 2017 version from [here](https://developerinsider.co/download-visual-studio-2017-web-installer-iso-community-professional-enterprise/). **Community** is the free version. Two elements will be needed to set up the environment for the build process. These must be added when using the Visual Studio Installer:  

* **Windows 8.1 SDK**: On the right there is an _Installation details_ section. Find this SDK and select it. 
* **x64 Visual C++ Toolset**: In the main _Workloads_ section, choose **Desktop development with C++**. Thus will enable a x64 command prompt that will be used for the build. After installing, check this up by pressing the `Win` button and search for `x64` (be careful to **not open a `x86_x64` prompt**). The name of this command prompt can change a bit depending on the Visual Studio installed.  

!!! Important
    Having other Visual Studio versions may cause conflict during the build process, even if these have been uninstalled (Visual Studio is not that good at getting rid of itself and erasing registers). To completely clean Visual Studio from the computer run `.\InstallCleanup.exe -full` found in `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout`. This may need admin permissions.   


#### Unreal Engine 4.22

Go to the [Unreal Engine](https://www.unrealengine.com/download) site and download the Epic Games Launcher. In the _Library_ section, inside the _Engine versions_ panel, download any Unreal Engine 4.22.x version. After installing it, make sure to run it in order to be sure that everything was properly installed.   

!!! Note
    This note will only be relevant if issues arise during the build process and manual build is required. Having VS2017 and UE4.22 installed, a **Generate Visual Studio project files** option should appear when doing right-click on **.uproject** files. If this option is not available, something went wrong while installing Unreal Engine and it may need to be reinstalled. Create a simple Unreal Engine project to check up.  

---
## CARLA build

!!! Important
    Lots of things have happened so far. It is highly advisable to do a quick restart of the system.  

#### Clone repository

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://github.com/carla-simulator/carla" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>

The project can be found in GitHub. It should be either downloaded and extracted or cloned on the local computer. To do so, open the **x64 terminal** previously installed and go to the path where CARLA will be installed. Then, run the following command: 

```cmd
git clone https://github.com/carla-simulator/carla
```

Now the latest content for the project, known as `master` branch in the repository, has been copied in local. 

!!! Note
    The `master` branch contains the latest fixes and features. Stable code is inside the `stable` branch, and it can be built by changing the branch. The same goes for previous CARLA releases. Always remember to check the current branch in git with `git branch`. 

#### Get assets

Only the assets package, the visual content, is yet to be downloaded. `\Util\ContentVersions.txt` contains the links to the assets for every CARLA version. These must be extracted in `Unreal\CarlaUE4\Content\Carla`. If the path doesn't exist, create it.  
Download the **latest** assets to work with the current version of CARLA. When working with branches containing previous releases of CARLA, make sure to download the proper assets.

#### make CARLA

Go to the root CARLA folder, the one cloned from the repository. It is time to do the automatic build. The process may take a while, it will download and install the necessary libraries. It might take 20-40 minutes, depending on hardware and internet connection. There are different make commands to build the different modules:   

!!! Warning
    Make sure to run __make launch__ to prepare the server and __make PythonAPI__ for the client.  
    Alternativelly __make libcarla__ will prepare the CARLA library to be imported anywhere. 

* **launch** will compile the server simulator and launch it in Unreal Engine. Press Play to start the spectator view and close the editor window to exit. Camera can be moved with WASD keys and rotated by clicking the scene while moving the mouse around: 
```cmd
make launch
```
The project may ask to build other instances such as `UE4Editor-Carla.dll` the first time. Agree in order to open the project. Also, during the first launch Unreal Editor may show warnings regarding shaders and mesh distance fields. These take some time to be loaded and the city will not show properly until then.  

* **PythonAPI** will compile the API client, necessary to grant control over the simulation. It is only needed the first time (remember to run it again when the CARLA code is updated). After building it, scripts can run. The following example will spawn some life into the city:   
```cmd
make PythonAPI && cd PythonAPI/Examples && python3 spawn_npc.py
```

!!! Note
    If the simulation is running at very low FPS rates, open the UE4.22 editor and go to "Edit>Editor preferences>Performance" and disable: **Use less CPU when in background**. 

Now everything is ready to go and CARLA has been successfully built. Here is a brief summary of the most useful `make` commands available:  

| Command          | Description  |
| ---------------- | --------------------- |
| `make help`      | Prints all available commands. |
| `make launch`    | Launches CARLA Editor. |
| `make PythonAPI` | Builds the CARLA client. |
| `make package`   | Builds CARLA and creates a packaged version for distribution. |
| `make clean`     | Deletes all the binaries and temporals generated by the build system. |
| `make rebuild`   | make clean and make launch both in one command. |  

<br>
Keep reading this section to learn more about how to update CARLA, the build itself and some advanced configuration options.
Otherwise, visit the __First steps__ section to learn about CARLA: 
<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Start reading First steps">
Go to First steps</a>
</p>
</div>



