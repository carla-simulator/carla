# The ros2 layer, top to bottom

Detail layer for `add-ros-publisher`. Read from
`LibCarla/source/carla/ros2/` and the UE4 plugin on the branch that ships them;
not live-verified here.

## Ownership per layer

| Layer | Files | Owns |
|---|---|---|
| call site | `Unreal/CarlaUE4/Plugins/Carla/Source/Carla/{Sensor,Actor,Game}/…` | when data is produced, and the transform passed |
| dispatch | `ROS2.h` / `ROS2.cpp` | actor↔publisher map, names, frames, TF, `Process*` entry points |
| publisher | `publishers/Carla*Publisher.{h,cpp}` | topic suffix, QoS, message filling |
| subscriber | `subscribers/*Subscriber.{h,cpp}` | topic suffix, message → `ROS2CallbackData` |
| impl | `publishers/PublisherImpl.h`, `subscribers/SubscriberImpl.h` | one message instance + middleware handle |
| middleware | `middleware/{fastdds,cyclonedds,zenoh}/…` | DDS/Zenoh specifics |
| types | `types/msg/*.h`, `types/Cdr*.h` | wire format ([[add-ros-message-type]]) |

Change exactly one layer per concern: a new topic touches call site + dispatch +
publisher; a new RMW touches only middleware + factory.

## Publisher contract

- Derive from `BasePublisher`, which stores `_actor`, `_base_topic_name`,
  `_frame_id` and declares the pure virtual `Publish()`.
- Hold `std::shared_ptr<PublisherImpl<Traits>>` where `Traits` is a one-line
  struct: `struct XMsgTraits { using msg_type = msg::X; };`. That typedef is the
  only binding between publisher and message type.
- `Init(topic, qos)` in the constructor; **failure only warns** and leaves an
  object that publishes nowhere. Log the topic in that warning like the existing
  publishers do — it is the sole clue.
- `Write(...)` fills `_impl->GetMessage()`; `Publish()` forwards to the impl.
  Splitting them lets the dispatch fill several messages (camera: image +
  camera_info) and publish once.
- Composition beats inheritance for multi-topic sensors: `CarlaDVSPublisher` owns
  a camera publisher **and** a point-cloud publisher rather than subclassing both.

### Topic suffixes in use

`/image`, `/camera_info` (`CarlaCameraPublisher`), `/point_cloud`
(`CarlaPointCloudPublisher`, inherited by lidar/semantic-lidar/radar/DVS-cloud),
and **no suffix** for IMU, GNSS and collision — they publish on the base name
itself. Fixed names: `rt/clock`, `rt/tf`, and — **on 0.9.x only** — `rt/carla/map`
(`CarlaMapPublisher` is absent on 0.10.0).

### QoS

`PublisherQos` (in `middleware/PublisherQos.h`) has three fields:
`durability` (`Volatile` | `TransientLocal`), `reliability` (`Reliable` |
`BestEffort`), `history_depth` (0 clamps to 1). Defaults are
reliable + volatile + depth 1.

- `PublisherQos::SensorData()` → best-effort. Use for image/point-cloud streams:
  a slow or vanished subscriber can then never block the publishing thread.
- `TransientLocal` = ROS 2 "latched". On 0.9.x only `rt/carla/map` uses it, so
  late joiners get the OpenDRIVE without waiting for a map change; on 0.10.0 that
  publisher is gone, so nothing latches by default. The struct is also renamed
  there: `middleware/QosProfile.h`, not `PublisherQos.h`.
- Consumers must match: a reliable subscriber will not match a best-effort
  publisher.

## Dispatch (`ROS2.cpp`)

- `GetOrCreateSensor(type, actor)` — a `switch` over `ESensors`, one `case` per
  publisher, caching in `_publishers`. `nullptr` means "no native publisher"
  (lane invasion, obstacle, RSS, world observer).
- Names: `GetActorRosName` → the `ros_name` attribute or `actor<id>`;
  `GetActorBaseTopicName` → `rt/carla/<name>`, or the parent's base + `/` + name
  when `_actor_parent_map` has an entry (`insert`, so the **immediate** parent
  wins). `GetParentFrameId` falls back to `"map"`.
- `ProcessDataFrom<X>(...)` — the entry point. Pattern: lock `_mutex`, get/create
  the publisher, `dynamic_pointer_cast` to the concrete type, `Write`, `Publish`,
  then write + publish the TF publisher. The mutex is a `recursive_mutex` because
  `RegisterSensor` calls `RegisterActor`.
- Registration happens in `UActorDispatcher::RegisterActor`: sensors always,
  vehicles **only when `role_name == "hero"`**. Attach-time parent registration is
  in `CarlaServer.cpp`'s attach RPC.
- Threading: `Process*`/`SetFrame` run on the UE4 tick thread, `Register*` on the
  RPC thread. Anything new must take the same lock.

## Subscriber contract

`BaseSubscriber` + `SubscriberImpl<Traits>`; `Init(topic)` binds the message and a
"new message" flag. `GetMessage()` converts to `ROS2CallbackData` (a
`boost::variant2` over the command types) and `ProcessMessages(callback)` hands it
to the `ActorCallback` registered in `RegisterVehicle`, where `ActorROS2Handler`
applies it to the UE4 actor. Existing pair: `/vehicle_control_cmd`
(`CarlaEgoVehicleControl`) and `/ackermann_control_cmd`
(`AckermannDriveStamped`). Adding a command means extending the variant and the
handler too, not just adding a subscriber.

## Middleware layer

`MiddlewareFactory` is a static factory over `IPublisherMiddleware` /
`ISubscriberMiddleware`, selected once at startup by `SetMiddleware()` (changing it
after the first entity is created is undefined). `IsMiddlewareAvailable` answers
from the `CARLA_ROS2_MIDDLEWARE_*` compile definitions — **all three are defined
for one build** (`LibCarla/cmake/ros2/CMakeLists.txt`), which is why one binary
serves any `--rmw` and can report `Available: …` honestly.

Per-RMW specifics worth knowing before touching them:

| RMW | Specifics |
|---|---|
| FastDDS | shared participant (`FastDDSSharedParticipant`) reused across endpoints; generic CDR pub/sub type wrapper; built static, patched to compile without exceptions |
| CycloneDDS | custom sertype (`CycloneDDSSertype.cpp`) to hand raw CDR to Cyclone |
| Zenoh | shared session (`ZenohSharedSession`), its own wire format shim (`ZenohWireFormat.h`), a session config JSON5, and an out-of-band **router** (`rmw_zenohd`) that must run |

`CARLA_ROS2_MIDDLEWARE_TESTING` swaps the real middlewares out so
`test_ros2_middleware.cpp` can exercise the layer with a fake — the seam to use
when adding tests. `LIBCARLA_WITH_GTEST` additionally lets a test inject a
middleware before `Init`.

## Build wiring

- `LibCarla/cmake/ros2/CMakeLists.txt` globs `*.cpp` in `carla/ros2/`,
  `publishers/`, `subscribers/`, `listeners/`, `types/` and each
  `middleware/<name>/`. New files in those directories are picked up
  automatically; a new directory needs a glob line.
- Two targets, `carla_ros2` and `carla_ros2_debug`, both compiled with
  `-fexceptions` (the rest of LibCarla is not) — beware of exceptions escaping
  into non-exception code across the boundary.
- The server plugin links it only when `Ros2 ON` produced `WITH_ROS2`
  ([[build-carla-ue4]] `ROS2=1`), so **LibCarla alone rebuilding is never enough**
  to see a new topic: the plugin must be rebuilt too.
