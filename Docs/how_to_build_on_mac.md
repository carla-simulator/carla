# How to build CARLA on Mac OSX 

Note that unlike the Linux build, this one uses the standard Mac compiler tools and
the standard Unreal Engine 4.21 distribution.

Prerequisites
-------------

### Install Xcode 9.4

*Later versions of Xcode may work, but using the same version that is used by Unreal 4.21 seems a safe bet.*

You should be able to install from [Apple's developer downloads](https://developer.apple.com/download/more/).

If you have installed more than one version of Xcode, you should activate 9.4 using
the `xcode-select` command line utility. This will set `/usr/bin/clang` and `/usr/bin/clang++`
to the appropriate versions, and these are what will be used in the build.

Note that Apple's has it's own clang versioning scheme that tracks the Xcode versions,
so it is not obvious how the features compare to the LLVM distributions, but this version
should fully support c++14 features.

**On Mac OSX Mojave 10.14 you also need to install the MacOS headers using:**

~~~sh
sudo installer -pkg /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg -target /
~~~

### Install Unreal Engine 4.21

Install the [Epic Games Launcher](https://www.epicgames.com/unrealtournament/download)
and use it to download version 4.21 of the Unreal Engine. The default install location
is `/Users/Shared/Epic Games/`, but it is a good idea to eliminate the space in the path
and instead use `/Users/Shared/EpicGames/` since some tools seem to have problems with
the space.

Although, you don't need to build the engine from source, you do need to add the file
[GenerateProjectFiles.sh](https://github.com/EpicGames/UnrealEngine/blob/4.21/GenerateProjectFiles.sh) to the root directory from a copy of the Unreal Engine source tree.

### Install the build tools and dependencies

~~~sh
$ brew install autoconf curl libtool ninja wget \
   libpng
~~~

Use shipping python or install using your favorite method (e.g. brew, download installer from python.org,
Anaconda/miniconda).

If using conda environment:

~~~sh
$ conda install -c conda-forge nose2
~~~

else:

~~~sh
$ pip2 install nose2
~~~

In order for `boost-python` to build properly, you may need to add a `user-config.jam` file
to your home directory describing the location of your python implementations. For instance,
to use the preinstalled python2 and a python 3.6 from python.org installer you might use:

~~~jam
import toolset : using ;

using python : 2.7 
	: /usr/bin/python2.7
	: /usr/include/python2.7
	;

using python : 3.6 
	: /Library/Frameworks/Python.framework/Versions/3.6/bin/python3.6
	: /Library/Frameworks/Python.framework/Versions/3.6/include/python3.6m
	: /Library/Frameworks/Python.framework/Versions/3.6/lib
	;
~~~

Currently, the build will only support the first listed python. Eventually, it should
support building apis for multiple python versions.

Build CARLA
-----------

Clone or download the project from our
[GitHub repository](https://github.com/carla-simulator/carla)

```sh
git clone https://github.com/carla-simulator/carla
```

Note that the `master` branch contains the latest fixes and features, for the
latest stable code may be best to switch to the latest release tag.

Now you need to download the assets package, to do so we provide a handy script
that downloads and extracts the latest version (note that the package is >12GB,
this step might take some time depending on your connection)

```sh
./Update.sh
```

For CARLA to find your Unreal Engine's installation folder you need to set the
following environment variable

```sh
export UE4_ROOT=/Users/shared/EpicGames/UE_4.21
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

