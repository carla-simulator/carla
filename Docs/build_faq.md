# F.A.Q.

Some of the most common issues regarding CARLA installation and builds are listed hereunder. More issues can be found in the [GitHub issues][issuelink] for the project. In case a doubt is not listed here, take a look at the forum and feel free to post it.
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

  It is advised to have at least 30/50GB. Building CARLA from source requires about 25GB of disk space, not counting Unreal Engine installation, which is quite a similar size. 

  Unreal Engine on Linux requires much more disk space as it keeps all the intermediate files, take a look at [this thread](https://answers.unrealengine.com/questions/430541/linux-engine-size.html) about the matter.
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Recommended hardware to run CARLA.
    </h5></summary>

  CARLA is a very performance demanding software, at the very minimum it needs for a 4GB GPU or even better, a dedicated GPU capable of running Unreal Engine. Take a look at [Unreal Engine's recommended hardware](https://wiki.unrealengine.com/Recommended_Hardware).

  </details>

---
## Linux build
<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    "CarlaUE4.sh" script does not appear when downloading from GitHub.
    </h5></summary>

  There is no `CarlaUE4.sh` script in the source version of CARLA. Follow the [build instructions](build_linux.md) to build CARLA from source. To directly get the `CarlaUE4.sh` script, follow the [quick start instructions](start_quickstart.md). 
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    "make launch" is not working on Linux.
    </h5></summary>

  Many different issues can be dragged during the build installation and finally show here, so here is a list of the most likely reasons why:  

* __Run Unreal Engine 4.22:__ Something may have failed when building Unreal Engine. Try running UE editor on its own and check out that it is the 4.22 release.
* __Download the assets:__ The server will not be able to run without the visual content, so this step is mandatory even when building CARLA for the first time. Go to the root CARLA folder and run the script to get the assets: `./Update.sh`
* __UE4_ROOT is not defined:__ The environment variable is not set. Remember to make it persistent session-wide by adding it to your `~/.bashrc` or `~/.profile`, otherwise it will need to be set for every new shell. Run `export UE4_ROOT=~/UnrealEngine_4.22` to set the variable this time. Make sure that the path leads to the UE 4.22 main folder.  
* __Check dependencies:__ Confirm that everything was installed properly during the first steps of the process. Maybe one of the commands was skipped, unsuccessful or the dependencies were not suitable for the system.
* __Delete CARLA and clone it again:__ Just in case something went wrong. Delete your version of CARLA and try cloning or downloading and extracting it again. Then follow the steps from there.   
* __Meet system requirements:__ Ubuntu version should be 16.04 or later. CARLA needs around 15GB of disk space and a dedicated GPU (or at least one with 4GB) to run. 

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

Something went wrong when trying to build CARLA. To know what happened, the build must be done using Visual Studio.  

1. Go to `carla/Unreal/CarlaUE4` and right-click the `CarlaUE4.uproject`. 
2. Click on __Generate Visual Studio project files__. 
3. Open the file generated with Visual Studio 2017. 
4. Compile the project with Visual Studio (shortcut is F7). The build will fail, but the issues found will be shown below.
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    CMake error shows but CMake is properly installed.
    </h5></summary>
	
This issue occurs when trying to use the _make_ command either to build the server or the client even though CMake is installed with an updated version and added to the environment path. The reason is that the issue is not really related with CMake but a conflict between Visual Studio versions.  
Completely erase Visual Studio versions from the system and leave only the Visual Studio 2017 to make sure that everything works properly.  
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Error C2440, C2672: compiler version.
    </h5></summary>
	
The build is not using the 2017 compiler due to conflict with other Visual Studio or Microsoft Compiler versions. Uninstall the other and rebuild again. If the problem still exists, it may be due to Visual Studio not being good at getting rid of itself. To completely clean Visual Studio from the computer go to `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout` and run `.\InstallCleanup.exe -full`. This may need admin permissions.

To keep the other version of Visual Studio edit ```%appdata%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml``` adding the following lines:

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

  Many different issues can be dragged during the build installation and finally show here, so here is a list of the most likely reasons why:  

* __Restart the system:__ There are many installations going in the Windows build and it would be a great idea to give the system a chance to restart and make sure that everything is updated properly. 
* __Run Unreal Engine 4.22:__ Something may have failed when building Unreal Engine. Try running UE editor on its own and make sure that it is the 4.22 release.
* __Download the assets:__ The server will not be able to run without the visual content, so this step is mandatory even when running CARLA for the first time. 
* __Visual Studio 2017:__ If there are other versions of Visual Studio installed or recently uninstalled, conflicts may arise. To completely clean Visual Studio from the computer go to `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout` and run `.\InstallCleanup.exe -full`.  
* __Delete CARLA and clone it again:__ Just in case something went wrong. Delete your version of CARLA and try cloning or downloading and extracting it again. Then follow the steps from there.  
* __Meet system requirements:__ CARLA needs around 30/50GB of disk space and a dedicated GPU (or at least one with 4GB) to run. 

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

This may happen, especially when building for the very first time. Just click on "Accept" to rebuild them. 
  </details>


---
## Running CARLA
<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Low FPS rate when running the server in Unreal Editor.
    </h5></summary>

  UE4 Editor goes to a low performance mode when out of focus. It can be disabled
  in the editor preferences. Go to "Edit->Editor Preferences->Performance" and
  disable the "Use Less CPU When in Background" option.

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    Can't run a script.
    </h5></summary>
	
Some scripts have requirements that can be found in the files named __Requirements.txt__ in the same path as the script itself. Be sure to get these in order to run the script. The majority of them can simply be installed with a `pip install` command.  

Sometimes on Windows, scripts cannot run just using `script_name.py` in their directory. Try adding `python script_name.py`. 

  </details>

<!-- ======================================================================= -->

  <details>
    <summary><h5 style="display:inline">
    Connect to the simulator while running within Unreal Editor.
    </h5></summary>

  Press the "Play" button and wait until the scene is loaded. At that point, a Python client can connect to the simulator as with the standalone simulator.

  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
   Can't run CARLA neither binary nor source build.
    </h5></summary>

  One of the most common issues is that NVIDIA drivers are outdated in the system where CARLA is intended to run. Please make sure that this is not the case. If the issue is still unresolved, take a look at the [forum](https://forum.carla.org/) and feel free to post with the specific issue. 
  </details>

<!-- ======================================================================= -->
  <details>
    <summary><h5 style="display:inline">
    ImportError: DLL load failed: The specified module could not be found.
    </h5></summary>
	
If this message appears when trying to run a script, one of the libraries needed has not been properly installed. As a work around, go to: `carla\Build\zlib-source\build`  
Copy the file named `zlib.dll` in the directory of the script. 
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

  In Linux, the recommended way is to run `make package` in the project folder. This method makes a packaged version of the project, including the Python API modules. This is the method used to make a release of CARLA for Linux.

  Alternatively, it is possible to compile a binary version of CARLA within Unreal Editor.
 Open the CarlaUE4 project, go to the menu "File -> Package Project", and select a platform. This takes a while, but it should generate a packaged version of CARLA to execute without Unreal Editor. 
  </details>

---
