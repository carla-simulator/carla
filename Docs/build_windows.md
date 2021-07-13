# Windows build

This guide details how to build CARLA from source on Windows. There are two parts. Part one details system requirements and installations of required software, and part two details how to actually build and run CARLA.  

The build process is long (4 hours or more) and involves several kinds of software. It is highly recommended to read through the guide fully before starting. 

If you come across errors or difficulties then have a look at the **[F.A.Q.](build_faq.md)** page which offers solutions for the most common complications. Alternatively, use the [CARLA forum](https://github.com/carla-simulator/carla/discussions) to post any queries you may have.

- [__Part One: Prerequisites__](#part-one-prerequisites)
    - [System requirements](#system-requirements)
    - [Software requirements](#software-requirements)
        - [Minor installations](#minor-installations)
        - [Python dependencies](#python-dependencies)
        - [Major installations](#major-installations)
            - [Visual Studio 2017](#visual-studio-2017)
            - [Unreal Engine](#unreal-engine)
- [__Part Two: Build CARLA__](#part-two-build-carla)
    - [Clone the CARLA repository](#clone-the-carla-repository)
    - [Get assets](#get-assets)
    - [Set Unreal Engine environment variable](#set-unreal-engine-environment-variable)
    - [Build CARLA](#build-carla)
    - [Other make commands](#other-make-commands)
- [__Summary__](#summary)


---
## Part One: Prerequisites

In this section you will find details of system requirements, minor and major software installations and Python dependencies that are required before you can begin to build CARLA. 
### System requirements

* __x64 system.__ The simulator should run in any 64 bits Windows system.  
* __165 GB disk space.__ CARLA itself will take around 32 GB and the related major software installations (including Unreal Engine) will take around 133 GB.
* __An adequate GPU.__ CARLA aims for realistic simulations, so the server needs at least a 6 GB GPU although 8 GB is recommended. A dedicated GPU is highly recommended for machine learning. 
* __Two TCP ports and good internet connection.__ 2000 and 2001 by default. Make sure that these ports are not blocked by firewalls or any other applications. 

### Software requirements

#### Minor installations

* [__CMake__](https://cmake.org/download/) generates standard build files from simple configuration files.  
* [__Git__](https://git-scm.com/downloads) is a version control system to manage CARLA repositories.  
* [__Make__](http://gnuwin32.sourceforge.net/packages/make.htm) generates the executables. It is necessary to use __Make version 3.81__, otherwise the build may fail. If you have multiple versions of Make installed, check that you are using version 3.81 in your PATH when building CARLA. You can check your default version of Make by running `make --version`.
* [__7Zip__](https://www.7-zip.org/) is a file compression software. This is required for automatic decompression of asset files and prevents errors during build time due to large files being extracted incorrectly or partially.
* [__Python3 x64__](https://www.python.org/downloads/) is the main scripting language in CARLA. Having a x32 version installed may cause conflict, so it is highly advisable to have it uninstalled.

!!! Important
    Be sure that the above programs are added to the [environment path](https://www.java.com/en/download/help/path.xml). Remember that the path added should correspond to the progam's `bin` directory.  
#### Python dependencies

CARLA requires some Python dependencies to be installed with `pip version >= 20.3`. Ensure you have a compatible version by running:

```sh
pip -V
```

If you need to upgrade:

```sh
pip install --upgrade pip
```

Once you have the correct version of `pip`, you can install the dependencies:

```sh
pip3 install --user setuptools
pip3 install --user wheel
```

#### Major installations
##### Visual Studio 2017

Get the 2017 version of Visual Studio from [here](https://developerinsider.co/download-visual-studio-2017-web-installer-iso-community-professional-enterprise/). Choose __Community__ for the free version. Use the _Visual Studio Installer_ to install three additional elements: 

* __Windows 8.1 SDK.__ Select it in the _Installation details_ section on the right or go to the _Indivdual Components_ tab and look under the _SDKs, libraries, and frameworks_ heading.
* __x64 Visual C++ Toolset.__ In the _Workloads_ section, choose __Desktop development with C++__. This will enable a x64 command prompt that will be used for the build. Check that it has been installed correctly by pressing the `Windows` button and searching for `x64`. Be careful __not to open a `x86_x64` prompt__.  
* __.NET framework 4.6.2__. In the _Workloads_ section, choose __.NET desktop development__ and then in the _Installation details_ panel on the right, select `.NET Framework 4.6.2 development tools`. This is required to build Unreal Engine. 

!!! Important
    Other Visual Studio versions may cause conflict. Even if these have been uninstalled, some registers may persist. To completely clean Visual Studio from the computer, go to `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout` and run `.\InstallCleanup.exe -full`  

##### Unreal Engine

Starting with version 0.9.12, CARLA uses a modified fork of Unreal Engine 4.26. This fork contains patches specific to CARLA.

Be aware that to download this fork of Unreal Engine, __you need to have a GitHub account linked to Unreal Engine's account__. If you don't have this set up, please follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github) before going any further.

To build the modified version of Unreal Engine:

__1.__ In a terminal, navigate to the location you want to save Unreal Engine and clone the _carla_ branch:

```sh
    git clone --depth 1 -b carla https://github.com/CarlaUnreal/UnrealEngine.git .
```

!!! Note 
    Keep the Unreal Engine folder as close as `C:\\` as you can because if the path exceeds a certain length then `Setup.bat` will return errors in step 3.

__2.__ Run the configuration scripts:

```sh
    Setup.bat
    GenerateProjectFiles.bat
```

__3.__ Compile the modified engine:

>1. Open the `UE4.sln` file inside the source folder with Visual Studio 2017.

>2. In the build bar ensure that you have selected 'Development Editor', 'Win64' and 'UnrealBuildTool' options. Check [this guide](https://docs.unrealengine.com/en-US/ProductionPipelines/DevelopmentSetup/BuildingUnrealEngine/index.html) if you need any help. 
        
>3. In the solution explorer, right-click `UE4` and select `Build`.

__4.__ Once the solution is compiled you can open the engine to check that everything was installed correctly by launching the executable `Engine\Binaries\Win64\UE4Editor.exe`.

!!! Note
    If the installation was successful, this should be recognised by Unreal Engine's version selector. You can check this by right-clicking on any `.uproject` file and selecting `Switch Unreal Engine version`. You should see a pop-up showing `Source Build at PATH` where PATH is the installation path that you have chosen. If you can not see this selector or the `Generate Visual Studio project files` when you right-click on `.uproject` files, something went wrong with the Unreal Engine installation and you will likely need to reinstall it correctly.

!!! Important
    A lot has happened so far. It is highly advisable to restart the computer before continuing.

---
## Part Two: Build CARLA 
 
### Clone the CARLA repository

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla" target="_blank" class="btn btn-neutral" title="Go to the CARLA repository">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>

The button above will take you to the official repository of the project. Either download from there and extract it locally or clone it using the following command:

```sh
    git clone https://github.com/carla-simulator/carla
```

!!! Note
    The `master` branch contains the current release of CARLA with the latest fixes and features. Previous CARLA versions are tagged with the version name. Always remember to check the current branch in git with the command `git branch`. 

### Get assets

Download the __latest__ assets to work with the current version of CARLA by running the following command in the CARLA root folder:

```sh
    Update.bat
```

The assets will be downloaded and extracted to the appropriate location if have 7zip installed. If you do not have this software installed, you will need to manually extract the file contents to `Unreal\CarlaUE4\Content\Carla`.

To download the assets for a __specific version__ of CARLA:

1. From the root CARLA directory, navigate to `\Util\ContentVersions.txt`. This document contains the links to the assets for all CARLA releases. 
2. Extract the assets in `Unreal\CarlaUE4\Content\Carla`. If the path doesn't exist, create it.  
3. Extract the file with a command similar to the following:

```sh
    tar -xvzf <assets_file_name>.tar.gz.tar -C C:\path\to\carla\Unreal\CarlaUE4\Content\Carla
```

### Set Unreal Engine environment variable

It is necessary to set an environment variable so that CARLA can find the Unreal Engine installation folder. This allows users to choose which specific version of Unreal Engine is to be used. If no environment variable is specified, then CARLA will search for Unreal Engine in the windows registry and use the first version it finds there.  

To set the environment variable:

1. Open Windows Control Panel and go to `Advanced System Settings` or search for `Advanced System Settings` in the Windows search bar.  
2. On the `Advanced` panel open `Environment Variables...`.  
3. Click `New...` to create the variable.  
4. Name the variable `UE4_ROOT` and choose the path to the installation folder of the desired Unreal Engine installation.  


### Build CARLA

This section outlines the commands to build CARLA. 

- All commands should be run in the root CARLA folder. 
- Commands should be executed via the __x64 Native Tools Command Prompt for VS 2017__. Open this by clicking the `Windows` key and searching for `x64`.

There are two parts to the build process for CARLA, compiling the client and compiling the server.

__1.__ __Compile the Python API client__:

The Python API client grants control over the simulation. Compilation of the Python API client is required the first time you build CARLA and again after you perform any updates. After the client is compiled, you will be able to run scripts to interact with the simulation.

The following command compiles the Python API client:

```sh
    make PythonAPI
```

Optionally, to compile the PythonAPI for a specific version of Python, run the below command in the root CARLA directory. Python versions 2.7, 3.6, 3.7, and 3.8 are supported.

```sh
    # Delete versions as required
    make PythonAPI ARGS="--python-version=2.7, 3.6, 3.7, 3.8"
```

Every time you run `make PythonAPI` a `.whl` file containing the CARLA client library will be built and installed for your system. This `.whl` cannot be distributed as it is specific for your OS. If you had a previous library installed, it will be uninstalled, and a new one will be installed according to the source code you are using. You can modify this behavior with the following flag:

- `make PythonAPI --no-install-wheel` will only build the `.whl` file but not install it on your system.

!!! Important
    Previous versions of CARLA created a `.egg` file containing the CARLA client library. __In versions 0.9.12+ this behavior changes significantly; `.egg` files are no longer used.__ `make PythonAPI` will install the library using `pip` and a `.whl` file.

    If you are building a version of CARLA prior to 0.9.12, please select the correct version of the documentation in the bottom right-hand corner.


__2.__ __Compile the server__:

The following command compiles and launches Unreal Engine. Run this command each time you want to launch the server or use the Unreal Engine editor:

```sh
    make launch
```

The project may ask to build other instances such as `UE4Editor-Carla.dll` the first time. Agree in order to open the project. During the first launch, the editor may show warnings regarding shaders and mesh distance fields. These take some time to be loaded and the map will not show properly until then.

__3.__ __Start the simulation__:

Press **Play** to start the server simulation. The camera can be moved with `WASD` keys and rotated by clicking the scene while moving the mouse around.  

Test the simulator using the example scripts inside `PythonAPI\examples`.  With the simulator running, open a new terminal for each script and run the following commands to spawn some life into the town and create a weather cycle:

```sh
        # Terminal A 
        cd PythonAPI\examples
        pip install -r requirements.txt
        python3 spawn_npc.py  

        # Terminal B
        cd PythonAPI\examples
        python3 dynamic_weather.py 
```

!!! Important
    If the simulation is running at a very low FPS rate, go to `Edit -> Editor preferences -> Performance` in the Unreal Engine editor and disable `Use less CPU when in background`.

### Other make commands

There are more `make` commands that you may find useful. Find them in the table below:  

| Command | Description |
| ------- | ------- |
| `make help`                                                           | Prints all available commands.                                        |
| `make launch`                                                         | Launches CARLA server in Editor window.                               |
| `make PythonAPI`                                                      | Builds the CARLA client.                                              |
| `make LibCarla`                                                       | Prepares the CARLA library to be imported anywhere.                   |
| `make package`                                                        | Builds CARLA and creates a packaged version for distribution.         |
| `make clean`                                                          | Deletes all the binaries and temporals generated by the build system. |
| `make rebuild`                                                        | `make clean` and `make launch` both in one command.                   |

---

## Summary

Below is a summary of the requirements and commands needed to build CARLA on Windows:

```sh
# Make sure to meet the minimum requirements:
#
# x64 system
# 165 GB disk space
# 6 - 8 GB GPU
# Two TCP ports and good internet connection

# Necessary software: 
#   CMake
#   Git
#   Make
#   Python3 x64
#   pip>=20.3
#   Modified Unreal Engine 4.24
#   Visual Studio 2017 with Windows 8.1 SDK, x64 Visual C++ Toolset and .NET framework 4.6.2

# Set environment variables for the software

# Upgrade to pip>=20.3
pip install --upgrade pip

# Install Python library dependencies:

pip3 install --user setuptools
pip3 install --user wheel

# Clone the CARLA repository
git clone https://github.com/carla-simulator/carla

# Get assets
# Set UE4_ROOT environment variable 

# make the CARLA client and the CARLA server
# open the x64 Native Tools Command Prompt for VS 2017 to execute the following commands
make PythonAPI
make launch

# Press play in the Editor to initialize the server
# Run example scripts to test CARLA
# Terminal A 
cd PythonAPI\examples
pip install -r requirements.txt
python3 spawn_npc.py 
# Terminal B
cd PythonAPI\examples
python3 dynamic_weather.py 
```

---

Read the **[F.A.Q.](build_faq.md)** page or post in the [CARLA forum](https://github.com/carla-simulator/carla/discussions) for any issues regarding this guide.  

Now that you have built CARLA, learn how to update the CARLA build or take your first steps in the simulation, and learn some core concepts.

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



