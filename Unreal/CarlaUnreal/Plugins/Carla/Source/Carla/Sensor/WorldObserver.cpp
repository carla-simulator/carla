// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/WorldObserver.h"
#include "Carla.h"
#include "Carla/Actor/ActorData.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Traffic/TrafficLightComponent.h"
#include "Carla/Traffic/TrafficLightController.h"
#include "Carla/Traffic/TrafficLightGroup.h"
#include "Carla/Traffic/TrafficSignBase.h"
#include "Carla/Traffic/SignComponent.h"
#include "Carla/Walker/WalkerController.h"

#include <util/disable-ue4-macros.h>
#include <carla/rpc/String.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/data/ActorDynamicState.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreGlobals.h"
#include <util/ue-header-guard-end.h>

static auto FWorldObserver_GetActorState(const FCarlaActor &View, const FActorRegistry &Registry)
{
  using AType = FCarlaActor::ActorType;

  carla::sensor::data::ActorDynamicState::TypeDependentState state{};

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
          auto* TrafficLightView = Registry.FindCarlaActor(TrafficLight);
          if(TrafficLightView)
          {
            state.vehicle_data.traffic_light_id = TrafficLightView->GetActorId();
          }
          else
          {
            state.vehicle_data.has_traffic_light = false;
          }
        }
        else
        {
          state.vehicle_data.has_traffic_light = false;
        }
      }
      // Get the failure state by checking the rollover one as it is the only one currently implemented.
      // This will have to be expanded once more states are added
      state.vehicle_data.failure_state = Vehicle->GetFailureState();
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

static auto FWorldObserver_GetDormantActorState(const FCarlaActor &View, const FActorRegistry &Registry)
{
  using AType = FCarlaActor::ActorType;

  carla::sensor::data::ActorDynamicState::TypeDependentState state{};

  if (AType::Vehicle == View.GetActorType())
  {
      const FVehicleData* ActorData = View.GetActorData<FVehicleData>();
      state.vehicle_data.control = carla::rpc::VehicleControl{ActorData->Control};
      using TLS = carla::rpc::TrafficLightState;
      state.vehicle_data.traffic_light_state = TLS::Green;
      state.vehicle_data.speed_limit = ActorData->SpeedLimit;
      state.vehicle_data.has_traffic_light = false;
  }
  else if (AType::Walker == View.GetActorType())
  {
    const FWalkerData* ActorData = View.GetActorData<FWalkerData>();
    // auto Walker = Cast<APawn>(View.GetActor());
    // auto Controller = Walker != nullptr ? Cast<AWalkerController>(Walker->GetController()) : nullptr;
    // if (Controller != nullptr)
    // {
    //   state.walker_control = carla::rpc::WalkerControl{Controller->GetWalkerControl()};
    // }
    state.walker_control = ActorData->WalkerControl;
  }
  else if (AType::TrafficLight == View.GetActorType())
  {
    const FTrafficLightData* ActorData = View.GetActorData<FTrafficLightData>();
    const UTrafficLightController* Controller = ActorData->Controller;
    if(Controller)
    {
      using TLS = carla::rpc::TrafficLightState;
      const ATrafficLightGroup* Group = Controller->GetGroup();
      if(!Group)
      {
        UE_LOG(LogCarla, Error, TEXT("TrafficLight doesn't have any Group assigned"));
      }
      else
      {
        const FString fstring_sign_id = ActorData->SignId;
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
        state.traffic_light_data.state = static_cast<TLS>(Controller->GetCurrentState().State);
        state.traffic_light_data.green_time = Controller->GetGreenTime();
        state.traffic_light_data.yellow_time = Controller->GetYellowTime();
        state.traffic_light_data.red_time = Controller->GetRedTime();
        state.traffic_light_data.elapsed_time = Controller->GetElapsedTime();
        state.traffic_light_data.time_is_frozen = Group->IsFrozen();
        state.traffic_light_data.pole_index = ActorData->PoleIndex;
      }
    }
  }
  else if (AType::TrafficSign == View.GetActorType())
  {
    const FTrafficSignData* ActorData = View.GetActorData<FTrafficSignData>();
    const FString fstring_sign_id = ActorData->SignId;
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
  return state;
}

static carla::geom::Vector3D FWorldObserver_GetAngularVelocity(const AActor &Actor)
{
  const auto RootComponent = Cast<UPrimitiveComponent>(Actor.GetRootComponent());
  const FVector AngularVelocity =
      RootComponent != nullptr ?
          RootComponent->GetPhysicsAngularVelocityInDegrees() :
          FVector{0.0f, 0.0f, 0.0f};
  return
  {
      (float)AngularVelocity.X,
      (float)AngularVelocity.Y,
      (float)AngularVelocity.Z
  };
}

static carla::geom::Vector3D FWorldObserver_GetAcceleration(
    const FCarlaActor &View,
    const FVector &Velocity,
    const float DeltaSeconds)
{
  FVector &PreviousVelocity = View.GetActorInfo()->Velocity;
  const FVector Acceleration = (Velocity - PreviousVelocity) / DeltaSeconds;
  PreviousVelocity = Velocity;
  return
  {
      (float)Acceleration.X,
      (float)Acceleration.Y,
      (float)Acceleration.Z
  };
}

static carla::Buffer FWorldObserver_Serialize(
    carla::Buffer &&buffer,
    const UCarlaEpisode &Episode,
    float DeltaSeconds,
    bool MapChange,
    bool PendingLightUpdates)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  using Serializer = carla::sensor::s11n::EpisodeStateSerializer;
  using SimulationState = carla::sensor::s11n::EpisodeStateSerializer::SimulationState;
  using ActorDynamicState = carla::sensor::data::ActorDynamicState;


  const FActorRegistry &Registry = Episode.GetActorRegistry();

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

  constexpr float TO_METERS = 1e-2;

  // Write header.
  Serializer::Header header;
  header.episode_id = Episode.GetId();
  header.platform_timestamp = FPlatformTime::Seconds();
  header.delta_seconds = DeltaSeconds;
  FIntVector MapOrigin = Episode.GetCurrentMapOrigin();
  FIntVector MapOriginInMeters = MapOrigin / 100;
  header.map_origin = carla::geom::Vector3DInt{ MapOriginInMeters.X, MapOriginInMeters.Y, MapOriginInMeters.Z };

  uint8_t simulation_state = (SimulationState::MapChange * MapChange);
  simulation_state |= (SimulationState::PendingLightUpdate * PendingLightUpdates);

  header.simulation_state = static_cast<SimulationState>(simulation_state);

  write_data(header);

  // Write every actor.
  for (auto& It : Registry)
  {
    const FCarlaActor* View = It.Value.Get();
    const FActorInfo* ActorInfo = View->GetActorInfo();

    FTransform ActorTransform;
    FVector Velocity(0.0f);
    carla::geom::Vector3D AngularVelocity(0.0f, 0.0f, 0.0f);
    carla::geom::Vector3D Acceleration(0.0f, 0.0f, 0.0f);
    carla::sensor::data::ActorDynamicState::TypeDependentState State{};

    check(View);

    if(View->IsDormant())
    {
      const FActorData* ActorData = View->GetActorData();
      Velocity = TO_METERS * ActorData->Velocity;
      AngularVelocity = carla::geom::Vector3D
      {
          (float)ActorData->AngularVelocity.X,
          (float)ActorData->AngularVelocity.Y,
          (float)ActorData->AngularVelocity.Z
      };
      Acceleration = FWorldObserver_GetAcceleration(*View, Velocity, DeltaSeconds);
      State = FWorldObserver_GetDormantActorState(*View, Registry);
    }
    else
    {
      Velocity = TO_METERS * View->GetActor()->GetVelocity();
      AngularVelocity = FWorldObserver_GetAngularVelocity(*View->GetActor());
      Acceleration = FWorldObserver_GetAcceleration(*View, Velocity, DeltaSeconds);
      State = FWorldObserver_GetActorState(*View, Registry);
    }
    ActorTransform = View->GetActorGlobalTransform();

    ActorDynamicState info = {
      View->GetActorId(),
      View->GetActorState(),
      carla::geom::Transform(ActorTransform),
      carla::geom::Vector3D(Velocity.X, Velocity.Y, Velocity.Z),
      AngularVelocity,
      Acceleration,
      State
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
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  if (!Stream.IsStreamReady())
    return;

  auto AsyncStream = Stream.MakeAsyncDataStream(*this, Episode.GetElapsedGameTime());

  carla::Buffer buffer = FWorldObserver_Serialize(
      AsyncStream.PopBufferFromPool(),
      Episode,
      DeltaSecond,
      MapChange,
      PendingLightUpdates);

  AsyncStream.SerializeAndSend(*this, std::move(buffer));
}
