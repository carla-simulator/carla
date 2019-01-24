// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/WorldObserver.h"

#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Walker/WalkerController.h"

#include "CoreGlobals.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/recorder/Recorder.h>
#include <carla/recorder/RecorderPosition.h>
#include <carla/sensor/data/ActorDynamicState.h>
#include <compiler/enable-ue4-macros.h>

static auto AWorldObserver_GetActorState(const FActorView &View, const FActorRegistry &Registry)
{
  using AType = FActorView::ActorType;

  carla::sensor::data::ActorDynamicState::TypeDependentState state;

  if (AType::Vehicle == View.GetActorType())
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(View.GetActor());
    if (Vehicle != nullptr)
    {
      state.vehicle_data.control = carla::rpc::VehicleControl{Vehicle->GetVehicleControl()};
      auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
      if (Controller != nullptr)
      {
        using TLS = carla::rpc::TrafficLightState;
        state.vehicle_data.traffic_light_state = static_cast<TLS>(Controller->GetTrafficLightState());
        state.vehicle_data.speed_limit = Controller->GetSpeedLimit();
        auto TrafficLight = Controller->GetTrafficLight();
        if (TrafficLight != nullptr)
        {
          state.vehicle_data.has_traffic_light = true;
          auto TrafficLightView = Registry.Find(TrafficLight);
          state.vehicle_data.traffic_light_id = TrafficLightView.GetActorId();
        }
        else
        {
          state.vehicle_data.has_traffic_light = false;
        }
      }
    }
  }

  else if (AType::Walker == View.GetActorType())
  {
    auto Walker = Cast<APawn>(View.GetActor());
    auto Controller = Walker != nullptr ? Cast<AWalkerController>(Walker->GetController()) : nullptr;
    if (Controller != nullptr)
    {
      state.walker_control = carla::rpc::WalkerControl{Controller->GetWalkerControl()};
    }
  }
  else if (AType::TrafficLight == View.GetActorType())
  {
    auto TrafficLight = Cast<ATrafficLightBase>(View.GetActor());
    if (TrafficLight != nullptr)
    {
      using TLS = carla::rpc::TrafficLightState;
      state.traffic_light_data.state = static_cast<TLS>(TrafficLight->GetTrafficLightState());
      state.traffic_light_data.green_time = TrafficLight->GetGreenTime();
      state.traffic_light_data.yellow_time = TrafficLight->GetYellowTime();
      state.traffic_light_data.red_time = TrafficLight->GetRedTime();
      state.traffic_light_data.elapsed_time = TrafficLight->GetElapsedTime();
      state.traffic_light_data.time_is_frozen = TrafficLight->GetTimeIsFrozen();
    }
  }

  return state;
}

static carla::Buffer AWorldObserver_Serialize(
    carla::Buffer buffer,
    double game_timestamp,
    double platform_timestamp,
    const FActorRegistry &Registry,
    UCarlaEpisode *Episode)
{
  using Serializer = carla::sensor::s11n::EpisodeStateSerializer;
  using ActorDynamicState = carla::sensor::data::ActorDynamicState;

  // Set up buffer for writing.
  buffer.reset(sizeof(Serializer::Header) + sizeof(ActorDynamicState) * Registry.Num());
  auto begin = buffer.begin();
  auto write_data = [&begin](const auto &data) {
    std::memcpy(begin, &data, sizeof(data));
    begin += sizeof(data);
  };

  // Write header.
  Serializer::Header header = {game_timestamp, platform_timestamp};
  write_data(header);

  // Write every actor.
  for (auto &&View : Registry)
  {
    check(View.IsValid());
    constexpr float TO_METERS = 1e-2;
    const auto velocity = TO_METERS * View.GetActor()->GetVelocity();
    // get the angular velocity
    const auto RootComponent = Cast<UPrimitiveComponent>(View.GetActor()->GetRootComponent());
    FVector angularVelocity { 0.0f, 0.0f, 0.0f };
    if (RootComponent != nullptr)
    {
      angularVelocity = RootComponent->GetPhysicsAngularVelocityInDegrees();
    }
    ActorDynamicState info = {
      View.GetActorId(),
      View.GetActor()->GetActorTransform(),
      carla::geom::Vector3D{velocity.X, velocity.Y, velocity.Z},
      carla::geom::Vector3D{angularVelocity.X, angularVelocity.Y, angularVelocity.Z},
      AWorldObserver_GetActorState(View, Registry)
    };
    write_data(info);
  
    if (Episode->GetRecorder().isEnabled()) {
      // add to the recorder
      carla::recorder::RecorderPosition recPos { 
        actor_view.GetActorId(),
        info.transform,
        info.velocity,
        // TODO: use the angular velocity
        info.velocity
      };
      Episode->GetRecorder().addPosition(recPos);
    }
  }

  check(begin == buffer.end());
  return buffer;
}

AWorldObserver::AWorldObserver(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AWorldObserver::Tick(float DeltaSeconds)
{
  check(Episode != nullptr);
  Super::Tick(DeltaSeconds);

  GameTimeStamp += DeltaSeconds;

  auto AsyncStream = Stream.MakeAsyncDataStream(*this);

  auto buffer = AWorldObserver_Serialize(
      AsyncStream.PopBufferFromPool(),
      GameTimeStamp,
      FPlatformTime::Seconds(),
      Episode->GetActorRegistry(),
      Episode);

  Stream.Send_GameThread(*this, std::move(buffer));

  // recorder
  if (Episode->GetRecorder().isEnabled())
    Episode->GetRecorder().write();
  
  // replayer
  if (Episode->GetReplayer().isEnabled())
    Episode->GetReplayer().tick(DeltaSeconds);
}
