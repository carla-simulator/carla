# Building CARLA in Linux with Unreal Engine 5.5

!!! note
        The Unreal Engine 5 version of CARLA requires Ubuntu version 22.04 at minimum. It has not been configured to build on older Ubuntu versions.

## Set up the environment

This guide details how to build CARLA from source on Linux with Unreal Engine 5.5. 

Clone the `ue5-dev` branch of CARLA on your local machine:

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

Run the setup script:

```sh
cd CarlaUE5
bash -x  CarlaSetup.sh
```

The Setup.sh script installs all the required packages, including Cmake, debian packages, Python packages and Unreal Engine 5.5 and sets up the necessary environment variables. It also downloads the CARLA content, builds CARLA then launches the editor. 

Once this is complete, the script will launch the CARLA Unreal Engine 5 editor. **Note: This script can therefore take a long time to complete.**

The setup script should only be run the first time that you set up the build. Subsequently, when rebuilding, you should use the commands in the following section.

!!! note
        * This version of CARLA requires the **CARLA fork of Unreal Engine 5.5**. You need to link your GitHub account to Epic Games in order to gain permission to clone the UE repository. If you have not already linked your accounts, follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github)
        * For using CARLA Unreal Engine 5 previous builds, **ensure CARLA_UNREAL_ENGINE_PATH environment variable is defined** pointing to the CARLA Unreal Engine 5.5 absolute path. If this variable is not defined, the Setup.sh script will download and build CARLA Unreal Engine 5 and **this takes more than 1 extra hour of build and 225Gb of disk space**.
        * The Setup.sh script checks if there is any Python installed at the top of the PATH variable, and installs Python otherwise. **To use your own version of Python, ensure that the PATH variable is properly set for Python before running the script**.
        * CARLA cannot be built on an external disk, Ubuntu is not giving the required read/write/execution permissions for builds.


## Build and Run CARLA UE5

The setup script launches the following commands itself, you will need to use the following commands once you modify the code and wish to relaunch:

* Configure:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/LinuxToolchain.cmake \
-DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON
```

The command line options are as follows:

**G** - which build system to use
**S** - source path for CARLA repository
**B** - directory where the build goes

* Build CARLA:

```sh
cmake --build Build
```

* Build and install the Python API:

```sh
cmake --build Build --target carla-python-api-install
```

* Launch the editor:

```sh
cmake --build Build --target launch
```

## Build a package with CARLA UE5

```sh
cmake --build Build --target package
```

The package will be generated in the directory `$CARLA_PATH/Build/Package`

To build a development package, use the `package-development` target. This will build a package that outputs logs for debugging. 

## Run the package

Run the package with the following command from inside the package root folder.

```sh
./CarlaUnreal.sh
```

If you want to run the native ROS2 interface, add the `--ros2` argument

```sh
./CarlaUnreal.sh --ros2
```

If you want to install the Python API corresponding to the package you have built:

```sh
pip3 install PythonAPI/dist/carla-*.whl
```

## Presets

If you are building using multiple configurations, we recommend to use the preset system. To set up a preset, use the following command:

```sh
cmake --preset Linux-Development
```

This will create a folder within the build directory named `Linux-Development`. All other build artefacts for this configuration should then be directed into this folder, for example to launch the editor, run:

```sh
cmake --build Build/Linux-Development/ --target launch
```