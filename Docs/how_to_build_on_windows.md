<h1>How to build CARLA on Windows</h1>

!!! note
    We are working on an automated build system for Windows, you can follow
    the progress at [issue #21](https://github.com/carla-simulator/carla/issues/21).

# Manual Installation
## Necessary software:
- [Git](https://git-scm.com/downloads)
- [Make](http://gnuwin32.sourceforge.net/downlinks/make-bin-zip.php)
- [Cmake](https://cmake.org/download/)

!!! important
    Be sure that these programs are added to your path, so you can use them from your command prompt.

Also:

- [Unreal Engine](https://www.unrealengine.com/download) (v4.18.x)
- [Visual Studio](https://www.visualstudio.com/downloads/) (2017 preferably)

### Environment Setup
In order to build CARLA you must **enable the x64 Visual C++ Toolset**.
I recommend to use this environment for everything you do in this tutorial.

You have different options:

- **Recomended:** Use [`Visual Studio x64 Native Tools Command Prompt`](https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs). Just press the `Win` button and search for `x64`, because the name of this Command Prompt can change depending on the lenguage you have installed Visual Studio.
- [Enable a 64-Bit Visual C++ Toolset on the Command Line](https://msdn.microsoft.com/en-us/library/x4d2c09s.aspx) (the instructions will depend on the version of VS that you have).

### Clone the repository
Go to the path you want to install CARLA and type in your prompt:

```cmd
git clone https://github.com/carla-simulator/carla.git
```

### Download the assets
Download the assets from the version you need. Build the download link with `https://drive.google.com/open?id=` followed by the choosen hash that you will find in [`Util/ContentVersions.txt`](https://github.com/carla-simulator/carla/blob/master/Util/ContentVersions.txt).

Create the folder `Content` in `Unreal/CarlaUE4/` and unzip the content you just downloaded here.

## Dependencies
Download and build **for win64**:

- Boost 1.64
- Protobuf 3.3.2

### Building Boost for CARLA
_Under construction_

Follow the official documentatio to compile boost.
Put the generated binaries in `Util/Build/boost-install`.

### Building Protobuf for CARLA
#### From our batch file
Just use [this batch](https://gist.github.com/marcgpuig/57946f9b684f64e5f08487089c437ea3) script, it will download and compile a ready-to-use version. Put it in`Util/Build` and just run it.

#### Manually
If something goes wrong, just follow the protobuf [cmake tutorials](https://github.com/google/protobuf/blob/master/cmake/README.md), but add these cmake arguments:

```
-DCMAKE_BUILD_TYPE=Release
-Dprotobuf_BUILD_TESTS=OFF
-DCMAKE_CXX_FLAGS_RELEASE=/MD
-Dprotobuf_MSVC_STATIC_RUNTIME=OFF
```

Put the generated binaries in `Util/Build/protobuf-install`.

## Compiling CARLA server
With your Visual Studio propmt, navigate into the `carla` folder where you cloned the repo with

```cmd
cd carla
```
and use:

```cmd
path\to\carla> make
```

If you have done everything alright, **carla server** must be compiled successfully in `Util/Build/carlaserver-build`.

## Launch Unreal Engine
Double click `Unreal/CarlaUE4/CarlaUE4.uproject` and it will ask you to rebuild:
     
    UE4Editor-CarlaUE4.dll
    UE4Editor-Carla.dll
  
Agree.

## Compilation failures
If there are problems generating the dlls, right click on `CarlaUE4.uproject` and select `Generate Visual Studio project files`, so you can try compiling from the Visual Studio Editor and check the errors.

## Recompiling
You must delete the following folders to completely rebuild your project:

```
Unreal/CarlaUE4/Binaries
Unreal/CarlaUE4/Intermediate
Unreal/CarlaUE4/Plugins/Carla/Binaries
Unreal/CarlaUE4/Plugins/Carla/Intermediate
```

Then you can clear the old intermediate files for carla if you have some troubles with, then compile the CARLA server again

```cmd
path\to\carla> make clean & make
```

You can do it more automatically with your prompt in `carla` folder:

```cmd
path\to\carla> rmdir /q /s Unreal/CarlaUE4/Binaries Unreal/CarlaUE4/Intermediate Unreal/CarlaUE4/Saved Unreal/CarlaUE4/Plugins/Carla/Binaries Unreal/CarlaUE4/Plugins/Carla/Intermediate
path\to\carla> make clean
path\to\carla> make
path\to\carla> Unreal/CarlaUE4/CarlaUE4.uproject
```
