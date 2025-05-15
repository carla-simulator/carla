!!! warning
        This is a work in progress!! This version of CARLA is not considered a stable release. Over the following months many significant changes may be made to this branch which could break any modifications you make. We advise you to treat this branch as experimental.

# Building CARLA in Linux with Unreal Engine 5.3

!!! note
        This build process is implemented and tested for Ubuntu 22.04. We recommend to use this Ubuntu version. 

## Set up the environment

This guide details how to build CARLA from source on Linux with Unreal Engine 5.3. 

Clone the `ue5-dev` branch of CARLA on your local machine:

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

Run the setup script:

```sh
cd CarlaUE5
bash -x Setup.sh
```

The Setup.sh script installs all the required packages, including Cmake, debian packages, Python packages and Unreal Engine 5.3. It also downloads the CARLA content and builds CARLA. This script can therefore take a long time to complete. 

!!! note
        * This version of CARLA requires the **CARLA fork of Unreal Engine 5.3**. You need to link your GitHub account to Epic Games in order to gain permission to clone the UE repository. If you have not already linked your accounts, follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github)
        * For using CARLA Unreal Engine 5 previous builds, **ensure CARLA_UNREAL_ENGINE_PATH environment variable is defined** pointing to the CARLA Unreal Engine 5.3 absolute path. If this variable is not defined, the Setup.sh script will download and build CARLA Unreal Engine 5 and **this takes more than 1 extra hour of build and 225Gb of disk space**.
        * The Setup.sh script checks if there is any Python installed at the top of the PATH variable, and installs Python otherwise. **To use your own version of Python, ensure that the PATH variable is properly set for Python before running the script**.
        * CARLA cannot be built on an external disk, Ubuntu is not giving the required read/write/execution permissions for builds.


## Build and Run CARLA UE5

The setup script launches the following commands itself, you will need to use the following commands once you modify the code and wish to relaunch:

* Configure:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/LinuxToolchain.cmake \
-DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON \
-DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
```

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

## Run the package

Run the package with the following command.

```sh
./CarlaUnreal.sh
```

If you want to run the native ROS2 interface, add the `--ros2` argument

```sh
./CarlaUnreal.sh --ros2
```

If you want to install the Python API corresponding to the package you have built:

```sh
pip3 install PythonAPI/carla/dist/carla-*.whl
```

## Additional build targets

The procedure outlined above will download all necessary components to build CARLA, you may not want to 