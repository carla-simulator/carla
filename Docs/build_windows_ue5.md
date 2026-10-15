# Building CARLA in Windows with Unreal Engine 5.8

!!! Note
    The Windows build is less exercised than the Linux one, and packaging in particular is not fully tested. Report what breaks.

* __[Set up the environment](#set-up-the-environment)__
* __[Build and run CARLA UE5](#build-and-run-carla-ue5)__
* __[Build a package with CARLA UE5](#build-a-package-with-carla-ue5)__

---
## Set up the environment

This guide details how to build CARLA from source on Windows with Unreal Engine 5.8. If you are building CARLA for the first time, ensure that you **link your GitHub account to Epic Games** by following [this guide](https://www.unrealengine.com/en-US/ue-on-github).

Clone the `ue58-dev` branch of CARLA on your local machine:

```sh
git clone -b ue58-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

Run the setup script:

```sh
cd CarlaUE5
CarlaSetup.bat
```

`CarlaSetup.bat` installs everything needed — Visual Studio 2022, CMake, Python and its packages, and the CARLA fork of Unreal Engine 5.8 — then downloads the CARLA content and builds CARLA. Expect it to take hours.

It installs Python 3.8.10 when no Python is found at the top of `PATH`; 3.9 and 3.10 work too. To target an existing installation, pass `--python-root=C:\Path\To\PythonDir`, the folder that contains `python.exe`.

!!! note
    * This version of CARLA requires the **CARLA fork of Unreal Engine 5.8**. You need to link your GitHub account to Epic Games in order to gain permission to clone the UE repository. If you have not already linked your accounts, follow [this guide](https://www.unrealengine.com/en-US/ue-on-github)
    * For using CARLA Unreal Engine 5 previous builds, ensure CARLA_UNREAL_ENGINE_PATH environment variable is defined pointing to the CARLA Unreal Engine 5 absolute path. If it is not defined, `CarlaSetup.bat` downloads and builds the engine itself, which takes __two to three hours and about 225 GB of disk space__.
    * The CarlaSetup.bat script checks if there is any Python version installed at the top of the PATH variable, and installs Python otherwise. **To use your own version of Python, ensure that the PATH variable is properly set for Python before running the script**.
    * **Windows Developer Mode should be active**, otherwise build will fail. Please see [here](https://learn.microsoft.com/en-us/gaming/game-bar/guide/developer-mode) for instructions on how to activate Developer Mode.
    * **CARLA cannot be built on an external disk**, Windows does not give the required read/write/execution permissions for builds.

---
## Build and run CARLA UE5

`CarlaSetup.bat` runs these commands itself the first time. Use them directly to rebuild after changing the code.

!!! Warning
    `CARLA_UNREAL_ENGINE_PATH` must point at the absolute path of the CARLA Unreal Engine build. `CarlaSetup.bat` sets it; a setup done by other means may not have.

Run every command below from the __x64 Native Tools Command Prompt for VS 2022__, opened at the CarlaUE5 folder.

Configure once, with the preset for the configuration you want:

```sh
cmake --preset Release
```

The build then lives in `Build\Release`. There are three presets — `Debug`, `Development` and `Release` — each with its own directory under `Build\`. `Release` gives minimal logs and debug information; the other two give progressively more.

| Target | What it does |
| --- | --- |
| `cmake --build Build\Release` | build everything |
| `cmake --build Build\Release --target launch` | build and open the editor |
| `cmake --build Build\Release --target carla-python-api-install` | build and install the Python client |
| `cmake --build Build\Release --target package` | cook and package a shipping server |
| `cmake --build Build\Release --target carla-help` | list every target and option |

!!! Note
    The native ROS 2 interface (`-DENABLE_ROS2=ON`) is built on Linux only; `CarlaSetup.bat` does not enable it.

---
## Build a package with CARLA UE5

```sh
cmake --build Build\Release --target package
```

Use `package-development` instead for a package that writes debugging logs.

The package lands in `Build\Release\Package\Carla-<version>-Win64-Shipping\`, alongside a `.zip` of the same. The Python wheel that matches it is in `PythonAPI\carla\dist` inside the package.

!!! Warning
    On Windows the cook needs a Zen server already running, because the package target passes `-NoZenAutoLaunch`. Start `zenserver.exe --port 8558` before packaging, and leave it running until the stage step finishes.

### Select which maps to cook

By default a package cooks every map listed in `DefaultGame.ini` — `Town10HD_Opt`, `OpenDriveMap`, `TestMaps/EmptyMap`, `Mine_01` and `Town15/Town15`. To cook a subset, set `CARLA_MAPS_TO_COOK` when configuring, as a `+`-separated list of map package paths:

```sh
cmake --preset Release -DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/Town10HD_Opt+/Game/Carla/Maps/Mine_01"
```

Use the full map package path (`/Game/Carla/Maps/<MapName>`), not a filesystem path. Note that the large maps nest: Town15 is `/Game/Carla/Maps/Town15/Town15`. Leaving `CARLA_MAPS_TO_COOK` empty, the default, cooks the full list from `DefaultGame.ini`.

!!! Note
    `Town10HD_Opt` is the default map a package boots into. If your selection excludes it, set a different default map in `DefaultEngine.ini` before packaging, otherwise the packaged server starts with a missing-map error.

### Run the package

Extract the package that you built, then from a command terminal open in the root folder of the extracted package, run the `CarlaUnreal.exe` file:

```sh
CarlaUnreal.exe
```
