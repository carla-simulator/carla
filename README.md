CARLA
=====

Plugin for Unreal Engine 4.

Setup
-----

#### Linux

The following has been tested only in Ubuntu 16.04.

Install clang 3.9

    sudo apt-get install -y build-essential
    wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -
    sudo apt-get update
    sudo apt-get install -y clang-3.9 clang++-3.9

You may need to change your default clang version to compile Unreal

    sudo ln -s /usr/bin/clang-3.9 /usr/bin/clang
    sudo ln -s /usr/bin/clang++-3.9 /usr/bin/clang++

Download and install Unreal Engine 4.15

    git clone --depth=1 -b 4.15 https://github.com/Epic... ~/UnrealEngine_4.15
    cd ~/UnrealEngine_4.15
    ./Setup.sh && ./GenerateProjects.sh && make

Clone CARLA to your project

    cd path/to/your/ue4-project
    mkdir -p Plugins && cd Plugins
    git clone --depth=1 ......  Carla
    cd Carla

Now run Setup,sh (will build all the dependencies, takes a while) and make

    ./Setup.sh
    make

Recommended to do a check to see if everything worked fine

    make check

Now build your project with Unreal normally.

#### Windows

Not yet available.

Run-time settings
-----------------

See [Resources/Example.CarlaSettings.ini](Resources/Example.CarlaSettings.ini).
