# ROS

![ros_carla](../img/ros_carla.png)

The [__Robotic Operating System (ROS)__](https://www.ros.org/) is a set of software libraries for robotics and autonomous driving applications. CARLA can be directly connected to ROS through its ROS interfaces, control signals can be sent to CARLA actors and sensor data can be accessed through ROS topics. 

There are two options for connecting CARLA and ROS.

- [__CARLA native interface__](#carla-native-ros2-interface): a ROS interface build directly into the CARLA server
- [__ROS Bridge__](#carla-ros-bridge): a separate library for transferring signals between ROS and CARLA

--- 

## CARLA native ROS2 interface

This is the recommended interface, since it offers the best performance with the lowest latency. The CARLA simulator supports ROS2 natively from the server. To use ROS2 with a CARLA package, launch the CARLA simulator from the command line with the `--ros2` command line option:

```sh
./CarlaUE4.sh --ros2
```

If you are building from source, you need to build with the ROS2 interface enabled using the following command:

```sh
make launch ARGS="--ros2 --editor-flags='--ros2'"
```

### Sensor data

The CARLA server will broadcast sensor data for any spawned sensors for which ROS is enabled. To enable a sensor for ROS use the `enable_for_ros()` method of the sensor class. You should also set the ROS name in the sensor blueprint prior to spawning:

```py
# Set topic name
bp = bp_lib.find('sensor.camera.rgb')
bp.set_attribute('ros_name', 'front_camera')

# Spawn the sensor and enable ROS
sensor = world.spawn_actor(sensor_blueprint, transform)
sensor.enable_for_ros()
```

In this case the image data will be published to: `/carla/front_camera/image`. 

If the camera is parented to an actor, for example the hero vehicle, the topic name will also include the role name given to the vehicle: `/carla/hero/front_camera/image`.

See the [ROS2 sensors reference](ros2_native_sensors.md) for details about the message formats used for each sensor type.

### Control data

Controls may be sent to hero vehicles:

```py
bp = bp.find("vehicle.lincoln.mkz_2020")
bp.set_attribute("role_name", "hero")
bp.set_attribute("ros_name", "hero")

ego = carla.spawn_actor(bp, spawn_point)
```

When the ego vehicle is spawned a subscriber will be created with ROS topic name: `/carla/hero/vehicle_control_cmd`. You can also send control information to `/carla/hero/ackermann_control_cmd` if you prefer to use the [Ackermann controller](https://github.com/ros-drivers/ackermann_msgs/blob/ros2/msg/AckermannDriveStamped.msg).

### CarlaEgoVehicleControl.msg

To send control messages, install the [ros-carla-msgs ROS package](https://github.com/carla-simulator/ros-carla-msgs/tree/master). The control message has the following fields:

| Field                                                                                                   | Type                                                                                                    | Description                                                                                             |
| ------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| `header`                                                                                                | [Header](https://docs.ros.org/en/melodic/api/std_msgs/html/msg/Header.html)                               | Time stamp and frame ID when the message is published.                                                  |
| `throttle`                                                                                              | float32                                                                                                 | Scalar value to cotrol the vehicle throttle: **[0.0, 1.0]**                                             |
| `steer`                                                                                                 | float32                                                                                                 | Scalar value to control the vehicle steering direction: **[-1.0, 1.0]** to control the vehicle steering |
| `brake`                                                                                                 | float32                                                                                                 | Scalar value to control the vehicle brakes: **[0.0, 1.0]**                                              |
| `hand_brake`                                                                                            | bool                                                                                                    | If **True**, the hand brake is enabled.                                                                 |
| `reverse`                                                                                               | bool                                                                                                    | If **True**, the vehicle will move reverse.                                                             |
| `gear`                                                                                                  | int32                                                                                                   | Changes between the available gears in a vehicle.                                                       |
| `manual_gear_shift`                                                                                     | bool                                                                                                    | If **True**, the gears will be shifted using `gear`.                                                    |

---

## CARLA ROS Bridge

The [__CARLA ROS Bridge__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/) is a library for connecting ROS to CARLA, it is compatible with both ROS 1 and ROS 2. Since the CARLA ROS Bridge is a separate package, there is additional latency compared to the native interface. The ROS Bridge continues to be provided to support ROS 1 and legacy implementations with ROS 2. 

