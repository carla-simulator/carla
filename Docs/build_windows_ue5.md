# Building CARLA in Windows with Unreal Engine 5.8

- [__Set up the environment__](#set-up-the-environment)
- [__Build and run CARLA UE5__](#build-and-run-carla-ue5)
- [__Build a package with CARLA UE5__](#build-a-package-with-carla-ue5)

## Set up the environment

This guide details how to build CARLA from source on Windows with Unreal Engine 5.8. If you are building CARLA for the first time, ensure that you **link your GitHub account to Epic Games** by following [this guide](https://www.unrealengine.com/en-US/ue-on-github).

!!! Note
    * To use an existing CARLA Unreal Engine 5.8 build, set `CARLA_UNREAL_ENGINE_PATH` to its absolute path. If it is not set, `CarlaSetup.bat` uses an existing `..\UnrealEngine5_carla` folder for that setup invocation. If neither is available, it downloads the engine into `..\UnrealEngine5_carla`, next to the CARLA folder, builds it and saves the variable for future terminals, which takes __more than one extra hour of build and about 225 GB of disk space__.
    * An existing Visual Studio 2022 (Community, Professional or Enterprise) is used as it is. It needs the MSVC v14.38 toolset, or v14.44.35211 or later; to add v14.38, import `Unreal\CarlaUnreal\.vsconfig` in the Visual Studio Installer.
    * **Windows Developer Mode should be active**, otherwise build will fail. Please see [here](https://learn.microsoft.com/en-us/gaming/game-bar/guide/developer-mode) for instructions on how to activate Developer Mode.
    * **CARLA cannot be built on an external disk**, Windows does not give the required read/write/execution permissions for builds.

Clone the `ue58-dev` branch of CARLA on your local machine:

```sh
git clone -b ue58-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

Run the setup script:

```sh
cd CarlaUE5
CarlaSetup.bat
```

The CarlaSetup.bat script installs all the required packages, including Visual Studio 2022 with CMake (if no Visual Studio 2022 is installed), Ninja, Python packages and the CARLA fork of Unreal Engine 5.8. It also downloads the CARLA content into `Unreal\CarlaUnreal\Content\Carla` and builds CARLA. This batch file can therefore take a long time to complete.

It installs Python 3.8.10 when no Python is found at the top of `PATH`; 3.9 and 3.10 work too. To target an existing installation, pass `--python-root=C:\Path\To\PythonDir`, the folder that contains `python.exe`.

Quote the path when it contains spaces, for example:

```sh
CarlaSetup.bat --python-root "C:\Path With Spaces\PythonDir"
```

Pass `--skip-prerequisites` (`-p`) to skip installing Visual Studio, Ninja, Python and its packages, and `--launch` (`-l`) to open the editor once the build finishes.

---
## Build and run CARLA UE5

`CarlaSetup.bat` runs these commands itself the first time. Use them directly to rebuild after changing the code.

Run every command below from the __x64 Native Tools Command Prompt for VS 2022__, opened at the CarlaUE5 folder.

Configure the build:

```sh
cmake -G Ninja -S . -B Build --toolchain=CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```

The command line options are as follows:

* **G** - which build system to use
* **S** - source path for CARLA repository
* **B** - directory where the build goes

If you passed `--python-root` to `CarlaSetup.bat`, add both `-DPython_ROOT_DIR=PATH` and `-DPython3_ROOT_DIR=PATH` to the above cmake command.

Configuring needs the NVIDIA DLSS SDK. CMake takes it from `-DCARLA_DLSS_SDK_PATH=<dir>` or the `DLSS_SDK` environment variable, otherwise from `%USERPROFILE%\SDKs\DLSS`, and clones it there from NVIDIA if it is missing. To build without DLSS, add `-DCARLA_DLSS_SDK_PATH=disabled`.

Then build what you need:

| Target | What it does |
| --- | --- |
| `cmake --build Build` | build everything |
| `cmake --build Build --target launch` | build and open the editor with a log window (to drop it, reconfigure with `-DCARLA_LAUNCH_ARGS=`; `-DCARLA_UNREAL_LOG_WINDOW=OFF` only works on a new build directory) |
| `cmake --build Build --target launch-only` | open the editor without rebuilding |
| `cmake --build Build --target carla-python-api-install` | build and install the Python client |
| `cmake --build Build --target carla-help` | list every target and option |

!!! Note
    The native ROS 2 interface (`-DENABLE_ROS2=ON`) is built on Linux only; `CarlaSetup.bat` does not enable it.

---
## Build a package with CARLA UE5

Build the package:

```sh
cmake --build Build --target package
```

Use `package-development` instead for a package that writes debugging logs. The `package-debug` and `package-test` targets build the other configurations.

The package lands in `Build\Package\Carla-<version>-Win64-Shipping\`, alongside a `.zip` of the same (configure with `-DCARLA_UNREAL_PACKAGE_NO_COMPRESSION=ON` to skip it). The Python wheel that matches it is in `PythonAPI\carla\dist` inside the package.

### Select which maps to cook

By default a package cooks every map listed in `DefaultGame.ini` — `Town10HD_Opt`, `OpenDriveMap`, `TestMaps/EmptyMap`, `Town12/Town12` and `Town15/Town15`. To cook a subset, set `CARLA_MAPS_TO_COOK` when configuring, as a `+`-separated list of map package paths:

```sh
cmake -G Ninja -S . -B Build --toolchain=CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/Town10HD_Opt+/Game/Carla/Maps/Town15/Town15"
```

Use the full map package path (`/Game/Carla/Maps/<MapName>`), not a filesystem path. Note that the large maps nest: Town15 is `/Game/Carla/Maps/Town15/Town15`. Leaving `CARLA_MAPS_TO_COOK` empty, the default, cooks the full list from `DefaultGame.ini`.

!!! Note
    `Town10HD_Opt` is the default map a package boots into. If your selection excludes it, set a different default map in `DefaultEngine.ini` before packaging, otherwise the packaged server starts with a missing-map error.

### Run the package

Extract the package that you built, then from a command terminal open in the root folder of the extracted package, run the `CarlaUnreal.exe` file:

```sh
CarlaUnreal.exe
```

### Content packs

The `package` target also writes `carla-<version>-Win64-release-metadata.tar.gz` in `Build\Package`, next to the package, which content packs are built against. See [Content packs](content_authoring_packs.md).
