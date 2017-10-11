How to build CARLA on Linux
===========================

This guide has been tested only on Ubuntu 16.04.

Install the build tools and dependencies

    $ sudo apt-get install build-essential cmake ninja-build python3-pip
    $ sudo pip3 install protobuf

To avoid compatibility issues between Unreal Engine and the CARLA dependencies,
the best configuration is to compile everything with the same compiler version
and C++ runtime library. We use clang 3.9 and LLVM's libc++.

Install clang 3.9

    $ wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -
    $ sudo apt-get update
    $ sudo apt-get install -y clang-3.9 clang++-3.9

You may need to change your default clang version to compile Unreal

    $ sudo ln -s /usr/bin/clang-3.9 /usr/bin/clang
    $ sudo ln -s /usr/bin/clang++-3.9 /usr/bin/clang++

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

Setup CARLA plugin
------------------

Go to "CARLAUE4/Plugins/Carla" folder and run the setup script. This downloads
and compile all the dependencies, takes a while

    $ ./Setup.sh

Now you can use the Makefile to compile and test the code. However, the rebuild
script of the main project will do that already, see next section.

You can run the unit tests to check if everything worked fine (note that these
tests launch the python client, they require python3 and protobuf for python3
installed)

    $ make check

Build and launch CARLAUE4
-------------------------

In the root folder of CARLAUE4 you can find "Rebuild.sh" script. This deletes
all intermediate files, rebuilds whole CARLA, and launches the editor. Use it
every time you update CARLA.

    $ UE4_ROOT=~/UnrealEngine_4.17 ./Rebuild.sh

It looks at the environment variable `UE4_ROOT` to find the right version of
Unreal Engine. You can also add this variable to your "~/.bashrc" or similar.
