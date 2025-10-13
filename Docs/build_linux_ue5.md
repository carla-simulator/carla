# Building CARLA in Linux with Unreal Engine 5.5

!!! note
    The Unreal Engine 5 version of CARLA requires Ubuntu version 22.04 at minimum. It has not been configured to build on older Ubuntu versions.

* __[Set up the environment](#set-up-the-environment)__  
* __[Build and run CARLA UE5](#build-and-run-carla-ue5)__  
* __[Build a package with CARLA UE5](#build-a-package-with-carla-ue5)__ 
* __[Build presets](#build-presets)__ 
* __[Extended build instructions](#extended-build-instructions)__ 

## Set up the environment

Clone this repository locally from GitHub, specifying the *ue5-dev* branch:

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

In order to build CARLA, you need acces to the CARLA fork of Unreal Engine 5.5. In order to access this repository, you must first link your GitHub account to Epic Games by following [this guide](https://www.unrealengine.com/en-US/ue-on-github). You then also need to use your git credentials to authorise the download of the Unreal Engine 5.5 repository. 

Run the setup script from a terminal open in the CARLA root directory:

```sh
cd CarlaUE5
./CarlaSetup.sh --interactive
```

The setup script will prompt you for your sudo password, in order to install the prerequisites. It will then prompt you for your GitHub credentials in order to authorise the download of the Unreal Engine repository. 

__Building in Linux unattended__:

If you want to run the setup script unattended, your git credentials need to be stored in an environment variable. Add your github credentials to your `.bashrc` file:

```sh
export GIT_LOCAL_CREDENTIALS=username@github_token
```

Then run the setup script using the following command:

```sh
cd CarlaUE5
sudo -E ./CarlaSetup.sh
```

This will download and install Unreal Engine 5.5, install the prerequisites and build CARLA. It may take some time to complete and use a significant amount of disk space.

If you prefer to add the git credentials in the terminal, use the following command:

```sh
cd CarlaUE5
sudo -E env GIT_LOCAL_CREDENTIALS=github_username@github_token ./CarlaSetup.sh 
```

!!! note
    The setup script will install by default Python 3 using apt. If you want to target an existing Python installation, you should use the `--python-root=PATH_TO_PYTHON` argument with the relevant Python installation path. You can use whereis python3 in your chosen environment and strip the `/python3` suffix from the path.

The CarlaSetup.sh script installs all the required packages, including Cmake, debian packages, Python packages and Unreal Engine 5.5 and sets up the necessary environment variables. It also downloads the CARLA content.

Once this is complete, the script will launch the CARLA Unreal Engine 5 editor. **Note: This script can therefore take a long time to complete.**

The setup script should only be run the first time that you set up the build. Subsequently, when rebuilding, you should use the commands in the following section.

!!! note
    * This version of CARLA requires the **CARLA fork of Unreal Engine 5.5**. You need to link your GitHub account to Epic Games in order to gain permission to clone the UE repository. If you have not already linked your accounts, follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github)
    * For using CARLA Unreal Engine 5 previous builds, **ensure CARLA_UNREAL_ENGINE_PATH environment variable is defined** pointing to the CARLA Unreal Engine 5.5 absolute path. If this variable is not defined, the CarlaSetup.sh script will download and build CARLA Unreal Engine 5 and **this takes more than 1 extra hour of build and 225Gb of disk space**.
    * The CarlaSetup.sh script checks if there is any Python installed at the top of the PATH variable, and installs Python otherwise. **To use your own version of Python, ensure that the PATH variable is properly set for Python before running the script**.
    * CARLA cannot be built on an external disk, Ubuntu is not giving the required read/write/execution permissions for builds.

You may want to follow the setup steps separately, if, for example, you are including additional libraries or if the CarlaSetup.sh script fails for some reason. Please refer to [the extended instructions](#extended-build-instructions) to proceed.

## Build and run CARLA UE5

The setup script launches the following commands itself, you will need to use the following commands once you modify the code and wish to relaunch:

* Configure:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON
```

!!! Note
    If you intend to target a specific Python installation, you should add both these arguments to the above cmake command: `-DPython_ROOT_DIR=PATH` and `-DPython3_ROOT_DIR=PATH`.


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

### Run the package

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
pip3 install PythonAPI/dist/carla-***.whl
```

## Build presets

If you are building using multiple configurations, we recommend to use the preset system. To set up a preset, use the following command:

```sh
cmake --preset Linux-Development
```

This will create a folder within the build directory named `Linux-Development`. All other build artifacts for this configuration should then be directed into this folder, for example to launch the editor, run:

```sh
cmake --build Build/Linux-Development/ --target launch
```

There are 3 presets to choose from, `Linux-Debug`, `Linux-Development` and `Linux-Release`. The release preset provides minimal debug information or logs. The debug and development presets provide differing levels of terminal output and logs to help diagnose problems in CARLA installations. 

---

## Extended build instructions

Use these instructions if you need more control over the setup process or the setup script has failed for some reason:

### Install prerequisites

Before building CARLA, ensure the prerequisites are installed with the following commands:

```sh
sudo apt update
sudo apt install build-essential ninja-build libvulkan1 python3 python3-dev python3-pip git git-lfs
```

### Clone and build the Unreal Engine 5.5

Clone the unreal engine from this fork:

```sh
git clone https://github.com/CarlaUnreal/UnrealEngine

```

Enter the repository root directory and switch to the ue5-dev-carla branch:

```sh
git checkout ue5-dev-carla
```

Then build Unreal Engine 5.5:

```sh
./Setup.sh && ./GenerateProjectFiles.sh && make
```

!!! note
    The first time building the Unreal Engine may take up to 3 hours to complete. 

Set up an environment variable to indicate the location of the Unreal Engine 5.5. In your `.bashrc` file, add the following line:

```sh
export CARLA_UNREAL_ENGINE_PATH=<PATH_TO_UNREAL_ENGINE_FOLDER>
```

### Clone the CARLA code and content

Clone the CARLA repository to your local machine:

```sh
git clone https://github.com/carla-simulator/carla.git
```

Enter the cloned repository and navigate to the content folder `CARLA_ROOT/carla/Unreal/CarlaUnreal/Content` (if it doesn't exist, create it). From inside this folder, clone the content repository:

```sh
git clone --single-branch --depth 1 -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla
```

This command may take some time since it is downloading a lot of data.

### Build and launch CARLA

Once the Unreal Engine 5.5 is built and all the CARLA code and content is successfully downloaded, you can build CARLA. Enter the root folder of the CARLA code repository and run the following commands:

Create your preferred build preset:

```sh
cmake --preset Linux-Release
```

Compile the Python API:

```sh
cmake --build Build/Linux-Release/ --target carla-python-api-install
```

To launch the editor:

```sh
cmake --build Build/Linux-Release/ --target launch
```

To build a package:

```sh
cmake --build Build/Linux-Release/ --target package
```
