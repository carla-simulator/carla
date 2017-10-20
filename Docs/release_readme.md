CARLA UE4
=========

To run the game with the second city

    $ ./CarlaUE4.sh /Game/Maps/CARLA_ORIGIN_1

To run the game at fixed time-step, e.g. 30 FPS

    $ ./CarlaUE4.sh -benchmark -fps=30

To run the game windowed at a given resolution

    $ ./CarlaUE4.sh -windowed -ResX=800 -ResY=600

Other CARLA related command-line options

  * `-carla-settings=<ini-file-path>` Load settings from the given INI file. See Example.CarlaSettings.ini.
  * `-carla-world-port=<port-number>` Listen for client connections at <port-number>, write and read ports are set to <port-number>+1 and <port-number>+2 respectively. Activates networking.
  * `-carla-no-networking` Disable networking. Overrides any other settings file.
  * `-carla-no-hud` Do not display the HUD by default.

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

In-game controls
----------------

    W            : throttle
    S            : brake
    AD           : steer
    Q            : toggle reverse
    Space        : hand-brake

    P            : toggle autopilot

    Arrow keys   : move camera
    PgUp PgDn    : zoom in and out
    mouse wheel  : zoom in and out
    Tab          : toggle on-board camera

    R            : restart level
    G            : toggle HUD
    C            : change weather/lighting

    Enter        : jump
    F            : use the force
