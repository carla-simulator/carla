# Linux build

* [__Linux build command summary__](#linux-build-command-summary)  
* [__Requirements__](#requirements)  
	* System specifics
	* Dependencies 
* [__GitHub__](#github)  
	* Create a GitHub account  
	* Install git  
	* Link GitHub and Unreal Engine 
* [__Unreal Engine__](#unreal-engine)
	* Download UE4.22  
	* Build UE4.22  
* [__CARLA build__](#carla-build)  
	* Clone repository  
	* Get assets  
	* Set the environment variable 
	* make CARLA  

The build process can be quite long and tedious. The **[F.A.Q.](build_faq.md)** section contains the most common issues and solutions that appear during the installation. However, the CARLA forum is open for anybody to post unexpected issues, doubts or suggestions. There is a specific section for installation issues on Linux. Feel free to login and become part of the community. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>

---
## Linux build command summary 

<details>
<summary> Show command lines to build on Linux</summary>

```sh
# Make sure to meet the minimum requirements and read the documentation to understand each step.

# Install dependencies. 
sudo apt-get update &&
sudo apt-get install wget software-properties-common &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test &&
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - &&
sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main" &&
sudo apt-get update

# Additional dependencies for Ubuntu 18.04.
sudo apt-get install build-essential clang-7 lld-7 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev &&
pip2 install --user setuptools &&
pip3 install --user setuptools 

# Additional dependencies for previous Ubuntu versions. 
sudo apt-get install build-essential clang-7 lld-7 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev &&
pip2 install --user setuptools &&
pip3 install --user setuptools 

# Change default clang version.
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-7/bin/clang++ 170 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-7/bin/clang 170

# Get a GitHub and a UE account, and link both. 
# Install git. 

# Build Unreal Engine 4.22.
git clone --depth=1 -b 4.22 https://github.com/EpicGames/UnrealEngine.git ~/UnrealEngine_4.22
cd ~/UnrealEngine_4.22
./Setup.sh && ./GenerateProjectFiles.sh && make

# Open the UE Editor to check everything works properly.
cd ~/UnrealEngine_4.22/Engine/Binaries/Linux && ./UE4Editor

# Clone the CARLA repository. 
git clone https://github.com/carla-simulator/carla

# Get the CARLA assets. 
cd ~/carla
./Update.sh

# Set the environment variable. 
export UE4_ROOT=~/UnrealEngine_4.22

# make the CARLA server and the CARLA client. 
make launch
make PythonAPI 

# Run an example script to test CARLA. 
cd PythonAPI/examples
python3 spawn_npc.py
```
</details>

---
## Requirements

### System specifics

* __Ubuntu 16.04 or later.__ Currently migrating to Ubuntu 18.  
* __30GB disk space.__ Installing all the software needed and CARLA itself will require quite a lot of space, especially Unreal Engine. Make sure to have around 30/50GB of free disk space.  
* __An adequate GPU.__ CARLA aims for realistic simulations, so the server needs at least a 4GB GPU. A dedicated GPU is highly recommended for machine learning.  
* __Two TCP ports and good internet connection.__ 2000 and 2001 by default. Be sure neither the firewall nor any other application block these.  


### Dependencies

CARLA needs many dependencies to run. Some of them are built automatically during this process, such as *Boost.Python*. Others are binaries that should be installed before starting the build (*cmake*, *clang*, different versions of *Python* and much more). In order to do so, run the commands below in a terminal window. 

```sh
sudo apt-get update &&
sudo apt-get install wget software-properties-common &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test &&
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - &&
sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main" &&
sudo apt-get update
```

!!! Important
    The following commands differ depending on the Ubuntu version. While the only change is `libpng16-dev` becoming `libpng-dev`, the full set of commands is here twice to ease the copy. 

__Ubuntu 18.04__.
```sh
sudo apt-get install build-essential clang-7 lld-7 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev &&
pip2 install --user setuptools &&
pip3 install --user setuptools 
```
__Previous Ubuntu__ versions. 
```sh
sudo apt-get install build-essential clang-7 lld-7 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev &&
pip2 install --user setuptools &&
pip3 install --user setuptools 
```

To avoid compatibility issues between Unreal Engine and the CARLA dependencies, it is recommended to use the same compiler version and C++ runtime library to compile everything. The CARLA team uses clang-7 and LLVM's libc++. Change the default clang version to compile Unreal Engine and the CARLA dependencies.

```sh
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-7/bin/clang++ 170 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-7/bin/clang 170
```

---
## GitHub

A [GitHub](https://github.com/) account will be needed, as CARLA content is organized in different repositories in there. Also, [git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git) will be used in this build guide when facilitating commands to be run in terminal.  
  
In order to access the Unreal Engine repositories, which are set to private, create an [Unreal Engine](https://www.unrealengine.com/en-US/feed) account and connect it to a GitHub account. To do so, there is a section in Unreal Engine's profile settings under the name of __Connected accounts__. [Here](https://www.unrealengine.com/en-US/blog/updated-authentication-process-for-connecting-epic-github-accounts) is a brief explanation just in case. 

---
## Unreal Engine

The current version of CARLA runs on __Unreal Engine 4.22__ only. The path is irrelevant, but for the sake of this tutorial, installation will be done under `~/UnrealEngine_4.22`. If the path chosen differs, remember to change it accordingly when running the commands on terminal.   

!!! Note
    Alternatively, there is this [guide](https://docs.unrealengine.com/en-US/Platforms/Linux/BeginnerLinuxDeveloper/SettingUpAnUnrealWorkflow/index.html) to build UE on Linux. When consulting it, remember that CARLA will need the __4.22 release__, not the latest. 

Clone the content for Unreal Engine 4.22 in a local computer.
```sh
git clone --depth=1 -b 4.22 https://github.com/EpicGames/UnrealEngine.git ~/UnrealEngine_4.22
```
Get into said folder. Remember, this is the path where UE4.22 has been cloned.
```sh
cd ~/UnrealEngine_4.22
```
Make the build. 
```sh
./Setup.sh && ./GenerateProjectFiles.sh && make
```

Unreal Engine should be installed in the system. Run `Engine/Binaries/Linux/UE4Editor.sh` to open the Editor and check it out.
```sh
cd ~/UnrealEngine_4.22/Engine/Binaries/Linux && ./UE4Editor
```

In case something went wrong, it is related with Unreal Engine There is not much CARLA can do about it. However, the [build documentation](https://wiki.unrealengine.com/Building_On_Linux) provided by Unreal Engine may be helpful.

---
## CARLA build
The system should be ready to start building CARLA. Just for clarity, a brief summary so far.  

* Minimum technical requirements to run CARLA are suitable.  
* Dependencies have been properly installed.  
* GitHub account is ready.  
* Unreal Engine 4.22 runs smooth.  

!!! Note
    Downloading aria2 with `sudo apt-get install aria2` will speed up the following commands.  

### Clone repository

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla" target="_blank" class="btn btn-neutral" title="Go to the CARLA repository">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>
The official repository of the project. Either download and extract it or clone it using the following command line. 

```sh
git clone https://github.com/carla-simulator/carla
```

Now the latest content for the project, known as `master` branch in the repository, has been copied in local. 

!!! Note
    The `master` branch contains the latest fixes and features. Stable code is inside the `stable` and previous CARLA versions have their own branch. Always remember to check the current branch in git with the command `git branch`. 

### Get assets

Only the assets package is yet to be donwloaded. These are stored separately to make the repository a bit lighter. CARLA cannot be built without the assets. There is a script that downloads and extracts the latest content version. The package is >3GB, so downloading it may take some time.  

Get into the root carla folder. The path should correspond with the repository just cloned:
```sh
cd ~/carla
```
Run the script to get the assets. 
```sh
./Update.sh
```
!!! Important
    To get the assets currently in development, visit [Update CARLA](build_update.md#get-development-assets) and read __Get development assets__. 


### Set the environment variable

This is necessary for CARLA to find the Unreal Engine 4.22 installation folder.

```sh
export UE4_ROOT=~/UnrealEngine_4.22
```

The variable should be added to `~/.bashrc` or `~/.profile` to be set persistently session-wide. Otherwise, it will only be accessible from the current shell.

### make CARLA

The last step is to finally build CARLA. There are different `make` commands to build the different modules. All of them run in the root CARLA folder.  

!!! Warning
    Make sure to run __make launch__ to prepare the server and __make PythonAPI__ for the client.  
    Alternatively __make libcarla__ will prepare the CARLA library to be imported anywhere. 

* __make launch__ compiles the server simulator and launches Unreal Engine. Press **Play** to start the spectator view and close the editor window to exit. Camera can be moved with `WASD` keys and rotated by clicking the scene while moving the mouse around.  
```sh
make launch
```  
The project may ask to build other instances such as `UE4Editor-Carla.dll` the first time. Agree in order to open the project. During the first launch, the editor may show warnings regarding shaders and mesh distance fields. These take some time to be loaded and the city will not show properly until then.  

* __make PythonAPI__ compiles the API client, necessary to grant control over the simulation. It is only needed the first time. Remember to run it again when updating CARLA. Scripts will be able to run after this command is executed. The following example will spawn some life into the town. 

```sh
make PythonAPI && cd PythonAPI/examples && python3 spawn_npc.py
``` 
!!! Important
    If the simulation is running at very low FPS rates, go to `Edit/Editor preferences/Performance` in the UE editor and disable __Use less CPU when in background__. 

Now CARLA is ready to go. Here is a brief summary of the most useful `make` commands available.  

<table class ="defTable">
<thead>
<th>Command</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>make help</code> </td>
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
<td>make clean and make launch both in one command.</td>
</tbody>
</table>

---
<br>
Keep reading this section to learn how to update the build or take some first steps in CARLA. 
<div class="build-buttons">
<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="Learn about CARLA core concepts">
Go to __First steps__</a>
</p>

<p>
<a href="../build_update" target="_blank" class="btn btn-neutral" title="Learn how to update the build">
Update CARLA</a>
</p>
</div>
