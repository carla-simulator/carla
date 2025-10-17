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
    - [Build CARLA with Make](#build-carla-with-make)
        - [Compile the Python API client](#1-compile-the-python-api-client)
        - [Compile the server](#2-compile-the-server)
        - [Start the simulation](#3-start-the-simulation)
    - [Additional Make options](#additional-make-options)
    - [Running tests](#running-tests)

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

CARLA requires numerous software tools for compilation. Some are built during the CARLA build process itself, such as *Boost.Python*. Others are binaries that should be installed before starting the build (*cmake*, different versions of *Python*, etc.). 

#### Ubuntu 22.04
```sh
sudo apt-get update
sudo apt-get install build-essential g++-12 cmake ninja-build libvulkan1 python3 python3-dev python3-pip python3-venv autoconf wget curl rsync unzip git git-lfs libpng-dev libtiff5-dev libjpeg-dev
```

#### Ubuntu 20.04
```sh
sudo apt-get update
sudo apt-get install build-essential g++-9 cmake ninja-build libvulkan1 python3 python3-dev python3-pip python3-venv autoconf wget curl rsync unzip git git-lfs libpng-dev libtiff5-dev libjpeg-dev
```

## Building Unreal Engine

This version of CARLA uses a modified fork of Unreal Engine 4.26. This fork contains patches specific to CARLA.

Be aware that to download this fork of Unreal Engine, __you need to have a GitHub account linked to the Epic Games organization__. If you don't have this link already set up, please follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github) before going any further.

__1.__ **Clone the content for CARLA's fork of Unreal Engine 4.26 to your local computer**:

```sh
git clone --depth 1 -b carla https://github.com/CarlaUnreal/UnrealEngine.git ~/UnrealEngine_4.26
```
!!! Note
    Since github doesn't allow authentication with usename/password anymore, a personal authentication token can be used to clone the UnrealEngine repository. Here's the command to clone with OAuth.

```sh
git clone --depth 1 -b carla https://oauth2:TOKEN@github.com/CarlaUnreal/UnrealEngine.git ~/UnrealEngine_4.26
```

__2.__ **Navigate into the directory where you cloned the Unreal Engine repository**:
```sh
cd ~/UnrealEngine_4.26
```

__3.__ **Set up and build with `make`. This may take an hour or two depending on your system**:
```sh
./Setup.sh && ./GenerateProjectFiles.sh && make
```
!!! Warning
    Do not use `-j` tag to use all processor cores, e.g., `make -j$(nproc)`. This will cause the build to fail. Clang will use all available cores anyway.  

__4.__ **Open the Editor to check that Unreal Engine has been installed properly**:
```sh
cd ~/UnrealEngine_4.26/Engine/Binaries/Linux && ./UE4Editor
```

__5.__ S**et the Unreal Engine environment variable**:

For CARLA to locate the correct installation of Unreal Engine, an environment variable is needed.

To set the variable for this session only:

```sh
export UE4_ROOT=~/UnrealEngine_4.26
```

You may want to set the environment variable in your `.bashrc` or `.profile`, so that it is always set. Open `.bashrc` or `.profile` with `gedit` and add the line above near the end of the file and save:

```sh
cd ~
gedit .bashrc # or .profile
#OR, from the command line:
#echo "export UE4_ROOT=~/UnrealEngine_4.26" >> ~/.bashrc
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

### Set up the CARLA_UE4_ROOT environment variable

For the following build commands, it is convenient to create a `CARLA_UE4_ROOT` environment variable to locate the root folder of the CARLA repository where you have cloned the code. Run the following command in your shell (you may also want to add it to `.bashrc` or `.profile` for future sessions):

```sh
export CARLA_UE4_ROOT=/path/to/carla/folder
```

If you choose not to use an environment variable, replace `${CARLA_UE4_ROOT}` in the following commands with the appropriate directory locationl.

### Download the CARLA content

CARLA comes with a large repository of 3D assets including maps, vehicles and pedestrians. To work on a build-from source version of CARLA you need to download a version of the content corresponding to your current update of the CARLA code.

If you are working on the latest updates of the `ue4-dev` branch you will need to download the latest version of the content. There are two ways to achieve this:

__1. Using the content update script__: This script downloads the latest package of the CARLA content as a `tar.gz` archive and decompresses the archive into the `${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla` directory:

```sh
./Update.sh
```

__2. Using Git__: Using Git, you will establish a git repository for the content in the `${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla` directory. **This is the preferred method if you intend to commit content updates to CARLA (or your own fork of CARLA)**. From the root directory of the CARLA code repository, run the following command (if you have your own fork of the CARLA content, change the target remote repository accordingly):

```sh
git clone -b master https://bitbucket.org/carla-simulator/carla-content ${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla
```

#### Downloading the assets in an archive for a specific CARLA version

You may want to download the assets for a specific CARLA version for some purposes:

1. From the root CARLA directory, navigate to `${CARLA_UE4_ROOT}/Util/ContentVersions.txt`. This document contains the links to the assets for all CARLA releases. 
2. Extract the assets in `${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla`. If the path doesn't exist, create it.  
3. Extract the file with a command similar to the following:

```sh
tar -xvzf <assets_archive>.tar.gz.tar -C ${CARLA_UE4_ROOT}$/Unreal/CarlaUE4/Content/Carla
```

---

### Build CARLA with Make

The following commands should be run from the root folder of the CARLA repository that you earlier downloaded or cloned with git. There are two parts to the build process for CARLA, compiling the client and compiling the server.

#### 1. Compile the Python API client

The Python API client grants control over the simulation. Compilation of the Python API client is required the first time you build CARLA and again after you perform any updates. After the client is compiled, you will be able to run scripts to interact with the simulation.

Install the Python prerequisites:

```sh
python3 -m pip install --upgrade -r ${CARLA_UE4_ROOT}/PythonAPI/carla/requirements.txt
```

Then build the Python API with the following command:

```sh
make PythonAPI
``` 

!!! note
    **NumPy 2 error**: If the Python installation or environment that you are using to build CARLA has *numpy>=2.0.0* installed, this will cause an error during the build process due to conflicting dependencies. This should be the first thing to check when encountering errors related to Boost. Check your NumPy version using `python3 -m pip show numpy`.

**Building the Python API for a specific Python version**

The above command will compile CARLA with the system default Python version, which is called when you run `python3` on the command line. If you wish to build CARLA for other Python versions, we recommend you use virtual environments.

For some Python versions, the Deadsnakes repository may be needed:

```sh
# The Deadsnakes PPA may be needed for some Python versions
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt update
```

For the version of Python you are targeting, install Python along with the development headers and the Python `venv` package:

```sh
sudo apt-get install python3.X python3.X-dev python3.X-venv # Replace X with correct version number
```

Create a new virtual environment for your target Python version:

```sh
#Replace X with the appropriate Python version number and "myenv" with a name of your choice
python3.X -m venv myenv
```

Activate the new virtual environment, then install the CARLA requirements:

```sh
source myenv/bin/activate
(myenv): python3 -m pip install --upgrade -r ${CARLA_UE4_ROOT}/PythonAPI/carla/requirements.txt
```

Finally, run `make PythonAPI` with the Python virtual environment activated:

```sh
(myenv): make PythonAPI
```

The CARLA Python API wheel will be generated in `${CARLA_UE4_ROOT}/PythonAPI/carla/dist`. The name of the wheel will depend upon the current CARLA version and the chosen Python version. Install the wheel with PIP:

```sh
# CARLA 0.9.16, Python 3.8
python3 -m pip install ${CARLA_UE4_ROOT}/PythonAPI/carla/dist/carla-0.9.16-cp38-linux_x86_64.whl

# CARLA 0.9.16, Python 3.10
#python3 -m pip install ${CARLA_UE4_ROOT}/PythonAPI/carla/dist/carla-0.9.16-cp310-linux_x86_64.whl
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

!!! note
    **NumPy 2 error**: `make launch` can be affected by the NumPy 2 conflict, check the NumPy version in your Python installation using PIP: `python3 -m pip show numpy`. If it is version *2.0.0* or later, you will need to downgrade to *numpy<2.0.0*.

#### 3. Start the simulation

Press **Play** to start the server simulation. The camera can be moved with `WASD` keys and rotated by clicking the scene while moving the mouse around.  

Test the simulator using the example scripts inside `PythonAPI\examples`.  With the simulator running, open a new terminal for each script and run the following commands to spawn some life into the town and create a weather cycle:

```sh
# Terminal A 
cd ${CARLA_UE4_ROOT}/PythonAPI/examples
python3 -m pip install -r requirements.txt
python3 generate_traffic.py  

# Terminal B
cd ${CARLA_UE4_ROOT}/PythonAPI/examples
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

### Running tests

CARLA's code comes with a suite of tests designed to detect regressions in fundamental functionality that might be introduced by new code changes. CARLA's CI/CD system runs these tests for each nightly build and release before uploading new packages. If you are managing your own build we recommend that you run the test suite at least periodically to detect breaking changes.

First, create a CARLA package from your latest changes:

```sh
make package
```

The Package will be created in the Dist folder, it will have a name dependent on the last commit, run the simulator from the newly build package. Substitute the appropriate package ID, which will depend on the latest commit:

```sh
./Dist/CARLA_<package_id>/LinuxNoEditor/CarlaUE4.sh --ros2 -RenderOffScreen --carla-rpc-port=<port> --carla-streaming-port=0 -nosound
```

Once the simulator is running, run the smoke tests:

```sh
make smoke_tests ARGS="--xml --python-version=<python_version> --target-wheel-platform=manylinux_2_31_x86_64
```

Then, finally, run the examples:

```sh
make run-examples ARGS="localhost <port>"
```

You will be alerted on the command line if any tests fail. You can find the smoke tests in `${CARLA_ROOT}/PythonAPI/test/smoke`. 

---

Read the **[F.A.Q.](build_faq.md)** page or post in the [CARLA forum](https://github.com/carla-simulator/carla/discussions) for any issues regarding this guide.  

Up next, learn how to update the CARLA build or take your first steps in the simulation, and learn some core concepts.  
<div class="build-buttons">

<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Learn about CARLA core concepts">
First steps</a>
</p>

</div>
