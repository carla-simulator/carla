<h1>How to build CARLA on Windows</h1>

<h3>Necessary software</h3>
- [Git](https://git-scm.com/downloads)
- [Make](http://gnuwin32.sourceforge.net/downlinks/make-bin-zip.php)
- [Cmake](https://cmake.org/download/)

!!! important
    Be sure that these programs are added to your path, so you can use them from your command prompt.

Also:

- [Unreal Engine](https://www.unrealengine.com/download) (v4.18.x)
- [Visual Studio](https://www.visualstudio.com/downloads/) (2017 preferably)

<h3>Environment Setup</h3>
In order to build CARLA you must **enable the x64 Visual C++ Toolset**.
I recommend to use this environment for everything you do in this tutorial.
You have different options:

- **Recomended:** Use [`Visual Studio x64 Native Tools Command Prompt`](https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs). Just press the `Win` button and search for `x64` (be careful to not **unintentionally open a `x86_x64` prompt**), because the name of this Command Prompt can change depending on the lenguage you have Visual Studio installed.
- [Enable a 64-Bit Visual C++ Toolset on the Command Line](https://msdn.microsoft.com/en-us/library/x4d2c09s.aspx) (the instructions will depend on the version of VS that you have).

!!! note
    Take care if you have **Cygwin** installed. This could cause the errors like  
    `/usr/bin/sh: rd: command not found` While executing `Reuild.bat`.

<h3>Clone the repository</h3>
Go to the path you want to install CARLA and use git to download the project using the following command:

```cmd
git clone https://github.com/carla-simulator/carla.git
```

and get in the created folder:

```cmd
cd carla
```

# Automatic installation
Once you have downloaded the repo you can start with the automatic installation.
This process may take a while, it will download and install the necessary Boost and Protobuf libraries. Expect 20-40 minutes, depending on your hardware and internet connection.

Script        | Use
------------- | ----
`Setup.bat`   | Downloads and installs all the external dependencies, automatically calling the scripts in `Util/InstallersWin/`.
`Rebuild.bat` | Builds carlaserver and launch the Unreal project.

<h3>Installation</h3>
1. After navigate to your carla folder, run:

        Setup.bat -j 8 --boost-toolset msvc-14.1

    `-j n` will try to parallelize the git download and NMake compilation. If not specified, no optimization will be made.

    To see available commands, like which `--boost-toolset` you have to use depending on your Visual Studio version, use:

        Setup.bat --help

1. When it's done, and if everything went well, `Setup.bat` will provide a link where you can download the assets manually. If you get some errors you can try the **Manual Installation**, open a new issue on [GitHub](https://github.com/carla-simulator/carla/issues/) or just ask on the [Windows Discord channel](https://discord.gg/42KJdRj).

1. Unzip these assets into `Unreal/CarlaUE4/Content`. Create it if the folder is not there.

1. Now let's compile carlaserver and start the Unreal project. Run:

        Rebuild.bat

1. Later on it will ask you to rebuild:

        UE4Editor-CarlaUE4.dll
        UE4Editor-Carla.dll

    Agree and the project will be opened in the Unreal Engine in a few minutes.

# Manual Installation

!!! note
    Since version **0.8.3**, it is recomended to follow the automatic installation.

<h3>Download the assets</h3>
Download the assets from the version you need. Take a look at [`Util/ContentVersions.txt`](https://github.com/carla-simulator/carla/blob/master/Util/ContentVersions.txt) and follow the provided instructions to build the download link from the version's hash.

Create the folder `Content` in `Unreal/CarlaUE4/` and unzip the content you just downloaded here.

<h3>Dependencies</h3>
Download and build **for win64**:

- Boost 1.64
- Protobuf 3.3.2

<h3>Building Boost for CARLA</h3>
Follow the official documentation to compile boost.
Put the generated binaries in `Util/Build/boost-install`:

```c
Util
└── Build
    └── boost-install
        ├── boost-1_64
        |   └── boost
        |       ├── accumulators
        |       ├── algorithm
        |       ├── align
        |       └── [...] // and all the other libraries
        └── lib
            ├── libboost_date_time-vc141-mt-1_64.lib
            └── libboost_system-vc141-mt-1_64.lib
```

<h3>Building Protobuf for CARLA</h3>
<h4>From our batch file</h4>
Just use [this batch](https://gist.github.com/marcgpuig/57946f9b684f64e5f08487089c437ea3) script, it will download and compile a ready-to-use version. Put it in`Util/Build` and just run it.

<h4>Manually</h4>
If something goes wrong, just follow the protobuf [cmake tutorials](https://github.com/google/protobuf/blob/master/cmake/README.md), but add these cmake arguments:

```
-DCMAKE_BUILD_TYPE=Release
-Dprotobuf_BUILD_TESTS=OFF
-DCMAKE_CXX_FLAGS_RELEASE=/MD
-Dprotobuf_MSVC_STATIC_RUNTIME=OFF
```

Put the generated binaries in `Util/Build/protobuf-install`.

<h3>Compiling CARLA server</h3>
With your Visual Studio propmt, navigate into the `carla` folder where you cloned the repo with

```cmd
cd carla
```
and use:

```cmd
path\to\carla> make
```

If you have done everything alright, **carla server** must be installed successfully in `Unreal/CarlaUE4/Plugins/Carla/CarlaServer`.

<h3>Launch Unreal Engine</h3>
Double click `Unreal/CarlaUE4/CarlaUE4.uproject` and it will ask you to rebuild:

    UE4Editor-CarlaUE4.dll
    UE4Editor-Carla.dll

Agree.

<h3>Compilation failures</h3>
If there are problems generating the dlls, right click on `CarlaUE4.uproject` and select `Generate Visual Studio project files`, so you can try compiling from the Visual Studio Editor and check the errors.

# Recompiling
<h4>Automatic</h4>
Just run `Rebuild.bat`:

```
path\to\carla> Rebuild
```

<h4>Manual</h4>
You must delete the following folders to **completely rebuild** your project:

```cmd
Unreal/CarlaUE4/Binaries
Unreal/CarlaUE4/Intermediate
Unreal/CarlaUE4/Plugins/Carla/Binaries
Unreal/CarlaUE4/Plugins/Carla/Intermediate
```

Then you can clean the project and compile it again:

```
path\to\carla> make clean & make
```

You can do it more automatically with your prompt in `carla` folder:

```
path\to\carla> rmdir /q /s Unreal/CarlaUE4/Binaries Unreal/CarlaUE4/Intermediate Unreal/CarlaUE4/Saved Unreal/CarlaUE4/Plugins/Carla/Binaries Unreal/CarlaUE4/Plugins/Carla/Intermediate
path\to\carla> make clean
path\to\carla> make
path\to\carla> Unreal/CarlaUE4/CarlaUE4.uproject
```
