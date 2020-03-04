// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/WorldObserver.h"

#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Traffic/TrafficLightComponent.h"
#include "Carla/Traffic/TrafficLightController.h"
#include "Carla/Traffic/TrafficLightGroup.h"
#include "Carla/Walker/WalkerController.h"

#include "CoreGlobals.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/data/ActorDynamicState.h>
#include <compiler/enable-ue4-macros.h>

static auto FWorldObserver_GetActorState(const FActorView &View, const FActorRegistry &Registry)
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

      UActorComponent* TrafficLightComponent = TrafficLight->FindComponentByClass<UTrafficLightComponent>();

      if(TrafficLightComponent == nullptr) {
        // Old way: traffic lights are actors
        using TLS = carla::rpc::TrafficLightState;
        state.traffic_light_data.state = static_cast<TLS>(TrafficLight->GetTrafficLightState());
        state.traffic_light_data.green_time = TrafficLight->GetGreenTime();
        state.traffic_light_data.yellow_time = TrafficLight->GetYellowTime();
        state.traffic_light_data.red_time = TrafficLight->GetRedTime();
        state.traffic_light_data.elapsed_time = TrafficLight->GetElapsedTime();
        state.traffic_light_data.time_is_frozen = TrafficLight->GetTimeIsFrozen();
        state.traffic_light_data.pole_index = TrafficLight->GetPoleIndex();
      }
    }
  }

  return state;
}

static void FWorldObserver_GetActorComponentsState(
  const FActorView &View,
  const FActorRegistry &Registry,
  TArray<carla::sensor::data::ComponentDynamicState>& Out)
{
  using AType = FActorView::ActorType;

  if (AType::TrafficLight == View.GetActorType())
  {
    auto TrafficLightActor = Cast<ATrafficLightBase>(View.GetActor());
    if (TrafficLightActor == nullptr)
    {
      return;
    }

    TArray<UTrafficLightComponent*> TrafficLights;
    TrafficLightActor->GetComponents<UTrafficLightComponent>(TrafficLights);

    for(auto& TrafficLight : TrafficLights)
    {
      using TLS = carla::rpc::TrafficLightState;

      UTrafficLightController* Controller =  TrafficLight->GetController();
      ATrafficLightGroup* Group = TrafficLight->GetGroup();

      carla::sensor::data::ComponentDynamicState CompState;
      CompState.transform = TrafficLight->GetComponentTransform();
      CompState.state.traffic_light_data.state = static_cast<TLS>(TrafficLight->GetLightState());
      CompState.state.traffic_light_data.green_time = Controller->GetGreenTime();
      CompState.state.traffic_light_data.yellow_time = Controller->GetYellowTime();
      CompState.state.traffic_light_data.red_time = Controller->GetRedTime();
      CompState.state.traffic_light_data.elapsed_time = Group->GetElapsedTime();
      CompState.state.traffic_light_data.time_is_frozen = Group->IsFrozen();
      // Nobody is using this right now, perhaps we should remove it?
      CompState.state.traffic_light_data.pole_index = 0;

      Out.Push(CompState);
    }
  }
}

static carla::geom::Vector3D FWorldObserver_GetAngularVelocity(const AActor &Actor)
{
  const auto RootComponent = Cast<UPrimitiveComponent>(Actor.GetRootComponent());
  const FVector AngularVelocity =
      RootComponent != nullptr ?
          RootComponent->GetPhysicsAngularVelocityInDegrees() :
          FVector{0.0f, 0.0f, 0.0f};
  return {AngularVelocity.X, AngularVelocity.Y, AngularVelocity.Z};
}

static carla::geom::Vector3D FWorldObserver_GetAcceleration(
    const FActorView &View,
    const FVector &Velocity,
    const float DeltaSeconds)
{
  FVector &PreviousVelocity = View.GetActorInfo()->Velocity;
  const FVector Acceleration = (Velocity - PreviousVelocity) / DeltaSeconds;
  PreviousVelocity = Velocity;
  return {Acceleration.X, Acceleration.Y, Acceleration.Z};
}

static carla::Buffer FWorldObserver_Serialize(
    carla::Buffer &&buffer,
    const UCarlaEpisode &Episode,
    float DeltaSeconds)
{

  using Serializer = carla::sensor::s11n::EpisodeStateSerializer;
  using ActorDynamicState = carla::sensor::data::ActorDynamicState;
  using ComponentDynamicState = carla::sensor::data::ComponentDynamicState;

  const auto &Registry = Episode.GetActorRegistry();

  auto total_size = sizeof(Serializer::Header) + sizeof(ActorDynamicState) * Registry.Num();
  auto current_size = 0;
  // Set up buffer for writing.
  buffer.reset(total_size);
  auto write_data = [&current_size, &buffer](const auto &data)
  {
    auto begin = buffer.begin() + current_size;
    std::memcpy(begin, &data, sizeof(data));
    current_size += sizeof(data);
  };

  // Write header.
  Serializer::Header header;
  header.episode_id = Episode.GetId();
  header.platform_timestamp = FPlatformTime::Seconds();
  header.delta_seconds = DeltaSeconds;
  write_data(header);

  // Write every actor.
  for (auto &&View : Registry)
  {
    check(View.IsValid());
    constexpr float TO_METERS = 1e-2;
    const auto Velocity = TO_METERS * View.GetActor()->GetVelocity();

    ActorDynamicState info = {
      View.GetActorId(),
      View.GetActor()->GetActorTransform(),
      carla::geom::Vector3D{Velocity.X, Velocity.Y, Velocity.Z},
      FWorldObserver_GetAngularVelocity(*View.GetActor()),
      FWorldObserver_GetAcceleration(View, Velocity, DeltaSeconds),
      0,
      FWorldObserver_GetActorState(View, Registry),
    };

    TArray<ComponentDynamicState> ComponentsState;
    FWorldObserver_GetActorComponentsState(View, Registry, ComponentsState);
    info.num_components = ComponentsState.Num();

    write_data(info);

    if(info.num_components > 0) {
      total_size += info.num_components * sizeof(ComponentDynamicState);
      buffer.resize(total_size);

      for(auto& CompState : ComponentsState)
      {
        write_data(CompState);
      }
    }
  }

  // Shrink buffer
  buffer.resize(current_size);

  check(buffer.size() == current_size);

  return std::move(buffer);
}

void FWorldObserver::BroadcastTick(const UCarlaEpisode &Episode, float DeltaSeconds)
{
  auto AsyncStream = Stream.MakeAsyncDataStream(*this, Episode.GetElapsedGameTime());

  auto buffer = FWorldObserver_Serialize(
      AsyncStream.PopBufferFromPool(),
      Episode,
      DeltaSeconds);

  AsyncStream.Send(*this, std::move(buffer));
}
