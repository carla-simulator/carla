# Autoware

CARLA can be driven by [Autoware](https://autoware.org/) with no bridge process: the server publishes sensor data and vehicle status straight onto ROS 2 and applies Autoware's control commands to the ego vehicle. The integration ships with two driving modes:

- **`classical`**: the full Autoware stack, with NDT localization against a point-cloud map, lidar perception (`perception_mode:=lidar`), planning and the trajectory-follower controller. Traffic-light states come from the simulator, see [Known limitations](#known-limitations).
- **`e2e`**: the camera-only VAD end-to-end model, fed by six surround cameras, with ground-truth localization and the same trajectory follower.

Everything lives in `PythonAPI/examples/av_stacks/autoware/`: an installer for Autoware, a launcher that starts CARLA and Autoware together, and the tools that build the maps Autoware needs. Its `README.md` covers each step in more detail than this page.

- [__Before you start__](#before-you-start)
- [__Run Autoware__](#run-autoware)
- [__Drive the ego from your own script__](#drive-the-ego-from-your-own-script)
- [__Topics__](#topics)
- [__Prepare a map__](#prepare-a-map)
- [__Known limitations__](#known-limitations)

## Before you start

| Requirement | Detail |
|---|---|
| CARLA build | Built with `-DENABLE_ROS2=ON` (off by default, see [Linux build](build_linux_ue5.md)). The launcher prefers a packaged server, looking for `CarlaUnreal.sh` in `$CARLA_ROOT` or under `Build/*/Package/`, and falls back to the editor in `-game` mode when `UE_ROOT` is set. `--server editor` forces that fallback, which always runs the plugin binaries currently built in the tree. |
| OS and ROS 2 | Ubuntu 22.04 with ROS 2 Humble, or Ubuntu 24.04 with ROS 2 Jazzy. |
| GPU | NVIDIA GPU recommended; required for `e2e`, with CUDA, cuDNN and TensorRT already installed. |
| Disk | About 100 GB for an Autoware source build. |
| UDP buffers | 64 MB socket-buffer limits, set before starting any ROS 2 process. |

Raise and persist the UDP buffer limits:

```sh
sudo sh -c 'sysctl -w net.core.rmem_max=67108864 net.core.wmem_max=67108864 && printf "net.core.rmem_max=67108864\nnet.core.wmem_max=67108864\n" > /etc/sysctl.d/99-carla-dds.conf'
```

!!! Warning
    With the default 4 MB limit the lidar stream overruns the receive sockets and Autoware emergency-stops the vehicle. The limit is fixed when a socket is created, so restart every ROS 2 process, including the simulator, after changing it.

## Run Autoware

Run these from `PythonAPI/examples/av_stacks/autoware/`. First install Autoware outside the CARLA tree:

```sh
./install/install_autoware.sh --check       # report what is installed, change nothing
./install/install_autoware.sh               # source build, classical mode
./install/install_autoware.sh --with-vad    # source build plus the VAD model, e2e mode
```

Then start a session. The launcher starts the CARLA server with ROS 2, loads the town, spawns the ego and its sensors with `autoware_demo.py`, launches Autoware, initializes localization and, with `--goal`, engages autonomous driving:

```sh
# Classical stack, drive to a goal given in CARLA coordinates (x, y, yaw in degrees)
./run/run_carla_autoware.sh --mode classical --spawn-index 24 --goal "-1.16,28.37,0.16"

# End-to-end VAD, drives hands-free around the Town10 outer ring
./run/run_carla_autoware.sh --mode e2e
```

Stop a session from another terminal with `./run/stop_all.sh`. The most used options of `run_carla_autoware.sh` are listed below; `--help` prints all of them.

| Option | Default | Description |
|---|---|---|
| `--mode classical\|e2e` | required | Autoware stack to launch. |
| `--town NAME` | `Town10HD_Opt` | Town to load. |
| `--stack auto\|source\|docker` | `auto` | Classical only: run Autoware from the source workspace or the official docker image. `e2e` always uses the source workspace. |
| `--goal "X,Y,YAW"` | none | Classical only: drive to this goal after startup. Must be centred on a driving lane and heading-aligned. |
| `--spawn-index N` | demo default; `52` for `e2e` | Spawn point of the ego. |
| `--rmw fastdds\|cyclonedds\|zenoh` | `fastdds` | Middleware of the simulator. Autoware always runs on CycloneDDS, or Zenoh with `zenoh`. |
| `--domain-id N` | `42` | ROS domain id, passed to the server as `-ros-domain-id`. |
| `--carla-rpc-port N` | `2000` | CARLA RPC port. |
| `--carla-root DIR` | `$CARLA_ROOT` | Directory containing `CarlaUnreal.sh`. |
| `--map-path DIR` | `map_tools/maps/<town>` | Map artifacts to load, see [Prepare a map](#prepare-a-map). |
| `--with-rviz` | off | Also start RViz. Without `--goal`, set the goal there and engage from its OperationMode panel. |
| `--no-auto` | off | Skip localization init, goal and engage after launch. |
| `--no-gates` | off | Classical only: skip the checks that refuse to engage when the localized pose disagrees with ground truth. |
| `--dry-run` | off | Print every command without running anything. |

## Drive the ego from your own script

`PythonAPI/examples/autoware_demo.py` spawns the ego and the sensor kit Autoware expects, switches the world to synchronous mode and ticks it. The launcher runs it for you; run it directly against a server started with `--ros2` to use your own Autoware launch.

To make any vehicle drivable by Autoware, give it the `hero` or `ego` role name, set `ros2_autoware_control` before spawning, and attach a `sensor.other.vehicle_status` sensor:

```py
bp_lib = world.get_blueprint_library()

ego_bp = bp_lib.find('vehicle.lincoln.mkz')
ego_bp.set_attribute('role_name', 'ego')
ego_bp.set_attribute('ros2_autoware_control', 'true')
ego = world.spawn_actor(ego_bp, spawn_point)

# Publishes /vehicle/status/*; does not need enable_for_ros()
world.spawn_actor(bp_lib.find('sensor.other.vehicle_status'), carla.Transform(), attach_to=ego)
```

With `ros2_autoware_control` the vehicle listens to Autoware's command topics instead of `/carla/<name>/vehicle_control_cmd` or the Ackermann topic. For localization, use the [Autoware GNSS sensor](ref_sensors.md#autoware-gnss-sensor), which publishes the pose in the map frame. Other sensors take Autoware's topic names through the `ros_topic_name` blueprint attribute and are enabled with [`enable_for_ros()`](python_api.md#carla.Sensor.enable_for_ros).

| `autoware_demo.py` option | Default | Description |
|---|---|---|
| `--host H` | `127.0.0.1` | Server IP. |
| `-p`, `--port P` | `2000` | Server RPC port. |
| `--load_map [NAME]` | none | Load a map first; `Town10HD_Opt` when the flag is given without a name. |
| `--force_reload` | off | Reload the map even if it is already loaded. |
| `--spawn_index N` | random | Ego spawn point, from [`get_ego_spawn_points()`](python_api.md#carla.World.get_ego_spawn_points) or, if the map has none, the map spawn points. |
| `--hz_rate N` | `100` | Fixed simulation rate in Hz; `0` or `None` for a variable time step. |
| `--time_scale T` | `1.0` | Simulation speed relative to real time. |
| `--resync` | off | Jump to the current time when the loop lags behind real time. |
| `--run_async` | off | Run in asynchronous mode. |
| `--substepping` | off | Enable physics substepping. |
| `--mgrs_off` | off | Use `sensor.other.gnss` instead of `sensor.other.autoware_gnss`, with no MGRS offset. |
| `--follow` | off | Keep the spectator behind the ego. |
| `--list_maps` | off | Print the available maps and exit. |
| `--timeout S` | `60` | RPC timeout in seconds. Raise it if the server is still loading the map. |
| `-v`, `--verbose` | off | Print debug information. |

!!! Note
    `autoware_demo.py` is the only client allowed to tick the world. Do not run another synchronous client, such as a Traffic Manager script, against the same server.

## Topics

CARLA publishes these for the vehicle registered with `ros2_autoware_control`. Topic names are fixed, so one Autoware vehicle runs per ROS domain.

| Topic | Type | Source |
|---|---|---|
| `/vehicle/status/velocity_status` | `autoware_vehicle_msgs/VelocityReport` | `sensor.other.vehicle_status` |
| `/vehicle/status/steering_status` | `autoware_vehicle_msgs/SteeringReport` | `sensor.other.vehicle_status` |
| `/vehicle/status/control_mode` | `autoware_vehicle_msgs/ControlModeReport` | `sensor.other.vehicle_status` |
| `/vehicle/status/gear_status` | `autoware_vehicle_msgs/GearReport` | `sensor.other.vehicle_status` |
| `/vehicle/status/turn_indicators_status` | `autoware_vehicle_msgs/TurnIndicatorsReport` | `sensor.other.vehicle_status` |
| `/vehicle/status/hazard_lights_status` | `autoware_vehicle_msgs/HazardLightsReport` | `sensor.other.vehicle_status` |

CARLA subscribes to these:

| Topic | Type | Effect |
|---|---|---|
| `/control/command/control_cmd` | `autoware_control_msgs/Control` | Drives the vehicle: longitudinal acceleration and steering tire angle. |
| `/control/command/gear_cmd` | `autoware_vehicle_msgs/GearCommand` | Received, not applied. |
| `/control/command/turn_indicators_cmd` | `autoware_vehicle_msgs/TurnIndicatorsCommand` | Received, not applied. |
| `/control/command/hazard_lights_cmd` | `autoware_vehicle_msgs/HazardLightsCommand` | Received, not applied. |
| `/control/command/emergency_cmd` | `tier4_vehicle_msgs/VehicleEmergencyStamped` | Received, not applied. |
| `/vehicle/engage` | `autoware_vehicle_msgs/Engage` | Received, not applied. |

`autoware_demo.py` publishes the sensor kit on these topics:

| Topic | Type | Sensor |
|---|---|---|
| `/sensing/lidar/top/pointcloud_raw_ex` | `sensor_msgs/PointCloud2` (XYZIRCAEDT) | 16-channel `sensor.lidar.ray_cast`, 10 Hz |
| `/sensing/camera/front/image`, `/camera_info` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo` | `sensor.camera.rgb`, 1280×720, 10 Hz |
| `/sensing/camera/traffic_light/image`, `/camera_info` | `sensor_msgs/Image`, `sensor_msgs/CameraInfo` | `sensor.camera.rgb`, 1920×1080, 10 Hz |
| `/sensing/imu/tamagawa/imu_raw` | `sensor_msgs/Imu` | `sensor.other.imu`, 30 Hz |
| `/sensing/gnss/pose`, `/sensing/gnss/pose_with_covariance` | `geometry_msgs/Pose`, `geometry_msgs/PoseWithCovarianceStamped` | `sensor.other.autoware_gnss`, every tick |

In `e2e` mode, `run/spawn_vad_rig.py` adds six 800×450 cameras with `ros_topic_name` set to `/sensing/camera/CAM_*/image_raw`, so each one publishes on `/sensing/camera/CAM_*/image_raw/image` and `/image_raw/camera_info`.

## Prepare a map

Autoware does not read OpenDRIVE. Each town needs a directory with `pointcloud_map.pcd`, `lanelet2_map.osm` and `map_projector_info.yaml` (`projector_type: Local`). Town10HD ships ready in `map_tools/maps/Town10HD/`, which the launcher uses by default.

For another town, either download the prebuilt maps (Town01 to Town07 and Town10HD):

```sh
./map_tools/fetch_prebuilt_maps.sh Town03
```

or generate them from a running server, which works for any town. The converter needs the packages in `map_tools/requirements.txt` and the CARLA wheel in a virtual environment:

```sh
python3 -m venv ~/.venvs/carla-map-tools
~/.venvs/carla-map-tools/bin/pip install -r map_tools/requirements.txt <path-to>/PythonAPI/carla/dist/carla-*.whl
~/.venvs/carla-map-tools/bin/python map_tools/generate_map_artifacts.py --town Town12 --out ./map_tools/maps/Town12 --tick
```

`--tick` makes the tool the client that ticks a synchronous world; omit it only if another client already ticks. To convert an OpenDRIVE file offline, use `map_tools/generate_lanelet2_map.py --xodr <file>`. Point the launcher at the result with `--town` and, if it is not under `map_tools/maps/`, `--map-path`.

## Known limitations

- **Traffic lights come from the simulator, not from perception.** In `classical` mode the launcher starts two helpers that read CARLA's traffic-light states, match each light to its lanelet2 regulatory element and publish them on Autoware's external input, `/perception/traffic_light_recognition/external/traffic_signals`. The camera classifier is deliberately left unfed, because it reports `UNKNOWN` for CARLA's rendered lights and that would be merged with the authoritative state. `e2e` mode publishes no traffic-light state at all.
- **Only `control_cmd` moves the vehicle.** Gear, indicator, hazard, emergency and engage commands are received but not applied, and `control_mode` always reports `AUTONOMOUS`.
- **Steering is tuned for the Lincoln MKZ.** Steering compensation was measured on it and Autoware's `sample_vehicle` gains match it; other vehicles track their path more loosely until re-tuned.
- **Goals must be on a driving lane.** An off-lane goal is rejected by Autoware's mission planner after startup, and the vehicle never moves.
- **The simulation step drives sensor rates.** Changing `--hz_rate` changes the effective sensor frequencies Autoware sees. The launcher runs `autoware_demo.py --hz_rate 20 --resync`, which is the validated configuration, rather than the script's own default of 100.
- **VAD is not validated on Unreal Engine 5.8 visuals.** Its weights were trained on CARLA 0.9.15 imagery; treat `e2e` as a pipeline reference. Its first run builds TensorRT engines, which takes a long time once and is then cached.
