CARLA Simulator
===============

[![Documentation](https://readthedocs.org/projects/carla/badge/?version=latest)](https://carla-ue5.readthedocs.io)

[![carla.org](Docs/img/btn/web.png)](http://carla.org)
[![download](Docs/img/btn/download.png)](https://github.com/carla-simulator/carla/releases)
[![documentation](Docs/img/btn/docs.png)](https://carla-ue5.readthedocs.io)
[![forum](Docs/img/btn/forum.png)](https://github.com/carla-simulator/carla/discussions)
[![discord](Docs/img/btn/chat.png)](https://discord.gg/8kqACuC)

CARLA is an open-source simulator for autonomous driving research. CARLA has been developed from the ground up to support development, training, and
validation of autonomous driving systems. In addition to open-source code and protocols, CARLA provides open digital assets (urban layouts, buildings,
vehicles) that were created for this purpose and can be used freely. The simulation platform supports flexible specification of sensor suites and
environmental conditions.

[![CARLA Video](Docs/img/carla_ue5_readme_img.webp)](https://www.youtube.com/watch?v=q4V9GYjA1pE)

>[!NOTE]
> This is the development branch `ue5-dev` for the **Unreal Engine 5.5 version of CARLA**. This branch exists in parallel with the Unreal Engine 4.26 version of CARLA, in the `ue4-dev` branch. Please be sure that this version of CARLA is suitable for your needs as there are significant differences between the UE 5.5 and UE 4.26 versions of CARLA. 

### Recommended system

* Intel i7 gen 9th - 11th / Intel i9 gen 9th - 11th / AMD Ryzen 7 / AMD Ryzen 9
* +32 Gb RAM memory 
* NVIDIA RTX 3070/3080/3090 / NVIDIA RTX 4090 or better
* 16 Gb or more VRAM
* Ubuntu 22.04 or Windows 11

 >[!NOTE]
> Ubuntu version 22.04 and Windows version 11 are required, the Unreal Engine 5.5 version of CARLA will not work on Ubuntu 20.04 or Windows 10 or lower. 

## Documentation

The [CARLA documentation](https://carla-ue5.readthedocs.io/en/latest/) is hosted on ReadTheDocs. Please see the following key links:

- [Building on Linux](https://carla-ue5.readthedocs.io/en/latest/build_linux_ue5/)
- [Building on Windows](https://carla-ue5.readthedocs.io/en/latest/build_windows_ue5/)
- [First steps](https://carla-ue5.readthedocs.io/en/latest/tuto_first_steps/)
- [CARLA asset catalogue](https://carla-ue5.readthedocs.io/en/latest/catalogue/)
- [Python API reference](https://carla-ue5.readthedocs.io/en/latest/python_api/)
- [Blueprint library](https://carla-ue5.readthedocs.io/en/latest/bp_library/)

## CARLA Ecosystem
Repositories associated with the CARLA simulation platform:

* [**CARLA Autonomous Driving leaderboard**](https://leaderboard.carla.org/): Automatic platform to validate Autonomous Driving stacks
* [**Scenario_Runner**](https://github.com/carla-simulator/scenario_runner): Engine to execute traffic scenarios in CARLA 0.9.X
* [**ROS-bridge**](https://github.com/carla-simulator/ros-bridge): Interface to connect CARLA 0.9.X to ROS
* [**Driving-benchmarks**](https://github.com/carla-simulator/driving-benchmarks): Benchmark tools for Autonomous Driving tasks
* [**Conditional Imitation-Learning**](https://github.com/felipecode/coiltraine): Training and testing Conditional Imitation Learning models in CARLA
* [**AutoWare AV stack**](https://github.com/carla-simulator/carla-autoware): Bridge to connect AutoWare AV stack to CARLA
* [**Reinforcement-Learning**](https://github.com/carla-simulator/reinforcement-learning): Code for running Conditional Reinforcement Learning models in CARLA
* [**RoadRunner**](https://www.mathworks.com/products/roadrunner.html): MATLAB GUI based application to create road networks in OpenDrive format 
* [**Map Editor**](https://github.com/carla-simulator/carla-map-editor): Standalone GUI application to enhance RoadRunner maps with traffic lights and traffic signs information


**Like what you see? Star us on GitHub to support the project!**

Paper
-----

If you use CARLA, please cite our CoRL’17 paper.

_CARLA: An Open Urban Driving Simulator_<br>Alexey Dosovitskiy, German Ros,
Felipe Codevilla, Antonio Lopez, Vladlen Koltun; PMLR 78:1-16
[[PDF](http://proceedings.mlr.press/v78/dosovitskiy17a/dosovitskiy17a.pdf)]
[[talk](https://www.youtube.com/watch?v=xfyK03MEZ9Q&feature=youtu.be&t=2h44m30s)]


```
@inproceedings{Dosovitskiy17,
  title = {{CARLA}: {An} Open Urban Driving Simulator},
  author = {Alexey Dosovitskiy and German Ros and Felipe Codevilla and Antonio Lopez and Vladlen Koltun},
  booktitle = {Proceedings of the 1st Annual Conference on Robot Learning},
  pages = {1--16},
  year = {2017}
}
```

## Building CARLA with Unreal Engine 5.5
--------------

Clone this repository locally from GitHub, specifying the *ue5-dev* branch:

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

In order to build CARLA, you need acces to the CARLA fork of Unreal Engine 5.5. In order to access this repository, you must first link your GitHub account to Epic Games by following [this guide](https://www.unrealengine.com/en-US/ue-on-github). You then also need to use your git credentials to authorise the download of the Unreal Engine 5.5 repository. 

__Building in Linux__:

Run the setup script from a terminal open in the CARLA root directory:

```sh
cd CarlaUE5
./CarlaSetup.sh --interactive
```

The setup script will prompt you for your sudo password, in order to install the prerequisites. It will then prompt you for your GitHub credentials in order to authorise the download of the Unreal Engine repository. 

The setup script will install by default Python 3 using apt. If you want to target an existing Python installation, you should use the `--python-root=PATH_TO_PYTHON` argument with the relevant Python installation path. You can use whereis python3 in your chosen environment and strip the `/python3` suffix from the path.

__Building in Linux unattended__:

If you want to run the setup script unattended, your git credentials need to be stored in an environment variable. Add your github credentials to your `.bashrc` file:

```sh
export GIT_LOCAL_CREDENTIALS=username@github_token
```

Then run the setup script using the following command:

```sh
cd CarlaUE5
sudo -E ./CarlaSetup.sh
```

This will download and install Unreal Engine 5.5, install the prerequisites and build CARLA. It may take some time to complete and use a significant amount of disk space.

If you prefer to add the git credentials in the terminal, use the following command:

```sh
cd CarlaUE5
sudo -E env GIT_LOCAL_CREDENTIALS=github_username@github_token ./CarlaSetup.sh 
```

__Building in Windows__:

To build in Windows, run the batch script:

```sh
cd CarlaUE5
CarlaSetup.bat
```

Unattended mode is currently unavailable in Windows, you will need to enter GitHub credentials or administrator privileges when prompted.

## Rebuilding CARLA

Once the setup is complete, you can execute subsequent builds with the following commands in a terminal open in the CARLA root directory. In Linux, run these commands in a standard terminal. In Windows, open the x64 Native Tools Command Prompt for Visual Studio 2022.

__Configure__:

Linux:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON
```

Windows:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/Toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```

>[!NOTE]
> If you intend to target a specific Python installation, you should add both these arguments to the above cmake command: `-DPython_ROOT_DIR=PATH` and `-DPython3_ROOT_DIR=PATH`.

__Build__:

Linux and Windows:

```sh
cmake --build Build
```

__Build and install the Python API__:


Linux and windows:

```sh
cmake --build Build --target carla-python-api-install
```

__Launch the editor__:

```sh
cmake --build Build --target launch
```

For more instructions on building CARLA UE5, please consult the build documentation for [Linux](https://carla-ue5.readthedocs.io/en/latest/build_linux_ue5/) or [Windows](https://carla-ue5.readthedocs.io/en/latest/build_windows_ue5/).

Contributing
------------

Please take a look at our [Contribution guidelines][contriblink].

[contriblink]: https://carla-ue5.readthedocs.io/en/latest/cont_contribution_guidelines/

Licenses
-------

## CARLA licenses

CARLA specific code is distributed under the MIT License.

CARLA specific assets are distributed under the CC-BY License.

## CARLA Dependency and Integration licenses

Unreal Engine 5 follows its [own license terms](https://www.unrealengine.com/en-US/faq).

CARLA uses some dependencies related to 3rd party integrations:
- [PROJ](https://proj.org/), a generic coordinate transformation software which uses the [X/MIT open source license](https://proj.org/about.html#license).
- [SQLite](https://www.sqlite.org), part of the PROJ dependencies, which is [in the public domain](https://www.sqlite.org/purchase/license).
- [Xerces-C](https://xerces.apache.org/xerces-c/), a validating XML parser, which is made available under the [Apache Software License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.html).
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page), a C++ template library for linear algebra which uses the [MPL2 license](https://www.mozilla.org/en-US/MPL/2.0/).

CARLA uses the Autodesk FBX SDK for converting FBX to OBJ in the import process of maps. This step is optional, and the SDK is located [here](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0)

This software contains Autodesk® FBX® code developed by Autodesk, Inc. Copyright 2020 Autodesk, Inc. All rights, reserved. Such code is provided "as is" and Autodesk, Inc. disclaims any and all warranties, whether express or implied, including without limitation the implied warranties of merchantability, fitness for a particular purpose or non-infringement of third party rights. In no event shall Autodesk, Inc. be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of such code."
