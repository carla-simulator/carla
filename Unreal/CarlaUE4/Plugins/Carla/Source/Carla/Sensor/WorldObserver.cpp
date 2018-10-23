// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/WorldObserver.h"

#include "Carla/Traffic/TrafficLightBase.h"

#include "CoreGlobals.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>

static uint8 AWorldObserver_GetActorState(const FActorView &View)
{
  if (View.IsTrafficLight())
  {
    auto TrafficLight = Cast<ATrafficLightBase>(View.GetActor());
    if (TrafficLight != nullptr)
    {
      return static_cast<uint8>(TrafficLight->GetTrafficSignState());
    }
  }
  return 0u;
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
    constexpr float TO_METERS = 1e-3;
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
