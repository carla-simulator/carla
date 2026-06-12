# ROS2 Native Example

This example demonstrates how to utilize the ROS 2 native interface in CARLA. It includes a one-command demo, the map and lidar demo, that spawns a sensorized hero vehicle on autopilot and renders it in RViz together with the lane network of the map and the live lidar point cloud, all in a single world-fixed frame.

## What the demo shows

The native ROS 2 interface publishes a latched `std_msgs/String` with the full OpenDRIVE description of the current map on `/carla/map` (transient local durability, re-published on every map load). Any node can join late, read the map once, and combine it with the live sensor streams. The demo uses that topic to draw the real lane geometry of the town in RViz:

* **Lane markers**: the OpenDRIVE string is parsed client-side and converted into a latched `visualization_msgs/MarkerArray` on `/carla/map_markers`. Lane boundaries are computed from the true lane width given by the OpenDRIVE (centerline plus and minus half the lane width), and each lane edge is published as one continuous polyline.
* **Hero vehicle and sensors**: a vehicle with camera, lidar, GNSS and IMU drives on autopilot, with every sensor publishing natively (no bridge).
* **Combined view**: the simulator publishes the REP-105 transform tree natively (`map->odom` latched on `/tf_static`, `odom->hero` on `/tf` every frame), so the lidar point cloud and the vehicle TF tree render at their world position on top of the lane network.

## Files

| File | Role |
|------|------|
| `ros2_native.py` | Spawns the hero vehicle defined in `stack.json` (camera, lidar, GNSS, IMU) and drives it on autopilot. Destroys the actors and restores the world settings on shutdown. |
| `stack.json` | Sensor setup of the hero vehicle. Edit it to adjust the sensors. |
| `run_map_and_lidar_demo.sh` | Demo entry point. Builds the demo image if missing and runs the demo stack in Docker. |
| `run_rviz.sh` | Runs RViz in Docker with the bundled preset. |
| `rviz/ros2_native.rviz` | RViz preset: camera panel, lidar point cloud, TF tree and the map markers display (transient local, fixed frame `map`). |
| `Dockerfile`, `config/` | Base RViz image and the RMW configuration files mounted into the containers. |

The `map_and_lidar_demo/` folder holds the internals of the demo image:

| File | Role |
|------|------|
| `map_and_lidar_demo/Dockerfile` | Demo image: the RViz image extended with the carla Python wheel and the helpers below. |
| `map_and_lidar_demo/build.sh` | Builds the `carla-map-and-lidar-demo-<distro>-<rmw>` image. Run automatically by `run_map_and_lidar_demo.sh`. |
| `map_and_lidar_demo/launcher.sh` | In-image entry point. Launches the helpers and stops them all together when the first one exits or the container is stopped. |
| `map_and_lidar_demo/map_to_markers.py` | Subscribes to the latched `/carla/map`, parses the OpenDRIVE with the carla Python package (no simulator connection needed) and publishes the lane markers on `/carla/map_markers`. |
| `map_and_lidar_demo/cleanup.py` | Destroys leftover hero vehicles and their sensors and restores asynchronous mode. Run automatically at stack startup and shutdown so an unclean exit never leaks a second vehicle publishing on the same topics. |

## Prerequisites

To run this example, ensure `docker` is installed in your system, which is used to launch the demo stack and an instance of `rviz` for visualizing the data.

The demo image needs the carla Python wheel from `PythonAPI/carla/dist`, built with `make PythonAPI` (cp310 for humble, cp312 for jazzy; pass `--wheel=<path>` to `map_and_lidar_demo/build.sh` to use another one).

## Usage

### Step 1: Start the CARLA Simulator with ROS2 enabled

```bash
# If running a package:
./CarlaUE4.sh --ros2

# If running the editor:
make launch ARGS="--ros2 --editor-flags='--ros2'"
```

> [!NOTE]
> To use the Zenoh middleware, add `--rmw=zenoh` to the launch command above and to the scripts below, and start a Zenoh router first:
> ```bash
> docker run --rm --net=host carla-rviz-<distro>-zenoh ros2 run rmw_zenoh_cpp rmw_zenohd
> ```
> The `carla-rviz-<distro>-zenoh` image is built the first time you run `run_rviz.sh` (`<distro>` is `humble` or `jazzy`).

### Step 2: Run the map and lidar demo

```bash
./run_map_and_lidar_demo.sh
```

Options:

| Option | Default | Description |
|--------|---------|-------------|
| `--distro=<distro>` | `humble` | ROS 2 distribution: `humble` or `jazzy`. |
| `--rmw=<middleware>` | `fastdds` | RMW implementation: `fastdds`, `cyclonedds` or `zenoh`. Must match the `--rmw=` the simulator was started with. |
| `--host=<host>` | `localhost` | IP of the host CARLA Simulator. |
| `--port=<port>` | `2000` | TCP port of the CARLA Simulator. Must match `-carla-rpc-port` if the server uses a non-default port. |
| `--waypoint-distance=<meters>` | `2.0` | Distance between sampled lane points. Lower values give smoother curves at the cost of more markers. |
| `--map-only` | off | Only publish the map markers, do not spawn the vehicle stack. |

The first run builds the `carla-map-and-lidar-demo-<distro>-<rmw>` image automatically. Stop the demo with a single Ctrl+C (or `docker stop carla-map-and-lidar-demo-<distro>-<rmw>`); the stack destroys the vehicle and its sensors, restores the world settings and removes any leftovers before exiting. A second concurrent run fails fast instead of spawning a duplicate vehicle on the same topics.

### Step 3: Run RViz to visualize the result

```bash
./run_rviz.sh
```

With the bundled preset (`Fixed Frame: map`) you get the combined view: the town lane network, the hero TF tree driving along it, the lidar point cloud rendered at the vehicle's world position and the camera image panel.

Tips:

* The lane centerlines are published in their own marker namespace (`lane_centerlines`) and hidden by default. Enable them under the Map display Namespaces in RViz.
* To follow the vehicle automatically, set `Views -> Target Frame` to `hero` (the `Orbit` view keeps a fixed compass orientation, `ThirdPersonFollower` rotates with the vehicle heading).
* If RViz started before the transforms were available it may have dropped the latched map markers. Toggle the Map display checkbox to force a resubscription.

### Running the helpers standalone

Each helper can also run directly from any ROS 2 environment that has the `carla` Python package installed:

```bash
python3 ros2_native.py --file stack.json
python3 map_and_lidar_demo/map_to_markers.py
python3 map_and_lidar_demo/cleanup.py
```

`ros2_native.py` and `map_and_lidar_demo/cleanup.py` only need the `carla` package (no ROS environment).
