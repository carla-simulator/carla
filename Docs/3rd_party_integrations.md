
# 3rd Party Integrations

CARLA has been developed to integrate with several 3rd party applications in order to maximise its utility and extensability. The following  

*   [__ROS bridge__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/)
*   [__SUMO__](adv_sumo.md)  
*   [__Chrono__](tuto_G_chrono.md)
*   [__OpenDRIVE__](adv_opendrive.md) 

---
## ROS bridge

__Full documentation of the ROS bridge is found [__here__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/).__

The ROS bridge enables two-way communication between ROS and CARLA. The information from the CARLA server is translated to ROS topics. In the same way, the messages sent between nodes in ROS get translated to commands to be applied in CARLA.

The ROS bridge is compatible with both ROS 1 and ROS 2.

The ROS bridge boasts the following features:

- Provides sensor data for LIDAR, Semantic LIDAR, Cameras (depth, segmentation, rgb, dvs), GNSS, Radar and IMU.
- Provides object data such as transforms, traffic light status, visualisation markers, collision and lane invasion.
- Control of AD agents through steering, throttle and brake.
- Control of aspects of the CARLA simulation like synchronous mode, playing and pausing the simulation and setting simulation parameters.

---

## SUMO

CARLA has developed a co-simulation feature with [__SUMO__](https://www.eclipse.org/sumo/). This allows to distribute the tasks at will, and exploit the capabilities of each simulation in favour of the user.

Please refer to the full documentation [__here__](adv_sumo.md).

---

## Chrono physics

[__Chrono__](https://projectchrono.org/) is a multi-physics simulation engine providing high realism vehicle dynamics using templates. CARLA's Chrono integraion allows CARLA users to add Chrono templates to simulate vehicle dynamics. Please refer to the full documentation [__here__](tuto_G_chrono.md).

---

## OpenDRIVE

[__OpenDRIVE__](https://www.asam.net/standards/detail/opendrive/) is an open format specification used to describe the logic of a road network intended to standardise the discription of road networks in digital format and allow different applications to exchange data on road networks. Please refer to the full documentation [__here__](adv_opendrive.md)