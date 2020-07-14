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
#include "Carla/Traffic/TrafficSignBase.h"
#include "Carla/Traffic/SignComponent.h"
#include "Carla/Walker/WalkerController.h"

#include "CoreGlobals.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/String.h>
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
      auto* TrafficLightComponent =
          TrafficLight->GetTrafficLightComponent();

      using TLS = carla::rpc::TrafficLightState;

      if(TrafficLightComponent == nullptr)
      {
        // Old way: traffic lights are actors
        state.traffic_light_data.sign_id[0] = '\0';
        state.traffic_light_data.state = static_cast<TLS>(TrafficLight->GetTrafficLightState());
        state.traffic_light_data.green_time = TrafficLight->GetGreenTime();
        state.traffic_light_data.yellow_time = TrafficLight->GetYellowTime();
        state.traffic_light_data.red_time = TrafficLight->GetRedTime();
        state.traffic_light_data.elapsed_time = TrafficLight->GetElapsedTime();
        state.traffic_light_data.time_is_frozen = TrafficLight->GetTimeIsFrozen();
        state.traffic_light_data.pole_index = TrafficLight->GetPoleIndex();
      }
      else
      {
        const UTrafficLightController* Controller =  TrafficLightComponent->GetController();
        const ATrafficLightGroup* Group = TrafficLightComponent->GetGroup();

        if (!Controller)
        {
          UE_LOG(LogCarla, Error, TEXT("TrafficLightComponent doesn't have any Controller assigned"));
        }
        else if (!Group)
        {
          UE_LOG(LogCarla, Error, TEXT("TrafficLightComponent doesn't have any Group assigned"));
        }
        else
        {
          const FString fstring_sign_id = TrafficLightComponent->GetSignId();
          const std::string sign_id = carla::rpc::FromFString(fstring_sign_id);
          constexpr size_t max_size = sizeof(state.traffic_light_data.sign_id);
          size_t sign_id_length = sign_id.length();
          if(max_size < sign_id_length)
          {
            UE_LOG(LogCarla, Warning, TEXT("The max size of a signal id is 32. %s (%d)"), *fstring_sign_id, sign_id.length());
            sign_id_length = max_size;
          }
          std::memset(state.traffic_light_data.sign_id, '\0', max_size);
          std::memcpy(state.traffic_light_data.sign_id, sign_id.c_str(), sign_id_length);
          state.traffic_light_data.state = static_cast<TLS>(TrafficLightComponent->GetLightState());
          state.traffic_light_data.green_time = Controller->GetGreenTime();
          state.traffic_light_data.yellow_time = Controller->GetYellowTime();
          state.traffic_light_data.red_time = Controller->GetRedTime();
          state.traffic_light_data.elapsed_time = Controller->GetElapsedTime();
          state.traffic_light_data.time_is_frozen = Group->IsFrozen();
          state.traffic_light_data.pole_index = TrafficLight->GetPoleIndex();
        }
      }
    }
  }
  else if (AType::TrafficSign == View.GetActorType())
  {
    auto TrafficSign = Cast<ATrafficSignBase>(View.GetActor());
    if (TrafficSign != nullptr)
    {
      USignComponent* TrafficSignComponent =
        Cast<USignComponent>(TrafficSign->FindComponentByClass<USignComponent>());

      if(TrafficSignComponent)
      {
        const FString fstring_sign_id = TrafficSignComponent->GetSignId();
        const std::string sign_id = carla::rpc::FromFString(fstring_sign_id);
        constexpr size_t max_size = sizeof(state.traffic_sign_data.sign_id);
        size_t sign_id_length = sign_id.length();
        if(max_size < sign_id_length)
        {
          UE_LOG(LogCarla, Warning, TEXT("The max size of a signal id is 32. %s (%d)"), *fstring_sign_id, sign_id.length());
          sign_id_length = max_size;
        }
        std::memset(state.traffic_light_data.sign_id, '\0', max_size);
        std::memcpy(state.traffic_sign_data.sign_id, sign_id.c_str(), sign_id_length);
      }
    }
  }
  return state;
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
    float DeltaSeconds,
    bool MapChange,
    bool PendingLightUpdates)
{

  using Serializer = carla::sensor::s11n::EpisodeStateSerializer;
  using SimulationState = carla::sensor::s11n::EpisodeStateSerializer::SimulationState;
  using ActorDynamicState = carla::sensor::data::ActorDynamicState;


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

  uint8_t simulation_state = (SimulationState::MapChange * MapChange);
  simulation_state |= (SimulationState::PendingLightUpdate * PendingLightUpdates);

  header.simulation_state = static_cast<SimulationState>(simulation_state);

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
      FWorldObserver_GetActorState(View, Registry),
    };
    write_data(info);
  }

  // Shrink buffer
  buffer.resize(current_size);

  check(buffer.size() == current_size);

  return std::move(buffer);
}

void FWorldObserver::BroadcastTick(
    const UCarlaEpisode &Episode,
    float DeltaSecond,
    bool MapChange,
    bool PendingLightUpdates)
{
  auto AsyncStream = Stream.MakeAsyncDataStream(*this, Episode.GetElapsedGameTime());

  auto buffer = FWorldObserver_Serialize(
      AsyncStream.PopBufferFromPool(),
      Episode,
      DeltaSecond,
      MapChange,
      PendingLightUpdates);

  AsyncStream.Send(*this, std::move(buffer));
}
