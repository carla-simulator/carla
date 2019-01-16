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
#include <carla/sensor/data/ActorDynamicState.h>
#include <compiler/enable-ue4-macros.h>

static auto AWorldObserver_GetActorState(const FActorView &View)
{
  using AType = FActorView::ActorType;

  carla::sensor::data::ActorDynamicState::TypeDependentState state;

  if (AType::Vehicle == View.GetActorType())
  {
    auto Vehicle = Cast<ACarlaWheeledVehicle>(View.GetActor());
    if (Vehicle != nullptr)
    {
      state.vehicle_control = carla::rpc::VehicleControl{Vehicle->GetVehicleControl()};
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
      state.traffic_light_state = static_cast<TLS>(TrafficLight->GetTrafficSignState());
    }
  }

  return state;
}

static carla::Buffer AWorldObserver_Serialize(
    carla::Buffer buffer,
    double game_timestamp,
    double platform_timestamp,
    const FActorRegistry &Registry)
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
  for (auto &&pair : Registry) {
    auto &&actor_view = pair.second;
    check(actor_view.GetActor() != nullptr);
    constexpr float TO_METERS = 1e-2;
    const auto velocity = TO_METERS * actor_view.GetActor()->GetVelocity();
    ActorDynamicState info = {
      actor_view.GetActorId(),
      actor_view.GetActor()->GetActorTransform(),
      carla::geom::Vector3D{velocity.X, velocity.Y, velocity.Z},
      AWorldObserver_GetActorState(actor_view)
    };
    write_data(info);
  }

  check(begin == buffer.end());
  return buffer;
}

AWorldObserver::AWorldObserver(const FObjectInitializer& ObjectInitializer)
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

  auto buffer = AWorldObserver_Serialize(
      Stream.PopBufferFromPool(),
      GameTimeStamp,
      FPlatformTime::Seconds(),
      Episode->GetActorRegistry());

  Stream.Send_GameThread(*this, std::move(buffer));
}
