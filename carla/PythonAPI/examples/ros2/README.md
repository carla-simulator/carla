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
