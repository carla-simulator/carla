# Scripts catalogue

CARLA ships a set of ready-to-run Python scripts. Use them to see a feature working, to check an install, or as the starting point for your own code. They live in two directories of the repository or the package:

- `PythonAPI/examples` — demonstrations of the API, each one runnable on its own.
- `PythonAPI/util` — small utilities for driving a running server from a shell.

Every script connects to `localhost:2000` unless `--host` and `--port` say otherwise, and takes `-h` for its full argument list. Start a server first.

```sh
cd PythonAPI/examples
python3 manual_control.py
```

!!! Note
    Running two scripts against the same server is normal — one driving traffic, another watching it. Only one of them may put the simulation in synchronous mode, though: if two clients both tick, they fight over the clock and the simulation stutters. Let whichever one owns the traffic run synchronously and leave the rest asynchronous.

---

## Drive and look around

| Script | What it does |
| ------ | ------------ |
| `manual_control.py` | Drive a vehicle with the keyboard in a Pygame window, cycling sensors, weather and vehicles live. The first script to run on a new install. `--autopilot` hands it to the Traffic Manager, `--post-process-profile` picks a [camera preset](camera_postprocess_presets.md). |
| `manual_control_fisheye.py` | The same, with the [fisheye cameras](ref_sensors.md#fisheye-cameras) in the sensor cycle. |
| `manual_control_rtlens.py` | The same, starting on the [ray-traced lens camera](ref_sensors.md#ray-traced-lens-camera) with its lens models. |
| `manual_control_5cam.py` | A five-camera AV rig on the vehicle, with DLSS Super Resolution toggled live on `D`. |
| `no_rendering_mode.py` | A 2D top-down view of the map drawn from the API, with the server rendering nothing. |
| `vehicle_gallery.py` | Spins every vehicle in the blueprint library in front of the camera, one after another. |
| `draw_skeleton.py` | Draws a pedestrian's bone skeleton over the simulation. |
| `vehicle_doors_demo.py` | Opens and closes each door of a vehicle that has them. |
| `vehicle_lights_demo.py` | Cycles a vehicle's light states. |
| `lane_explorer.py` | Walks the OpenDRIVE lanes of the current map and draws them as it goes. |

```sh
python3 manual_control.py --res 1280x720 --autopilot
```

---

## Traffic

| Script | What it does |
| ------ | ------------ |
| `generate_traffic.py` | Fills the map with vehicles and pedestrians on the Traffic Manager. `-n` vehicles, `-w` walkers, `--hybrid` for hybrid physics, `--asynch` to leave the simulation asynchronous. |
| `automatic_control.py` | Drives one vehicle to a destination with a [navigation agent](adv_agents.md), replanning when it arrives. |
| `invertedai_traffic.py` | Replaces the Traffic Manager with InvertedAI's behaviour models. See [InvertedAI](inverted_ai.md). |

```sh
python3 generate_traffic.py -n 80 -w 40
```

---

## Sensors

| Script | What it does |
| ------ | ------------ |
| `visualize_multiple_sensors.py` | Several sensors rendered side by side in one window. |
| `sensor_synchronization.py` | How to collect from several sensors on the same frame in synchronous mode. |
| `open3d_lidar.py` | Live LiDAR point cloud in an Open3D window. |
| `hss_lidar_top_down.py` | Top-down view of a [Hybrid Solid-State LiDAR](ref_sensors.md#hybrid-solid-state-lidar-sensor) scan. |
| `visualize_radar.py` | Radar returns drawn in the world as coloured points. |
| `dvs_test.py` | A quick check that the DVS camera produces events. |
| `interpolate_camera.py` | Moves a camera smoothly through keypoints read from an XML file, for repeatable fly-throughs. |
| `rain_lens_demo.py` | Rain on the camera, including the droplets on the lens. |

---

## Recording and replay

| Script | What it does |
| ------ | ------------ |
| `recorder_replay.py` | Replays a `.log`, following an actor, overriding the map, restoring the recorded weather (`--replay-weather`) or watching from above (`--top-view`). |
| `util/start_replaying.py` | A minimal replay, for a log you only want to watch. |
| `util/show_recorder_file_info.py` | Prints what is inside a recording: actors, frames, collisions and blocked actors. |
| `util/recorder_comparer.py` | Compares two recordings frame by frame, for determinism checks. |

See [Recorder](adv_recorder.md) for the API behind these.

---

## Weather and light

| Script | What it does |
| ------ | ------------ |
| `dynamic_day_night_weather_cycle.py` | Runs a day-night cycle and a weather cycle against a live server. |
| `util/set_weather.py` | Sets the weather from the command line. |
| `util/config.py` | The general server utility: change map (`-m`), load an OpenDRIVE file (`-x`), set the time step (`--delta-seconds`), toggle rendering, list blueprints (`--list-blueprints`), inspect the simulation (`-i`). |

```sh
python3 ../util/config.py -m Town05_Opt
```

---

## Benchmarks and test suites

| Script | What it does |
| ------ | ------------ |
| `rig_perf_demo.py` | Measures the delivered frame rate of an N-camera rig, then replays the run into a labelled grid video. |
| `lw_suite.py`, `lightweather_test.py` | Functional tests of the light and weather API. |
| `dvs_suite.py` | DVS camera tests: synchronous mode, attributes, resolution and lifecycle. |
| `autoware_demo.py` | The TIER IV sensor-kit rig, for the [Autoware](ros2_native.md) integration. |

---

## Map and world utilities

Everything in `PythonAPI/util`, run from that directory:

| Script | What it does |
| ------ | ------------ |
| `config.py` | Map, rendering, time step and simulation settings — see above. |
| `extract_spawn_points.py` | Writes the map's spawn points to a CSV. |
| `spectator_transform.py` | Prints or sets the spectator's transform, for repeatable viewpoints. |
| `show_topology.py`, `show_junctions.py`, `show_crosswalks.py` | Draw the road graph, the junctions and the crosswalks over the map. |
| `show_traffic_lights.py`, `manage_traffic_light.py` | Inspect and control traffic lights. |
| `manage_environment_objects.py` | List and toggle the map's environment objects. |
| `change_map_layer.py` | Load and unload the layers of a layered (`_Opt`) map. |
| `apply_texture.py` | Replaces the texture of an object in the map from an image file. |
