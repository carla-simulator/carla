# ROS bridge

The ROS bridge is a separate package that connects a running CARLA server to a [ROS](https://www.ros.org/) graph. It translates the information coming from the server into ROS topics, and the messages published by ROS nodes into commands applied in the simulation. It runs as its own process, alongside CARLA, and works with both ROS 1 and ROS 2.

It provides:

- Sensor data for LIDAR, semantic LIDAR, cameras (depth, segmentation, RGB, DVS), GNSS, radar and IMU.
- Object data such as transforms, traffic light status, visualisation markers, collisions and lane invasions.
- Control of AD agents through steering, throttle and brake.
- Control of the simulation itself: synchronous mode, playing and pausing, and the simulation parameters.

!!! Warning
    The bridge targets CARLA 0.9.13, from the Unreal Engine 4 line, and is not used by this version. CARLA on Unreal Engine 5 publishes to ROS 2 from inside the simulator, with no bridge process: see [ROS 2](ros2_native.md).

__Full documentation of the ROS bridge is found [__here__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/).__
