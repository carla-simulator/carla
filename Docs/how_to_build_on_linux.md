How to build CARLA on Linux
===========================

This guide has been tested only on Ubuntu 16.04.

Install the build tools and dependencies

    $ sudo apt-get install build-essential cmake ninja-build python3-pip python-dev curl autoconf libtool
    $ sudo pip3 install protobuf

To avoid compatibility issues between Unreal Engine and the CARLA dependencies,
the best configuration is to compile everything with the same compiler version
and C++ runtime library. We use clang 3.9 and LLVM's libc++.

Install clang 3.9

    $ sudo apt-get install clang-3.9

You may need to change your default clang version to compile Unreal

    $ sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-3.9/bin/clang++ 100
    $ sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-3.9/bin/clang 100

Build Unreal Engine
-------------------

Download and compile Unreal Engine 4.17. Here we will assume you install it at
"~/UnrealEngine_4.17", but you can install it anywhere, just replace the path
where necessary.

Unreal Engine repositories are set to private. In order to gain access you need
to add your GitHub username when you sign up at https://unrealengine.com.

    $ git clone --depth=1 -b 4.17 https://github.com/EpicGames/UnrealEngine.git ~/UnrealEngine_4.17
    $ cd ~/UnrealEngine_4.17
    $ ./Setup.sh && ./GenerateProjectFiles.sh && make

Check Unreal's documentation
["Building On Linux"](https://wiki.unrealengine.com/Building_On_Linux) if any of
the steps above fail.

Build CARLA
-----------

Run the setup script to download the content and build all dependencies. It
takes a while

    $ ./Setup.sh

Once it's done it should print "Success" if everything went well.

Download Epic Games' Automotive Materials package and install it under
"Unreal/CarlaUE4/Content/AutomotiveMaterials".
[How to download automotive materials](how_to_add_automotive_materials.md).

IMPORTANT: Do not forget to link the automotive materials after opening the
project.

NOTE: Due to license restrictions, pedestrians are not include in the CARLA open
source project (only in the compiled binaries). Some warnings may appear when
starting the project related to this. We are working to find a solution.

To build CARLA, use the rebuild script. This script deletes all intermediate
files, rebuilds whole CARLA, and launches the editor. Use it too for making a
clean rebuild of CARLA

    $ UE4_ROOT=~/UnrealEngine_4.17 ./Rebuild.sh

It looks at the environment variable `UE4_ROOT` to find the right version of
Unreal Engine. You can also add this variable to your "~/.bashrc" or similar.

Once the project is opened, it is required to manually link Epic's Automotive
Material to our vehicles.
[How to link automotive materials](how_to_add_automotive_materials.md).

Later, if you need to compile some changes without doing a full rebuild, you can
use the Makefile generated in the Unreal project folder

    $ cd Unreal/CarlaUE4
    $ make CarlaUE4Editor

Launching the editor
--------------------

To open the editor once the project is already built

    $ cd Unreal/CarlaUE4
    $ ~/UnrealEngine_4.17/Engine/Binaries/Linux/UE4Editor "$PWD/CarlaUE4.uproject"

Test (Optional)
---------------

A set of unit tests is available for testing the CarlaServer library (note that
these tests launch the python client, they require python3 and protobuf for
python3 installed, as well as ports 2000 and 4000 available)

    $ make check
