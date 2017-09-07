CARLA Documentation
===================

This is the documentation of both CARLAUE4 and the CARLA plugin.

The _CARLA plugin_ is a plugin for Unreal Engine 4.17 that contains most of the
code and functionality of CARLA. It is meant to be independent of the project
assets, although some parts are designed with the current assets in mind. It can
be added to any project and provides the base classes to be used in your
project.

_CARLAUE4_ is an Unreal Engine project containing all the assets and mounted
scenes. It includes the CARLA plugin.

To download the whole project use [git lfs](https://git-lfs.github.com/)
(replace `<user>` with your bitbucket username)

    git lfs clone --recursive https://<user>@bitbucket.org/carla-cvc/carla-ue4.git CarlaUE4

Once CARLA is built, be sure to configure
[CARLA Settings](carla_settings.md) before hitting play.

Index
-----

#### Setup

  * [How to build on Linux](how_to_build_on_linux.md)
  * [How to build on Windows](how_to_build_on_windows.md)
  * [How to run CarlaUE4 binary](release_readme.md)
  * [CARLA Settings](carla_settings.md)

#### Development

  * [How to make a release](how_to_make_a_release.md)
  * [How to add assets](how_to_add_assets.md)
  * [CarlaServer documentation](carla_server.md)
