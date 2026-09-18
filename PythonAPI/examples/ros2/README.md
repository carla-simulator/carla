# ROS2 Native Example

This example demonstrates how to utilize the ROS 2 native interface in CARLA.
The map-and-lidar demo consumes the latched `/carla/map` OpenDRIVE topic and
renders the town lane network, hero transform tree, camera, and live lidar in
one map-fixed RViz view. Run `./run_map_and_lidar_demo.sh` after starting CARLA
with `--ros2`, then run `./run_rviz.sh`.

## Prerequisites

To run this example, ensure `docker` is installed in your system, which is used to launch an instance of `rviz` for visualizing sensor data.


## Usage

### Step 1: Start the CARLA Simulator with ROS2 enabled
Launch the CARLA simulator with the ROS 2 integration enabled:

```bash
./CarlaUnreal.sh --ros2
```

To select the middleware, add the `--rmw=` argument (`fastdds` by default, or
`cyclonedds` or `zenoh` on Linux):

```bash
./CarlaUnreal.sh --ros2 --rmw=cyclonedds
```

> [!NOTE]
> `--rmw=zenoh` needs a Zenoh router running before the simulator starts. Start it once
> with the rviz image, e.g. `docker run --rm --net=host carla-rviz-humble-zenoh ros2 run
> rmw_zenoh_cpp rmw_zenohd`.

### Step 2: Run the ROS2 Example

Execute the ROS 2 example script:

```bash
python3 ros2_native.py --file stack.json
```

* The `stack.json` file defines the sensor configuration.
* You can edit this file to adjust the sensor setup according to your requirements.


### Step 3: Run RViz to Visualize Sensor Data

Start `rviz` to visualize the sensor output from CARLA:

> [!NOTE]
Docker must be installed on your system to complete this step.

```bash
./run_rviz.sh
```

`run_rviz.sh` accepts `--distro=<humble|jazzy>` and `--rmw=<fastdds|cyclonedds|zenoh>`. The
`--rmw` value must match the middleware the simulator was launched with:

```bash
./run_rviz.sh --distro=humble --rmw=cyclonedds
```

### Map-and-lidar demo

The map demo needs a CARLA Python wheel built for the same Linux runtime as
the ROS Docker image. Do not pass a wheel from a newer host distribution: it
can import on the host but fail in ROS Humble because of its newer glibc
requirement. The build checks `import carla` inside the image before tagging
the demo image.

Build the compatible wheel reproducibly from this checkout, using the matching
CARLA development image and Unreal Engine path:

```bash
export CARLA_UNREAL_ENGINE_PATH=/absolute/path/to/UnrealEngine
./map_and_lidar_demo/build_target_wheel.sh --distro=humble
```

This writes the CPython 3.10 wheel to
`Build/ros2-wheel-humble/PythonAPI/dist/`. The demo locates that exact path by
default. To use a separately built wheel, pass it explicitly instead:

```bash
./run_map_and_lidar_demo.sh --distro=humble --rmw=fastdds \
  --wheel=/absolute/path/to/carla-0.10.0-cp310-cp310-linux_x86_64.whl
./run_rviz.sh --distro=humble --rmw=fastdds
```

The image build verifies the wheel's glibc/Python ABI, package version, and
the CARLA APIs used by the demo. Build the wheel from this checkout (or a
protocol-compatible checkout); an arbitrary importable CARLA wheel is not
sufficient.

RViz starts in a third-person view following the `hero` frame. It includes a
transient-local map-marker display and best-effort camera/lidar displays; the
demo must be running before `hero` exists.

The demo's CARLA actor has a private `hero_carla_map_and_lidar_demo` role while
retaining `hero` as its public ROS name and TF frame. At startup it removes only
leftover actors with that private role, so an interrupted prior run cannot make
the TF helper attach to an unrelated client-owned `hero` vehicle.

### Optional: Custom ROS 2 domain id

By default CARLA and `rviz` communicate on the default ROS 2 domain. To isolate the
session on a specific domain, launch the server with `--ros-domain-id=<N>` and pass the
same value to both helper scripts:

```bash
# Server:
./CarlaUnreal.sh --ros2 --ros-domain-id=42

# RViz on the matching domain:
./run_map_and_lidar_demo.sh --ros-domain-id=42
./run_rviz.sh --ros-domain-id=42
```

The domain id must be an integer in the range 0-232 and must match on both sides for the
topics to be discovered.

If you omit `--ros-domain-id`, the server falls back to the standard `ROS_DOMAIN_ID`
environment variable, and then to the default domain 0. For example, exporting the variable
before launching applies the same domain without the option:

```bash
export ROS_DOMAIN_ID=42
./CarlaUnreal.sh --ros2          # server uses domain 42
./run_map_and_lidar_demo.sh      # helper inherits ROS_DOMAIN_ID=42
./run_rviz.sh                    # RViz inherits ROS_DOMAIN_ID=42
```

When both are set, `--ros-domain-id` takes precedence over `ROS_DOMAIN_ID`.

### Map topic and QoS

On every episode start, CARLA publishes the current OpenDRIVE XML on
`/carla/map` (`rt/carla/map` at the DDS layer) as `std_msgs/msg/String`. It
uses reliable, transient-local, keep-last-one QoS, so a subscriber that starts
after the simulator receives the current map immediately. The message has no
ROS header by design: this preserves compatibility with carla-ros-bridge's
`/carla/map` topic. A new map load replaces the cached sample.

Camera image/camera-info and point-cloud publishers use ROS 2 sensor-data QoS
(best effort) so a slow visualisation subscriber cannot stall CARLA's sensor
publishing path. Configure RViz image and point-cloud displays for best-effort
reliability; configure a `/carla/map` subscriber for transient-local durability.

FastDDS is the end-to-end validated configuration for this demo. CycloneDDS
and Zenoh are exposed for native ROS2 interoperability, but require a local
backend-specific validation before relying on the map demo in production.
For a PR validation, start CARLA with `--ros2`, then start the demo after the
server is already running; the map must appear without reloading the episode.
This explicitly checks the transient-local late-subscriber behavior of
`/carla/map`.
