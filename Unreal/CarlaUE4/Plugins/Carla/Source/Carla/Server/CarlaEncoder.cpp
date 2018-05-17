// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CarlaEncoder.h"

#include "Agent/TrafficSignAgentComponent.h"
#include "Agent/VehicleAgentComponent.h"
#include "Agent/WalkerAgentComponent.h"
#include "Game/CarlaPlayerState.h"
#include "Settings/SensorDescription.h"

#include "GameFramework/PlayerStart.h"

#include <cstring>

// Conversion from centimeters to meters.
static constexpr float TO_METERS = 1e-2;

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static void Encode(const FVector &Vector, carla_vector3d &Data)
{
  Data = {Vector.X, Vector.Y, Vector.Z};
}

static void Encode(const FRotator &Rotator, carla_rotation3d &Data)
{
  Data.pitch = Rotator.Pitch;
  Data.roll = Rotator.Roll;
  Data.yaw = Rotator.Yaw;
}

static void Encode(const FTransform &Transform, carla_transform &Data)
{
  Encode(Transform.GetLocation() * TO_METERS, Data.location);
  Encode(Transform.GetRotation().GetForwardVector(), Data.orientation);
  Encode(Transform.Rotator(), Data.rotation);
}

static TUniquePtr<const char[]> Encode(
    const USensorDescription &SensorDescription,
    carla_sensor_definition &Data)
{
  Data.id = SensorDescription.GetId();
  Data.type = [](const FString &Type) {
#define CARLA_CHECK_TYPE(Str) if (Type == TEXT(#Str)) return CARLA_SERVER_ ## Str;
      CARLA_CHECK_TYPE(CAMERA)
      CARLA_CHECK_TYPE(LIDAR_RAY_CAST)
      else return CARLA_SERVER_SENSOR_UNKNOWN;
#undef CARLA_CHECK_TYPE
  }(SensorDescription.Type);
  auto Memory = FCarlaEncoder::Encode(SensorDescription.Name);
  Data.name = Memory.Get();
  return Memory;
}

// =============================================================================
// -- FCarlaEncoder static methods ---------------------------------------------
// =============================================================================

TUniquePtr<const char[]> FCarlaEncoder::Encode(const FString &String)
{
  const char *Ptr = TCHAR_TO_UTF8(*String);
  auto Buffer = MakeUnique<char[]>(std::strlen(Ptr) + 1u); // + null terminator.
#if defined(_WIN32)
  strcpy_s(Buffer.Get(),String.Len()+1, Ptr);
#else
  std::strcpy(Buffer.Get(), Ptr);
#endif
  return TUniquePtr<const char[]>(Buffer.Release());
}

void FCarlaEncoder::Encode(
    const TArray<APlayerStart *> &AvailableStartSpots,
    TArray<carla_transform> &Data)
{
  const int32 NumberOfStartSpots = AvailableStartSpots.Num();
  Data.AddUninitialized(NumberOfStartSpots);
  for (auto i = 0; i < NumberOfStartSpots; ++i) {
    check(AvailableStartSpots[i] != nullptr);
    ::Encode(AvailableStartSpots[i]->GetActorTransform(), Data[i]);
  }
}

void FCarlaEncoder::Encode(
    const TArray<USensorDescription *> &SensorDescriptions,
    TArray<carla_sensor_definition> &Data,
    TArray<TUniquePtr<const char[]>> &SensorNames)
{
  const int32 NumberOfSensors = SensorDescriptions.Num();
  Data.AddUninitialized(NumberOfSensors);
  SensorNames.Reserve(NumberOfSensors);
  for (auto i = 0; i < NumberOfSensors; ++i) {
    check(SensorDescriptions[i] != nullptr);
    SensorNames.Emplace(::Encode(*SensorDescriptions[i], Data[i]));
  }
}

void FCarlaEncoder::Encode(
    const ACarlaPlayerState &PlayerState,
    carla_measurements &Data)
{
  Data.frame_number = PlayerState.GetFrameNumber();
  Data.platform_timestamp = PlayerState.GetPlatformTimeStamp();
  Data.game_timestamp = PlayerState.GetGameTimeStamp();
  auto &Player = Data.player_measurements;
  ::Encode(PlayerState.GetTransform(), Player.transform);
  ::Encode(PlayerState.GetBoundingBoxTransform(), Player.bounding_box.transform);
  ::Encode(PlayerState.GetBoundingBoxExtent() * TO_METERS, Player.bounding_box.extent);
  ::Encode(PlayerState.GetAcceleration() * TO_METERS, Player.acceleration);
  Player.forward_speed = PlayerState.GetForwardSpeed() * TO_METERS;
  Player.collision_vehicles = PlayerState.GetCollisionIntensityCars() * TO_METERS;
  Player.collision_pedestrians = PlayerState.GetCollisionIntensityPedestrians() * TO_METERS;
  Player.collision_other = PlayerState.GetCollisionIntensityOther() * TO_METERS;
  Player.intersection_otherlane = PlayerState.GetOtherLaneIntersectionFactor();
  Player.intersection_offroad = PlayerState.GetOffRoadIntersectionFactor();
  Player.autopilot_control.steer = PlayerState.GetSteer();
  Player.autopilot_control.throttle = PlayerState.GetThrottle();
  Player.autopilot_control.brake = PlayerState.GetBrake();
  Player.autopilot_control.hand_brake = PlayerState.GetHandBrake();
  Player.autopilot_control.reverse = PlayerState.GetCurrentGear() < 0;
}

void FCarlaEncoder::Encode(
    const TArray<const UAgentComponent *> &Agents,
    TArray<carla_agent> &Data)
{
  const int32 NumberOfAgents = Agents.Num();
  Data.AddUninitialized(NumberOfAgents);
  for (auto i = 0; i < NumberOfAgents; ++i) {
    check(Agents[i] != nullptr);
    Encode(*Agents[i], Data[i]);
  }
}

void FCarlaEncoder::Encode(const UAgentComponent &AgentComponent, carla_agent &AgentData)
{
  AgentData.id = AgentComponent.GetId();
  FCarlaEncoder Encoder(AgentData);
  AgentComponent.AcceptVisitor(Encoder);
}

// =============================================================================
// -- FCarlaEncoder ------------------------------------------------------------
// =============================================================================

FCarlaEncoder::FCarlaEncoder(carla_agent &InData) : Data(InData) {}

void FCarlaEncoder::Visit(const UTrafficSignAgentComponent &Agent)
{
  constexpr float TO_METERS_PER_SECOND = 1.0f / 3.6f;
  ::Encode(Agent.GetComponentTransform(), Data.transform);
  auto &TrafficSign = Agent.GetTrafficSign();
  switch (TrafficSign.GetTrafficSignState()) {
    case ETrafficSignState::TrafficLightRed:
      Data.type = CARLA_SERVER_AGENT_TRAFFICLIGHT_RED;
      break;
    case ETrafficSignState::TrafficLightYellow:
      Data.type = CARLA_SERVER_AGENT_TRAFFICLIGHT_YELLOW;
      break;
    case ETrafficSignState::TrafficLightGreen:
      Data.type = CARLA_SERVER_AGENT_TRAFFICLIGHT_GREEN;
      break;
    case ETrafficSignState::SpeedLimit_30:
      Data.type = CARLA_SERVER_AGENT_SPEEDLIMITSIGN;
      Data.forward_speed = 30.0f * TO_METERS_PER_SECOND;
      break;
    case ETrafficSignState::SpeedLimit_40:
      Data.type = CARLA_SERVER_AGENT_SPEEDLIMITSIGN;
      Data.forward_speed = 40.0f * TO_METERS_PER_SECOND;
      break;
    case ETrafficSignState::SpeedLimit_50:
      Data.type = CARLA_SERVER_AGENT_SPEEDLIMITSIGN;
      Data.forward_speed = 50.0f * TO_METERS_PER_SECOND;
      break;
    case ETrafficSignState::SpeedLimit_60:
      Data.type = CARLA_SERVER_AGENT_SPEEDLIMITSIGN;
      Data.forward_speed = 60.0f * TO_METERS_PER_SECOND;
      break;
    case ETrafficSignState::SpeedLimit_90:
      Data.type = CARLA_SERVER_AGENT_SPEEDLIMITSIGN;
      Data.forward_speed = 90.0f * TO_METERS_PER_SECOND;
      break;
    case ETrafficSignState::SpeedLimit_100:
      Data.type = CARLA_SERVER_AGENT_SPEEDLIMITSIGN;
      Data.forward_speed = 100.0f * TO_METERS_PER_SECOND;
      break;
    case ETrafficSignState::SpeedLimit_120:
      Data.type = CARLA_SERVER_AGENT_SPEEDLIMITSIGN;
      Data.forward_speed = 120.0f * TO_METERS_PER_SECOND;
      break;
    case ETrafficSignState::SpeedLimit_130:
      Data.type = CARLA_SERVER_AGENT_SPEEDLIMITSIGN;
      Data.forward_speed = 130.0f * TO_METERS_PER_SECOND;
      break;
    default:
      UE_LOG(LogCarla, Error, TEXT("Unknown traffic sign!"));
  }
}

void FCarlaEncoder::Visit(const UVehicleAgentComponent &Agent)
{
  ::Encode(Agent.GetTransform(), Data.transform);
  Data.type = CARLA_SERVER_AGENT_VEHICLE;
  Data.forward_speed = Agent.GetForwardSpeed() * TO_METERS;
  ::Encode(Agent.GetBoundingBoxTransform(), Data.bounding_box.transform);
  ::Encode(Agent.GetBoundingBoxExtent() * TO_METERS, Data.bounding_box.extent);
}

void FCarlaEncoder::Visit(const UWalkerAgentComponent &Agent)
{
  ::Encode(Agent.GetTransform(), Data.transform);
  Data.type = CARLA_SERVER_AGENT_PEDESTRIAN;
  Data.forward_speed = Agent.GetForwardSpeed() * TO_METERS;
  ::Encode(Agent.GetBoundingBoxTransform(), Data.bounding_box.transform);
  ::Encode(Agent.GetBoundingBoxExtent() * TO_METERS, Data.bounding_box.extent);
}
