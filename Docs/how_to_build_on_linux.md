<h1>Linux build</h1>

  * [__Requirements__](#requirements):  
	* System specifics
	* Dependencies 
  * [__GitHub__](#github):  
	* Create a GitHub account  
	* Install git  
	* Link GitHub and Unreal Engine 
  * [__Unreal Engine__](#unreal-engine):
	* Download UE4.22  
	* Build UE4.22  
  * [__CARLA build__](#carla-build): 
	* Clone repository  
	* Get assets  
	* Set the environment variable 
	* make CARLA  

The build process can be quite long and tedious. This documentation tries to make things clear, but the CARLA forum is open to anybody who wants to post unexpected issues, doubts or suggestions. There is a specific section for installation issues in Linux. Feel free to login and become part of the community. 

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</p>
</div>

---------------
##Requirements
<h4>System specifics</h4>

  * __Ubuntu 16.04 or later:__ Currently migrating to Ubuntu 18, but there is support up to Ubuntu 16.04.
  * __Two TCP ports:__ 2000 and 1 by default. Be sure neither firewall nor any other application are blocking these. 
  * __An adequate GPU:__ CARLA aims for realistic simulations, so the server needs at least a 4GB GPU. A dedicated GPU is highly recommended for machine learning. 


<h4>Dependencies</h4>

CARLA needs many dependencies to run. Some of them are built automatically during this process, such as *Boost.Python*. Others are binaries that should be installed before starting the build (*cmake*, *clang*, different versions of *Python* and much more). In order to do so, run the commands below in a terminal window. 

```sh
sudo apt-get update &&
sudo apt-get install wget software-properties-common &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test &&
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - &&
sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main" &&
sudo apt-get update
```

!!! important
    The following commands differ depending on Ubuntu version. While the only change is `libpng16-dev` becoming `libpng-dev`, the full set of commands is here twice to ease the copy. 

__Ubuntu 18.04__:
```sh
sudo apt-get install build-essential clang-7 lld-7 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync &&
pip2 install --user setuptools &&
pip3 install --user setuptools 
```
__Previous Ubuntu__ versions: 
```sh
sudo apt-get install build-essential clang-7 lld-7 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync &&
pip2 install --user setuptools &&
pip3 install --user setuptools 
```

Finally, to avoid compatibility issues between Unreal Engine and the CARLA dependencies, it is recommended to use the same compiler version and C++ runtime library to compile everything. The CARLA team uses clang-7 and LLVM's libc++. Change your default clang version to compile Unreal Engine and the CARLA dependencies.

```sh
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-7/bin/clang++ 170 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-7/bin/clang 170
```

-------------------
##GitHub

First of all, a [GitHub](https://github.com/) account will be needed, as CARLA content is organized in different repositories in there. Also, [git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git) will be used in this build guide when facilitating commands to be run in terminal.    
  
In order to access the Unreal Engine repositories, which are set to private, create an [Unreal Engine](https://www.unrealengine.com/en-US/feed) account and connect it to a GitHub account. To do so, there is a section in Unreal Engine's profile settings under the name of __Connected accounts__. [Here](https://www.unrealengine.com/en-US/blog/updated-authentication-process-for-connecting-epic-github-accounts) is a brief explanation just in case. 

-------------------
##Unreal Engine

The current version of CARLA runs on __Unreal Engine 4.22__ only, so the following steps will be downloading this version and building it. The path is irrelevant, but for the sake of this tutorial, installation will be done under `~/UnrealEngine_4.22`. If the path chosen differs, remember to change it accordingly when running the commands on terminal.   
The process described below can be found in this [complete guide](https://docs.unrealengine.com/en-US/Platforms/Linux/BeginnerLinuxDeveloper/SettingUpAnUnrealWorkflow/index.html) on how to build Unreal Engine on Linux. It can be consulted if preferred, but remember though that CARLA will need the __4.22 release__, not the latest. 

Clone the content for Unreal Engine 4.22 in a local computer:
```sh
git clone --depth=1 -b 4.22 https://github.com/EpicGames/UnrealEngine.git ~/UnrealEngine_4.22
```
Get into said folder. Remember, this is the path where you cloned UE4.22:
```sh
cd ~/UnrealEngine_4.22
```
Run the commands necessary to make the build. 
```sh
./Setup.sh && ./GenerateProjectFiles.sh && make
```

Unreal Engine should be installed in the system. To try running it go to _Engine/Binaries/Linux_ folder and run _the UE4Editor.sh_:
```sh
cd ~/UnrealEngine_4.22/Engine/Binaries/Linux && ./UE4Editor
```

If anything goes wrong, it is related with Unreal Engine and there is not much CARLA can do about it. However, checking the guide mentioned above or visiting the [build documentation](https://wiki.unrealengine.com/Building_On_Linux) provided by Unreal Engine could be helpful.

-----------
## CARLA build
The system should be ready to start building CARLA. Just for clarity, a brief summary so far:  

  * Minimum technical requirements to run CARLA are suitable.
  * Dependencies have been properly installed.
  * GitHub account is ready. 
  * Unreal Engine 4.22 runs smooth.

!!! Note
    Optionally you can download aria2 (with `sudo apt-get install aria2`) so the following commands run a bit faster. 

<h4>Clone repository</h4>

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://github.com/carla-simulator/carla" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
<span class="icon icon-github"></span> CARLA repository</a>
</p>
</div>
The project can be found in GitHub, inside the repository. Either download and extract it or clone it on the local computer using the following command line: 

```sh
git clone https://github.com/carla-simulator/carla
```

Now the latest content for the project, known as `master` branch in the repository, has been copied in local. 

!!! important
    The `master` branch contains the latest fixes and features. Stable code is inside the `stable` branch, and it can be built by changing the branch. Always remember to check the current branch in git. 

<h4>Get assets</h4>

Only the assets package, the visual content, is yet to be donwloaded. These are stored separately to make the repository a bit lighter. CARLA cannot be built without the assets, so there is a script that downloads and extracts the latest content version (this package is >3GB, it might take some time depending on internet connection).  
Get into the root carla folder. The path should correspond with the repository just cloned:
```sh
cd ~/carla
```
Run the script to get the assets:
```sh
./Update.sh
```


<h4>Set the environment variable </h4>

For CARLA to find the Unreal Engine 4.22 installation folder, an environment variable needs to be set.

```sh
export UE4_ROOT=~/UnrealEngine_4.22
```

This variable should be added to `~/.bashrc` or `~/.profile` to set it persistently session-wide. Otherwise, it will only be accessible for current shell.

<h4>make CARLA</h4>

The last step is to finally build CARLA. There are different `make` commands to build the different modules. All of them run in the root CARLA folder: 

  * __launch__ will compile the server simulator and launch it in Unreal Engine. Press **Play** to start the spectator view and close the editor window to exit. Camera can be moved with WASD keys and rotated by clicking the scene while moving the mouse around:  
```sh
make launch
``` 
  * __PythonAPI__ will compile the API client, necessary to grant control over the simulation. After building it, scripts can run. The following example will run a dynamic weather on the scene: 
```sh
make PythonAPI && cd PythonAPI/examples && ./manual_control.py
``` 
  * __package__ will create a precompiled version of CARLA. It acts as a quickstart and will run without the UE editor: 
```sh
make package
``` 
  * __help__ will show all commands available: 
```sh
make help
``` 

With this, everything is set and done. CARLA is finally ready to run. Keep reading this section to learn more about how to update CARLA, the build itself and some advanced configuration options.  
Otherwise, visit the __First steps__ section to start using and learning about CARLA: 
<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="../python_api_tutorial" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
Go to First steps</a>
</p>
</div>
