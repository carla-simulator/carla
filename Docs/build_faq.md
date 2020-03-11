# F.A.Q.

Some of the most common issues regarding CARLA installation and builds are listed hereunder. Some more can be found in the [GitHub issues][issuelink] for the project. In case any doubt is not listed here, take a look at the forum and feel free to post it.
[issuelink]: https://github.com/carla-simulator/carla/issues?utf8=%E2%9C%93&q=label%3Aquestion+ 
<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>

---
## System requirements
<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Expected disk space to build CARLA.
    </h5></summary>

It is advised to have at least 30/50GB. Building CARLA requires about 25GB of disk space, plus Unreal Engine, which is quite a similar size. 

Unreal Engine on Linux requires much more disk space as it keeps all the intermediate files. [This thread](https://answers.unrealengine.com/questions/430541/linux-engine-size.html) discusses the matter.
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Recommended hardware to run CARLA.
    </h5></summary>

CARLA is a very performance demanding software. At the very minimum it needs for a 4GB GPU or, even better, a dedicated GPU capable of running Unreal Engine.  

Take a look at [Unreal Engine's recommended hardware](https://wiki.unrealengine.com/Recommended_Hardware).
  </details>

---
## Linux build
<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    "CarlaUE4.sh" script does not appear when downloading from GitHub.
    </h5></summary>

There is no `CarlaUE4.sh` script in the source version of CARLA. Follow the [build instructions](build_linux.md) to build CARLA from source.  

To run CARLA using `CarlaUE4.sh`, follow the [quick start installation](start_quickstart.md).  
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    "make launch" is not working on Linux.
    </h5></summary>

Many different issues can be dragged during the build installation, and show like this. Here is a list of the most likely reasons why.  

* __Run Unreal Engine 4.22.__ Something may have failed when building Unreal Engine. Try running UE editor on its own and check out that it is the 4.22 release.  
* __Download the assets.__ The server will not be able to run without the visual content. This step is mandatory.  
* __UE4_ROOT is not defined.__ The environment variable is not set. Remember to make it persistent session-wide by adding it to the `~/.bashrc` or `~/.profile`. Otherwise it will need to be set for every new shell. Run `export UE4_ROOT=~/UnrealEngine_4.22` to set the variable this time.  
* __Check dependencies.__ Make sure that everything was installed properly. Maybe one of the commands was skipped, unsuccessful or the dependencies were not suitable for the system.
* __Delete CARLA and clone it again.__ Just in case something went wrong. Delete CARLA and clone or download it again.  
* __Meet system requirements.__ Ubuntu version should be 16.04 or later. CARLA needs around 15GB of disk space and a dedicated GPU (or at least one with 4GB) to run.  

Other specific reasons for a system to show conflicts with CARLA may occur. Please, post these on the forum so the team can get to know more about them.   
  </details>

---
## Windows build

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    "CarlaUE4.exe" does not appear when downloading from GitHub.
    </h5></summary>

There is no `CarlaUE4.exe` executable in the source version of CARLA. Follow the [build instructions](build_windows.md) to build CARLA from source. To directly get the `CarlaUE4.exe`, follow the [quick start instructions](start_quickstart.md).  

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    CarlaUE4 could not be compiled. Try rebuilding it from source manually. 
    </h5></summary>

Something went wrong when trying to build CARLA. Rebuild using Visual Studio to discover what happened.  

__1.__ Go to `carla/Unreal/CarlaUE4` and right-click the `CarlaUE4.uproject`.  
__2.__ Click on __Generate Visual Studio project files__.  
__3.__ Open the file generated with Visual Studio 2017.  
__4.__ Compile the project with Visual Studio. The shortcut is F7. The build will fail, but the issues found will be shown below.
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    CMake error shows but CMake is properly installed.
    </h5></summary>
	
This issue occurs when trying to use the _make_ command either to build the server or the client. Even if CMake is installed, updated, and added to the environment path. There may be a conflict between Visual Studio versions.  

Leave only VS2017 and completely erase the rest.  
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Error C2440, C2672: compiler version.
    </h5></summary>
	
The build is not using the 2017 compiler due to conflicts with other Visual Studio or Microsoft Compiler versions. Uninstall these and rebuild again.  

Visual Studio is not good at getting rid of itself. To completely clean Visual Studio from the computer go to `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout` and run `.\InstallCleanup.exe -full`. This may need admin permissions.  

To keep other Visual Studio versions, edit ```%appdata%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml``` by adding the following lines. 

```xml
<VCProjectFileGenerator>
    <Version>VisualStudio2017</Version>
</VCProjectFileGenerator>

<WindowsPlatform>
    <Compiler>VisualStudio2017</Compiler>
</WindowsPlatform>
```
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    "make launch" is not working on Windows.
    </h5></summary>

Many different issues can be dragged during the build installation, and show like this. Here is a list of the most likely reasons why.  

* __Restart the computer.__ There are many going on in the Windows build. Restart and make sure that everything is updated properly.  
* __Run Unreal Engine 4.22.__ Something may have failed when building Unreal Engine. Run the Editor and check that 4.22 is being used.  
* __Download the assets.__ The server will not be able to run without the visual content. This step is mandatory.  
* __Visual Studio 2017.__ If there are other versions of Visual Studio installed or recently uninstalled, conflicts may arise. To completely clean Visual Studio from the computer go to `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout` and run `.\InstallCleanup.exe -full`.  
* __Delete CARLA and clone it again.__ Just in case something went wrong. Delete CARLA and clone or download it again.  
* __Meet system requirements.__ CARLA needs around 30/50GB of disk space and a dedicated GPU (or at least one with 4GB) to run.  

Other specific reasons for a system to show conflicts with CARLA may occur. Please, post these on the forum so the team can get to know more about them.   
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Make is missing libintl3.dll or/and libiconv2.dll.
    </h5></summary>

Download the [dependencies](http://gnuwin32.sourceforge.net/downlinks/make-dep-zip.php) and extract the _bin_ content into the __make__ installation path.   
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Modules are missing or built with a different engine version.  
    </h5></summary>

Click on __Accept__ to rebuild them. 
  </details>

---
## Running CARLA
<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Low FPS rate when running the server in Unreal Editor.
    </h5></summary>

UE4 Editor goes to a low performance mode when out of focus.  

Go to `Edit/Editor Preferences/Performance` in the editor preferences, and disable the "Use Less CPU When in Background" option.

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Can't run a script.
    </h5></summary>
	
Some scripts have requirements. These are listed in files named __Requirements.txt__, in the same path as the script itself. Be sure to check these in order to run the script. The majority of them can be installed with a simple `pip` command.  

Sometimes on Windows, scripts cannot run with just `> script_name.py`. Try adding `> python script_name.py`, and make sure to be in the right directory.  

  </details>

<!-- ======================================================================= -->

  <details>
    <summary><h5 style="display:inline">
    Connect to the simulator while running within Unreal Editor.
    </h5></summary>

Click on __Play__ and wait until the scene is loaded. At that point, a Python client can connect to the simulator as with the standalone simulator.

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
   Can't run CARLA neither binary nor source build.
    </h5></summary>

NVIDIA drivers may be outdated. Make sure that this is not the case. If the issue is still unresolved, take a look at the [forum](https://forum.carla.org/) and post the specific issue. 
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    ImportError: DLL load failed: The specified module could not be found.
    </h5></summary>

One of the libraries needed has not been properly installed. As a work around, go to `carla\Build\zlib-source\build`, and copy the file named `zlib.dll` in the directory of the script. 
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    ImportError: DLL load failed while importing libcarla: %1 is not a valid Win32 app. 
    </h5></summary>

A 32-bit Python version is creating conflicts when trying to run a script. Uninstall it and leave only the Python3 x64 required. 
  </details>

---
## Other
<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Fatal error: 'version.h' has been modified since the precompiled header.
    </h5></summary>

This happens from time to time due to Linux updates. There is a special target in the Makefile for this issue. It takes a long time but fixes the issue:

      $ make hard-clean
      $ make CarlaUE4Editor

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Create a binary version of CARLA.
    </h5></summary>

In Linux, run `make package` in the project folder. The package will include the project, and the Python API modules.  

Alternatively, it is possible to compile a binary version of CARLA within Unreal Editor. Open the CarlaUE4 project, go to the menu `File/Package Project`, and select a platform. This may take a while. 
  </details>

---
