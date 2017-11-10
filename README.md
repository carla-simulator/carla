CARLA Simulator
===============

We introduce **CARLA**, an open-source simulator for autonomous driving
research. CARLA has been developed from the ground up to support development,
training, and validation of autonomous urban driving systems. In addition to
open-source code and protocols, CARLA provides open digital assets (urban
layouts, buildings, vehicles) that were created for this purpose and can be used
freely. The simulation platform supports flexible specification of sensor suites
and environmental conditions.

[![CARLA Video](Docs/img/video_thumbnail.png)](https://youtu.be/dj7AmsLq1Gk)

[Get the latest release here.](https://github.com/carla-simulator/carla/releases/latest)

For instructions on how to use and compile CARLA, check out
[CARLA Documentation](Docs/index.md).

Paper
-----

If you use CARLA, please cite our CoRL’17 paper.

_CARLA: An Open Urban Driving Simulator_<br>Alexey Dosovitskiy, German Ros,
Felipe Codevilla, Antonio Lopez, Vladlen Koltun; PMLR 78:1-16
[[PDF](http://proceedings.mlr.press/v78/dosovitskiy17a/dosovitskiy17a.pdf)]


```
@inproceedings{Dosovitskiy17,
  title = {{CARLA}: {An} Open Urban Driving Simulator},
  author = {Alexey Dosovitskiy and German Ros and Felipe Codevilla and Antonio Lopez and Vladlen Koltun},
  booktitle = {Proceedings of the 1st Annual Conference on Robot Learning},
  pages = {1--16},
  year = {2017}
}
```

License
-------

CARLA specific code is distributed under MIT License.

CARLA specific assets are distributed under CC-BY License.

Note that UE4 itself and the UE4 free automotive materials follow their own
license terms.

CARLA uses free automotive materials from Epic Games. For compiling CARLA, these
materials must be dowloanded from the UE4 marketplace and manually linked in
CARLA following the instructions provided in the documentation.

CARLA uses pedestrians created with Adobe Fuse, which is a free tool for that
purpose. Currently, we cannot redistribute these pedestrians models. They can
only be used in the provided executable.
