# ROS2 Native Example

This example demonstrates how to utilize the ROS 2 native interface in CARLA.

## Prerequisites

To run this example, ensure `docker` is installed in your system, which is used to launch an instance of `rviz` for
visualizing sensor data.

## Usage

### Step 1: Start the CARLA Simulator with ROS2 enabled

Launch the CARLA simulator with the ROS 2 integration enabled:

```bash
# If running a package:
./CarlaUE4.sh --ros2

# If running the editor:
make launch ARGS="--ros2 --editor-flags='--ros2'"
```

> [!NOTE]
> To use the Zenoh middleware, add `--rmw=zenoh` to the launch command above and to `run_rviz.sh`, and start a Zenoh router first:
> ```bash
> docker run --rm --net=host carla-rviz-<distro>-zenoh ros2 run rmw_zenoh_cpp rmw_zenohd
> ```
> The `carla-rviz-<distro>-zenoh` image is built the first time you run `run_rviz.sh` (`<distro>` is `humble` or `jazzy`).

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
> Docker must be installed on your system to complete this step.

```bash
./run_rviz.sh
```

### Optional: Custom ROS 2 domain id

By default CARLA and `rviz` communicate on the default ROS 2 domain. To isolate the
session on a specific domain, launch the server with `--ros-domain-id=<N>` and pass the
same value to `run_rviz.sh`:

```bash
# Server (package or editor):
./CarlaUE4.sh --ros2 --ros-domain-id=42
make launch ARGS="--ros2 --ros-domain-id=42"

# RViz on the matching domain:
./run_rviz.sh --ros-domain-id=42
```

The domain id must be an integer in the range 0-232 and must match on both sides for the
topics to be discovered.
