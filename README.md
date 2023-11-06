CARLA Simulator
===============

[![Build Status](https://travis-ci.org/carla-simulator/carla.svg?branch=master)](https://travis-ci.org/carla-simulator/carla)
[![Documentation](https://readthedocs.org/projects/carla/badge/?version=latest)](http://carla.readthedocs.io)

[![carla.org](Docs/img/btn/web.png)](http://carla.org)
[![download](Docs/img/btn/download.png)](https://github.com/carla-simulator/carla/blob/master/Docs/download.md)
[![documentation](Docs/img/btn/docs.png)](http://carla.readthedocs.io)
[![forum](Docs/img/btn/forum.png)](https://github.com/carla-simulator/carla/discussions)
[![discord](Docs/img/btn/chat.png)](https://discord.gg/8kqACuC)

CARLA is an open-source simulator for autonomous driving research. CARLA has been developed from the ground up to support development, training, and
validation of autonomous driving systems. In addition to open-source code and protocols, CARLA provides open digital assets (urban layouts, buildings,
vehicles) that were created for this purpose and can be used freely. The simulation platform supports flexible specification of sensor suites and
environmental conditions.

[![CARLA Video](Docs/img/video_thumbnail_0910.jpg)](https://www.youtube.com/watch?v=7jej46ALVRE)

If you want to benchmark your model in the same conditions as in our CoRL’17
paper, check out
[Benchmarking](https://github.com/carla-simulator/driving-benchmarks).

Linux:
* [**Get CARLA overnight build**](http://carla-releases.s3.amazonaws.com/Linux/Dev/CARLA_Latest.tar.gz)
* [**Get AdditionalMaps overnight build**](http://carla-releases.s3.amazonaws.com/Linux/Dev/AdditionalMaps_Latest.tar.gz)

Windows:
* [**Get CARLA overnight build**](http://carla-releases.s3.amazonaws.com/Windows/Dev/CARLA_Latest.zip)
* [**Get AdditionalMaps overnight build**](http://carla-releases.s3.amazonaws.com/Windows/Dev/AdditionalMaps_Latest.zip)

### Recommended system

* Intel i7 gen 9th - 11th / Intel i9 gen 9th - 11th / AMD ryzen 7 / AMD ryzen 9
* +16 GB RAM memory 
* NVIDIA RTX 2070 / NVIDIA RTX 2080 / NVIDIA RTX 3070, NVIDIA RTX 3080
* Ubuntu 18.04

## CARLA Ecosystem
Repositories associated to the CARLA simulation platform:

* [**CARLA Autonomous Driving leaderboard**](https://leaderboard.carla.org/): Automatic platform to validate Autonomous Driving stacks
* [**Scenario_Runner**](https://github.com/carla-simulator/scenario_runner): Engine to execute traffic scenarios in CARLA 0.9.X
* [**ROS-bridge**](https://github.com/carla-simulator/ros-bridge): Interface to connect CARLA 0.9.X to ROS
* [**Driving-benchmarks**](https://github.com/carla-simulator/driving-benchmarks): Benchmark tools for Autonomous Driving tasks
* [**Conditional Imitation-Learning**](https://github.com/felipecode/coiltraine): Training and testing Conditional Imitation Learning models in CARLA
* [**AutoWare AV stack**](https://github.com/carla-simulator/carla-autoware): Bridge to connect AutoWare AV stack to CARLA
* [**Reinforcement-Learning**](https://github.com/carla-simulator/reinforcement-learning): Code for running Conditional Reinforcement Learning models in CARLA
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

Building CARLA
--------------

Use `git clone` or download the project from this page. Note that the master branch contains the most recent release of CARLA with the latest fixes and features.

Then follow the instruction at [How to build on Linux][buildlinuxlink] or [How to build on Windows][buildwindowslink].  
The Linux build needs for an UE patch to solve some visualization issues regarding Vulkan. Those already working with a Linux build should install the patch and make the UE build again using the following commands.  
```sh
# Download and install the UE patch  
cd ~/UnrealEngine_4.24
wget https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/UE_Patch/430667-13636743-patch.txt ~/430667-13636743-patch.txt
patch --strip=4 < ~/430667-13636743-patch.txt
# Build UE
./Setup.sh && ./GenerateProjectFiles.sh && make
```

Unfortunately we don't have official instructions to build on Mac yet, please check the progress at [issue #150][issue150].

[buildlinuxlink]: https://carla.readthedocs.io/en/latest/build_linux/
[buildwindowslink]: https://carla.readthedocs.io/en/latest/build_windows/
[issue150]: https://github.com/carla-simulator/carla/issues/150

Contributing
------------

Please take a look at our [Contribution guidelines][contriblink].

[contriblink]: https://carla.readthedocs.io/en/latest/cont_contribution_guidelines/

F.A.Q.
------

If you run into problems, check our
[FAQ](https://carla.readthedocs.io/en/latest/build_faq/).

CARLA Talks
------
The team creates some additional content for users, besides the docs. This is a great way to cover different subjects such as detailed explanations for a specific module, latest improvements in a feature, future work and much more.  

__CARLA Talks 2020 (May):__  

*   __General__  
	*   Art improvements: environment and rendering — [video](https://youtu.be/ZZaHevsz8W8) | [slides](https://drive.google.com/file/d/1l9Ztaq0Q8fNN5YPU4-5vL13eZUwsQl5P/view?usp=sharing)  
	*   Core implementations: synchrony, snapshots and landmarks — [video](https://youtu.be/nyyTLmphqY4) | [slides](https://drive.google.com/file/d/1yaOwf1419qWZqE1gTSrrknsWOhawEWh_/view?usp=sharing)
	*   Data ingestion — [video](https://youtu.be/mHiUUZ4xC9o) | [slides](https://drive.google.com/file/d/10uNBAMreKajYimIhwCqSYXjhfVs2bX31/view?usp=sharing)  
	*   Pedestrians and their implementation — [video](https://youtu.be/Uoz2ihDwaWA) | [slides](https://drive.google.com/file/d/1Tsosin7BLP1k558shtbzUdo2ZXVKy5CB/view?usp=sharing)  
	*   Sensors in CARLA — [video](https://youtu.be/T8qCSet8WK0) | [slides](https://drive.google.com/file/d/1UO8ZAIOp-1xaBzcFMfn_IoipycVkUo4q/view?usp=sharing)  
*   __Modules__  
	*   Improvements in the Traffic Manager — [video](https://youtu.be/n9cufaJ17eA) | [slides](https://drive.google.com/file/d/1R9uNZ6pYHSZoEBxs2vYK7swiriKbbuxo/view?usp=sharing)  
	*   Integration of autoware and ROS — [video](https://youtu.be/ChIgcC2scwU) | [slides](https://drive.google.com/file/d/1uO6nBaFirrllb08OeqGAMVLApQ6EbgAt/view?usp=sharing)  
	*   Introducing ScenarioRunner — [video](https://youtu.be/dcnnNJowqzM) | [slides](https://drive.google.com/file/d/1zgoH_kLOfIw117FJGm2IVZZAIRw9U2Q0/view?usp=sharing)  
	*   OpenSCENARIO support — [slides](https://drive.google.com/file/d/1g6ATxZRTWEdstiZwfBN1_T_x_WwZs0zE/view?usp=sharing)  
*   __Features__  
	*   Co-Simulations with SUMO and PTV-Vissim — [video](https://youtu.be/PuFSbj1PU94) | [slides](https://drive.google.com/file/d/10DgMNUBqKqWBrdiwBiAIT4DdR9ObCquI/view?usp=sharing)  
	*   Integration of RSS-lib — [slides](https://drive.google.com/file/d/1whREmrCv67fOMipgCk6kkiW4VPODig0A/view?usp=sharing)  
	*   The External Sensor Interface (ESI) — [video](https://youtu.be/5hXHPV9FIeY) | [slides](https://drive.google.com/file/d/1VWFaEoS12siW6NtQDUkm44BVO7tveRbJ/view?usp=sharing)  
	*   The OpenDRIVE Standalone Mode — [video](https://youtu.be/U25GhofVV1Q) | [slides](https://drive.google.com/file/d/1D5VsgfX7dmgPWn7UtDDid3-OdS1HI4pY/view?usp=sharing)  

Licenses
-------

#### CARLA licenses

CARLA specific code is distributed under MIT License.

CARLA specific assets are distributed under CC-BY License.

#### CARLA Dependency and Integration licenses

The ad-rss-lib library compiled and linked by the [RSS Integration build variant](Docs/adv_rss.md) introduces [LGPL-2.1-only License](https://opensource.org/licenses/LGPL-2.1).

Unreal Engine 4 follows its [own license terms](https://www.unrealengine.com/en-US/faq).

CARLA uses three dependencies as part of the SUMO integration:
- [PROJ](https://proj.org/), a generic coordinate transformation software which uses the [X/MIT open source license](https://proj.org/about.html#license).
- [SQLite](https://www.sqlite.org), part of the PROJ dependencies, which is [in the public domain](https://www.sqlite.org/purchase/license).
- [Xerces-C](https://xerces.apache.org/xerces-c/), a validating XML parser, which is made available under the [Apache Software License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.html).

CARLA uses one dependency as part of the Chrono integration:
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page), a C++ template library for linear algebra which uses the [MPL2 license](https://www.mozilla.org/en-US/MPL/2.0/).

CARLA uses the Autodesk FBX SDK for converting FBX to OBJ in the import process of maps. This step is optional, and the SDK is located [here](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0)

This software contains Autodesk® FBX® code developed by Autodesk, Inc. Copyright 2020 Autodesk, Inc. All rights, reserved. Such code is provided "as is" and Autodesk, Inc. disclaims any and all warranties, whether express or implied, including without limitation the implied warranties of merchantability, fitness for a particular purpose or non-infringement of third party rights. In no event shall Autodesk, Inc. be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of such code."
