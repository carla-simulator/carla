# ROS

![ros_carla](../img/ros_carla.png)

The [__Robotic Operating System (ROS)__](https://www.ros.org/) is a set of software libraries for robotic and autonomous driving applications. CARLA can be directly connected to ROS through its ROS interfaces, control signals can be sent to CARLA actors and sensor data can be accessed through ROS topics. 

There are two options for connecting CARLA and ROS.

- __CARLA native interface__: a ROS interface build directly into the CARLA server
- __ROS Bridge__: a separate library for transfering signals between ROS and CARLA

## CARLA native ROS interface

This is the recommended interface, since it offers the best performance with the lowest latency. At the moment the native interface only supports ROS 2. If you are using ROS 1, you must use the ROS Bridge.

### DDS backend selection

The native ROS 2 interface is built on top of a DDS (Data Distribution Service) middleware. CARLA supports two DDS backends:

| Backend | Description                                                                       |
| ------- |-----------------------------------------------------------------------------------|
| **Fast DDS** (default) | eProsima Fast DDS: the reference implementation used by ROS 2 Humble and earlier. |
| **Cyclone DDS** | Eclipse Cyclone DDS: a lightweight alternative used by ROS 2 Iron and later.      |

Both backends produce identical OMG CDR bytes on the wire and are fully interoperable with any standards-compliant ROS 2 subscriber regardless of which DDS implementation the subscriber uses.

#### Building with a specific DDS backend

Pass `--included-dds` to `make setup` and `make LibCarla` to select which backend(s) to compile:

```sh
# Build with Fast DDS only (default, backward-compatible)
make setup ARGS="--ros2 --included-dds=fastdds"
make LibCarla ARGS="--ros2 --included-dds=fastdds"

# Build with Cyclone DDS only
make setup ARGS="--ros2 --included-dds=cyclonedds"
make LibCarla ARGS="--ros2 --included-dds=cyclonedds"

# Build with both backends (Fast DDS is the default at runtime unless --dds-backend is specified)
make setup ARGS="--ros2 --included-dds=fastdds,cyclonedds"
make LibCarla ARGS="--ros2 --included-dds=fastdds,cyclonedds"
```

#### Selecting the DDS backend at runtime

When the server is launched with `--ros2`, the active DDS backend is selected with `--dds-backend`:

```sh
# Use Fast DDS (default if flag is omitted)
./CarlaUE4.sh --ros2 --dds-backend=fastdds

# Use Cyclone DDS
./CarlaUE4.sh --ros2 --dds-backend=cyclonedds
```

If `--dds-backend` is not specified, Fast DDS is used as the default.

#### Error handling for unavailable backends

If you request a DDS backend that was not compiled into the build (e.g. `--dds-backend=cyclonedds` on a binary built with `--included-dds=fastdds` only), CARLA will:

1. Log an error message listing which backends are available in the current build.
2. Disable ROS2 for the entire session, no topics will be published and no subscribers will be created.
3. No silent fallback occurs. CARLA will never quietly switch to a different backend.

Similarly, if the `--dds-backend` value is not one of `fastdds` or `cyclonedds` (e.g. a typo like `--dds-backend=otherdds`), ROS2 is disabled with an error listing the valid values.

To recover, restart the server with a valid `--dds-backend` value, or rebuild CARLA with the desired backend included.

## CARLA ROS Bridge

The [__CARLA ROS Bridge__](https://carla.readthedocs.io/projects/ros-bridge/en/latest/) is a library for connecting ROS to CARLA, it is compatible with both ROS 1 and ROS 2. Since the CARLA ROS Bridge is a separate package, there is additional latency compared to the native interface. The ROS Bridge is still provide to support ROS 1 and legacy implementations with ROS 2. 

