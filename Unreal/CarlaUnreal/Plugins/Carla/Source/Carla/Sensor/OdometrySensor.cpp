// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/OdometrySensor.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Components/PrimitiveComponent.h"

#include <util/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <util/enable-ue4-macros.h>

AOdometrySensor::AOdometrySensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PostPhysics;
}

FActorDefinition AOdometrySensor::GetSensorDefinition()
{
  FActorDefinition Definition =
      UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(TEXT("other"), TEXT("odometry"));

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

void AOdometrySensor::Set(const FActorDescription &Description)
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

static FVector GetActorAngularVelocityInRadians(const AActor &Actor)
{
  const auto RootComponent = Cast<UPrimitiveComponent>(Actor.GetRootComponent());
  return RootComponent != nullptr ?
      RootComponent->GetPhysicsAngularVelocityInRadians() :
      FVector::ZeroVector;
}

void AOdometrySensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AOdometrySensor::PostPhysTick);
  auto DataStream = GetDataStream(*this);

#if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("AOdometrySensor ROS2 Send");

    constexpr float TO_METERS = 1e-2f;
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor* ParentActor = GetAttachParentActor();
    AActor* OdometryActor = ParentActor != nullptr ? ParentActor : this;

    FTransform OdometryTransform = OdometryActor->GetActorTransform();
    ALargeMapManager *LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
    if (LargeMap)
    {
      OdometryTransform = LargeMap->LocalToGlobalTransform(OdometryTransform);
    }

    if (bIgnoreAltitude)
    {
      FVector Location = OdometryTransform.GetLocation();
      Location.Z = 0.0f;
      OdometryTransform.SetLocation(Location);
    }

    if (bIgnoreTilt)
    {
      FRotator Rotation = OdometryTransform.Rotator();
      Rotation.Roll = 0.0f;
      Rotation.Pitch = 0.0f;
      OdometryTransform.SetRotation(Rotation.Quaternion());
    }

    const FVector LinearVelocity = TO_METERS * OdometryActor->GetVelocity();
    const FVector AngularVelocity = GetActorAngularVelocityInRadians(*OdometryActor);

    if (ParentActor)
    {
      FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
      ROS2->ProcessDataFromOdometry(
          DataStream.GetSensorType(),
          StreamId,
          LocalTransformRelativeToParent,
          OdometryTransform,
          carla::geom::Vector3D(LinearVelocity.X, LinearVelocity.Y, LinearVelocity.Z),
          carla::geom::Vector3D(AngularVelocity.X, AngularVelocity.Y, AngularVelocity.Z),
          this,
          true);
    }
    else
    {
      ROS2->ProcessDataFromOdometry(
          DataStream.GetSensorType(),
          StreamId,
          DataStream.GetSensorTransform(),
          OdometryTransform,
          carla::geom::Vector3D(LinearVelocity.X, LinearVelocity.Y, LinearVelocity.Z),
          carla::geom::Vector3D(AngularVelocity.X, AngularVelocity.Y, AngularVelocity.Z),
          this,
          false);
    }
  }
#endif
}
