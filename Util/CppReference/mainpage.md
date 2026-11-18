# CARLA C++ reference

Reference generated from the comments in CARLA's C++ headers. Use the search box
(top right) to jump to any class or function, or start from one of the areas below.

## LibCarla — client and server library

| Area | Namespace | What it holds |
| --- | --- | --- |
| Client API | \ref carla::client "carla::client" | World, actors, blueprints, sensors as the C++ client sees them. |
| RPC | \ref carla::rpc "carla::rpc" | Types exchanged between client and server. |
| Roads | \ref carla::road "carla::road" | OpenDRIVE road network, waypoints, lanes. |
| OpenDRIVE parser | \ref carla::opendrive "carla::opendrive" | Loading `.xodr` into the road map. |
| Traffic Manager | \ref carla::traffic_manager "carla::traffic_manager" | Autopilot for vehicles. |
| Sensors | \ref carla::sensor "carla::sensor" | Sensor data and its serialization. |
| Streaming | \ref carla::streaming "carla::streaming" | Sensor data transport. |
| ROS 2 | \ref carla::ros2 "carla::ros2" | Native ROS 2 publishers and subscribers. |
| Multi-GPU | \ref carla::multigpu "carla::multigpu" | Primary/secondary server protocol. |
| Navigation agents | \ref carla::agents::navigation "carla::agents::navigation" | Basic, behavior and constant-velocity agents, route planner. |
| Pedestrian navigation | \ref carla::nav "carla::nav" | Walker navigation mesh. |
| Geometry | \ref carla::geom "carla::geom" | Vectors, transforms, bounding boxes. |

## Carla plugin — server side in Unreal

| Area | Start at | What it holds |
| --- | --- | --- |
| Episode | \ref UCarlaEpisode, \ref ACarlaGameModeBase | The running simulation and its world. |
| Server | \ref FCarlaServer | The RPC endpoints the client calls. |
| Actors | \ref UActorDispatcher | Spawning actors from blueprints. |
| Vehicles | \ref ACarlaWheeledVehicle | Vehicle physics and control. |
| Sensors | \ref ASensor, \ref ASceneCaptureSensor | Sensor actors. |
| Traffic | \ref ATrafficLightManager | Traffic lights and signs. |
| Walkers | \ref AWalkerAIController | Pedestrian AI. |
| Recorder | \ref ACarlaRecorder | Recording and replay. |
| Settings | \ref UCarlaSettings | Server settings. |

Full lists: [namespaces](namespaces.html) · [classes](annotated.html) · [files](files.html).
For the Python API and tutorials see <https://carla.readthedocs.io>.
