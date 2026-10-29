# Building CARLA in Linux with Unreal Engine 5.8

!!! Note
    CARLA supports Ubuntu 22.04 and 24.04. It has not been configured to build on older Ubuntu versions. If you are using another Ubuntu version, or experiencing compilation issues, you can use the Docker-based development environment to build CARLA inside an Ubuntu 22.04 container. See [CARLA Docker Dev Environment](build_devcontainer.md) for instructions.

The default way to build CARLA is to let your AI coding agent do it, following procedures tested by the CARLA team:

!!! agent "Ask your agent"
    *"Clone CARLA from the ue58-dev branch, build it with Unreal Engine 5.8 and launch the editor."*

    Skills: `build-carla-ue58`, `run-carla-ue58-server`. [Connect your agent](agent_skills.md#connect-your-agent).

If you prefer to build CARLA by hand, follow the steps below.

- [__Set up the environment__](#set-up-the-environment)
- [__Build and run CARLA__](#build-and-run-carla)
- [__Build a package with CARLA__](#build-a-package-with-carla)
- [__Build presets__](#build-presets)
- [__Extended build instructions__](#extended-build-instructions)

---
## Set up the environment

Clone this repository locally from GitHub, specifying the *ue58-dev* branch:

```sh
git clone -b ue58-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

In order to build CARLA, you need access to the CARLA fork of Unreal Engine 5.8. In order to access this repository, you must first link your GitHub account to Epic Games by following [this guide](https://www.unrealengine.com/en-US/ue-on-github). You then also need to use your git credentials to authorise the download of the Unreal Engine 5.8 repository.

Run the setup script from a terminal open in the CARLA root directory:

```sh
cd CarlaUE5
./CarlaSetup.sh --interactive
```

The `CarlaSetup.sh` script installs all the required packages, including CMake, debian packages, Python packages and Unreal Engine 5.8, sets up the necessary environment variables and builds CARLA. It also downloads the CARLA content and the NVIDIA DLSS SDK. It may take some time to complete and use a significant amount of disk space.

The setup script will prompt you for your sudo password, in order to install the prerequisites. It will then prompt you for your GitHub credentials in order to authorise the download of the Unreal Engine repository.

__Building in Linux unattended__:

If you want to run the setup script unattended, cache your sudo password with `sudo -v` and store your git credentials in `GIT_LOCAL_CREDENTIALS`.

```sh
sudo -v   # Used to cache the password for 15 minutes
export GIT_LOCAL_CREDENTIALS=github_username@github_token
```

The credentials are split on `@`, so a username that contains one will not work. Use `--interactive` in that case.

Then run the setup script without the interactive flag:

```sh
cd CarlaUE5
./CarlaSetup.sh
```

!!! Note
    The setup script will install by default Python 3 using apt. If you want to target an existing Python installation, you should use the `--python-root=PATH_TO_PYTHON` argument with the relevant Python installation path. You can use `whereis python3` in your chosen environment and strip the `/python3` suffix from the path.

If you pass `--launch`, the script will then launch the CARLA Unreal Engine 5 editor.

The setup script should only be run the first time that you set up the build. Subsequently, when rebuilding, you should use the commands in the following section.

!!! Note
    * To use an existing build of the CARLA Unreal Engine 5.8 fork, **ensure the CARLA_UNREAL_ENGINE_PATH environment variable is defined** pointing to its absolute path. If this variable is not defined, the `CarlaSetup.sh` script looks for an `UnrealEngine5_carla` folder next to the CARLA root and assumes it is already built. If there is no such folder either, the script will download and build CARLA Unreal Engine 5 and **this takes two to three extra hours of build and 225 GB of disk space**.
    * CARLA cannot be built on an external disk, Ubuntu is not giving the required read/write/execution permissions for builds.

You may want to follow the setup steps separately, if, for example, you are including additional libraries or if the `CarlaSetup.sh` script fails for some reason. Please refer to [the extended instructions](#extended-build-instructions) to proceed.

---
## Build and run CARLA

The setup script launches the following commands itself. You will need to use the following commands once you modify the code and wish to relaunch:

* Configure:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON
```

!!! Note
    If you intend to target a specific Python installation, you should add both these arguments to the above cmake command: `-DPython_ROOT_DIR=PATH` and `-DPython3_ROOT_DIR=PATH`.

The command line options are as follows:

* **G** - which build system to use
* **S** - source path for CARLA repository
* **B** - directory where the build goes

`-DENABLE_ROS2=ON` builds the native ROS 2 interface. To cook only some of the maps into a package, add `-DCARLA_MAPS_TO_COOK=...` to the configure command, see [Select which maps to cook](#select-which-maps-to-cook).

Then build what you need:

| Command | What it does |
| --- | --- |
| `cmake --build Build` | Build CARLA |
| `cmake --build Build --target carla-python-api-install` | Build and install the Python API |
| `cmake --build Build --target launch` | Build and launch the editor |
| `cmake --build Build --target launch-only` | Launch the editor without rebuilding |
| `cmake --build Build --target package` | Build a package, see [below](#build-a-package-with-carla) |
| `cmake --build Build --target carla-help` | List the main CARLA targets and configure options |

---
## Build a package with CARLA

!!! agent "Ask your agent"
    *"Package CARLA with only Town10HD_Opt and Town15, and check that the package starts."*

    Skills: `package-carla-ue58`, `run-carla-ue58-server`. [Connect your agent](agent_skills.md#connect-your-agent).

Build the package:

```sh
cmake --build Build --target package
```

The package will be generated in the directory `Build/Package/Carla-<version>-Linux-Shipping` of the CARLA root, alongside a `.tar.gz` of the same.

To build a development package, use the `package-development` target. This will build a package that outputs logs for debugging. The `package-debug`, `package-debuggame` and `package-test` targets build the other configurations, and `package-shipping` builds the `Shipping` configuration, which `package` also builds with the `Release` configure command above.

### Select which maps to cook

By default a package cooks every map listed in `DefaultGame.ini` (`Town10HD_Opt`, `OpenDriveMap`, `TestMaps/EmptyMap`, `Town12/Town12`, `Town15/Town15`). If you only need a subset of the maps, set the `CARLA_MAPS_TO_COOK` option when you configure the build to a `+`-separated list of map package paths. The package then cooks those maps plus the default map `Town10HD_Opt`, and is correspondingly smaller (a Town10-only package is around 16 GB).

```sh
# Cook a single map
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON \
    -DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/Town10HD_Opt"

# Cook several maps
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON \
    -DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/Town10HD_Opt+/Game/Carla/Maps/Town15/Town15"
```

Use the full map package path (`/Game/Carla/Maps/<MapName>`), not a filesystem path. The large maps nest: Town15 is `/Game/Carla/Maps/Town15/Town15`. Leaving `CARLA_MAPS_TO_COOK` empty (the default) cooks the full map list from `DefaultGame.ini`, so the standard build is unchanged.

!!! Note
    `Town10HD_Opt` is the default map a package boots into. It is cooked even if your selection does not list it.

### Run the package

Run the package with the following command from inside the package root folder.

```sh
./Linux/CarlaUnreal.sh
```

If you want to run the native ROS 2 interface, add the `-ros2` argument:

```sh
./Linux/CarlaUnreal.sh -ros2
```

If you want to install the Python API corresponding to the package you have built:

```sh
pip3 install PythonAPI/carla/dist/carla-*.whl
```

### Content packs

The `package` target also writes `<release>-release-metadata.tar.gz` (about 2 MB) next to the package, which content packs are built against. See [Content packs](content_authoring_packs.md).

---
## Build presets

If you are building using multiple configurations, we recommend using the preset system. To set up a preset, use the following command:

```sh
cmake --preset Development
```

This will create a folder within the build directory named `Development`. All other build artifacts for this configuration should then be directed into this folder, for example to launch the editor, run:

```sh
cmake --build Build/Development/ --target launch
```

There are 3 presets to choose from, `Debug`, `Development` and `Release`. The debug, development and release presets set `CMAKE_BUILD_TYPE` to `Debug`, `RelWithDebInfo` and `Release`, and the `package` target then builds a `Debug`, a `Development` or a `Shipping` package respectively. The presets do not enable ROS 2; add `-DENABLE_ROS2=ON` to the preset command if you need the native ROS 2 interface.

---
## Extended build instructions

Use these instructions if you need more control over the setup process or the setup script has failed for some reason:

### Install prerequisites

Before building CARLA, ensure the prerequisites are installed with the following commands:

```sh
sudo apt update
sudo apt install build-essential make ninja-build libvulkan1 libpng-dev libtiff-dev libjpeg-dev tzdata sed curl libtool rsync libxml2-dev git git-lfs libnss3-dev libatk-bridge2.0-dev libxkbcommon-dev libgbm-dev libpango1.0-dev libasound2-dev libsdl2-dev libfreetype-dev python3 python3-dev python3-pip
git lfs install
```

CARLA also requires CMake 3.27.2 or later (the setup script installs 3.28.3 if it finds no CMake 3.28 or later) and the Python packages listed in `requirements.txt`. Ubuntu 22.04 comes with an older version of CMake. Alternatively, you can install all of the prerequisites, including CMake, by running the following script from the CARLA repository:

```sh
bash Util/SetupUtils/InstallPrerequisites.sh --python-path=python3
```

The CARLA build uses the NVIDIA DLSS SDK for the ray-traced lens cameras. If you already have it, set `DLSS_SDK` to its location. Otherwise, download it with:

```sh
bash Util/SetupUtils/SetupDLSS.sh
export DLSS_SDK=$HOME/SDKs/DLSS
```

To build without it, configure with `-DCARLA_DLSS_SDK_PATH=disabled`, or set `DLSS_SDK=disabled`.

### Clone and build Unreal Engine 5.8

Clone Unreal Engine from this fork:

```sh
git clone -b ue58-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
cd UnrealEngine5_carla
```

Then build Unreal Engine 5.8:

```sh
./Setup.sh && ./GenerateProjectFiles.sh && make
```

!!! Note
    Building the Unreal Engine for the first time takes two to three hours and about 225 GB of disk space.

Set up an environment variable to indicate the location of the Unreal Engine 5.8. In your `.bashrc` file, add the following line:

```sh
export CARLA_UNREAL_ENGINE_PATH=<PATH_TO_UNREAL_ENGINE_FOLDER>
```

### Clone the CARLA code and content

Clone the CARLA repository to your local machine:

```sh
git clone -b ue58-dev https://github.com/carla-simulator/carla.git
```

Enter the cloned repository and create the content folder `Unreal/CarlaUnreal/Content` if it does not exist. From inside it, clone the content repository:

```sh
git clone --single-branch --depth 1 -b ue58-dev-carla https://bitbucket.org/carla-simulator/carla-content.git Carla
```

This command may take some time since it is downloading a lot of data.

### Build and launch CARLA

Once the engine is built and the CARLA code and content are in place, build CARLA from the root of the repository.

Configure the build:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON
```

Compile the Python API:

```sh
cmake --build Build --target carla-python-api-install
```

To launch the editor:

```sh
cmake --build Build --target launch
```

To build a package:

```sh
cmake --build Build --target package
```

From here on, use the commands in [Build and run CARLA](#build-and-run-carla).
