!!! warning
        This is a work in progress!! This version of CARLA is not considered a stable release. Over the following months many significant changes may be made to this branch which could break any modifications you make. We advise you to treat this branch as experimental.

# Building CARLA in Windows with Unreal Engine 5.3

## Set up the environment

This guide details how to build CARLA from source on Windows with Unreal Engine 5.3. 

Clone the `ue5-dev` branch of CARLA on your local machine:

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

Run the setup script:

```sh
cd CarlaUE5
Setup.bat
```

The Setup.bat script installs all the required packages, including Visual Studio 2022, Cmake, Python packages and Unreal Engine 5. It also downloads the CARLA content and builds CARLA. This batch file can therefore take a long time to complete. 

!!! note
        * This version of CARLA requires the **CARLA fork of Unreal Engine 5.3**. You need to link your GitHub account to Epic Games in order to gain permission to clone the UE repository. If you have not already linked your accounts, follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github)
        * For using CARLA Unreal Engine 5 previous builds, ensure CARLA_UNREAL_ENGINE_PATH environment variable is defined pointing to the CARLA Unreal Engine 5 absolute path. If this variable is not defined, Setup.bat script will download and build CARLA Unreal Engine 5 and **this takes more than 1 extra hour of build and a 225Gb of disk space**.
        * Setup.bat script checks if there is any Python version installed at the top of the PATH variable, and installs Python otherwise. **To use your own version of Python, ensure that the PATH variable is properly set for Python before running the script**.
        * **Windows Developer Mode should be active**, otherwise build will fail. Please see [here](https://learn.microsoft.com/en-us/gaming/game-bar/guide/developer-mode) for instructions on how to activate Developer Mode.
        * **CARLA cannot be built on an external disk**, Windows does not give the required read/write/execution permissions for builds.


## Build and Run CARLA UE5

The Setup.bat file launches the following commands itself, you will need to use the following commands once you modify the code and wish to relaunch:

!!! warning
       Ensure CARLA_UNREAL_ENGINE_PATH environment variable is defined pointing to the CARLA Unreal Engine 5.3 absolute path. Setup.bat sets this variable, but I may not be set if another approach was followed to install the requirements. 

* **Configure**. Open x64 Native Tools Command Prompt for VS 2022 at the CarlaUE5 folder and runn the following command:

```sh
cmake -G Ninja -S . -B Build -DCMAKE_BUILD_TYPE=Release -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=%CARLA_UNREAL_ENGINE_PATH%
```

* **Build CARLA**. Open the x64 Native Tools Command Prompt for VS 2022 at the CarlaUE5 folder and run the following command:

```sh
cmake --build Build
```

* **Build and install the Python API**. Open the x64 Native Tools Command Prompt for VS 2022 at the CarlaUE5 folder and run the following command::

```sh
cmake --build Build --target carla-python-api-install
```

* **Launch the editor**. Open the x64 Native Tools Command Prompt for VS 2022 at the CarlaUE5 folder and run the following command:

```sh
cmake --build Build --target launch
```

## Build a package with CARLA UE5

!!! warning
        The package build for Carla UE5 is not yet fully tested for Windows.

Open the x64 Native Tools Command Prompt for VS 2022 at the CarlaUE5 folder and run the following command:

```sh
cmake --build Build --target package
```

The package will be generated in the directory `Build/Package`

## Run the package

The package build is not yet tested for Windows
