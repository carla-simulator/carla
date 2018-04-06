<h1>CARLA Settings</h1>

> _This document is a work in progress and might be incomplete._

CarlaSettings.ini
-----------------

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

Weather presets
---------------

The weather and lighting conditions can be chosen from a set of predefined
settings. To select one, set the `WeatherId` key in CarlaSettings.ini. The
following presets are available

  * 0 - Default
  * 1 - ClearNoon
  * 2 - CloudyNoon
  * 3 - WetNoon
  * 4 - WetCloudyNoon
  * 5 - MidRainyNoon
  * 6 - HardRainNoon
  * 7 - SoftRainNoon
  * 8 - ClearSunset
  * 9 - CloudySunset
  * 10 - WetSunset
  * 11 - WetCloudySunset
  * 12 - MidRainSunset
  * 13 - HardRainSunset
  * 14 - SoftRainSunset

E.g., to choose the weather to be hard-rain at noon, add to CarlaSettings.ini

```
[CARLA/LevelSettings]
WeatherId=6
```

Simulator command-line options
------------------------------

  * `-carla-server` Launches CARLA as server, the execution hangs until a client connects.
  * `-carla-settings="Path/To/CarlaSettings.ini"` Load settings from the given INI file. See Example.CarlaSettings.ini.
  * `-carla-world-port=N` Listen for client connections at port N, agent ports are set to N+1 and N+2 respectively. Activates server.
  * `-carla-no-hud` Do not display the HUD by default.
  * `-carla-no-networking` Disable networking. Overrides `-carla-server` if present.
