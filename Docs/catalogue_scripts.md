# Scripts catalogue

This document outlines the available example Python scripts and utilities for the CARLA Python API. You can use these scripts for learning CARLA's Python API, performing utility functions or testing and to form the basis of your own scripts. 

* Manual control


## Manual control

This script allows a user to manually control a vehicle through a CARLA map using the keyboard, with a visualization of sensor output in a Pygame window. It is one of the first scripts new users should experiment with to explore CARLA maps and to understand the behavior of vehicles and sensors. It is also very useful for testing and debugging when changes have been made to core CARLA functionality relating to rendering, physics, traffic or sensing that might affect appearance, driving behavior or sensor output.

The script spawns a vehicle (the ego vehicle) at a randomly selected spawn point in the map and allows manual control of the vehicle through the arrow keys or WASD keys. Other keyboard shortcuts are available to change vehicles, change gears, change sensor type or start recording and many other functions. 

Manual control can be combined with the [generate traffic script](#generate-traffic) to drive a vehicle through traffic. Launch the generate traffic script prior to launching manual control. Do not attempt to run both scripts in synchronous mode, there should only be 1 client running in synchronous mode. By default manual control runs in asynchronous mode and generate traffic runs in synchronous mode, hence they will work together smoothly in their default synchrony configuration. 

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

The manual control script has multiple command line arguments for configuration:

| Argument | Short form | Default | Description |
|----------|------------|---------|-------------|
| --verbose | -v | - | Print debug info |
| --host | -h | 127.0.0.1 | Host IP address |
| --port | -p | 2000 | TCP port for CARLA client |
| --autopilot | -a | - | Enable autopilot for ego vehicle |
| --res | - | 1280x720 | Pixel resolution of all camera sensors |
| --filter | - | vehicle.* | Filter for vehicle type |
| --generation | - | 2 | Specify vehicle model generation |
| --rolename | - | hero | Rolename assigned to the ego vehicle |
| --gamma | - | 2.2 | Gamma correction of the RGB camera |
| --sync | - | Deactivated | Activate the script in synchronous mode |

---

## Automatic control



---

## Generate traffic

This script generates traffic of varying density distributed across a chosen CARLA map. Pedestrians are also generated. The density of traffic and pedestrians can be chosen via command line arguments. The generate traffic script can be run in combination with manual control to drive a vehicle around a map populated with traffic, 

### Command line arguments 

| Argument | Short form | Default | Description |
|----------|------------|---------|-------------|
| --number-of-vehicles | -n | 30 | Number of vehicles to spawn |
| --number-of-walkers | -w | 10 | Number of pedestrians to spawn |
| --host | -h | 127.0.0.1 | Host IP address |
| --port | -p | 2000 | TCP port for CARLA client |
| --safe |  |  | Don't spawn vehicles prone to accidents |
| --filterv |  | vehicle.* | Filter vehicle models with string |
| --filterw |  | walker.pedestrian.* | Filter pedestrian models with string |
| --generationv |  | All | Specify vehicle generation, "1", "2" or "All" |
| --generationw |  | All | Specify pedestrian generation, "1", "2" or "All" |
| --tm-port |  | 8000 | Specify TCP port for the TM |
| --asynch |  | Deactivated | Run the script in synchronous mode |
| --hybrid |  | Deactivated | Activate hybrid mode for the TM |
| --seed | -s |  | Integer seed for random generation (activates the deterministic mode for the TM) |
| --seedw |  |  | Integer seed for the pedestrian module |
| --car-lights-on | | False | Enable automatic light managment by the TM |
| --hero | | False | Nominate a hero vehicle |
| --respawn | | False | Automatically respawn dormant vehicles in large maps |
| --no-rendering | | Deactivated | Activate no-rendering mode for the CARLA server |

---

