# ROS Bridge

__Full documentation of the ROS bridge is found [__here__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/).__

---

The ROS bridge enables two-way communication between ROS and CARLA. The information from the CARLA server is translated to ROS topics. In the same way, the messages sent between nodes in ROS get translated to commands to be applied in CARLA.

The ROS bridge is compatible with both ROS 1 and ROS 2.

The ROS bridge boasts the following features:

- Provides sensor data for LIDAR, Semantic LIDAR, Cameras (depth, segmentation, rgb, dvs), GNSS, Radar and IMU.
- Provides object data such as transforms, traffic light status, visualisation markers, collision and lane invasion.
- Control of AD agents through steering, throttle and brake.
- Control of aspects of the CARLA simulation like synchronous mode, playing and pausing the simulation and setting simulation parameters.
