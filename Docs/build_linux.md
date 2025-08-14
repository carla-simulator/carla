# Linux build

This guide details how to build CARLA from source on Linux. The build process is long (4 hours or more) and involves several kinds of software. It is highly recommended to read through the guide fully before starting. 

If you come across errors or difficulties then have a look at the **[F.A.Q.](build_faq.md)** page which offers solutions for the most common complications. Alternatively, use the [CARLA forum](https://github.com/carla-simulator/carla/discussions) to post any queries you may have.

- [__Prerequisites__](#part-one-prerequisites)
    - [System requirements](#system-requirements)
    - [Software requirements](#software-requirements)
- [__Building Unreal Engine__](#building-unreal-engine)
- [__Building CARLA__](#building-carla)
    - [Clone the CARLA repository](#clone-the-carla-repository)
    - [Download the CARLA content](#download-the-carla-content)
    - [Set the Unreal Engine environment variable](#set-the-unreal-engine-environment-variable)
    - [Build CARLA with Make](#build-carla-with-make)
        - [Compile the Python API client](#1-compile-the-python-api-client)
        - [Compile the server](#2-compile-the-server)
        - [Start the simulation](#3-start-the-simulation)
    - [Additional Make options](#additional-make-options)

---
## Part One: Prerequisites

### System requirements

* __Ubuntu 20.04 or 22.04__: The current dev branch of CARLA is tested regularly on Ubuntu 20.04 and Ubuntu 22.04. It may be possible to build CARLA in earlier Ubuntu versions but we recommend a minimum of version 20.04. CARLA has not been tested internally in Ubuntu 24.04, therefore we recommend to stay with a maximum of Ubuntu 22.04.
* __130 GB disk space__: CARLA will take around 31 GB and Unreal Engine will take around 91 GB so have about 130 GB free to account for both of these plus additional minor software installations. 
* __A high-performance GPU__: CARLA places a high demand on the GPU, therefore it is recommended to use a minimum of an NVIDIA RTX 2000 series (e.g. 2070) or better with at least 6 Gb of VRAM, preferably 8 Gb or more.
* __A high-performance CPU__: CARLA also benefits from a CPU with solid performance. We recommend a minimum of an Intel Core i7 with 4 or more cores (or equivalent). 
* __Two TCP ports and good internet connection__: 2000 and 2001 by default. Make sure that these ports are not blocked by firewalls or any other applications. 
* __Python 3.8 or higher__ is recommended.

### Software requirements

CARLA requires numerous software tools for compilation. Some are built during the CARLA build process itself, such as *Boost.Python*. Others are binaries that should be installed before starting the build (*cmake*, *clang*, different versions of *Python*, etc.). To install these requirements, run the following commands:

```sh
sudo apt-get update &&
sudo apt-get install wget software-properties-common &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test &&
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add
```

To avoid compatibility issues between Unreal Engine and the CARLA dependencies, use the same compiler version and C++ runtime library to compile everything. The CARLA team uses clang-10 and LLVM's libc++. Change the default clang version to compile Unreal Engine and the CARLA dependencies.

#### Ubuntu 22.04
```sh
sudo apt-add-repository "deb http://archive.ubuntu.com/ubuntu jammy main universe"
sudo apt-get update
sudo apt-get install build-essential clang-10 lld-10 g++-12 cmake ninja-build libvulkan1 python python3 python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev git git-lfs
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-10/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-10/bin/clang 180 &&
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 180
```

#### Ubuntu 20.04
```sh
sudo apt-add-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal main"
sudo apt-get update
sudo apt-get install build-essential clang-10 lld-10 g++-7 cmake ninja-build libvulkan1 python python-dev python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev git
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-10/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-10/bin/clang 180
```

#### Python dependencies

The CARLA build creates Python wheels for the API. These can be installed using PIP. Version 20.3 or higher is required. To check if you have a suitable version, run the following command:

```sh
pip3 -V
```

If you need to upgrade:

```sh
pip3 install --upgrade pip
```

You also need to install some Python dependencies:

```sh
pip3 install --user -r CARLA_ROOT/PythonAPI/carla/requirements.txt
```

---

## Building Unreal Engine

This version of CARLA uses a modified fork of Unreal Engine 4.26. This fork contains patches specific to CARLA.

Be aware that to download this fork of Unreal Engine, __you need to have a GitHub account linked to the Epic Games organization__. If you don't have this link already set up, please follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github) before going any further.

__1.__ Clone the content for CARLA's fork of Unreal Engine 4.26 to your local computer:

```sh
    git clone --depth 1 -b carla https://github.com/CarlaUnreal/UnrealEngine.git ~/UnrealEngine_4.26
```
!!! Note
    Since github doesn't allow authentication with usename/password anymore, a personal authentication token can be used to clone the UnrealEngine repository. Here's the command to clone with OAuth.

```sh
    git clone --depth 1 -b carla https://oauth2:TOKEN@github.com/CarlaUnreal/UnrealEngine.git ~/UnrealEngine_4.26
```

__2.__ Navigate into the directory where you cloned the Unreal Engine repository:
```sh
    cd ~/UnrealEngine_4.26
```

__3.__ Set up and build with `make`. This may take an hour or two depending on your system. 
```sh
    ./Setup.sh && ./GenerateProjectFiles.sh && make
```
!!! Warning
    Do not use `-j` tag to use all processor cores, e.g., `make -j$(nproc)`. This will cause the build to fail. Clang will use all available cores anyway.  

__4.__ Open the Editor to check that Unreal Engine has been installed properly.
```sh
    cd ~/UnrealEngine_4.26/Engine/Binaries/Linux && ./UE4Editor
```

---

## Building CARLA 

!!! Note
    Downloading aria2 with `sudo apt-get install aria2` will speed up the following commands.

### Clone the CARLA repository

Clone the `ue4-dev` branch of the CARLA repository with the following command:

```sh
        git clone -b ue4-dev https://github.com/carla-simulator/carla
```

You can download the repository as a ZIP archive directly from the [CARLA GitHub repository page](https://github.com/carla-simulator/carla) if you prefer not to use Git.

!!! Note
    The `master` branch contains the latest official release of CARLA, while the `ue4-dev` branch has all the latest development updates. Previous CARLA versions are tagged with the version name. Always remember to check the current branch in git with the command `git branch`. 


### Download the CARLA content

CARLA comes with a large repository of 3D assets including maps, vehicles and pedestrians. To work on a build-from source version of CARLA you need to download a version of the content corresponding to your current update of the CARLA code.

If you are working on the latest updates of the `ue4-dev` branch you will need to download the latest version of the content. There are two ways to achieve this:

__1. Using the content update script__: This script downloads the latest package of the CARLA content as a `tar.gz` archive and decompresses the archive into the `CARLA_ROOT/Unreal/CarlaUE4/Content/Carla` directory:

```sh
./Update.sh
```

__2. Using Git__: Using Git, you will establish a git repository for the content in the `CARLA_ROOT/Unreal/CarlaUE4/Content/Carla` directory. **This is the preferred method if you intend to commit content updates to CARLA (or your own fork of CARLA)**. From the root directory of the CARLA code repository, run the following command (if you have your own fork of the CARLA content, change the target remote repository accordingly):

```sh
git clone https://bitbucket.org/carla-simulator/carla-content Unreal/CarlaUE4/Content/Carla
```

#### Downloading the assets in an archive for a specific CARLA version

You may want to download the assets for a specific CARLA version for some purposes:

1. From the root CARLA directory, navigate to `/Util/ContentVersions.txt`. This document contains the links to the assets for all CARLA releases. 
2. Extract the assets in `Unreal\CarlaUE4\Content\Carla`. If the path doesn't exist, create it.  
3. Extract the file with a command similar to the following:

```sh
tar -xvzf <assets_archive>.tar.gz.tar -C /path/to/carla/Unreal/CarlaUE4/Content/Carla
```

### Set the Unreal Engine environment variable

For CARLA to locate the correct installation of Unreal Engine, an environment variable is needed.

To set the variable for this session only:

```sh
export UE4_ROOT=~/UnrealEngine_4.26
```

You may want to set the environment variable in your `.bashrc` or `.profile`, so that it is always set. Open `.bashrc` or `.profile` with `gedit` and add the line above near the end of the file and save:

```sh
cd ~
gedit .bashrc # or .profile
```

---

### Build CARLA with Make

The following commands should be run from the root folder of the CARLA repository that you earlier downloaded or cloned with git. There are two parts to the build process for CARLA, compiling the client and compiling the server.

#### 1. Compile the Python API client

The Python API client grants control over the simulation. Compilation of the Python API client is required the first time you build CARLA and again after you perform any updates. After the client is compiled, you will be able to run scripts to interact with the simulation.

The following command compiles the Python API client:

```sh
make PythonAPI
```

**Building the Python API for a specific Python version**

The above command will build the Python API for the system Python version. You can target a specific version of Python up to 3.11 with the following instructions:

* Install the target Python version at the system level with the development headers and the distutils, replace *X* with the desired version:

```
# The Deadsnakes PPA may be needed for some Python versions
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt update

# Replace the X for the desired Python version
sudo apt install python3.X python3.X-dev python3.X-distutils
```

* We recommend to disable any virtual environment managers such as PyEnv, Rye or Conda. These might interfere with the installation.

* Then run the following command in the root CARLA root directory: 

```sh
# Delete versions as required
make PythonAPI ARGS="--python-version=3.8, 3.9, 3.10, 3.11"
```

* **For Python 3.12 and 3.13**: the *distutils* library does not exist for Ubuntu and therefore *setuptools* may
 need to be updated, run the following commands to update *setuptools*:

```sh
sudo apt install python3.12 python3.12-dev python3.12-venv
python3.12 -m ensurepip --upgrade
python3.12 -m pip install --upgrade pip setuptools

# Then run make from the CARLA root directory
make PythonAPI ARGS="--python-version=3.12"
```

* If you are using a non-standard Python installation or a Python virtual environment manager like PyEnv, Rye or Conda. Instead of the `--python-version` argument it may be better to use the `--python-root` argument (you can locate the installation using `which python3`):

```sh
make PythonAPI ARGS="--python-root=/path/to/python/installation"
```

The CARLA Python API wheel will be generated in `CARLA_ROOT/PythonAPI/carla/dist`. The name of the wheel will depend upon the current CARLA version and the chosen Python version. Install the wheel with PIP:

```sh
# CARLA 0.9.16, Python 3.8
pip3 install CARLA_ROOT/PythonAPI/carla/dist/carla-0.9.16-cp38-linux_x86_64.whl

# CARLA 0.9.16, Python 3.10
#pip3 install CARLA_ROOT/PythonAPI/carla/dist/carla-0.9.16-cp310-linux_x86_64.whl
```

!!! Warning
    Issues can arise through the use of different methods to install the CARLA client library and having different versions of CARLA on your system. It is recommended to use virtual environments when installing the `.whl` and to [uninstall](build_faq.md#how-do-i-uninstall-the-carla-client-library) any previously installed client libraries before installing new ones.

#### 2. Compile the server

The following command compiles and launches the Unreal Engine editor. Run this command each time you want to launch the server or use the Unreal Engine editor:

```sh
make launch
```

During the first launch, the editor may show warnings regarding shaders and mesh distance fields. These take some time to be loaded and the map will not show properly until then. Subsequent launches of the editor will be quicker.

![ue4_editor_open](img/ue4_editor_open.png)

#### 3. Start the simulation

Press **Play** to start the server simulation. The camera can be moved with `WASD` keys and rotated by clicking the scene while moving the mouse around.  

Test the simulator using the example scripts inside `PythonAPI\examples`.  With the simulator running, open a new terminal for each script and run the following commands to spawn some life into the town and create a weather cycle:

```sh
# Terminal A 
cd PythonAPI/examples
python3 -m pip install -r requirements.txt
python3 generate_traffic.py  

# Terminal B
cd PythonAPI/examples
python3 dynamic_weather.py 
```

!!! Important
    If the simulation is running at a very low FPS rate, go to `Edit -> Editor preferences -> Performance` in the Unreal Engine editor and disable `Use less CPU when in background`.

---

### Additional Make options

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

Read the **[F.A.Q.](build_faq.md)** page or post in the [CARLA forum](https://github.com/carla-simulator/carla/discussions) for any issues regarding this guide.  

Up next, learn how to update the CARLA build or take your first steps in the simulation, and learn some core concepts.  
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
