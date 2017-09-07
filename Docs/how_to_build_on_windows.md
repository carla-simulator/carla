How to build CARLA on Windows
=============================

Install Unreal Engine 4.17 and Visual Studio 2015. (Later versions of Visual
Studio work, but you will need to adapt "Rebuild.bat" to find the right
"vcvarsall.bat").

Setup CARLA plugin
------------------

A setup script is not yet available. You need to compile yourself boost,
protobuf and googletest, and install them under ./Util/Build as

    Util/
        Build/
            boost-install/
            googletest-install/
            protobuf-install/


You can also download the precompiled binaries of these libraries
[here](http://datasets.cvc.uab.es/CARLA/CarlaUE4_Dependencies_Win64.zip)
(requires password).

Some Unix utilities need to be installed for the build system to work, including
Make and CMake.

Build and launch CARLAUE4
-------------------------

Once the dependencies are there, you just need to run the rebuild script
(double-click works). This script deletes all intermediate files, rebuilds CARLA
plugin, and launches the editor. You will be prompted a couple of times.

    $ Rebuild.bat

If it warns you about plugin CARLA not being compatible, ignore the warning and
do not disable the plugin.

If the rebuild script fails, you will need to
[build CARLA plugin](#Build CARLA Plugin) yourself. Then build CARLAUE4 from
Visual Studio.

Other
-----

#### Build CARLA Plugin

Go to the folder CARLAUE4/Plugins/Carla, and open a terminal there.

Dependencies need to be installed under ./Util/Build.

Configure your environment running "vcvarsall.bat". For Visual Studio 2015, this
script is located at (don't forget the "amd64" part)

    $ "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

Then run make

    $ make
