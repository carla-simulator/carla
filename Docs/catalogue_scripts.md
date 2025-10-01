# Scripts catalogue

This document outlines the available example Python scripts and utilities for the CARLA Python API. You can use these scripts for learning CARLA's Python API, performing utility functions or testing and to form the basis of your own scripts. 

* Manual control


## Manual control

This script allows a user to manually control a vehicle through a CARLA map using the keyboard. It is one of the first scripts new users should experiment with to explore CARLA maps and to understand the behavior of vehicles and sensors. It is also very useful for testing and debugging when changes have been made to core CARLA functionality relating to rendering, physics, traffic or sensing that might affect appearance, driving behavior or sensor output.

The script spawns a vehicle (the ego vehicle) at a randomly selected spawn point in the map and allows manual control of the vehicle through the arrow keys or WASD keys. Other keyboard shortcuts are available to change vehicles, change gears, change sensor type or start recording and many other functions. 

### Control keys

| Key | Control |
|-----|---------|
| W, &uarr;    | throttle |
| S, &darr;    | brake |
| A/D, &larr;, &rarr; | steer left/right |
| Q            | toggle reverse |
| Space        | hand-brake |
| P            | toggle autopilot |
| M            | toggle manual transmission |
| ,/.          | gear up/down |
| CTRL + W     | toggle constant velocity mode at 60 km/h |
| L            | toggle next light type |
| SHIFT + L    | toggle high beam |
| Z/X          | toggle right/left blinker |
| I            | toggle interior light |
| TAB          | change sensor position |
| ` or N       | next sensor |
| [1-9]        | change to sensor [1-9] |
| G            | toggle radar visualization |
| C            | change weather (Shift+C reverse) |
| Backspace    | change vehicle |
| O            | open/close all doors of vehicle |
| T            | toggle vehicle's telemetry |
| V            | Select next map layer (Shift+V reverse) |
| B            | Load current selected map layer (Shift+B to unload) |
| R            | toggle recording images to disk |
| CTRL + R     | toggle recording of simulation (replacing any previous) |
| CTRL + P     | start replaying last recorded simulation |
| CTRL + +     | increments the start time of the replay by 1 second (+SHIFT = 10 seconds) |
| CTRL + -     | decrements the start time of the replay by 1 second (+SHIFT = 10 seconds) |
| F1           | toggle HUD |
| H/?          | toggle help |
| ESC          | quit |

### Command line arguments

The manual control script has multiple command line arguments for configuration, 

| Argument | Short form | Default | Description |
|----------|------------|---------|-------------|
| --verbose | -v | - | Print debug info |
| --host | - | 127.0.0.1 | Host IP address |
| --port | -p | 2000 | TCP port for CARLA client |
| --autopilot | -a | - | Enable autopilot for ego vehicle |
| --res | - | 1280x720 | Pixel resolution of all camera sensors |
| --filter | - | vehicle.* | Filter for vehicle type |
| --generation | - | 2 | Specify vehicle model generation |
| --rolename | - | hero | Rolename assigned to the ego vehicle |
| --gamma | - | 2.2 | Gamma correction of the RGB camera |
| --sync | - | Deactivated | Activate the script in synchronous mode |

