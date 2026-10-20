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

## Linux build

* ["CarlaUnreal.sh" script does not appear when downloading from GitHub.](#carlaunrealsh-script-does-not-appear-when-downloading-from-github)
* [The editor does not build or does not open on Linux.](#the-editor-does-not-build-or-does-not-open-on-linux)
* [Cloning the Unreal Engine repository shows an error.](#cloning-the-unreal-engine-repository-shows-an-error)
* [AttributeError: module 'carla' has no attribute 'Client' when running a script.](#attributeerror-module-carla-has-no-attribute-client-when-running-a-script)
* [Cannot run example scripts or "RuntimeError: rpc::rpc_error during call in function version".](#cannot-run-example-scripts-or-runtimeerror-rpcrpc_error-during-call-in-function-version)

## Windows build

* ["CarlaUnreal.exe" does not appear when downloading from GitHub.](#carlaunrealexe-does-not-appear-when-downloading-from-github)
* [CarlaUnreal could not be compiled.](#carlaunreal-could-not-be-compiled)
* [CMake or compiler errors on Windows.](#cmake-or-compiler-errors-on-windows)
* [The editor does not build or does not open on Windows.](#the-editor-does-not-build-or-does-not-open-on-windows)
* [Modules are missing or built with a different engine version.](#modules-are-missing-or-built-with-a-different-engine-version)
* [Packaging hangs or fails at the cook on Windows.](#packaging-hangs-or-fails-at-the-cook-on-windows)

## Running CARLA

* [Low FPS rate when running the server in Unreal Editor.](#low-fps-rate-when-running-the-server-in-unreal-editor)
* [Can't run a script.](#cant-run-a-script)
* [Connect to the simulator while running within Unreal Editor.](#connect-to-the-simulator-while-running-within-unreal-editor)
* [Can't run CARLA neither binary nor source build.](#cant-run-carla-neither-binary-nor-source-build)
* [ImportError: DLL load failed while importing libcarla: %1 is not a valid Win32 app.](#importerror-dll-load-failed-while-importing-libcarla-1-is-not-a-valid-win32-app)
* [ImportError: No module named 'carla'](#importerror-no-module-named-carla)

## Other

* [Create a binary version of CARLA.](#create-a-binary-version-of-carla)
* [Can I package CARLA for Windows on a Linux machine and vice versa?](#can-i-package-carla-for-windows-on-a-linux-machine-and-vice-versa)
* [How do I uninstall the CARLA client library?](#how-do-i-uninstall-the-carla-client-library)

---

<!-- ======================================================================= -->
## System requirements
<!-- ======================================================================= -->

###### Expected disk space to build CARLA.

> Plan for about 225 GB with the Unreal Engine 5.8 fork, plus about 30 GB for each package you build. Setting `CARLA_UNREAL_ENGINE_PATH` to an existing build of the engine saves the engine's share. See the [Linux](build_linux_ue5.md) and [Windows](build_windows_ue5.md) build guides.

<!-- ======================================================================= -->

###### Recommended hardware to run CARLA.

> CARLA is a performance demanding software and needs a dedicated GPU. As a reference, a server started with `-game` on Town10HD_Opt uses about 8 GB of video memory, and the Unreal Editor with the same map loaded about 15 GB, so the editor and a server do not fit together on a 24 GB card.
>
> Take a look at [Unreal Engine's hardware and software specifications](https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine).

---

## Linux build
<!-- ======================================================================= -->
###### "CarlaUnreal.sh" script does not appear when downloading from GitHub.

> There is no `CarlaUnreal.sh` script in the source version of CARLA. Follow the [build instructions](build_linux_ue5.md) to build CARLA from source.
>
> To run CARLA using `CarlaUnreal.sh`, follow the [quick start installation](start_quickstart.md), or build a package with `cmake --build Build/Release --target package`.

<!-- ======================================================================= -->

###### The editor does not build or does not open on Linux.

> Many different issues show up as a failing `cmake --build Build/Release --target launch`. These are the most likely reasons:
>
> * __Use the CARLA fork of Unreal Engine 5.8.__ CARLA does not build against a stock engine. Check that `CARLA_UNREAL_ENGINE_PATH` points at a build of the `ue58-dev-carla` branch of the fork, and that its editor starts on its own.
> * __Set `CARLA_UNREAL_ENGINE_PATH` for every shell.__ Add `export CARLA_UNREAL_ENGINE_PATH=<path_to_the_engine>` to `~/.bashrc` or `~/.profile`, otherwise it has to be set in every new terminal.
> * __Download the content.__ The server will not run without it. The content repository has to be cloned into `Unreal/CarlaUnreal/Content/Carla`.
> * __Set the DLSS SDK.__ The engine build needs `DLSS_SDK` pointing at the SDK (`CarlaSetup.sh` fetches it into `~/SDKs/DLSS`), or `export DLSS_SDK=disabled` to opt out.
> * __Configure before building.__ Run `cmake --preset Release` once; the build then lives in `Build/Release`.
> * __Check the system.__ The UE5 version of CARLA supports Ubuntu 22.04 and 24.04, and cannot be built on an external disk.
> * __Check dependencies.__ `bash Util/SetupUtils/InstallPrerequisites.sh --python-path=python3` installs the same prerequisites as `CarlaSetup.sh`.
>
> Other specific reasons for a system to show conflicts with CARLA may occur. Please, post these on the [forum](https://github.com/carla-simulator/carla/discussions/) so the team can get to know more about them.

<!-- ======================================================================= -->

###### Cloning the Unreal Engine repository shows an error.

> __1. Is the Epic Games account linked to GitHub?__ The engine fork is private. To clone it, create an [Epic Games](https://www.unrealengine.com/en-US/) account and [link it to your GitHub account](https://www.unrealengine.com/en-US/ue-on-github), then accept the invitation to the Epic Games organization on GitHub.
>
> __2. Are the credentials right?__ `CarlaSetup.sh` reads them from `GIT_LOCAL_CREDENTIALS=github_username@github_token`, or asks for them with `--interactive`. Use a GitHub token, not your password, and use `--interactive` if your username contains an `@`.
>
> __3. Is git properly installed?__ Sometimes an error shows incompatibilities with the `https` protocol. It can be solved by reinstalling git:
>
>      sudo apt-get remove git
>      sudo apt install git git-lfs

<!-- ======================================================================= -->

###### AttributeError: module 'carla' has no attribute 'Client' when running a script.

> Python is importing a `carla` folder that does not contain the compiled client, most often `PythonAPI/carla` itself when the script runs from inside it. Check which module is loaded:
>
>      python3 -c "import carla; print(carla.__file__)"
>
> Run the script from another directory, and install the client into the interpreter you use:
>
>      cmake --build Build/Release --target carla-python-api-install

<!-- ======================================================================= -->

###### Cannot run example scripts or "RuntimeError: rpc::rpc_error during call in function version".

> The output looks like this:
>
>      RuntimeError: rpc::rpc_error during call in function version
>
> The client could not get the version from the server. The server is not running, is still loading a map, or listens on another port, or the client and the server are different versions of CARLA.
>
> * Wait until the server has finished loading, and check the port: `carla.Client('localhost', 2000)` by default, `-carla-rpc-port=N` on the server otherwise.
> * Use the client that matches the server. For a source build, rebuild and reinstall it with `cmake --build Build/Release --target carla-python-api-install`. For a package, install the wheel shipped with it: `pip3 install PythonAPI/carla/dist/carla-*.whl`.
> * Check the installed client with `pip3 show carla`, and remove any other copy that `python3 -c "import carla; print(carla.__file__)"` finds first.

---

## Windows build
<!-- ======================================================================= -->

###### "CarlaUnreal.exe" does not appear when downloading from GitHub.

> There is no `CarlaUnreal.exe` executable in the source version of CARLA. Follow the [build instructions](build_windows_ue5.md) to build CARLA from source. To directly get the `CarlaUnreal.exe`, follow the [quick start instructions](start_quickstart.md).

<!-- ======================================================================= -->

###### CarlaUnreal could not be compiled.

> Something went wrong when building CARLA. Rebuild from a terminal to see the first error, which is the one that matters:
>
>      cmake --build Build\Release
>
> If the error comes from the Unreal modules, generate the Visual Studio project files (right-click `Unreal\CarlaUnreal\CarlaUnreal.uproject` and select __Generate Visual Studio project files__), open the solution with Visual Studio 2022 and build it to see the errors in context.

<!-- ======================================================================= -->

###### CMake or compiler errors on Windows.

> CARLA builds with Visual Studio 2022, which `CarlaSetup.bat` installs. Other Visual Studio versions installed side by side can make CMake or the Unreal Build Tool pick the wrong compiler, and show errors such as C2440 or C2672.
>
> Run the build from the __x64 Native Tools Command Prompt for VS 2022__, so that the right compiler is on the path. To keep other versions installed but force Unreal to use 2022, edit `%appdata%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml`:
>
> ```
> <VCProjectFileGenerator>
>   <Version>VisualStudio2022</Version>
> </VCProjectFileGenerator>
> <WindowsPlatform>
>   <Compiler>VisualStudio2022</Compiler>
> </WindowsPlatform>
> ```

<!-- ======================================================================= -->

###### The editor does not build or does not open on Windows.

> Many different issues show up as a failing `cmake --build Build\Release --target launch`. These are the most likely reasons:
>
> * __Restart the computer.__ There is a lot going on during the Windows setup. Restart and make sure that everything is updated properly.
> * __Use the CARLA fork of Unreal Engine 5.8.__ `CARLA_UNREAL_ENGINE_PATH` must hold the absolute path of the engine build. `CarlaSetup.bat` sets it; a setup done by other means may not have.
> * __Download the content.__ The server will not run without it.
> * __Visual Studio 2022.__ See [CMake or compiler errors on Windows](#cmake-or-compiler-errors-on-windows).
> * __Do not build on an external disk.__ Windows does not give the required permissions there.
>
> Other specific reasons for a system to show conflicts with CARLA may occur. Please, post these on the [forum](https://github.com/carla-simulator/carla/discussions/) so the team can get to know more about them.

<!-- ======================================================================= -->

###### Modules are missing or built with a different engine version.

> Click on __Accept__ to rebuild them.

<!-- ======================================================================= -->

###### Packaging hangs or fails at the cook.

> The `package` target passes `-NoZenAutoLaunch` to the cook, so a Zen server has to be running already. Start `zenserver --port 8558` on Linux or `zenserver.exe --port 8558` on Windows before packaging, from `Engine/Binaries/<platform>` in the engine directory, and leave it running until the package is done. See Build a package for [Linux](build_linux_ue5.md#build-a-package-with-carla-ue5) or [Windows](build_windows_ue5.md#build-a-package-with-carla-ue5).

---

## Running CARLA
<!-- ======================================================================= -->

###### Low FPS rate when running the server in Unreal Editor.

> The Unreal Editor goes to a low performance mode when out of focus.
>
> Go to __Edit > Editor Preferences > General > Performance__, and disable the __Use Less CPU when in Background__ option.

<!-- ======================================================================= -->

###### Can't run a script.

> Some scripts have requirements. These are listed in files named `requirements.txt`, in the same path as the script itself. Be sure to check these in order to run the script. The majority of them can be installed with a simple `pip3 install -r requirements.txt` command.
>
> Sometimes on Windows, scripts cannot run with just `> script_name.py`. Try `> python script_name.py`, and make sure to be in the right directory.

<!-- ======================================================================= -->

###### Connect to the simulator while running within Unreal Editor.

> Click on __Play__ and wait until the scene is loaded. At that point, a Python client can connect to the simulator on port 2000 as with the standalone simulator.

<!-- ======================================================================= -->

###### Can't run CARLA neither binary nor source build.

> NVIDIA drivers may be outdated, or the GPU may not support Vulkan (Linux) or DirectX 12 (Windows). Update the drivers first. If the issue is still unresolved, take a look at the [forum](https://github.com/carla-simulator/carla/discussions/) and post the specific issue.

<!-- ======================================================================= -->

###### ImportError: DLL load failed while importing libcarla: %1 is not a valid Win32 app.

> A 32-bit Python version is creating conflicts when trying to run a script. Uninstall it and leave only the 64-bit Python 3.

<!-- ======================================================================= -->

###### ImportError: No module named 'carla'

> Python cannot find the CARLA client library. It is distributed as a wheel (`.whl`), and has to be installed into the interpreter that runs the script:
>
> * From a source build: `cmake --build Build/Release --target carla-python-api-install` builds the wheel and installs it with pip.
> * From a package: `pip3 install PythonAPI/carla/dist/carla-*.whl`.
>
> The wheel is built for one Python version. Check that the interpreter running the script is the one the wheel was built or installed for:
>
>      python3 --version
>      python3 -c 'import carla; print("Success")'
>
> Virtual environments and Conda can complicate this: activate the environment before installing the wheel, so that pip installs it where the script will look for it. See [Install the client library](start_quickstart.md#install-client-library) for the other ways to get it.

---

## Other
<!-- ======================================================================= -->

###### Create a binary version of CARLA.

> Run `cmake --build Build/Release --target package` from the root of the repository. The package lands in `Build/Release/Package/`, with the server and the matching Python wheel. See [Build a package](build_linux_ue5.md#build-a-package-with-carla-ue5) for the options, such as the maps to cook.

<!-- ======================================================================= -->

###### Can I package CARLA for Windows on a Linux machine and vice versa?

> Although this feature is available for Unreal Engine, it is not available in CARLA. We have a number of dependencies that are not supported to be cross compiled.

<!-- ======================================================================= -->

###### How do I uninstall the CARLA client library?

> If you installed the client library using pip, uninstall it by running:
>
>      pip3 uninstall carla
