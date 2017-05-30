CARLA UE4
=========

To run the game at fixed time-step, e.g. 30 FPS

    $ CarlaUE4.sh -benchmark -fps=30

Other CARLA related command-line options

  * `-carla-settings=<ini-file-path>` Load settings from the given INI file. See Example.CarlaSettings.ini.
  * `-world-port=<port-number>` Listen for client connections at <port-number>, write and read ports are set to <port-number>+1 and <port-number>+2 respectively.
  * `-carla-no-networking` Disable networking. Overrides any settings file.

To activate semantic segmentation
---------------------------------

In the config file `CarlaUE4/Saved/Config/LinuxNoEditor/Engine.ini`, add the
following

```
[/Script/Engine.RendererSettings]
r.CustomDepth=3
```

Weather presets
---------------

To change the weather and sun light, set `WeatherId` in CarlaSettings.ini
from the following

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
