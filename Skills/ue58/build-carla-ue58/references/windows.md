# Building CARLA UE 5.8 on Windows

Taken from `Docs/build_windows_ue5.md`. Not run by this skill: nothing here was verified on a Windows machine, so treat each step as documented, not measured.

## Before you start

- Link your GitHub account to Epic Games, or the engine fork cannot be cloned.
- Turn on Windows Developer Mode; the build fails without it.
- Clone onto an internal disk. Windows does not give external disks the read, write and execute permissions the build needs.
- Visual Studio 2022 (Community, Professional or Enterprise) is used as installed. It needs the MSVC v14.38 toolset, or v14.44.35211 or later. To add v14.38, import `Unreal\CarlaUnreal\.vsconfig` in the Visual Studio Installer.
- Without `CARLA_UNREAL_ENGINE_PATH` and without a `..\UnrealEngine5_carla` folder, `CarlaSetup.bat` downloads and builds the engine there: more than one extra hour and about 225 GB. It sets `CARLA_UNREAL_ENGINE_PATH` only when it downloads the engine itself.

## CarlaSetup.bat

```bat
git clone -b ue58-dev https://github.com/carla-simulator/carla.git CarlaUE5
cd CarlaUE5
CarlaSetup.bat
```

It installs Visual Studio 2022 with CMake (when none is installed), Ninja, Python and its packages and the engine fork, downloads the content into `Unreal\CarlaUnreal\Content\Carla`, and builds CARLA.

| Flag | Effect |
|---|---|
| `--skip-prerequisites`, `-p` | skip Visual Studio, Ninja, Python and its packages |
| `--launch`, `-l` | open the editor when the build finishes |
| `--python-root=C:\Path\To\PythonDir` | use this Python (the folder holding `python.exe`); otherwise the first one on `PATH`, and Python 3.8.10 is installed when there is none. 3.9 and 3.10 also work. |

## Rebuilding

Run from the **x64 Native Tools Command Prompt for VS 2022**, opened at the checkout:

```bat
cmake -G Ninja -S . -B Build --toolchain=CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```

With `--python-root`, add `-DPython_ROOT_DIR=PATH -DPython3_ROOT_DIR=PATH`. DLSS comes from `-DCARLA_DLSS_SDK_PATH=<dir>`, the `DLSS_SDK` variable, or `%USERPROFILE%\SDKs\DLSS` (cloned there when missing); `-DCARLA_DLSS_SDK_PATH=disabled` builds without it.

| Target | What it does |
|---|---|
| `cmake --build Build` | LibCarla, its dependencies and the examples; not the editor |
| `cmake --build Build --target carla-unreal-editor` | the editor, without opening it |
| `cmake --build Build --target launch` | build and open the editor with a log window |
| `cmake --build Build --target launch-only` | open the editor without rebuilding |
| `cmake --build Build --target carla-python-api-install` | build and install the Python client |
| `cmake --build Build --target carla-help` | list every target and option |

To drop the log window, reconfigure with `-DCARLA_LAUNCH_ARGS=`; `-DCARLA_UNREAL_LOG_WINDOW=OFF` only takes effect on a new build directory. The native ROS 2 interface is Linux-only.

## Packaging

`cmake --build Build --target package` writes `Build\Package\Carla-<version>-Win64-Shipping\` and a `.zip` of it (`-DCARLA_UNREAL_PACKAGE_NO_COMPRESSION=ON` skips the zip), plus `carla-<version>-Win64-release-metadata.tar.gz` for content packs. The matching wheel is in the package's `PythonAPI\carla\dist`. `package-development`, `package-debug`, `package-debuggame`, `package-test` and `package-shipping` build the other configurations. `CARLA_MAPS_TO_COOK` selects maps exactly as on Linux. Start the package with `CarlaUnreal.exe` from its root; the run-carla-ue58-server skill's `package` mode drives it from Git Bash.
