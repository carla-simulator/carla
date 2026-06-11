// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/TFSensor.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"

#include <util/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <util/enable-ue4-macros.h>

ATFSensor::ATFSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PostPhysics;
}

FActorDefinition ATFSensor::GetSensorDefinition()
{
  FActorDefinition Definition =
  UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(TEXT("other"), TEXT("tf"));

  FActorVariation Tick;
  Tick.Id = TEXT("sensor_tick");
  Tick.Type = EActorAttributeType::Float;
  Tick.RecommendedValues = {TEXT("0.0")};
  Tick.bRestrictToRecommended = false;
  Definition.Variations.Emplace(Tick);

  FActorVariation IgnoreAltitude;
  IgnoreAltitude.Id = TEXT("ignore_altitude");
  IgnoreAltitude.Type = EActorAttributeType::Bool;
  IgnoreAltitude.RecommendedValues = {TEXT("False")};
  IgnoreAltitude.bRestrictToRecommended = false;
  Definition.Variations.Emplace(IgnoreAltitude);

  FActorVariation IgnoreTilt;
  IgnoreTilt.Id = TEXT("ignore_tilt");
  IgnoreTilt.Type = EActorAttributeType::Bool;
  IgnoreTilt.RecommendedValues = {TEXT("False")};
  IgnoreTilt.bRestrictToRecommended = false;
  Definition.Variations.Emplace(IgnoreTilt);

  return Definition;
}

void ATFSensor::Set(const FActorDescription &Description)
{
  Super::Set(Description);

  bIgnoreAltitude = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      TEXT("ignore_altitude"),
      Description.Variations,
      false);
  bIgnoreTilt = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      TEXT("ignore_tilt"),
      Description.Variations,
      false);
}

void ATFSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ATFSensor::PostPhysTick);

#if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ATFSensor ROS2 Send");

    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor *ParentActor = GetAttachParentActor();
    AActor *TransformActor = ParentActor != nullptr ? ParentActor : this;

    FTransform Transform = TransformActor->GetActorTransform();
    ALargeMapManager *LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
    if (LargeMap)
    {
      Transform = LargeMap->LocalToGlobalTransform(Transform);
    }

    if (bIgnoreAltitude)
    {
      FVector Location = Transform.GetLocation();
      Location.Z = 0.0f;
      Transform.SetLocation(Location);
    }

    if (bIgnoreTilt)
    {
      FRotator Rotation = Transform.Rotator();
      Rotation.Roll = 0.0f;
      Rotation.Pitch = 0.0f;
      Transform.SetRotation(Rotation.Quaternion());
    }

    ROS2->ProcessDataFromTF(
        StreamId,
        Transform,
        TransformActor);
  }
#endif
}
