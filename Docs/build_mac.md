# Mac build

This guide details how to build CARLA from source on MacOS Monterey (Apple Silicon). This work is mostly experimental and serves primarily as a proof-of-concept for those interested in building CARLA 0.9.13 on the Apple Silicon devices. 

---
## Prerequisites

### System requirements

* __MacOS 12__ This is for building on MacOS Monterey (12). The latest version this was tested on is 12.1 on my M1 Max.
* __130 GB disk space.__ Carla will take around 31 GB and Unreal Engine will take around 91 GB so have about 130 GB free to account for both of these plus additional minor software installations. 
* __An adequate GPU.__ The M1 GPU's are very capable, though it is still recommended to get the Pro/Max's since I've found the performance to scale nearly linearly with number of GPU cores. 
* __Two TCP ports and good internet connection.__ 2000 and 2001 by default. Make sure that these ports are not blocked by firewalls or any other applications. 

..warning::
    __If you are upgrading from CARLA 0.9.12 to 0.9.13__: you must first upgrade the CARLA fork of the UE4 engine to the latest version. See the [__Unreal Engine__](#unreal-engine) section for details on upgrading UE4

**WARNING**: UE4-carla does not build on MacOS 12 by default! See [this discussion](https://github.com/carla-simulator/carla/discussions/4848) for more details.


### Software requirements

CARLA requires many different kinds of software to run. Some are built during the CARLA build process itself, such as *Boost.Python*. Others are binaries that should be installed before starting the build (*cmake*, *clang*, different versions of *Python*, etc.). To install these requirements, run the following commands:

```bash
brew update && brew upgrade

brew install --build-from-source mono

brew install coreutils ninja wget autoconf automake curl libtool libpng aria2 libiconv
```

I recommend installing python through conda on mac:
```bash
conda create --name carla python=3.8
conda activate carla
conda install numpy distro wheel
pip install setuptools==47.3.1 nose2
```

(Install xcode through the app store, I used the latest (at time of writing) 12.2.1)
And then open up xcode and install the `xcode command line developer tools`
```bash
xcode-select --install
sudo xcode-select -s /Applications/Xcode.app
```

Finally, you'll also need [Rosetta 2](https://support.apple.com/en-us/HT211861) for the translation between x86 (UE4 prerequisite) and arm64 (native Apple Silicon)

---

## Unreal Engine

For now, refer to the Linux guide for how to build the engine, but note the [strange problem](https://github.com/carla-simulator/carla/discussions/4848) with building the Carla fork (not Vanilla 4.26.2) on Apple silicon.

## Build CARLA 

Note: This time, the build procedure is somewhat hacky and fragmanted by nature of using different architectures, but it still works reasonably well. The procedure is as follows:
1. Build LibCarla in x86
2. Build the UE4 carla game (also x86)
3. Build LibCarla (AGAIN) in arm64
4. Build the PythonAPI in arm64
5. Add the arm64 Python arm .egg file to your PYTHONPATH
6. Build the UE4 package in x86 (back to x86)


This is because the python scripts work much better in `arm64` but the game itself (which depends on UE4) must be built for x86 and run on rosetta2.

The way I recommend you go about this is as follows:
```bash
# git clone and enter mac branch
./Update.sh # wait a while for the asset fetching to complete

# go to Util/Buildtools/Environment.sh
# make sure the ARCH (for IS_MAC) is set to "x86_64"

make LibCarla # in x86
...

make launch # in x86 (uses UE4 build and xcode cmd line)
...

# go to Util/Buildtools/Environment.sh
# make sure the ARCH (for IS_MAC) is set to "arm64"
mv Build Build.x86 # denoting the current (x86) build as such

make LibCarla # now in arm64!
...

make PythonAPI # now in arm64
...

make check # unit tests to verify everything works!
...

mv Build Build.arm64

# now you can switch between (re)building arm64 and x86 by renaming
# the Build.XYZ to just Build
# example:

mv Build.x86 Build

make package
...

```

Also, as an aside, since (almost) all the PythonAPI scripts look for a `'linux-x86_64'` `.egg` file, you'll not be able to find it unless you modify EVERY single file, since this is too much work, you can simply add this to your PYTHONPATH via:
```bash
export PYTHONPATH="${PYTHONPATH}:/PATH/TO/CARLA/PythonAPI/carla/dist/carla-0.9.13-py3.8-macosx-11.0-arm64.egg"
# I simply put this in my .zshrc file
```
Then you should be able to quickly `import carla` without hassle!