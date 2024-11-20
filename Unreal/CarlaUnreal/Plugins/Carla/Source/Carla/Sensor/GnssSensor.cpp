// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/GnssSensor.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"

#include <util/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include "carla/ros2/ROS2.h"
#include <util/enable-ue4-macros.h>

AGnssSensor::AGnssSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

FActorDefinition AGnssSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeGnssDefinition();
}

void AGnssSensor::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UActorBlueprintFunctionLibrary::SetGnss(ActorDescription, this);
}

void AGnssSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AGnssSensor::PostPhysTick);

  FVector ActorLocation = GetActorLocation();
  ALargeMapManager * LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  if (LargeMap)
  {
    ActorLocation = LargeMap->LocalToGlobalLocation(ActorLocation);
  }
  carla::geom::Location Location = ActorLocation;
  carla::geom::GeoLocation CurrentLocation = CurrentGeoReference.Transform(Location);

  // Compute the noise for the sensor
  const float LatError = RandomEngine->GetNormalDistribution(0.0f, LatitudeDeviation);
  const float LonError = RandomEngine->GetNormalDistribution(0.0f, LongitudeDeviation);
  const float AltError = RandomEngine->GetNormalDistribution(0.0f, AltitudeDeviation);

  // Apply the noise to the sensor
  LatitudeValue = CurrentLocation.latitude + LatitudeBias + LatError;
  LongitudeValue = CurrentLocation.longitude + LongitudeBias + LonError;
  AltitudeValue = CurrentLocation.altitude + AltitudeBias + AltError;

  auto DataStream = GetDataStream(*this);

  // ROS2
  #if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor* ParentActor = GetAttachParentActor();
    if (ParentActor)
    {
      FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
      ROS2->ProcessDataFromGNSS(DataStream.GetSensorType(), StreamId, LocalTransformRelativeToParent, carla::geom::GeoLocation{LatitudeValue, LongitudeValue, AltitudeValue}, this);
    }
    else
    {
      ROS2->ProcessDataFromGNSS(DataStream.GetSensorType(), StreamId, DataStream.GetSensorTransform(), carla::geom::GeoLocation{LatitudeValue, LongitudeValue, AltitudeValue}, this);
    }
  }
  #endif
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("AGnssSensor Stream Send");
    DataStream.SerializeAndSend(*this, carla::geom::GeoLocation{LatitudeValue, LongitudeValue, AltitudeValue});
  }
}

void AGnssSensor::SetLatitudeDeviation(float Value)
{
  LatitudeDeviation = Value;
}

void AGnssSensor::SetLongitudeDeviation(float Value)
{
  LongitudeDeviation = Value;
}

void AGnssSensor::SetAltitudeDeviation(float Value)
{
  AltitudeDeviation = Value;
}

void AGnssSensor::SetLatitudeBias(float Value)
{
  LatitudeBias = Value;
}

void AGnssSensor::SetLongitudeBias(float Value)
{
  LongitudeBias = Value;
}

void AGnssSensor::SetAltitudeBias(float Value)
{
  AltitudeBias = Value;
}

float AGnssSensor::GetLatitudeDeviation() const
{
  return LatitudeDeviation;
}
float AGnssSensor::GetLongitudeDeviation() const
{
  return LongitudeDeviation;
}
float AGnssSensor::GetAltitudeDeviation() const
{
  return AltitudeDeviation;
}

float AGnssSensor::GetLatitudeBias() const
{
  return LatitudeBias;
}
float AGnssSensor::GetLongitudeBias() const
{
  return LongitudeBias;
}
float AGnssSensor::GetAltitudeBias() const
{
  return AltitudeBias;
}

double AGnssSensor::GetLatitudeValue() const
{
  return LatitudeValue;
}

double AGnssSensor::GetLongitudeValue() const
{
  return LongitudeValue;
}

double AGnssSensor::GetAltitudeValue() const
{
  return AltitudeValue;
}

void AGnssSensor::BeginPlay()
{
  Super::BeginPlay();

  const UCarlaEpisode* episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  CurrentGeoReference = episode->GetGeoReference();
}
