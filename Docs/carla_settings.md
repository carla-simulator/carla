CARLA Settings
==============

CARLA reads its settings from a "CarlaSettings.ini" file. This file controls
most aspects of the simulation, and it is loaded every time a new episode is
started (every time the level is loaded).

Settings are loaded following the next hierarchy, with values later in the
hierarchy overriding earlier values.

  1. `{CarlaFolder}/Unreal/CarlaUE4/Config/CarlaSettings.ini`.
  2. File provided by command-line argument `-carla-settings="Path/To/CarlaSettings.ini"`.
  3. Other command-line arguments as `-carla-server` or `-world-port`.
  4. Settings file sent by the client on every new episode.

Take a look at the [CARLA Settings example][settingslink].

[settingslink]: https://github.com/carla-simulator/carla/blob/master/Docs/Example.CarlaSettings.ini
