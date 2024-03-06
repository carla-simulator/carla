# F.A.Q.

Some of the most common issues regarding CARLA installation and builds are listed here. Some more can be found in the [GitHub issues](https://github.com/carla-simulator/carla/issues?utf8=%E2%9C%93&q=label%3Aquestion+) for the project. In case you don't find your doubt listed here, have a look in the forum and feel free to ask there.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>

---

## System requirements

* [Expected disk space to build CARLA.](#expected-disk-space-to-build-carla)
* [Recommended hardware to run CARLA.](#recommended-hardware-to-run-carla)

---

## Linux build

* ["CarlaUnreal.sh" script does not appear when downloading from GitHub.](#carlaue4sh-script-does-not-appear-when-downloading-from-github)
* ["make launch" is not working on Linux.](#make-launch-is-not-working-on-linux)
* [Cloning the Unreal Engine repository shows an error.](#cloning-the-unreal-engine-repository-shows-an-error)
* [AttributeError: module 'carla' has no attribute 'Client' when running a script.](#attributeerror-module-carla-has-no-attribute-client-when-running-a-script)
* [Cannot run example scripts or "RuntimeError: rpc::rpc_error during call in function version".](#cannot-run-example-scripts-or-runtimeerror-rpcrpc_error-during-call-in-function-version)

---

## Windows build

* ["CarlaUnreal.exe" does not appear when downloading from GitHub.](#carlaue4exe-does-not-appear-when-downloading-from-github)
* [CarlaUnreal could not be compiled. Try rebuilding it from source manually.](#carlaue4-could-not-be-compiled-try-rebuilding-it-from-source-manually)
* [CMake error shows even though CMake is properly installed.](#cmake-error-shows-even-though-cmake-is-properly-installed)
* [Error C2440, C2672: compiler version.](#error-c2440-c2672-compiler-version)
* ["make launch" is not working on Windows.](#make-launch-is-not-working-on-windows)
* [Make is missing libintl3.dll or/and libiconv2.dll.](#make-is-missing-libintl3dll-orand-libiconv2dll)
* [Modules are missing or built with a different engine version.](#modules-are-missing-or-built-with-a-different-engine-version)
* [There is no `dist` folder in `PythonAPI/carla` despite a successful output message.](#there-is-no-dist-folder-in-pythonapicarla-despite-a-successful-output-message)

---

## Running Carla

* [Low FPS rate when running the server in Unreal Editor.](#low-fps-rate-when-running-the-server-in-unreal-editor)
* [Can't run a script.](#cant-run-a-script)
* [Connect to the simulator while running within Unreal Editor.](#connect-to-the-simulator-while-running-within-unreal-editor)
* [Can't run CARLA neither binary nor source build.](#cant-run-carla-neither-binary-nor-source-build)
* [ImportError: DLL load failed: The specified module could not be found.](#importerror-dll-load-failed-the-specified-module-could-not-be-found)
* [ImportError: DLL load failed while importing libcarla: %1 is not a valid Win32 app.](#importerror-dll-load-failed-while-importing-libcarla-1-is-not-a-valid-win32-app)
* [ImportError: No module named 'carla'](#importerror-no-module-named-carla)

---

## Other

* [Fatal error: 'version.h' has been modified since the precompiled header.](#fatal-error-versionh-has-been-modified-since-the-precompiled-header)
* [Create a binary version of CARLA.](#create-a-binary-version-of-carla)
* [Can I package CARLA for Windows on a Linux machine and vice versa?](#can-i-package-carla-for-windows-on-a-linux-machine-and-vice-versa)
* [How do I uninstall the CARLA client library?](#how-do-i-uninstall-the-carla-client-library)

---

<!-- ======================================================================= -->
## System requirements
<!-- ======================================================================= -->

###### Expected disk space to build CARLA.

> It is advised to have at least 170GB free. Building CARLA requires about 35GB of disk space, plus Unreal Engine which requires about 95-135GB.

<!-- ======================================================================= -->

###### Recommended hardware to run CARLA.

> CARLA is a performance demanding software. At the very minimum it requires a 6GB GPU or, even better, a dedicated GPU capable of running Unreal Engine.  
>
> Take a look at [Unreal Engine's recommended hardware](https://www.ue4community.wiki/recommended-hardware-x1p9qyg0).

---

## Linux build
<!-- ======================================================================= -->
###### "CarlaUnreal.sh" script does not appear when downloading from GitHub.

> There is no `CarlaUnreal.sh` script in the source version of CARLA. Follow the [build instructions](build_linux.md) to build CARLA from source.  
> 
> To run CARLA using `CarlaUnreal.sh`, follow the [quick start installation](start_quickstart.md).

<!-- ======================================================================= -->

###### "make launch" is not working on Linux.

> Many different issues can be dragged out during the build installation and will manifest themselves like this. Here is a list of the most likely reasons why:  
> 
> * __Run Unreal Engine 4.26.__ Something may have failed when building Unreal Engine. Try running UE editor on its own and check that it is the 4.26 release.  
> * __Download the assets.__ The server will not be able to run without the visual content. This step is mandatory.  
> * __UE4_ROOT is not defined.__ The environment variable is not set. Remember to make it persistent session-wide by adding it to the `~/.bashrc` or `~/.profile`. Otherwise it will need to be set for every new shell. Run `export UE4_ROOT=<path_to_unreal_4-26>` to set the variable this time.  
> * __Check dependencies.__ Make sure that everything was installed properly. Maybe one of the commands was skipped, unsuccessful or the dependencies were not suitable for the system.
> * __Delete CARLA and clone it again.__ Just in case something went wrong. Delete CARLA and clone or download it again.  
> * __Meet system requirements.__ Ubuntu version should be 16.04 or later. CARLA needs around 170GB of disk space and a dedicated GPU (or at least one with 6GB) to run.
> 
> Other specific reasons for a system to show conflicts with CARLA may occur. Please, post these on the [forum](https://github.com/carla-simulator/carla/discussions/) so the team can get to know more about them.   

<!-- ======================================================================= -->

###### Cloning the Unreal Engine repository shows an error.

> __1. Is the Unreal Engine account activated?__ The UE repository is private. In order to clone it, create the [UE](https://www.unrealengine.com/en-US/) account, activate it (check the verification mail), and [link your GitHub](https://www.unrealengine.com/en-US/blog/updated-authentication-process-for-connecting-epic-github-accounts) account.  
> 
> __2. Is git properly installed?__ Sometimes an error shows incompatibilities with the `https` protocol. It can be solved easily by uninstalling and reinstalling git. Open a terminal and run the following commands:
>
>      sudo apt-get remove git #Uninstall git
>      sudo apt install git-all #install git
> 

<!-- ======================================================================= -->

###### AttributeError: module 'carla' has no attribute 'Client' when running a script.

> Run the following command.
> 
> 
>       pip3 install -Iv setuptools==47.3.1
>  
>
> And build the PythonAPI again. 
> 
>
>      make PythonAPI
>
>
> Try to build the docs to test if everything is running properly. A successful message should show.
>
>      make PythonAPI.docs

<!-- ======================================================================= -->

###### Cannot run example scripts or "RuntimeError: rpc::rpc_error during call in function version".

> ![faq_rpc_error](img/faq_rpc_error.jpg)
>
> If running a script returns an output similar to this, there is a problem with the `.egg` file in the PythonAPI. 

!!! Important
    If you are using 0.9.12+, there are several methods to use/install the client library. If you are using one of the newer methods for the client library (`.whl` or PyPi download) the information in this section will not be relevant to you.
> 
> First of all, open `<root_carla>/PythonAPI/carla/dist`. There should be an `.egg` file for the corresponding CARLA and Python version you are using (similar to `carla-0.X.X-pyX.X-linux-x86_64.egg`). Make sure the file matches the Python version you are using. To check your Python version use the following command.  
> 
> 
>       python3 --version
>       # or for Python 2
>       python --version
> 
>
> If either the file is missing or you think it could be corrupted, try rebuilding again.  
>
>      make clean
>      make PythonAPI
>      make launch
>
>  
> Now try one of the example scripts again. 
>
>      cd PythonAPI/examples
>      python3 dynamic_weather.py
>
> If the error persists, the problem is probably related with your PythonPATH. These scripts automatically look for the `.egg` file associated with the build, so maybe there is another `.egg` file in your PythonPATH interfering with the process. Show the content of the PythonPATH with the following command.  
>
>
>      echo $PYTHONPATH
>
> Look up in the output for other instances of `.egg` files in a route similar to `PythonAPI/carla/dist`, and get rid of these. They probably belong to other instances of CARLA installations. For example, if you also installed CARLA via *apt-get*, you can remove it with the following command, and the PythonPATH will be cleaned too.  
>
>      sudo apt-get purge carla-simulator
>
> Ultimately there is the option to add the `.egg` file of your build to the PythonPATH using the `~/.bashrc`. This is not the recommended way. It would be better to have a clear PythonPATH and simply add the path to the necessary `.egg` files in the scripts.  
>
> First, open `~/.bashrc`.
>
>      gedit ~/.bashrc
>
> 
> Add the following lines to `~/.bashrc`. These store the path to the build `.egg` file, so that Python can automatically find it. Save the file, and reset the terminal for changes to be effective.
>
> ```
> export PYTHONPATH=$PYTHONPATH:"${CARLA_ROOT}/PythonAPI/carla/dist/$(ls ${CARLA_ROOT}/PythonAPI/carla/dist | grep py3.)"
> export PYTHONPATH=$PYTHONPATH:${CARLA_ROOT}/PythonAPI/carla
> ```
> 
> After cleaning the PythonPATH or adding the path to the build `.egg` file, all the example scripts should work properly.  

---

## Windows build
<!-- ======================================================================= -->

###### "CarlaUnreal.exe" does not appear when downloading from GitHub.

> There is no `CarlaUnreal.exe` executable in the source version of CARLA. Follow the [build instructions](build_windows.md) to build CARLA from source. To directly get the `CarlaUnreal.exe`, follow the [quick start instructions](start_quickstart.md).  

<!-- ======================================================================= -->

###### CarlaUnreal could not be compiled. Try rebuilding it from source manually.

> Something went wrong when trying to build CARLA. Rebuild using Visual Studio to discover what happened.  
>
> __1.__ Go to `carla/Unreal/CarlaUnreal` and right-click the `CarlaUnreal.uproject`.  
> __2.__ Click on __Generate Visual Studio project files__.  
> __3.__ Open the file generated with Visual Studio 2019.  
> __4.__ Compile the project with Visual Studio. The shortcut is F7. The build will fail, but the issues found will be shown below.
>
> Different issues may result in this specific error message. The user [@tamakoji](https://github.com/tamakoji) solved a recurrent case where the source code hadn't been cloned properly and the CARLA version could not be set (when downloading this as a .zip from git).  
>
> *   __Check the `Build/CMakeLists.txt.in`.__ If it shows as `set(CARLA_VERSION )` do the following:  
>
> __1.__ Go to `Setup.bat` line 198.  
>
> __2.__ Update the line from: 
> 
> ```
> for /f %%i in ('git describe --tags --dirty --always') do set carla_version=%%i
> ```
> 
> to:
> 
> ```
> for /f %%i in ('git describe --tags --dirty --always') do set carla_version="0.9.9"
> ```

<!-- ======================================================================= -->

###### CMake error shows even though CMake is properly installed.

> This issue occurs when trying to use the _make_ command either to build the server or the client. Even if CMake is installed, updated and added to the environment path. There may be a conflict between Visual Studio versions.  
>
> Leave only VS2019 and completely erase the rest.  

<!-- ======================================================================= -->

###### Error C2440, C2672: compiler version.

> The build is not using the 2019 compiler due to conflicts with other Visual Studio or Microsoft Compiler versions. Uninstall these and rebuild again.  
>
> Visual Studio is not good at getting rid of itself. To completely clean Visual Studio from the computer go to `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout` and run `.\InstallCleanup.exe -full`. This may need admin permissions.  
>
> To keep other Visual Studio versions, edit ```%appdata%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml``` by adding the following lines:
>
> ```
>  <VCProjectFileGenerator>
>    <Version>VisualStudio2019</Version>
> </VCProjectFileGenerator>
> ```
> 
> ``` 
>  <WindowsPlatform>
>    <Compiler>VisualStudio2019</Compiler>
> </WindowsPlatform>
> ```  

<!-- ======================================================================= -->

###### "make launch" is not working on Windows.

> Many different issues can be dragged out during the build installation and manifest themselves like this. Here is a list of the most likely reasons why: 
> 
> * __Restart the computer.__ There is a lot going on during the Windows build. Restart and make sure that everything is updated properly.  
> * __Run Unreal Engine 4.26.__ Something may have failed when building Unreal Engine. Run the Editor and check that version 4.26 is being used.  
> * __Download the assets.__ The server will not be able to run without the visual content. This step is mandatory.  
> * __Visual Studio 2019.__ If there are other versions of Visual Studio installed or recently uninstalled, conflicts may arise. To completely clean Visual Studio from the computer go to `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout` and run `.\InstallCleanup.exe -full`.  
> * __Delete CARLA and clone it again.__ Just in case something went wrong. Delete CARLA and clone or download it again.  
> * __Meet system requirements.__ CARLA needs around 170GB of disk space and a dedicated GPU (or at least one with 6GB) to run.  
>
> Other specific reasons for a system to show conflicts with CARLA may occur. Please, post these on the [forum](https://github.com/carla-simulator/carla/discussions/) so the team can get to know more about them.

<!-- ======================================================================= -->

###### Make is missing libintl3.dll or/and libiconv2.dll.

> Download the [dependencies](http://gnuwin32.sourceforge.net/downlinks/make-dep-zip.php) and extract the `bin` content into the `make` installation path.   

<!-- ======================================================================= -->

###### Modules are missing or built with a different engine version.

> Click on __Accept__ to rebuild them. 

<!-- ======================================================================= -->

###### There is no `dist` folder in `PythonAPI/carla` despite a successful output message.

>In Windows, the `make PythonAPI` command can return a message that the Python API was installed successfully when it actually wasn't. If there is no `dist` folder created in the `PythonAPI/carla` directory after you see this output, then look at the command output higher up. It is likely an error occurred and the build needs to be retried after correcting the error and running `make clean`.

---

## Running CARLA
<!-- ======================================================================= -->

###### Low FPS rate when running the server in Unreal Editor.

> UE4 Editor goes to a low performance mode when out of focus.  
>
> Go to `Edit/Editor Preferences/Performance` in the editor preferences, and disable the "Use Less CPU When in Background" option.

<!-- ======================================================================= -->

###### Can't run a script.

> Some scripts have requirements. These are listed in files named __Requirements.txt__, in the same path as the script itself. Be sure to check these in order to run the script. The majority of them can be installed with a simple `pip` command.  
>
> Sometimes on Windows, scripts cannot run with just `> script_name.py`. Try adding `> python3 script_name.py`, and make sure to be in the right directory.  

<!-- ======================================================================= -->

###### Connect to the simulator while running within Unreal Editor.

> Click on __Play__ and wait until the scene is loaded. At that point, a Python client can connect to the simulator as with the standalone simulator.

<!-- ======================================================================= -->

###### Can't run CARLA neither binary nor source build.

> NVIDIA drivers may be outdated. Make sure that this is not the case. If the issue is still unresolved, take a look at the [forum](https://github.com/carla-simulator/carla/discussions/) and post the specific issue. 

<!-- ======================================================================= -->

###### ImportError: DLL load failed: The specified module could not be found.

> One of the libraries needed has not been properly installed. As a work around, go to `carla\Build\zlib-source\build`, and copy the file named `zlib.dll` in the directory of the script. 

<!-- ======================================================================= -->

###### ImportError: DLL load failed while importing libcarla: %1 is not a valid Win32 app.

> A 32-bit Python version is creating conflicts when trying to run a script. Uninstall it and leave only the Python3 x64 required. 

<!-- ======================================================================= -->

###### ImportError: No module named 'carla'

> This error occurs because Python cannot find the CARLA library. The CARLA library is contained in an `.egg` file, located in the directory `PythonAPI/carla/dist` and all the example scripts will look for it in this directory. The `.egg` file follows the nomenclature of `carla-<carla-version>-py<python-version>-<operating-system>.egg`.
>

!!! Important
    CARLA only used `.egg` files for the client library in versions prior to 0.9.12. If you are using 0.9.12+, there are several methods to use/install the client library. If you are using one of the newer methods for the client library (`.whl` or PyPi download) the information in this section will not be relevant to you.

    Read more about the newer methods to use/install the client library in the [__Quickstart tutorial__](start_quickstart.md#carla-0912).

>If you are using a packaged version of CARLA, there will be several `.egg` files, corresponding to different versions of Python, depending on the version of CARLA. Make sure you are running the scripts with one of these Python versions. To check the default Python version, type the following into the command line:
>
>
>       python3 --version
>       # or
>       python --version 
>

>If you built Python from source, the `.egg` file will be built according to the default Python version on your system. In Linux this will be the default Python version returned for:


>       /usr/bin/env python3 --version
>       # or if you specify ARGS="--python-version=2"
>       /usr/bin/env python2 --version


>In Windows it will be the default Python version for:

>       py -3 --version

>Make sure you are running your scripts with the version of Python that corresponds to your `.egg` file.
>In Linux, you may also need to set your Python path to point to the CARLA `.egg`. To do this, run the following command:

>       export PYTHONPATH=$PYTHONPATH:<path/to/carla/>/PythonAPI/carla/dist/<your_egg_file>
>       # Check if CARLA can now be found
>       python3 -c 'import carla;print("Success")'

>Be aware that virtual environments, or other Python environments like Conda, can complicate the installation of CARLA. Make sure you have set up your Python defaults and paths accordingly.

---

## Other
<!-- ======================================================================= -->

###### Fatal error: 'version.h' has been modified since the precompiled header.

> This happens from time to time due to Linux updates. There is a special target in the Makefile for this issue. It takes a long time but fixes the issue:
>
>      make hard-clean
>      make CarlaUnrealEditor

<!-- ======================================================================= -->

###### Create a binary version of CARLA.

> In Linux, run `make package` in the project folder. The package will include the project, and the Python API modules.  
>
> Alternatively, it is possible to compile a binary version of CARLA within Unreal Editor. Open the CarlaUnreal project, go to the menu `File/Package Project`, and select a platform. This may take a while. 

<!-- ======================================================================= -->

###### Can I package CARLA for Windows on a Linux machine and vice versa?

>Although this feature is available for Unreal Engine, it is not available in CARLA. We have a number of dependencies that are not supported to be cross compiled.

<!-- ======================================================================= -->
###### How do I uninstall the CARLA client library?

>If you installed the client library using __pip/pip3__, you should uninstall it by running:

```sh
# Python 3
pip3 uninstall carla

# Python 2
pip uninstall carla
```

---
