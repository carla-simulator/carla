# ROS2 native interface

The CARLA simulator supports ROS2 directly from the server. Launch CARLA from the command line with the `--ros2` command line option:

```sh
./CarlaUnreal.sh --ros2
```

The CARLA server will broadcast sensor data for every spawned sensor as a ROS2 topic.