CARLA Simulator
===============

[![Build Status](https://travis-ci.org/carla-simulator/carla.svg?branch=master)](https://travis-ci.org/carla-simulator/carla)
[![Documentation](https://readthedocs.org/projects/docs/badge/?version=latest)](https://carla.readthedocs.io)
[![Waffle.io](https://badge.waffle.io/carla-simulator/carla.svg?columns=Next,In%20Progress,Review)](https://waffle.io/carla-simulator/carla)

CARLA is an open-source simulator for autonomous driving research. CARLA has
been developed from the ground up to support development, training, and
validation of autonomous urban driving systems. In addition to open-source code
and protocols, CARLA provides open digital assets (urban layouts, buildings,
vehicles) that were created for this purpose and can be used freely. The
simulation platform supports flexible specification of sensor suites and
environmental conditions.

[![CARLA Video](Docs/img/video_thumbnail.png)](https://youtu.be/Hp8Dz-Zek2E)

[Get the latest release here.](https://github.com/carla-simulator/carla/releases/latest)

For instructions on how to use and compile CARLA, check out
[CARLA Documentation](http://carla.readthedocs.io).

If you want to benchmark your model in the same conditions as in our CoRL’17
paper, check out
[Benchmarking](http://carla.readthedocs.io/en/latest/benchmark/).

News
----

- 05.02.2018 CARLA 0.7.1 available for Windows too, [release](https://github.com/carla-simulator/carla/releases/tag/0.7.1).
- 25.01.2018 CARLA 0.7.1 released: [change log](https://github.com/carla-simulator/carla/blob/master/CHANGELOG.md#carla-071), [release](https://github.com/carla-simulator/carla/releases/tag/0.7.1).
- 22.01.2018 Job opening: [C++ (UE4) Programmer](https://drive.google.com/open?id=1Hx0eUgpXl95d4IL9meEGhJECgSRos1T1).
- 28.11.2017 CARLA 0.7.0 released: [change log](https://github.com/carla-simulator/carla/blob/master/CHANGELOG.md#carla-070), [release](https://github.com/carla-simulator/carla/releases/tag/0.7.0).
- 15.11.2017 CARLA 0.6.0 released: [change log](https://github.com/carla-simulator/carla/blob/master/CHANGELOG.md#carla-060), [release](https://github.com/carla-simulator/carla/releases/tag/0.6.0).

Roadmap
-------

We are continuously working on improving CARLA, and we appreciate contributions
from the community. Our most immediate goals are:

- Releasing the methods evaluated in the CARLA paper
- Adding a LiDAR sensor
- Allowing for flexible and user-friendly import and editing of maps
- Allowing the users to control non-player characters (and therefore set up user-specified scenarios)

We will post a detailed roadmap and contribution guidelines soon - stay tuned!

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

Building CARLA
--------------

Use `git clone` or download the project from this page. Note that the master
branch contains the latest fixes and features, for the latest stable code may be
best to switch to the `stable` branch.

Then follow the instruction at [How to build on Linux][buildlink].

Unfortunately we don't have yet official instructions to build on other
platforms, please check the progress for [Windows][issue21] and [Mac][issue150].

[buildlink]: http://carla.readthedocs.io/en/latest/how_to_build_on_linux
[issue21]: https://github.com/carla-simulator/carla/issues/21
[issue150]: https://github.com/carla-simulator/carla/issues/150

Contributing
------------

Please take a look at our [Contribution guidelines][contriblink].

[contriblink]: http://carla.readthedocs.io/en/latest/CONTRIBUTING

F.A.Q.
------

If you run into problems, check our
[FAQ](http://carla.readthedocs.io/en/latest/faq/).

Jobs
----

We are currently looking for a new programmer to join our team

  * [C++ (UE4) Programmer](https://drive.google.com/open?id=1Hx0eUgpXl95d4IL9meEGhJECgSRos1T1)

License
-------

CARLA specific code is distributed under MIT License.

CARLA specific assets are distributed under CC-BY License.

Note that UE4 itself and the UE4 free automotive materials follow their own
license terms.

CARLA uses free automotive materials from Epic Games. For compiling CARLA, these
materials must be downloaded from the UE4 marketplace and manually linked in
CARLA following the instructions provided in the documentation.

CARLA uses pedestrians created with Adobe Fuse, which is a free tool for that
purpose. Currently, we cannot redistribute these pedestrians models. They can
only be used in the provided executable.
