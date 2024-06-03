!!! warning
                This is a work in progress!! This version of CARLA is not considered a stable release. Over the following months many significant changes may be made to this branch which could break any modifications you make. We advise you to treat this branch as experimental.

# Building CARLA in Linux with Unreal Engine 5.3

## Set up the environment

This guide details how to build CARLA from source on Linux with Unreal Engine 5. 

Clone the `ue5-dev` branch of CARLA on your local machine:

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

Run the setup script:

```sh
cd CarlaUE5
bash -x Setup.sh
```

The Setup.sh script installs all the required packages, including Cmake, debian packages, Python packages and Unreal Engine 5. It also downloads the CARLA content and builds CARLA. This script can therefore take a long time to complete. 

!!! note
                * This version of CARLA requires the **CARLA fork of Unreal Engine 5.3**. You need to link your GitHub account to Epic Games in order to gain permission to clone the UE repository. If you have not already linked your accounts, follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github)
                * For using CARLA Unreal Engine 5 previous builds, ensure CARLA_UNREAL_ENGINE_PATH environment variable is defined pointing to the CARLA Unreal Engine 5 absolute path. If this variable is not defined, Setup.sh script will download and build CARLA Unreal Engine 5 and this takes more than 1 extra hour of build and a lot of disk space.
                * Setup.sh script checks if there is any python installed at the top of the PATH variable, and installs python otherwise. To use your own version of python, ensure having the PATH variable properly set for python before running the script.
                * CARLA cannot be built on an external disk, Ubuntu is not giving the required read/write/execution permissions for builds.


## Build and Run CARLA UE5

The setup script launches the following commands itself, you will need to use the following commands once you modify the code and wish to relaunch:

* Configure:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/LinuxToolchain.cmake -DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
```

* Build CARLA:

```sh
cmake --build Build
```

* Build and install the Python API:

```sh
cmake --build Build --target carla-python-api-install
```

* Run the editor:

```sh
cmake --build Build --target launch