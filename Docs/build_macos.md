# Building CARLA on macOS with Unreal Engine 5.5

!!! note
    The Unreal Engine 5 version of CARLA on macOS targets Apple Silicon (M1 / M2 / M3 / M4). Apple drops support for Rosetta with macOS27 anyways. Verified host: macOS 26 (Tahoe), Xcode 26 with Apple Clang 21.

* __[Set up the environment](#set-up-the-environment)__
* __[Build and run CARLA UE5](#build-and-run-carla-ue5)__
* __[Build the libCARLA C++ client (alternative to the Python API)](#build-the-libcarla-c-client-alternative-to-the-python-api)__
* __[Build a package with CARLA UE5](#build-a-package-with-carla-ue5)__
* __[Build presets](#build-presets)__
* __[Extended build instructions](#extended-build-instructions)__

## Set up the environment

Clone this repository locally from GitHub, specifying the *ue5-dev* branch:

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
cd CarlaUE5
```

In order to build CARLA, you need access to the CARLA fork of Unreal Engine 5.5. In order to access this repository, you must first link your GitHub account to Epic Games by following [this guide](https://www.unrealengine.com/en-US/ue-on-github). You then also need to use your git credentials to authorise the download of the Unreal Engine 5.5 repository.

Install the prerequisites with Homebrew:

```sh
brew install cmake ninja gnu-getopt python@3.10 coreutils autoconf automake libpng libtiff jpeg-turbo libtool aria2 git-lfs qt@6
/opt/homebrew/bin/python3.10 -m pip install --user --break-system-packages 'numpy<2' distro build wheel setuptools
```

!!! warning
    Do not use a Python virtualenv / conda env. Use `pip install --user --break-system-packages`. The build volume must be APFS (not ExFAT / FAT32). Wine / Winery / Kegworks / D3DMetal wrappers are unsupported — this is a 100% *native* arm64 port for macOS.

Set `CARLA_UNREAL_ENGINE_PATH` to the location of the CARLA fork of Unreal Engine 5.5 (see [Extended build instructions](#extended-build-instructions) if you have not built it yet):

```sh
export CARLA_UNREAL_ENGINE_PATH=/your/path/to/CarlaUnrealEngine
```

## Build and run CARLA UE5

Configure:

```sh
cmake --preset Release \
  -DPython_EXECUTABLE=/opt/homebrew/bin/python3.10 \
  -DPython3_EXECUTABLE=/opt/homebrew/bin/python3.10
```

!!! note
    Both `Python_EXECUTABLE` and `Python3_EXECUTABLE` must be set explicitly. Boost.Python's nested `find_package(Python)` does not honour `Python3_EXECUTABLE` alone or you will need to find modify lines find_package(Python3) in some packages if you wish to later re-build your-way

Build CARLA:

```sh
cmake --build Build/Release
```

Build and install the Python API:

```sh
cmake --build Build/Release --target carla-python-api-install
```

Launch the simulator (loose-files run from the source tree):

```sh
cmake --build Build/Release --target launch-only
```

## Build the libCARLA C++ client (alternative to the Python API)

The same source tree builds a standalone `libcarla-client` static library that can be consumed from any C++ project via CMake `find_package(Carla)`. Use this when you do not need the Python wheel.

Configure with the C++ client target enabled and the Python API disabled:

```sh
cmake --preset Release -DBUILD_CARLA_CLIENT=ON -DBUILD_PYTHON_API=OFF
cmake --build Build/Release --target carla-client
cmake --install Build/Release --prefix /opt/carla
```

In your own project's `CMakeLists.txt`:

```cmake
find_package(Carla CONFIG REQUIRED)
target_link_libraries(myapp PRIVATE Carla::carla-client)
```

A minimal C++ client demo lives under `Examples/CppClient/`.

## Build a package with CARLA UE5

```sh
cmake --build Build/Release --target carla-unreal-package
```

The package is generated in `Build/Release/Package/`. After packaging, three Mac-only post-cook steps are required before the `.app` will launch.

### 1. Fix `@rpath` entries

UE5's macOS packaging emits `@loader_path` rpaths that resolve outside the `.app` bundle. Add the correct paths:

```sh
APP=$(ls -d Build/Release/Package/Carla-*-Mac-Shipping/Mac/CarlaUnreal-Mac-Shipping.app | tail -1)
BIN="$APP/Contents/MacOS/CarlaUnreal-Mac-Shipping"
for p in TBB MetalShaderConverter Ogg Vorbis EOSSDK; do
  install_name_tool -add_rpath "@executable_path/../UE/Engine/Binaries/ThirdParty/$p/Mac" "$BIN"
done
install_name_tool -add_rpath "@executable_path/../UE/Engine/Plugins/NNE/NNERuntimeORT/Binaries/ThirdParty/Onnxruntime/Mac" "$BIN"
```

### 2. Re-codesign

`install_name_tool` invalidates the existing signature. Re-sign adhoc:

```sh
codesign --force --deep --sign - "$APP"
/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f "$APP"
```

### 3. Allow inbound connections in the Application Firewall

The CARLA RPC server needs to `bind()` on TCP port 2000. Add the `.app` to the firewall allow-list:

`System Settings → Network → Firewall → Options → +` → select `CarlaUnreal-Mac-Shipping.app` → set "Allow incoming connections".

### Run the package

```sh
open -a "$APP" --args -windowed -ResX=1280 -ResY=720 -carla-rpc-port=2000
```

If you want the native ROS2 interface, add the `--ros2` argument. ROS2 bridge is currently disabled by default on macOS.

If you want to install the Python API corresponding to the package you have built:

```sh
pip3 install Build/Release/PythonAPI/dist/carla-*.whl
```

## Build presets

There are 3 presets to choose from: `Debug`, `Development`, `Release`. The release preset provides minimal debug information or logs.

```sh
cmake --preset Release
cmake --build Build/Release --target launch-only
```

---

## Extended build instructions

Use these instructions if you need more control over the setup process.

### Install prerequisites

```sh
brew install cmake ninja gnu-getopt python@3.10 coreutils autoconf automake libpng libtiff jpeg-turbo libtool aria2 git-lfs qt@6
/opt/homebrew/bin/python3.10 -m pip install --user --break-system-packages 'numpy<2' distro build wheel setuptools
```

### Clone and build the Unreal Engine 5.5

Clone the Unreal Engine fork:

```sh
git clone https://github.com/CarlaUnreal/UnrealEngine
cd UnrealEngine
git checkout ue5-dev-carla
```

Build Unreal Engine 5.5:

```sh
./Setup.command
./GenerateProjectFiles.command
./Engine/Build/BatchFiles/Mac/Build.sh ShaderCompileWorker Mac Development
./Engine/Build/BatchFiles/Mac/Build.sh UnrealEditor Mac Development
```

!!! note
    The first build of Unreal Engine may take 1–3 hours.

Set up an environment variable pointing to the engine:

```sh
export CARLA_UNREAL_ENGINE_PATH=$(pwd)
```

### Clone the CARLA code and content

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
cd CarlaUE5/Unreal/CarlaUnreal
mkdir -p Content
cd Content
git clone --single-branch --depth 1 -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla
```

The content repository uses Git LFS; the clone may take a while and consume tens of GB.

### Build and launch CARLA

```sh
cd CarlaUE5
cmake --preset Release \
  -DPython_EXECUTABLE=/opt/homebrew/bin/python3.10 \
  -DPython3_EXECUTABLE=/opt/homebrew/bin/python3.10
cmake --build Build/Release --target carla-python-api-install
cmake --build Build/Release --target launch-only
cmake --build Build/Release --target carla-unreal-package
```

After `carla-unreal-package`, run the [post-cook fixups](#1-fix-rpath-entries) before launching the `.app`.