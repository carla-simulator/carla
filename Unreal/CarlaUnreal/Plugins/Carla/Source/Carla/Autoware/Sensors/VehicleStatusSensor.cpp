// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Autoware/Sensors/VehicleStatusSensor.h"

#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vehicle/VehicleLightState.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/World.h"
#include "TimerManager.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <util/enable-ue4-macros.h>

AVehicleStatusSensor::AVehicleStatusSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PostPhysics;  // read ground-truth after physics update

  TargetRateHz = FMath::Clamp(TargetRateHz, MinRateHz, MaxRateHz);
  PrimaryActorTick.TickInterval = 1.0f / TargetRateHz;
}

FActorDefinition AVehicleStatusSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeVehicleStatusDefinition();
}

void AVehicleStatusSensor::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
}

void AVehicleStatusSensor::BeginPlay()
{
  Super::BeginPlay();
  UE_LOG(LogCarla, Log, TEXT("VehicleStatusSensor spawned."));
  GetWorldTimerManager().SetTimer(CheckParentTimerHandle, this, &AVehicleStatusSensor::CheckForParentVehicle, 1.0f, true);
}

void AVehicleStatusSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  Super::PostPhysTick(World, TickType, DeltaSeconds);
  CollectAndStream(DeltaSeconds);
}

void AVehicleStatusSensor::CheckForParentVehicle()
{
  if (OwningVehicle = FindAttachmentParentVehicle(); OwningVehicle.IsValid())
  {
    UE_LOG(LogCarla, Log, TEXT("VehicleStatusSensor attached to a vehicle: %s"), *OwningVehicle->GetName());
    GetWorldTimerManager().ClearTimer(CheckParentTimerHandle);
  }
}

TObjectPtr<ACarlaWheeledVehicle> AVehicleStatusSensor::FindAttachmentParentVehicle() const
{
  for (AActor* Parent = GetAttachParentActor(); Parent; Parent = Parent->GetAttachParentActor())
  {
    if (ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Parent))
    {
      return Vehicle;
    }
  }
  return nullptr;
}

void AVehicleStatusSensor::CollectAndStream(float /*DeltaSeconds*/)
{
  if (!OwningVehicle.IsValid())
  {
    return;
  }

  ACarlaWheeledVehicle* Vehicle = OwningVehicle.Get();
  if (!IsValid(Vehicle))
  {
    return;
  }

  // Update cached velocity info
  SetVelocityInfoToLocal(Vehicle);

  // Get Max Steering
  const auto MaxSteerAngleInRadians = FMath::DegreesToRadians(Vehicle->GetMaximumSteerAngle());

  // Control flags (b0 = reverse, b1 = manual gear shift)
  const auto& Control = Vehicle->GetVehicleControl();
  const auto Flags = (Control.bReverse ? 0x01 : 0) | (Control.bManualGearShift ? 0x02 : 0);

  // Turn mask (b0 = left blinker, b1 = right blinker, b2 = hazards)
  const auto& Lights = Vehicle->GetVehicleLightState();
  const bool bHazard = Lights.LeftBlinker && Lights.RightBlinker;
  const auto TurnMask = (Lights.LeftBlinker ? 0x01 : 0) | (Lights.RightBlinker ? 0x02 : 0) | (bHazard ? 0x04 : 0);

  const FVehicleStatusData Msg
  {
    static_cast<double>(GetWorld()->GetTimeSeconds()),
    static_cast<float>(VelocityInfo.GetSpeed()),
    static_cast<float>(VelocityInfo.Velocity.X),
    static_cast<float>(VelocityInfo.Velocity.Y),
    static_cast<float>(VelocityInfo.Velocity.Z),
    static_cast<float>(VelocityInfo.AngularVelocity.X),
    static_cast<float>(VelocityInfo.AngularVelocity.Y),
    static_cast<float>(VelocityInfo.AngularVelocity.Z),
    static_cast<float>(Vehicle->GetActorRotation().Pitch),
    static_cast<float>(Vehicle->GetActorRotation().Yaw),
    static_cast<float>(Vehicle->GetActorRotation().Roll),
    static_cast<float>(Vehicle->GetVehicleControl().Steer * MaxSteerAngleInRadians),
    static_cast<int32_t>(Vehicle->GetVehicleCurrentGear()),
    static_cast<uint8_t>(TurnMask),
    static_cast<uint8_t>(Flags),
  };

  auto DataStream = GetDataStream(*this);

  // ROS2 forwarding
#if defined(WITH_ROS2)
  if (auto ROS2 = carla::ros2::ROS2::GetInstance(); ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    ROS2->ProcessDataFromStatusSensor(
        DataStream.GetSensorType(),
        StreamId,
        GetActorTransform(),
        Msg,
        Vehicle,
        this);
  }
#endif

  // UE client streaming (field-wise VehicleStatusSerializer overload)
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("AVehicleStatusSensor Stream Send");
    DataStream.SerializeAndSend(
        *this,
        Msg.timestamp,
        Msg.speed_mps,
        Msg.vel_x_mps, Msg.vel_y_mps, Msg.vel_z_mps,
        Msg.angVel_x_mps, Msg.angVel_y_mps, Msg.angVel_z_mps,
        Msg.rot_pitch, Msg.rot_yaw, Msg.rot_roll,
        Msg.steer,
        Msg.gear,
        Msg.turn_mask,
        Msg.control_flags);
  }
}

void AVehicleStatusSensor::SetVelocityInfoToLocal(const AActor* VehicleActor)
{
  if (!VehicleActor)
  {
    return;
  }

  // World linear velocity (cm/s)
  const FVector WorldVel_cmps = VehicleActor->GetVelocity();

  // Convert to m/s, then rotate into local space
  const FQuat InvRot = VehicleActor->GetActorTransform().GetRotation().Inverse();
  VelocityInfo.Velocity = InvRot.RotateVector(CmpsToMps(WorldVel_cmps));

  // Physics angular velocity (rad/s, world space)
  FVector WorldAngVel = FVector::ZeroVector;
  if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(VehicleActor->GetRootComponent()))
  {
    if (RootPrim->IsSimulatingPhysics())
    {
      WorldAngVel = RootPrim->GetPhysicsAngularVelocityInRadians();
    }
  }

  // Rotate into local space
  VelocityInfo.AngularVelocity = InvRot.RotateVector(WorldAngVel);

  // Convert angular velocity vector into a Rotator for convenience
  VelocityInfo.RotationRate = VelocityInfo.AngularVelocity.Rotation();
}
