# Building CARLA in Linux with Unreal Engine 5.8

!!! Note
    The Unreal Engine 5 version of CARLA supports Ubuntu 22.04 and 24.04. It has not been configured to build on older Ubuntu versions.

!!! Note
    On another distribution, or to keep UE5's dependencies off your machine, build inside the [Docker development environment](build_devcontainer.md) instead. The source stays on the host; only the toolchain lives in the container.

* __[Set up the environment](#set-up-the-environment)__
* __[Build and run CARLA UE5](#build-and-run-carla-ue5)__
* __[Build a package with CARLA UE5](#build-a-package-with-carla-ue5)__
* __[Extended build instructions](#extended-build-instructions)__

## Set up the environment

Clone the repository from GitHub, on the `ue58-dev` branch:

```sh
git clone -b ue58-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

Building CARLA needs access to the CARLA fork of Unreal Engine 5.8. Link your GitHub account to Epic Games by following [this guide](https://www.unrealengine.com/en-US/ue-on-github) first, then use your git credentials to authorize the download.

| Requirement | Value |
| --- | --- |
| Operating system | Ubuntu 22.04 or 24.04 |
| CMake | 3.28 or later. Ubuntu 22.04 ships 3.22, so the setup script installs 3.28.3 under `/opt` |
| Disk space | about 225 GB with the engine, 30 GB more per package |
| Epic Games account | linked to GitHub, for the engine fork |
| DLSS SDK | required by the engine build; the setup script fetches it |

Run the setup script from the CARLA root directory:

```sh
cd CarlaUE5
./CarlaSetup.sh --interactive
```

It asks for your sudo password to install the prerequisites, and then for your GitHub credentials to download the Unreal Engine repository. Pass `--launch` to open the editor once the build finishes.

`CarlaSetup.sh` takes these options:

| Option | What it does |
| --- | --- |
| `--interactive` | Prompt for the GitHub credentials instead of reading `GIT_LOCAL_CREDENTIALS` |
| `--skip-prerequisites` | Do not install the apt and Python packages, for an environment that already has them |
| `--launch` | Open the editor once the build finishes |
| `--python-root=PATH` | Build the Python API against an existing interpreter |

__Building unattended__: store the sudo password and the git credentials in the environment first, then run the script without `--interactive`. The credentials are split on `@`, so a username containing one will not work; use `--interactive` in that case.

```sh
sudo -v   # Used to cache the password for 15 minutes
export GIT_LOCAL_CREDENTIALS=github_username@github_token

./CarlaSetup.sh
```

The script installs every prerequisite — apt packages, CMake, the Python requirements, Git LFS — clones and builds Unreal Engine 5.8, clones the CARLA content, fetches the NVIDIA DLSS SDK into `~/SDKs/DLSS`, sets the environment variables, and builds CARLA. Expect it to take hours.

Run it only the first time. Afterwards, rebuild with the commands in the next section.

!!! Note
    * Set `CARLA_UNREAL_ENGINE_PATH` to an existing build of the CARLA Unreal Engine fork to skip building it, which saves two to three hours and 225 GB.
    * `--python-root=PATH` targets an existing Python installation instead of installing one with apt. Use `whereis python3` and strip the trailing `/python3`.
    * CARLA cannot be built on an external disk: Ubuntu does not grant the required read, write and execute permissions there.
    * DLSS is mandatory unless you opt out explicitly with `export DLSS_SDK=disabled`, which falls back to NFOR and spatial upscaling.

To follow the steps by hand, for instance to add libraries or because the script failed, see the [extended instructions](#extended-build-instructions).

---
## Build and run CARLA UE5

Configure once, with the preset for the configuration you want:

```sh
cmake --preset Release
```

The build then lives in `Build/Release`. There are three presets — `Debug`, `Development` and `Release` — each with its own build directory under `Build/`. `Release` gives minimal logs and debug information; `Debug` and `Development` give progressively more, to diagnose problems.

!!! Note
    To target a specific Python installation, add `-DPython_ROOT_DIR=PATH` and `-DPython3_ROOT_DIR=PATH` to the configure command.

Then build what you need:

| Target | What it does |
| --- | --- |
| `cmake --build Build/Release` | build everything |
| `cmake --build Build/Release --target launch` | build and open the editor |
| `cmake --build Build/Release --target launch-only` | open the editor without rebuilding |
| `cmake --build Build/Release --target carla-python-api-install` | build and install the Python client |
| `cmake --build Build/Release --target package` | cook and package a shipping server |
| `cmake --build Build/Release --target carla-help` | list every target and option |

Useful configure options: `-DENABLE_ROS2=ON` for the native ROS 2 interface, `-DCARLA_MAPS_TO_COOK=...` to cook a subset of maps, `-DBUILD_LIBCARLA_TESTS=ON` for the unit tests. `carla-help` prints the full list.

---
## Build a package with CARLA UE5

```sh
cmake --build Build/Release --target package
```

The package lands in `Build/Release/Package/Carla-<version>-Linux-Shipping/`, alongside a `.tar.gz` of the same. Use the `package-development` target instead for a package that writes debugging logs, or `package-debug` / `package-test`.

Expect one to two hours and about 12 GB.

!!! Note
    The cook needs a Zen server that outlives it. On Linux this is handled by `[Zen.AutoLaunch] LimitProcessLifetime=False`, so nothing has to be started by hand. Do not copy the Windows recipe of launching `zenserver` manually.

### Select which maps to cook

By default a package cooks every map listed in `DefaultGame.ini` — `Town10HD_Opt`, `OpenDriveMap`, `TestMaps/EmptyMap`, `Mine_01` and `Town15/Town15` — which produces a package of around 28 GB. To cook a subset, set `CARLA_MAPS_TO_COOK` when configuring, as a `+`-separated list of map package paths. A Town10-only package is around 16 GB.

```sh
# Cook a single map
cmake --preset Release -DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/Town10HD_Opt"

# Cook several maps
cmake --preset Release -DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/Town10HD_Opt+/Game/Carla/Maps/Mine_01"
```

Use the full map package path (`/Game/Carla/Maps/<MapName>`), not a filesystem path. Note that the large maps nest: Town15 is `/Game/Carla/Maps/Town15/Town15`. Leaving `CARLA_MAPS_TO_COOK` empty, the default, cooks the full list from `DefaultGame.ini`.

!!! Note
    `Town10HD_Opt` is the default map a package boots into and the cooker always cooks the `GameDefaultMap` (only its persistent level, not its World Partition cells, when it is not in `CARLA_MAPS_TO_COOK`). If your selection excludes it, set a different default map in `DefaultEngine.ini` before packaging, otherwise the packaged server starts into an empty Town10.

### Run the package

The archive root holds `Linux/`, `PythonAPI/` and a `README`. Start the server from the root:

```sh
./Linux/CarlaUnreal.sh
```

To run the native ROS2 interface, add the `-ros2` argument:

```sh
./Linux/CarlaUnreal.sh -ros2
```

To install the Python API that matches the package you built:

```sh
pip3 install PythonAPI/carla/dist/carla-*.whl
```

### Content packs

The `package` targets also cook the package as a __base release__ (UAT `-createreleaseversion=<release>`, `carla-<version>-<platform>` by default, `CARLA_BASE_RELEASE` overrides the name). Two artifacts come out of this, next to the package:

* `CarlaUnreal/BaseRelease` inside the package: the release name the packaged server was cooked as. Content packs record the same string and the server refuses packs built against another release.
* `Build/Package/<release>-release-metadata.tar.gz` (about 1 MB): the release's asset registry (`<release>/Linux/AssetRegistry.bin` and `Metadata/DevelopmentAssetRegistry.bin`), which is what a content pack is cooked "based on". Publish it together with the package tarball; `carla-pack build --base <this file>` needs it and nothing else from the package build.

Packs under `Unreal/CarlaUnreal/Plugins/Packs/` must stay out of the base: after the cook the `package` target runs `Unreal/Package/CheckNoPackLeak.cmake`, which fails the build if any of those packs has package names under `/<Pack>/` in the cooked asset registry or files under `Plugins/Packs/<Pack>/` in the staged output (`CARLA_PACK_LEAK_IGNORE` skips a pack by name, for the rare pack named after a base content folder).

See [Content packs](content_packs.md) for authoring, building and installing packs. The `pack-<Name>` CMake targets build the packs found under `Unreal/CarlaUnreal/Plugins/Packs/` against this release, and `content-packs-test` runs the tool's unit tests.

---
## Extended build instructions

Use these instructions if you need more control over the setup process or the setup script has failed for some reason:

### Install prerequisites

The full list is what `CarlaSetup.sh` installs, so the simplest way to get it right is to run that one step on its own:

```sh
bash Util/SetupUtils/InstallPrerequisites.sh --python-path=python3
```

It installs the apt packages (`build-essential`, `ninja-build`, `libvulkan1`, `libpng-dev`, `libtiff-dev`, `libjpeg-dev`, `curl`, `libtool`, `rsync`, `libxml2-dev`, `git`, `git-lfs`, `libnss3-dev`, `libatk-bridge2.0-dev`, `libxkbcommon-dev`, `libgbm-dev`, `libpango1.0-dev`, `libasound2-dev`, `libsdl2-dev`, `libfreetype-dev`, and Python 3 with its headers and pip), runs `git lfs install`, installs `requirements.txt`, and installs CMake 3.28.3 under `/opt` when the system CMake is older than 3.28.

Then fetch the DLSS SDK, which the engine build requires:

```sh
bash Util/SetupUtils/SetupDLSS.sh
export DLSS_SDK=$HOME/SDKs/DLSS
```

Building without it is an explicit opt-out: `export DLSS_SDK=disabled`.

### Clone and build the Unreal Engine 5.8

Clone the unreal engine from this fork:

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
git clone https://github.com/carla-simulator/carla.git
```

Enter the cloned repository and create the content folder `Unreal/CarlaUnreal/Content` if it does not exist. From inside it, clone the content repository:

```sh
git clone --single-branch --depth 1 -b ue58-dev-carla https://bitbucket.org/carla-simulator/carla-content.git Carla
```

This command may take some time since it is downloading a lot of data.

### Build and launch CARLA

Once the engine is built and the CARLA code and content are in place, build CARLA from the root of the repository.

Configure a preset:

```sh
cmake --preset Release
```

Compile the Python API:

```sh
cmake --build Build/Release/ --target carla-python-api-install
```

To launch the editor:

```sh
cmake --build Build/Release/ --target launch
```

To build a package:

```sh
cmake --build Build/Release/ --target package
```

From here on, use the commands in [Build and run CARLA UE5](#build-and-run-carla-ue5).
