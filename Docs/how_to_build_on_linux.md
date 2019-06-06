<h1>How to build CARLA on Linux</h1>

!!! note
    CARLA requires Ubuntu 16.04 or later.

Install the build tools and dependencies

```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -
sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main"
sudo apt-get update
sudo apt-get install build-essential clang-7 lld-7 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl wget unzip autoconf libtool
pip2 install --user setuptools
pip3 install --user setuptools
```

To avoid compatibility issues between Unreal Engine and the CARLA dependencies,
the best configuration is to compile everything with the same compiler version
and C++ runtime library. We use clang 6.0 and LLVM's libc++. We recommend to
change your default clang version to compile Unreal Engine and the CARLA
dependencies

```sh
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-7/bin/clang++ 170
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-7/bin/clang 170
```

Build Unreal Engine
-------------------

!!! note
    Unreal Engine repositories are set to private. In order to gain access you
    need to add your GitHub username when you sign up at
    [www.unrealengine.com](https://www.unrealengine.com).

Download and compile Unreal Engine 4.22. Here we will assume you install it at
`~/UnrealEngine_4.22", but you can install it anywhere, just replace the path
where necessary.

```sh
git clone --depth=1 -b 4.22 https://github.com/EpicGames/UnrealEngine.git ~/UnrealEngine_4.22
cd ~/UnrealEngine_4.22
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
that downloads and extracts the latest version (note that this package is >3GB,
this step might take some time depending on your connection)

```sh
./Update.sh
```

For CARLA to find your Unreal Engine's installation folder you need to set the
following environment variable

```sh
export UE4_ROOT=~/UnrealEngine_4.22
```

You can also add this variable to your `~/.bashrc` or `~/.profile`.

Now that the environment is set up, you can use make to run different commands
and build the different modules

```sh
make launch     # Compiles the simulator and launches Unreal Engine's Editor.
make PythonAPI  # Compiles the PythonAPI module necessary for running the Python examples.
make package    # Compiles everything and creates a packaged version able to run without UE4 editor.
make help       # Print all available commands.
```

Updating CARLA
--------------

Every new release of CARLA, we release too a new package with the latest changes
in the CARLA assets. To download the latest version and recompile CARLA, run

```sh
make clean
git pull
./Update.sh
make launch
```

- - -

<h2>Assets repository (development only)</h2>

Our 3D assets, models, and maps have also a
[publicly available git repository][contentrepolink]. We regularly push latest
updates to this repository. However, using this version of the content is only
recommended to developers, as we often have work in progress maps and models.

Handling this repository requires [git-lfs][gitlfslink] installed in your
machine. Clone this repository to "Unreal/CarlaUE4/Content/Carla"

```sh
git lfs clone https://bitbucket.org/carla-simulator/carla-content Unreal/CarlaUE4/Content/Carla
```

It is recommended to clone with "git lfs clone" as this is significantly faster
in older versions of git.

[contentrepolink]: https://bitbucket.org/carla-simulator/carla-content
[gitlfslink]: https://git-lfs.github.com/
