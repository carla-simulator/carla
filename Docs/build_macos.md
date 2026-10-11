# Building CARLA on macOS with Unreal Engine 5.8

!!! note
    The Unreal Engine 5.8 version of CARLA on macOS targets Apple Silicon (M1 / M2 / M3 / M4 / M5). Rosetta wrappers are unsupported — this is a native arm64 build for macOS. Verified host: macOS 14+ / macOS 15+ (Apple Silicon), Xcode 15/16/26 with Apple Clang 16/21.

* __[Set up the environment](#set-up-the-environment)__
* __[Automated Build and Setup (CarlaSetup.sh)](#automated-build-and-setup-carlasetupsh)__
* __[Running the CARLA Simulator](#running-the-carla-simulator)__
* __[Incremental Rebuilds for C++ Developers](#incremental-rebuilds-for-c-developers)__
* __[Working with Content and Assets](#working-with-content-and-assets)__
* __[Build a package with CARLA UE 5.8](#build-a-package-with-carla-ue-58)__
* __[Build the libCARLA C++ client (alternative to the Python API)](#build-the-libcarla-c-client-alternative-to-the-python-api)__
* __[Extended build instructions (Manual Unreal Engine 5.8 Build)](#extended-build-instructions-manual-unreal-engine-58-build)__

---

## Set up the environment

Clone this repository locally from GitHub, specifying the *ue58-dev-mac* branch:

```sh
git clone -b ue58-dev-mac https://github.com/carla-simulator/carla.git CarlaUE58
cd CarlaUE58
```

In order to build CARLA, you need access to the CARLA fork of Unreal Engine 5.8. Link your GitHub account to Epic Games by following [this guide](https://www.unrealengine.com/en-US/ue-on-github), then authorise the download of the Unreal Engine repository.

### Prerequisites

1. **Xcode and Metal Toolchain Component:**
   Make sure Xcode and command line tools are installed and selected (`sudo xcode-select -s /Applications/Xcode.app`).
   Unreal Engine on macOS requires the Xcode Metal Toolchain to compile shaders for the Metal RHI. Download and install it via `xcodebuild`:

   ```sh
   xcodebuild -downloadComponent MetalToolchain
   ```

2. **System packages via Homebrew:**

   ```sh
   brew install cmake ninja gnu-getopt python@3.10 coreutils autoconf automake libpng libtiff jpeg-turbo libtool aria2 git-lfs qt@6
   ```

3. **Python dependencies:**
   Install required Python build and runtime packages using Python 3.10:

   ```sh
   /opt/homebrew/bin/python3.10 -m pip install --user --break-system-packages numpy distro build wheel setuptools scikit-build-core pygame requests psutil
   ```

!!! warning
    Do not use a Python virtualenv or conda environment for the CARLA build. Use `pip install --user --break-system-packages`. The build volume must be APFS (case-insensitive or case-sensitive APFS; not ExFAT / FAT32).

4. **Environment variables (Optional):**
   If you already have a checkout of the CARLA Unreal Engine 5.8 fork, point to it:

   ```sh
   export CARLA_UNREAL_ENGINE_PATH=/path/to/UnrealEngine5_carla
   ```
   *(If not set, `CarlaSetup.sh` will automatically clone and build it into a sibling folder).*

---

## Automated Build and Setup (`CarlaSetup.sh`)

CARLA provides a single automated script that performs the complete setup and build:

```sh
./CarlaSetup.sh
```

### What `CarlaSetup.sh` does automatically:
1. **Prerequisites & Tools:** Verifies macOS environment, Homebrew packages, and Git LFS.
2. **Content:** Clones the CARLA Content repository (`Unreal/CarlaUnreal/Content/Carla`) if not already present.
3. **Unreal Engine 5.8:** Clones `CarlaUnreal/UnrealEngine` (branch `ue58-dev-carla`), sets up DotNET and project files, and compiles `ShaderCompileWorker` and `UnrealEditor`.
4. **CMake Configuration:** Configures the CARLA project in `Build/` with Apple Silicon and Metal settings.
5. **CARLA Compilation:** Builds LibCarla, dependencies, and Unreal Engine plugins (`cmake --build Build`).
6. **Python API:** Builds and installs the `carla` Python 3.10 wheel directly into your user site-packages (`cmake --build Build --target carla-python-api-install`).

Once `./CarlaSetup.sh` completes, CARLA is fully built and ready to run.

!!! tip
    You can optionally pass `--launch` to have `CarlaSetup.sh` launch the Unreal Editor immediately after the build finishes:
    ```sh
    ./CarlaSetup.sh --launch
    ```

---

## Running the CARLA Simulator

There are two primary modes to run the simulator:

### Method A: "Game" Mode (Instant & Recommended for Development)

Game mode runs the engine binary directly against the project content in `-game` mode using Apple Silicon Metal. It starts immediately without needing to cook packages and supports all 29 maps.

- **Windowed Simulation (interactive window on desktop):**
  ```sh
  WINDOW=1 RES=1920x1080 bash Skills/ue58/run-carla-ue58-server/scripts/run_server.sh game Town10HD_Opt
  ```
  *Or directly via the engine binary:*
  ```sh
  $CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor \
    Unreal/CarlaUnreal/CarlaUnreal.uproject \
    Town10HD_Opt -game -metal -windowed -ResX=1920 -ResY=1080 -carla-rpc-port=2000
  ```

- **Headless / Background Simulation (for scripts, benchmarks, data collection):**
  ```sh
  DETACH=1 bash Skills/ue58/run-carla-ue58-server/scripts/run_server.sh game Town10HD_Opt
  ```
  To stop the server:
  ```sh
  bash Skills/ue58/run-carla-ue58-server/scripts/run_server.sh stop
  ```

### Method B: Unreal Editor GUI

Open the full Unreal Editor with the 3D viewport, scene outliner, and blueprint editors:

```sh
cmake --build Build --target launch-only
```

!!! note
    On the **very first launch**, Unreal Engine compiles the initial Derived Data Cache (DDC) and Metal shaders for all CARLA content. You will see worker processes utilizing all CPU cores. Allow this compilation to finish (typically 3–5 minutes on Apple Silicon M-series chips), after which the main editor window will appear. Once cached, subsequent launches open quickly.

To test the running server with Python clients:
```sh
# Probe the server status:
bash Skills/ue58/run-carla-ue58-server/scripts/run_server.sh probe

# Spawn autopilot traffic:
python3.10 PythonAPI/examples/generate_traffic.py -n 25

# Manual keyboard driving:
python3.10 PythonAPI/examples/manual_control.py
```

---

## Incremental Rebuilds for C++ Developers

If you make modifications to CARLA's C++ source code (`LibCarla/`, `Unreal/CarlaUnreal/Plugins/Carla/`, etc.), **you do not need to re-run `CarlaSetup.sh`**. Simply trigger an incremental compile using CMake:

```sh
# Rebuild modified CARLA C++ libraries and Unreal plugins:
cmake --build Build

# Rebuild and re-install Python API (if Python bindings were changed):
cmake --build Build --target carla-python-api-install
```

If you ever need to manually reconfigure CMake from scratch:
```sh
cmake -B Build \
  -DCARLA_DLSS_SDK_PATH=disabled \
  -DENABLE_ROS2=OFF \
  -DPython_EXECUTABLE=/opt/homebrew/bin/python3.10 \
  -DPython3_EXECUTABLE=/opt/homebrew/bin/python3.10
```

---

## Working with Content and Assets

Content changes inside `Unreal/CarlaUnreal/Content/Carla` (`.uasset` files, materials, blueprints, maps, textures) do **not** require running CMake or C++ compilation:

- **In Editor / Game Mode:** Unreal Engine automatically detects modified assets when starting up and compiles any new Metal shaders into the local Derived Data Cache (`Unreal/CarlaUnreal/DerivedDataCache`).
- **Pre-caching DDC headlessly:** You can pre-build shaders and cache in the background using the DDC commandlet:
  ```sh
  $CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor \
    Unreal/CarlaUnreal/CarlaUnreal.uproject \
    -run=DerivedDataCache -fill
  ```
- **In Packaged Builds:** Packaged builds read from cooked IoStore containers (`.pak`/`.ucas`/`.utoc`). If you change assets and want them in your packaged release, rerun `cmake --build Build --target package` (it uses `-iterate` for fast incremental cooking).
- **Clearing corrupted cache:** If shaders or materials ever appear corrupt, wipe the project DDC:
  ```sh
  rm -rf Unreal/CarlaUnreal/DerivedDataCache
  rm -rf Unreal/CarlaUnreal/Intermediate/CachedAssetRegistry.bin
  ```

---

## Build a package with CARLA UE 5.8

### 1. Set the Cook Scope (`CARLA_MAPS_TO_COOK`)

!!! important
    **Never leave the cook scope empty.** An empty scope falls back to `DefaultGame.ini`, which includes `Town15`. `Town15` has an upstream material reference defect (573 unresolvable `MaterialInstanceDynamic` references) that aborts cooking with `ExitCode=25`.
    Always configure an explicit list of maps separated by `+`, and always include `OpenDriveMap`:

```sh
# Example: Cook Town10HD_Opt and OpenDriveMap
cmake -B Build -DCARLA_MAPS_TO_COOK="Town10HD_Opt+OpenDriveMap"

# Or multiple towns:
cmake -B Build -DCARLA_MAPS_TO_COOK="Town10HD_Opt+Town01+Town02+OpenDriveMap"
```

Verify your active scope with:
```sh
bash Skills/ue58/package-carla-ue58/scripts/package.sh scope
```

### 2. Build the Package

```sh
# Shipping release:
cmake --build Build --target package

# Development release (includes logging and debug console):
cmake --build Build --target package-development
```

The package is generated in `Build/Package/Carla-<version>-Mac-Shipping/`, along with the release tarball `Carla-<version>-Mac-Shipping.tar.gz`.

### 3. Run the Packaged Server

You can run the packaged server via the provided `CarlaUnreal.sh` launcher:
```sh
./Build/Package/Carla-*-Mac-Shipping/CarlaUnreal.sh \
  Town10HD_Opt -metal -windowed -ResX=1920 -ResY=1080 -carla-rpc-port=2000
```

*Or directly via the `.app` bundle:*
```sh
./Build/Package/Carla-*-Mac-Shipping/Mac/CarlaUnreal-Mac-Shipping.app/Contents/MacOS/CarlaUnreal-Mac-Shipping \
  Town10HD_Opt -metal -windowed -ResX=1920 -ResY=1080 -carla-rpc-port=2000
```

*Or via the runner script:*
```sh
WINDOW=1 bash Skills/ue58/run-carla-ue58-server/scripts/run_server.sh package Town10HD_Opt
```

### 4. Firewall Settings (If remote clients cannot connect)

The CARLA RPC server binds TCP port 2000. If macOS blocks incoming connections:
Go to `System Settings → Network → Firewall → Options` → add `CarlaUnreal.app` and set "Allow incoming connections".

---

## Build the libCARLA C++ client (alternative to the Python API)

The same source tree builds a standalone `libcarla-client` static library that can be consumed from any C++ project via CMake `find_package(Carla)` without needing Unreal Engine.

```sh
cmake -B Build -DBUILD_CARLA_CLIENT=ON -DBUILD_PYTHON_API=OFF -DCARLA_DLSS_SDK_PATH=disabled
cmake --build Build --target carla-client
cmake --install Build --prefix /opt/carla
```

In your own project's `CMakeLists.txt`:

```cmake
find_package(Carla CONFIG REQUIRED)
target_link_libraries(myapp PRIVATE Carla::carla-client)
```

A minimal C++ client demo lives under `Examples/CppClient/`.

---

## Extended build instructions (Manual Unreal Engine 5.8 Build)

If you are not using `CarlaSetup.sh` and want to build the Unreal Engine fork manually:

### 1. Clone the CARLA Unreal Engine 5.8 fork and checkout the pinned commit

```sh
git clone -b ue58-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
cd UnrealEngine5_carla
git checkout 0703595a065dee51538720027ce9cc05bd464517
```

### 2. Setup, Generate Project Files, and Apply macOS Patch

```sh
./Setup.command
./GenerateProjectFiles.command

# Apply the macOS compatibility patch shipped in CARLA:
git apply /path/to/CarlaUE58/Util/Patches/UnrealEngine_macOS.patch
```

*(This patch resolves Apple Clang / Xcode SDK version mapping, C++20 designated initializer ordering in MetalViewport, SWindow lambda initialization, and Cocoa launch gating).*

### 4. Build Engine Binaries

```sh
./Engine/Build/BatchFiles/Mac/Build.sh ShaderCompileWorker Mac Development
./Engine/Build/BatchFiles/Mac/Build.sh UnrealEditor Mac Development
export CARLA_UNREAL_ENGINE_PATH=$PWD
```

### 5. Clone the CARLA content

```sh
cd /path/to/CarlaUE58/Unreal/CarlaUnreal
mkdir -p Content
cd Content
git clone --single-branch --depth 1 -b ue58-dev-carla https://bitbucket.org/carla-simulator/carla-content.git Carla
```
