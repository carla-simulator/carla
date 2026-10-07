---
name: add-ros-publisher
description: Adds a new topic to CARLA's native ROS 2 interface — a publisher (BasePublisher + PublisherImpl, topic suffix, QoS, registration in ROS2::GetOrCreateSensor, a ProcessDataFrom* entry point and its call site in the UE4 plugin) or a subscriber that lets ROS drive an actor. Also covers adding a middleware (RMW) behind IPublisherMiddleware. Use when the user asks to "publish X to ROS from CARLA", "add a ROS topic/publisher/subscriber", "make the obstacle/lane-invasion sensor publish", "add odometry to ROS", or "add another RMW".
license: MIT
compatibility: A CARLA checkout whose LibCarla/source/carla/ros2 exists (native ROS 2 branch) and the toolchain to build LibCarla with --ros2 plus the UE4 plugin. Editing only. Proving a new topic on the wire needs a ROS 2 consumer (visualize-ros-rviz) and a server built with ROS 2.
metadata:
  group: ros2
  requires: add-ros-message-type
  prerequisites: scripts/check_env.sh
  reference: references/layers.md
---

# Add a ROS 2 publisher or subscriber

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

A topic exists only when **four** things line up: a message type, a publisher
object, a registration so something creates it, and a call site that feeds it
data. Miss the last one and everything compiles while the topic never appears.

```
UE4 sensor/actor  --ProcessDataFrom*-->  ROS2 (dispatch, ROS2.cpp)
                                          |  GetOrCreateSensor / RegisterVehicle
                                          v
                            Carla<X>Publisher  (topic suffix + QoS)
                                          v
                            PublisherImpl<Traits>  -> MiddlewareFactory
                                          v
                     FastDDS | CycloneDDS | Zenoh middleware
```

Message types are [[add-ros-message-type]]; this skill is everything below the
type.

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

The publisher layer was refactored on 0.10.0, so the file you copy from differs.
The right-hand column is UE 5.8; **UE 5.5 has the same `BasicPublisher` refactor
but no `middleware/` directory**, so its QoS type is the pre-`QosProfile.h` one
and it has none of the Autoware rows ([[check-ue5-limitations]]):

| | 0.9.x | 0.10.0 |
|---|---|---|
| base class to inherit | `CarlaPublisher` | `BasicPublisher` (and `BasicSubscriber` / `BasicListener`) |
| QoS type | `PublisherQos.h` | `QosProfile.h` |
| middleware | `IPublisherMiddleware` impls | same, plus `ActiveMiddleware.{h,cpp}` |
| map publisher | `CarlaMapPublisher` | **removed** — do not model a new publisher on it |
| DVS camera | `CarlaDVSPublisher` | `CarlaDVSCameraPublisher` |
| Autoware | — | `AutowareGNSSPublisher`, `AutowareVehicleStatusPublisher`, `AutowareControlSubscriber`, `AutowareSteeringCompensation.h` |

0.10.0 also factors the per-sensor maths out of the publishers into reusable
headers you should use rather than re-derive: `CameraIntrinsics`, `ImuMath`,
`DvsEventEncoding`, `OpticalFlowEncoding`, `PointCloudFieldsLayout`,
`RadarPolarToCartesian`, `TransformQuaternion`, and `AckermannControlConversion`
on the subscriber side. A new `listeners/` directory holds `BasicListener`.

The registration point, the `ProcessDataFrom*` entry points and the UE plugin call
sites work the same way on both.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Confirm the message type exists in types/msg (else add-ros-message-type)
- [ ] Step 3: Write publishers/Carla<X>Publisher.h (+ .cpp for the Write logic)
- [ ] Step 4: Register it — GetOrCreateSensor case, or RegisterVehicle/Actor
- [ ] Step 5: Add the ProcessDataFrom<X> entry point in ROS2.h/.cpp
- [ ] Step 6: Call it from the UE4 side, inside `#if defined(WITH_ROS2)`
- [ ] Step 7: bash scripts/verify.sh, then rebuild the plugin (build-carla-ue4 ROS2=1)
- [ ] Step 8: Prove the topic — visualize-ros-rviz topics / hz
```

### Step 3: the publisher

Copy the shape of the simplest existing one (`CarlaGNSSPublisher`): derive from
`BasePublisher`, hold a `PublisherImpl<Traits>`, `Init` the topic in the
constructor, implement `Write(...)` and `Publish()`.

```cpp
class CarlaFooPublisher : public BasePublisher {
  public:
    struct FooMsgTraits { using msg_type = msg::Foo; };   // the only type binding

    CarlaFooPublisher(std::string base_topic_name, std::string frame_id):
      BasePublisher(base_topic_name, frame_id),
      _impl(std::make_shared<PublisherImpl<FooMsgTraits>>()) {
        // Suffix decides the topic; NO suffix means "publish on the base name",
        // which is what IMU/GNSS/collision do. Pass PublisherQos::SensorData()
        // for high-rate streams so a slow subscriber cannot stall the sim.
        if (!_impl->Init(GetBaseTopicName() + "/foo", PublisherQos::SensorData())) {
          log_warning("CarlaFooPublisher: Init failed for topic: ", GetBaseTopicName(), "/foo");
        }
    }
    bool Publish() { return _impl->Publish(); }
    bool Write(int32_t seconds, uint32_t nanoseconds, /* payload */);
  private:
    std::shared_ptr<PublisherImpl<FooMsgTraits>> _impl;
};
```

Header in `publishers/`, `Write` body in the matching `.cpp` if it is more than a
few lines. **No CMake edit is needed** — `LibCarla/cmake/ros2/CMakeLists.txt`
globs `publishers/*.cpp`, `subscribers/*.cpp`, `types/*.cpp`,
`middleware/*/*.cpp`. A brand-new *subdirectory* does need a glob entry.

Init failure is a **warning, not an error**: a typo'd topic leaves a live
publisher object that publishes nowhere. Watch for that line in the server log.

### Steps 4-6: wire it up

| For | Register in | Feed from |
|---|---|---|
| a sensor | a `case ESensors::<X>:` in `ROS2::GetOrCreateSensor` | `ProcessDataFrom<X>` called in that sensor's UE4 class |
| a vehicle/actor stream | `ROS2::RegisterVehicle` / `RegisterActor` | the actor's tick or an RPC path |
| a subscriber (ROS drives CARLA) | `RegisterVehicle`'s subscriber list | `ProcessMessages(callback)` → `ActorROS2Handler` |

The dispatch function is the seam: it takes the raw CARLA data, casts the base
publisher to the concrete type, `Write`s, `Publish`es, and (for sensors) also
updates the TF publisher. Copy `ProcessDataFromGNSS` — it is the shortest
complete example.

On the UE4 side the call must sit inside `#if defined(WITH_ROS2)` and be guarded
by `ROS2->IsEnabled()`; the transform passed is **relative to the attach parent**
when there is one:

```cpp
#if defined(WITH_ROS2)
auto ROS2 = carla::ros2::ROS2::GetInstance();
if (ROS2->IsEnabled()) {
  AActor* ParentActor = GetAttachParentActor();
  auto Transform = ParentActor ? GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform())
                               : GetActorTransform();
  ROS2->ProcessDataFromFoo(Stream.GetSensorType(), Transform, Data, this);
}
#endif
```

### Steps 7-8: verify

```bash
bash scripts/verify.sh                          # LibCarla + its tests, with --ros2
ROS2=1 bash ../../ue4/build-carla-ue4/scripts/06_build_editor.sh   # the plugin call site
# then, against a running ROS 2 server:
bash ../visualize-ros-rviz/scripts/ros_view.sh topics
bash ../visualize-ros-rviz/scripts/ros_view.sh hz /carla/<actor>/foo
```

`verify.sh` proves it compiles and the middleware layer still passes its tests.
**Only `hz`/`echo` proves the topic.** A publisher can build, register, and still
be silent: the most common reasons are the sensor not being enabled for ROS
([[create-sensor]] `--ros`), a UE4 call site that never runs, and a sensor whose
`Tick` is skipped because nothing listens.

## Sensors with no publisher yet

`ROS2::GetOrCreateSensor` deliberately returns `nullptr` for these, so they are
the natural targets — and each already has a `ProcessDataFrom*` or needs one:

| Sensor | Note |
|---|---|
| `other.lane_invasion` | `CarlaLineInvasion` message type already exists |
| `other.obstacle` | `ProcessDataFromObstacleDetection` exists in `ROS2.h` |
| `other.rss` | no message type yet |
| world observer | high volume; think before publishing |

## Adding a middleware (RMW)

Same layer, one level down: implement `IPublisherMiddleware` /
`ISubscriberMiddleware`, add the include + `case` in `MiddlewareFactory`, a
`Middleware` enum value and its `MiddlewareFromString` mapping, and a
`CARLA_ROS2_MIDDLEWARE_<NAME>` compile definition in
`LibCarla/cmake/ros2/CMakeLists.txt` (all three are defined at once, so
`IsMiddlewareAvailable` answers correctly at run time). Extend
`test_ros2_middleware.cpp`. Per-RMW quirks — Zenoh's router and wire format,
CycloneDDS's sertype, FastDDS's shared participant — are in
[`references/layers.md`](references/layers.md).

## Examples

**Example 1: publish the obstacle sensor**

User says: "make the obstacle detection sensor publish to ROS"

`CarlaLineInvasion`/a suitable type exists → write `CarlaObstaclePublisher` →
replace the `nullptr` case for `ESensors::ObstacleDetectionSensor` → fill in
`ProcessDataFromObstacleDetection` (declared already) → the call site in
`ObstacleDetectionSensor.cpp` already exists → verify → `hz`.

**Example 2: let ROS steer via a new command topic**

User says: "add a ROS topic to set the ego's target speed"

Type first ([[add-ros-message-type]]) → subscriber modelled on
`AckermannControlSubscriber` → register it in `RegisterVehicle` → extend
`ROS2CallbackData` and `ActorROS2Handler` to apply it → verify → `ros2 topic pub`.

**Example 3: a fourth RMW**

User says: "add support for another DDS implementation"

Follow "Adding a middleware": interface pair, factory case, enum + string, CMake
definition, test. No publisher changes at all — that is the point of the layer.

## Troubleshooting

**Problem: it all compiles but the topic never appears**
Cause: usually the call site, not the publisher — the `#if defined(WITH_ROS2)`
block is missing, unreachable, or the plugin was rebuilt without `ROS2=1`.
Solution: confirm `Ros2 ON` in `Config/OptionalModules.ini`, rebuild step 06 with
`ROS2=1`, then check the sensor is `enable_for_ros`-ed.

**Problem: `PublisherImpl: Failed to create middleware publisher`**
Cause: `MiddlewareFactory` has no case for the active middleware, or the RMW is
not compiled in.
Solution: check `IsMiddlewareAvailable` and the `CARLA_ROS2_MIDDLEWARE_*`
definitions in `LibCarla/cmake/ros2/CMakeLists.txt`.

**Problem: `Init failed for topic: …` in the server log, everything else fine**
Cause: an invalid topic name (empty `ros_name`, double slash) — Init only warns.
Solution: fix the name; remember unnamed actors become `actor<id>`.

**Problem: subscriber never receives, publisher works**
Cause: QoS mismatch (the peer is reliable+transient_local against a volatile
publisher), or the topic string differs by a suffix.
Solution: `visualize-ros-rviz info <topic>` shows endpoint counts and QoS on both
sides.

**Problem: a new `.cpp` is not compiled**
Cause: it is in a new subdirectory; the CMake glob lists directories explicitly.
Solution: add the glob line in `LibCarla/cmake/ros2/CMakeLists.txt`.

## Outputs

Edited sources in the checkout, a green `check.LibCarla`, and — after the plugin
rebuild — a new topic visible from ROS 2.

Layer-by-layer detail (publisher/subscriber contracts, QoS profiles, dispatch,
the middleware interfaces and their quirks) in
[`references/layers.md`](references/layers.md).
