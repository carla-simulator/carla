# ROS2 Native Example

This example demonstrates how to utilize the ROS 2 native interface in CARLA.

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

### Optional: Custom ROS 2 domain id

By default CARLA and `rviz` communicate on the default ROS 2 domain. To isolate the
session on a specific domain, launch the server with `--ros-domain-id=<N>` and pass the
same value to `run_rviz.sh`:

```bash
# Server:
./CarlaUnreal.sh --ros2 --ros-domain-id=42

# RViz on the matching domain:
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
./run_rviz.sh --ros-domain-id=42
```

When both are set, `--ros-domain-id` takes precedence over `ROS_DOMAIN_ID`.
