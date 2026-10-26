// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/streaming/Server.h>
#include <carla/streaming/detail/Token.h>

#include <memory>
#include <utility>

#ifndef WITH_ROS2
#  define WITH_ROS2
#  define CARLA_TEST_UNDEFINE_WITH_ROS2
#endif
#define TRACE_CPUPROFILER_EVENT_SCOPE(...)
#define TRACE_CPUPROFILER_EVENT_SCOPE_STR(...)

// Unreal transforms, serialization and DDS calls are inert. Stream state is real.
namespace sensor_ros_fixture {

struct UWorld {};
enum ELevelTick { Tick_All };

struct FTransform {
  FTransform GetRelativeTransform(const FTransform &) const { return {}; }
};

struct AActor {
  FTransform GetActorTransform() const { return {}; }
};

struct Payload {
  void Reset() {}
};

namespace carla {
namespace streaming = ::carla::streaming;
namespace ros2 {

struct ROS2 {
  bool enabled = true;
  int published = 0;

  static std::shared_ptr<ROS2> GetInstance() {
    static auto instance = std::make_shared<ROS2>();
    return instance;
  }

  bool IsEnabled() const { return enabled; }

  template <class... T>
  void ProcessDataFromLidar(T &&...) { ++published; }

  template <class... T>
  void ProcessDataFromSemanticLidar(T &&...) { ++published; }

  template <class... T>
  void ProcessDataFromRadar(T &&...) { ++published; }
};

} // namespace ros2
} // namespace carla

struct GateState {
  carla::streaming::Server server{0};
  carla::streaming::Stream stream = server.MakeStream();
  carla::streaming::detail::stream_id_type id =
      carla::streaming::detail::token_type(stream.token()).get_stream_id();
  int serialized = 0;

  void Set(bool enabled) {
    if (enabled) {
      server.EnableForROS(id);
    } else {
      server.DisableForROS(id);
    }
  }

  bool Enabled() { return server.IsEnabledForROS(id); }
};

struct DataStreamFixture {
  GateState *state;
  FTransform GetSensorTransform() const { return {}; }
  unsigned GetSensorType() const { return 7; }
  int PopBufferFromPool() { return 0; }

  template <class... T>
  void SerializeAndSend(T &&...) { ++state->serialized; }
};

struct SensorFixture {
  explicit SensorFixture(GateState &value) : state(value) {}

  GateState &state;
  bool attached = false;
  AActor parent;
  Payload LidarData, SemanticLidarData, RadarData;
  struct {
    unsigned Channels = 32;
    float UpperFovLimit = 10.0f, LowerFovLimit = -30.0f;
  } Description;

  bool IsEnabledForROS() const { return state.Enabled(); }
  auto GetToken() const { return state.stream.token(); }

  template <class T>
  DataStreamFixture GetDataStream(T &) { return {&state}; }

  FTransform GetActorTransform() const { return {}; }
  AActor *GetAttachParentActor() { return attached ? &parent : nullptr; }
  void CalculateCurrentVelocity(float) {}
  void SendLineTraces(float) {}
};

struct ARayCastLidar : SensorFixture {
  using SensorFixture::SensorFixture;
  void SendData(float);
  void Run() { SendData(0.05f); }
};

struct ARayCastSemanticLidar : SensorFixture {
  using SensorFixture::SensorFixture;
  void SendData(float);
  void Run() { SendData(0.05f); }
};

struct AHSSLidar : SensorFixture {
  using SensorFixture::SensorFixture;
  void SendData(float);
  void Run() { SendData(0.05f); }
};

struct ARadar : SensorFixture {
  using SensorFixture::SensorFixture;
  void PostPhysTick(UWorld *, ELevelTick, float);
  void Run() { PostPhysTick(nullptr, Tick_All, 0.05f); }
};

#include "sensor_ros_methods.inc"

template <class T>
class SensorRosGate : public testing::Test {};

using Types = testing::Types<ARayCastLidar, ARayCastSemanticLidar, AHSSLidar, ARadar>;
TYPED_TEST_SUITE(SensorRosGate, Types);

TYPED_TEST(SensorRosGate, PerSensorDisableAndReenablePreserveNativeSending) {
  GateState state;
  TypeParam sensor(state);
  auto ros = carla::ros2::ROS2::GetInstance();
  ros->enabled = true;
  ros->published = 0;
  for (bool attached : {false, true}) {
    sensor.attached = attached;
    state.Set(true);
    sensor.Run();
    const int enabled_count = ros->published;
    EXPECT_GT(enabled_count, 0);
    state.Set(false);
    sensor.Run();
    EXPECT_EQ(ros->published, enabled_count);
    state.Set(true);
    sensor.Run();
    EXPECT_EQ(ros->published, enabled_count + 1);
  }
  EXPECT_EQ(state.serialized, 6);
}

TYPED_TEST(SensorRosGate, GlobalDisableRemainsEffective) {
  GateState state;
  TypeParam sensor(state);
  state.Set(true);
  auto ros = carla::ros2::ROS2::GetInstance();
  ros->enabled = false;
  ros->published = 0;
  sensor.Run();
  EXPECT_EQ(ros->published, 0);
  EXPECT_EQ(state.serialized, 1);
}

} // namespace sensor_ros_fixture

#undef TRACE_CPUPROFILER_EVENT_SCOPE
#undef TRACE_CPUPROFILER_EVENT_SCOPE_STR
#ifdef CARLA_TEST_UNDEFINE_WITH_ROS2
#  undef WITH_ROS2
#  undef CARLA_TEST_UNDEFINE_WITH_ROS2
#endif
