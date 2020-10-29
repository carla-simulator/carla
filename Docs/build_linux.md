# Linux build

*   [__Linux build command summary__](#linux-build-command-summary)  
*   [__Requirements__](#requirements)  
	*   [System specifics](#system-specifics)  
	*   [Dependencies](#dependencies)  
*   [__GitHub__](#github)  
*   [__Unreal Engine__](#unreal-engine)  
*   [__CARLA build__](#carla-build)  
	*   [Clone repository](#clone-repository)  
	*   [Get assets](#get-assets)  
	*   [Set the environment variable](#set-the-environment-variable)  
	*   [make CARLA](#make-carla)  

The build process can be quite long and tedious. The **[F.A.Q.](build_faq.md)** page offers solution for the most common complications. Alternatively, use the [CARLA forum](https://forum.carla.org/c/installation-issues/linux) to post any unexpected issues that may occur.  

---
## Linux build command summary

<details>
<summary> Show command lines to build on Linux</summary>

```sh
# Make sure to meet the minimum requirements
# Read the complete documentation to understand each step

# Install dependencies
sudo apt-get update &&
sudo apt-get install wget software-properties-common &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test &&
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - &&
sudo apt-add-repository "deb http://apt.llvm.org/$(lsb_release -c --short)/ llvm-toolchain-$(lsb_release -c --short)-8 main" &&
sudo apt-get update

# Additional dependencies for Ubuntu 18.04
sudo apt-get install build-essential clang-8 lld-8 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev &&
pip2 install --user setuptools &&
pip3 install --user -Iv setuptools==47.3.1

# Additional dependencies for previous Ubuntu versions
sudo apt-get install build-essential clang-8 lld-8 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev &&
pip2 install --user setuptools &&
pip3 install --user -Iv setuptools==47.3.1 &&
pip2 install --user distro &&
pip3 install --user distro

# Change default clang version
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-8/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-8/bin/clang 180

# Get a GitHub and a UE account, and link both
# Install git

# Download Unreal Engine 4.24
git clone --depth=1 -b 4.24 https://github.com/EpicGames/UnrealEngine.git ~/UnrealEngine_4.24
cd ~/UnrealEngine_4.24

# Download and install the UE patch
wget https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/UE_Patch/430667-13636743-patch.txt 430667-13636743-patch.txt
patch --strip=4 < 430667-13636743-patch.txt

# Build UE
./Setup.sh && ./GenerateProjectFiles.sh && make

# Open the UE Editor to check everything works properly
cd ~/UnrealEngine_4.24/Engine/Binaries/Linux && ./UE4Editor

# Clone the CARLA repository
git clone https://github.com/carla-simulator/carla

# Get the CARLA assets
cd ~/carla
./Update.sh

# Set the environment variable
export UE4_ROOT=~/UnrealEngine_4.24

# make the CARLA client and the CARLA server
make PythonAPI
make launch

# Press play in the Editor to initialize the server
# Run example scripts to test CARLA
# Terminal A 
cd PythonAPI/examples
python3 spawn_npc.py 
# Terminal B
cd PythonAPI/examples
python3 spawn_npc.py
python3 dynamic_weather.py

# Optionally, to compile the PythonAPI for Python2, run the following command in the root CARLA directory
make PythonAPI ARGS="--python-version=2"
```

</details>

---
## Requirements

### System specifics

* __Ubuntu 18.04.__ CARLA provides support for previous Ubuntu versions up to 16.04. **However** proper compilers are needed for UE to work properly. Dependencies for Ubuntu 18.04 and previous versions are listed separatedly below. Make sure to install the ones corresponding to your system.
* __100GB disk space.__ The complete build will require quite a lot of space, especially the Unreal Engine build (around 80GB). Make sure to have around 100GB of free disk space.
* __An adequate GPU.__ CARLA aims for realistic simulations, so the server needs at least a 4GB GPU. A dedicated GPU is highly recommended for machine learning.
* __Two TCP ports and good internet connection.__ 2000 and 2001 by default. Be sure neither the firewall nor any other application block these.


### Dependencies

CARLA needs many dependencies to run. Some of them are built automatically during this process, such as *Boost.Python*. Others are binaries that should be installed before starting the build (*cmake*, *clang*, different versions of *Python* and much more). In order to do so, run the commands below in a terminal window.

```sh
sudo apt-get update &&
sudo apt-get install wget software-properties-common &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test &&
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - &&
sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main" &&
sudo apt-get update
```

!!! Warning
    The following commands depend on your Ubuntu version. Make sure to choose accordingly. 

__Ubuntu 18.04__.
```sh
sudo apt-get install build-essential clang-8 lld-8 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev &&
pip2 install --user setuptools &&
pip3 install --user -Iv setuptools==47.3.1 &&
pip2 install --user distro &&
pip3 install --user distro
```
__Previous Ubuntu__ versions.
```sh
sudo apt-get install build-essential clang-8 lld-8 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev &&
pip2 install --user setuptools &&
pip3 install --user -Iv setuptools==47.3.1 &&
pip2 install --user distro &&
pip3 install --user distro
```

__All Ubuntu systems__.  
To avoid compatibility issues between Unreal Engine and the CARLA dependencies, use the same compiler version and C++ runtime library to compile everything. The CARLA team uses clang-8 and LLVM's libc++. Change the default clang version to compile Unreal Engine and the CARLA dependencies.

```sh
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-8/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-8/bin/clang 180
```

---
## GitHub

__1.__ Create a [GitHub](https://github.com/) account. CARLA is organized in different GitHub repositories, so an account will be needed to clone said repositories.  

__2.__ Install [git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git) to manage the repositories via terminal.  

__3.__ Create an [Unreal Engine](https://www.unrealengine.com/en-US/feed) account to access the Unreal Engine repositories, which are set to private. 

__4.__ Connect both your GitHub and Unreal Engine accounts. Go to your personal settings in  there is a section in [Unreal Engine](https://www.unrealengine.com/en-US/)'s website. Click on `Connections > Accounts`, and link both accounts.  [Here](https://www.unrealengine.com/en-US/blog/updated-authentication-process-for-connecting-epic-github-accounts) is a brief explanation just in case.

!!! Warning
    New Unreal Engine accounts need activation. After creating the account, a verification mail will be sent. Check it out, or the UE repository will be shown as non-existent in the following steps.

---
## Unreal Engine

The current version of CARLA runs on __Unreal Engine 4.24__ only. In this guide, the installation will be done in `~/UnrealEngine_4.24`, but the path can be changed. If your path is different, change the following commands accordingly.  

!!! Note
    Alternatively to this section, there is another [guide](https://docs.unrealengine.com/en-US/Platforms/Linux/BeginnerLinuxDeveloper/SettingUpAnUnrealWorkflow/index.html) to build UE on Linux. When consulting it, remember that CARLA will need the __4.24 release__, not the latest.

__1.__ Clone the content for Unreal Engine 4.24 in your local computer.
```sh
git clone --depth=1 -b 4.24 https://github.com/EpicGames/UnrealEngine.git ~/UnrealEngine_4.24
```
__2.__ Get into the directory where UE4.24 has been cloned.
```sh
cd ~/UnrealEngine_4.24
```

__3.__ Download a patch for Unreal Engine. This patch fixes some Vulkan visualization issues that may occur when changing the map. Download and install it with the following commands.
```sh
wget https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/UE_Patch/430667-13636743-patch.txt 430667-13636743-patch.txt
patch --strip=4 < 430667-13636743-patch.txt
```

!!! Warning
    If UE has already been built, install the patch, and make the build again.


__4.__ Make the build. This may take an hour or two depending on your system. 
```sh
./Setup.sh && ./GenerateProjectFiles.sh && make
```

__5.__ Open the Editor to check that UE has been properly installed.
```sh
cd ~/UnrealEngine_4.24/Engine/Binaries/Linux && ./UE4Editor
```

Any issues this far are related with Unreal Engine. There is not much CARLA can do about it. However, the [build documentation](https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Build/BatchFiles/Linux/README.md) provided by Unreal Engine may be helpful.

---
## CARLA build
The system should be ready to start building CARLA. Just for clarity, a brief summary so far.

* Minimum technical requirements to run CARLA are suitable.
* Dependencies have been properly installed.
* GitHub account is ready.
* Unreal Engine 4.24 runs smooth.

!!! Note
    Downloading aria2 with `sudo apt-get install aria2` will speed up the following commands.

### Clone repository

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla" target="_blank" class="btn btn-neutral" title="Go to the CARLA repository">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>
The official repository of the project. Either download and extract it, or clone the repository with the following command line.

```sh
git clone https://github.com/carla-simulator/carla
```

Now the latest state of the simulator, known as `master` branch in the repository, has been copied in local. Here is brief introduction to the most relevant branches of the repository.  Remember that you can change and check your branches with the command `git branch`.  

*   __`master` branch__ — Latest fixes and features that have been tested. These will be featured in the next CARLA release.  
*   __`dev` branch__ — Latest fixes and features still in development and testing. This branch will be merged with `master` when the time for a new release comes.  
*   __`stable` branch__ — Latest version of CARLA tagged as stable. Previous CARLA versions also have their own branch.  


### Get assets

Download the assets, as they are necessary to run CARLA. These are stored in a separated package to reduce the size of the build. A script downloads and extracts the latest stable assets automatically. The package is >3GB, so the download may take some time.

__1.__ Get into your root carla directory. The path should correspond with the repository just cloned.
```sh
cd ~/carla
```
__2.__ Run the script to get the assets.
```sh
./Update.sh
```
!!! Important
    To download the assets currently in development, visit [Update CARLA](build_update.md#get-development-assets) and read __Get development assets__.


### Set the environment variable

This is necessary for CARLA to find the Unreal Engine 4.24 installation folder.

```sh
export UE4_ROOT=~/UnrealEngine_4.24
```

The variable should be added to `~/.bashrc` or `~/.profile` to be set persistently session-wide. Otherwise, it will only be accessible from the current shell. To do this, follow these steps.  

__1.__ Open `~/.bashrc`.  
```sh
gedit ~/.bashrc
```

__2.__ Write the environment variable in the `~/.bashrc` file: `export UE4_ROOT=~/UnrealEngine_4.24`  


__3.__ Save the file and reset the terminal.  



### make CARLA

The last step is to finally build CARLA. There are different `make` commands to build the different modules. All of them run in the root CARLA folder.

!!! Warning
    Make sure to run `make PythonAPI` to prepare the client and `make launch` for the server.
    Alternatively `make LibCarla` will prepare the CARLA library to be imported anywhere.

* __make PythonAPI__ compiles the API client, necessary to grant control over the simulation. It is only needed the first time. Remember to run it again when updating CARLA. Scripts will be able to run after this command is executed.  
```sh
make PythonAPI
```

* __make launch__ compiles the server simulator and launches Unreal Engine. Press **Play** to start the spectator view and close the editor window to exit. Camera can be moved with `WASD` keys and rotated by clicking the scene while moving the mouse around.  
```sh
make launch
```
The project may ask to build other instances such as `UE4Editor-Carla.dll` the first time. Agree in order to open the project. During the first launch, the editor may show warnings regarding shaders and mesh distance fields. These take some time to be loaded and the city will not show properly until then.


Finally, let's test the simulator. Inside `PythonAPI/examples` and `PythonAPI/util` there are some example scripts that may be especially useful for starters. The following commands will spawn some life into the town, and create a weather cycle. Each script should be run in one terminal 

```sh
# Support for Python2 was provided until 0.9.10 (not included)
# Terminal A 
cd PythonAPI/examples
python3 spawn_npc.py  
# Terminal B
cd PythonAPI/examples
python3 dynamic_weather.py 
```
!!! Important
    If the simulation is running at very low FPS rates, go to `Edit/Editor preferences/Performance` in the UE editor and disable __Use less CPU when in background__.

Optionally, to compile the PythonAPI for Python2, run the following command in the root CARLA directory.

```sh
make PythonAPI ARGS="--python-version=2"
```

Now CARLA is ready to go. Here is a brief summary of the most useful `make` commands available.

<table class ="defTable">
<thead>
<th>Command</th>
<th>Description</th>
</thead>
<tbody>
<td><code>make help</code> </td>
<td>Prints all available commands.</td>
</tr>
<tr>
<td><code>make launch</code></td>
<td>Launches CARLA server in Editor window.</td>
</tr>
<tr>
<td><code>make PythonAPI</code></td>
<td>Builds the CARLA client.</td>
</tr>
<tr>
<td><code>make package</code></td>
<td>Builds CARLA and creates a packaged version for distribution.</td>
</tr>
<tr>
<td><code>make clean</code></td>
<td>Deletes all the binaries and temporals generated by the build system.</td>
</tr>
<tr>
<td><code>make rebuild</code></td>
<td><code>make clean</code> and <code>make launch</code> both in one command.</td>
</tbody>
</table>

---

Read the **[F.A.Q.](build_faq.md)** page or post in the [CARLA forum](https://forum.carla.org/c/installation-issues/linux) for any issues regarding this guide.  

Some recommendations after finishing the build. Learn how to update the CARLA build or take your first steps in the simulation, and learn some core concepts.  
<div class="build-buttons">

<p>
<a href="../build_update" target="_blank" class="btn btn-neutral" title="Learn how to update the build">
Update CARLA</a>
</p>

<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Learn about CARLA core concepts">
First steps</a>
</p>

</div>
