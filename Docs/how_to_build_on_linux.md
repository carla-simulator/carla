<h1>How to build CARLA on Linux</h1>

!!! note
    CARLA requires Ubuntu 16.04 or later.

Install the build tools and dependencies

```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install build-essential clang-5.0 lld-5.0 g++-7 ninja-build python python-pip python-dev libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl wget unzip autoconf libtool
pip install --user setuptools nose2
```

Note that some dependencies require **CMake 3.9** or later installed in your
machine, you can retrieve the latest version from the
[CMake download page][cmakelink].

To avoid compatibility issues between Unreal Engine and the CARLA dependencies,
the best configuration is to compile everything with the same compiler version
and C++ runtime library. We use clang 5.0 and LLVM's libc++. We recommend to
change your default clang version to compile Unreal Engine and the CARLA
dependencies

```sh
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-5.0/bin/clang++ 101
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-5.0/bin/clang 101
```

[cmakelink]: https://cmake.org/download/

Build Unreal Engine
-------------------

!!! note
    Unreal Engine repositories are set to private. In order to gain access you
    need to add your GitHub username when you sign up at
    [www.unrealengine.com](https://www.unrealengine.com).

Download and compile Unreal Engine 4.19. Here we will assume you install it at
`~/UnrealEngine_4.19", but you can install it anywhere, just replace the path
where necessary.

```sh
git clone --depth=1 -b 4.19 https://github.com/EpicGames/UnrealEngine.git ~/UnrealEngine_4.19
cd ~/UnrealEngine_4.19
./Setup.sh && ./GenerateProjectFiles.sh && make
```

Check Unreal's documentation
["Building On Linux"](https://wiki.unrealengine.com/Building_On_Linux) if any of
the steps above fail.

Build CARLA
-----------

Clone or download the project from our
[GitHub repository](https://github.com/carla-simulator/carla)

```sh
git clone https://github.com/carla-simulator/carla
```

Note that the `master` branch contains the latest fixes and features, for the
latest stable code may be best to switch to the `stable` branch.

Now you need to download the assets package, to do so we provide a handy script
that downloads and extracts the latest version (note that the package is >10GB,
this step might take some time depending on your connection)

```sh
./Update.sh
```

For CARLA to find your Unreal Engine's installation folder you need to set the
following environment variable

```sh
export UE4_ROOT=~/UnrealEngine_4.19
```

You can also add this variable to your `~/.bashrc` or `~/.profile`.

Now that the environment is set up, you can run make to run different commands

```sh
make launch   # Compiles CARLA and launches Unreal Engine's Editor.
make package  # Compiles CARLA and creates a packaged version for distribution.
make help     # Print all available commands.
```

Updating CARLA
--------------

Every new release of CARLA we release a new package with the latest changes in
the CARLA assets. To download the latest version and recompile CARLA, run

```sh
make clean
git pull
./Update.sh
make launch
```
