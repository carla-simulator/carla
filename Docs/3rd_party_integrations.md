
# 3rd Party Integrations

CARLA has been developed to integrate with several 3rd party applications in order to maximise its utility and extensability. The following  

-   [__ROS bridge__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/)
-   [__SUMO__](adv_sumo.md)  
-   [__Scenic__](tuto_G_scenic.md)
-   [__CarSIM__](tuto_G_carsim_integration.md)
-   [__Chrono__](tuto_G_chrono.md)
-   [__OpenDRIVE__](adv_opendrive.md) 
-   [__PTV Vissim__](adv_ptv.md)
-   [__RSS__](adv_rss.md) 
-   [__AWS and RLlib__](tuto_G_rllib_integration.md)

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

## PTV Vissim

[__PTV Vissim__](https://www.ptvgroup.com/en/solutions/products/ptv-vissim/) is a proprietary software package providing a comprehensive traffic simulation solution with a powerful GUI. To use PTV-Vissim with CARLA refer to [__this guide__](adv_ptv.md)

---

## Scenic 

Scenic is a set of libraries and a language for scenario specification and scene generation. CARLA and scenic can work seemlessly together, read [__this guid__](tuto_G_scenic.md) to understand how to use scenic with CARLA. 

If you need to learn more about Scenic, then read their ["Getting Started with Scenic"](https://scenic-lang.readthedocs.io/en/latest/quickstart.html) guide and have a look at their tutorials for creating [static](https://scenic-lang.readthedocs.io/en/latest/tutorials/tutorial.html) and [dynamic](https://scenic-lang.readthedocs.io/en/latest/tutorials/dynamics.html) scenarios.

---

## CarSIM

CARLA's integration with CarSim allows vehicle controls in CARLA to be forwarded to CarSim. CarSim will do all required physics calculations of the vehicle and return the new state to CARLA. 

Learn how to use CARLA alongside CarSIM [here](tuto_G_carsim_integration.md).

## OpenDRIVE

[__OpenDRIVE__](https://www.asam.net/standards/detail/opendrive/) is an open format specification used to describe the logic of a road network intended to standardise the discription of road networks in digital format and allow different applications to exchange data on road networks. Please refer to the full documentation [__here__](adv_opendrive.md)

## RSS - Responsibility Sensitive Safety

CARLA integrates the [C++ Library for Responsibility Sensitive Safety](https://github.com/intel/ad-rss-lib) in the client library. This feature allows users to investigate behaviours of RSS without having to implement anything. CARLA will take care of providing the input, and applying the output to the AD systems on the fly. Refer to the full documentation [__here__](adv_rss.md)

## AWS and RLlib integration

The RLlib integration brings support between the Ray/RLlib library and CARLA, allowing the easy use of the CARLA environment for training and inference purposes. Ray is an open source framework that provides a simple, universal API for building distributed applications. Ray is packaged with RLlib, a scalable reinforcement learning library, and Tune, a scalable hyperparameter tuning library. Read more about operating CARLA on AWS and RLlib [__here__](tuto_G_rllib_integration.md).

## Chrono physics

[__Chrono__](https://projectchrono.org/) is a multi-physics simulation engine providing high realism vehicle dynamics using templates. CARLA's Chrono integraion allows CARLA users to add Chrono templates to simulate vehicle dynamics. Please refer to the full documentation [__here__](tuto_G_chrono.md).

---